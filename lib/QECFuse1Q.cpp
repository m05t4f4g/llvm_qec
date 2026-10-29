// lib/QECFuse1Q.cpp
#include "QEC/QECPasses.h"
#include "QEC/QECOps.h"
#include "QEC/QECTypes.h"

#include "mlir/IR/BuiltinOps.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

#include <cmath>
#include <complex>
#include <optional>

using namespace mlir;
using namespace mlir::qec;
using C = std::complex<double>;

static constexpr double kPi  = 3.14159265358979323846;
static constexpr double kTol = 1e-9;

namespace {

// ---------------------------------------------------------------------------
// Minimal 2×2 complex matrix, just enough for 1-qubit gate composition.
// ---------------------------------------------------------------------------
struct Mat2 {
    C a, b, c, d;   // row-major

    static Mat2 H()    { C s = 1.0 / std::sqrt(2.0); return {s, s, s, -s}; }
    static Mat2 X()    { return {{0,0}, {1,0}, {1,0}, {0,0}}; }
    static Mat2 Y()    { return {{0,0}, {0,-1}, {0,1}, {0,0}}; }
    static Mat2 Z()    { return {{1,0}, {0,0}, {0,0}, {-1,0}}; }
    static Mat2 S()    { return {{1,0}, {0,0}, {0,0}, {0,1}}; }
    static Mat2 Sdg()  { return {{1,0}, {0,0}, {0,0}, {0,-1}}; }
    static Mat2 T()    { return {{1,0}, {0,0}, {0,0}, std::polar(1.0,  kPi/4)}; }
    static Mat2 Tdg()  { return {{1,0}, {0,0}, {0,0}, std::polar(1.0, -kPi/4)}; }

    static Mat2 U3(double theta, double phi, double lambda) {
        double cs = std::cos(theta / 2), sn = std::sin(theta / 2);
        return {C(cs, 0),
                -std::polar(1.0, lambda) * sn,
                std::polar(1.0, phi) * sn,
                std::polar(1.0, phi + lambda) * cs};
    }

    Mat2 operator*(const Mat2 &o) const {
        return {a*o.a + b*o.c,  a*o.b + b*o.d,
                c*o.a + d*o.c,  c*o.b + d*o.d};
    }

    C det() const { return a * d - b * c; }

    // Is M equal to e^{iα}·I for some α?
    bool isIdentityUpToPhase() const {
        return std::abs(b) < kTol && std::abs(c) < kTol
            && std::abs(a - d) < kTol
            && std::abs(std::abs(a) - 1.0) < kTol;
    }

    // Decompose into U3(θ, φ, λ) up to a global phase.
    void toU3(double &theta, double &phi, double &lambda) const {
        double r00 = std::abs(a);
        if (r00 > 1.0) r00 = 1.0;                       // numerical safety
        theta = 2.0 * std::acos(r00);

        double psi   = (r00 > kTol) ? std::arg(a) : 0.0;
        double delta = std::arg(det());

        if (std::abs(std::sin(theta / 2)) < kTol) {
            // θ ≈ 0: only φ+λ matters; pick φ = 0.
            phi    = 0.0;
            lambda = delta - 2.0 * psi;
        } else {
            phi    = std::arg(c) - psi;
            lambda = delta - 2.0 * psi - phi;
        }
    }
};

// ---------------------------------------------------------------------------
// Dispatch: recognise any single-qubit op, return its unitary.
// ---------------------------------------------------------------------------
static std::optional<Mat2> getSingleQubitMatrix(Operation *op) {
    if (isa<H_Op>(op))   return Mat2::H();
    if (isa<S_Op>(op))   return Mat2::S();
    if (isa<Sdg_Op>(op)) return Mat2::Sdg();
    if (isa<T_Op>(op))   return Mat2::T();
    if (isa<Tdg_Op>(op)) return Mat2::Tdg();
    if (isa<X_Op>(op))   return Mat2::X();
    if (isa<Y_Op>(op))   return Mat2::Y();
    if (isa<Z_Op>(op))   return Mat2::Z();
    if (auto u = dyn_cast<U3_Op>(op))
        return Mat2::U3(u.getTheta().convertToDouble(),
                        u.getPhi().convertToDouble(),
                        u.getLambda().convertToDouble());
    return std::nullopt;
}

// ---------------------------------------------------------------------------
// Pattern: any 1q op whose input is produced by another 1q op.
//   inner: %mid = g_in(%in)
//   outer: %out = g_out(%mid)            (and %mid has no other users)
//   ⇒ %out = u3(g_out · g_in) %in       or  %out = %in (if product = identity)
// ---------------------------------------------------------------------------
struct FuseAdjacent1Q : public RewritePattern {
    FuseAdjacent1Q(MLIRContext *ctx)
        : RewritePattern(MatchAnyOpTypeTag(), /*benefit=*/1, ctx) {}

    LogicalResult matchAndRewrite(Operation *outer,
                                  PatternRewriter &rewriter) const override {
        auto outerMat = getSingleQubitMatrix(outer);
        if (!outerMat)
            return failure();

        Value mid = outer->getOperand(0);   // 1q ops have exactly one operand
        Operation *inner = mid.getDefiningOp();
        if (!inner)
            return failure();

        auto innerMat = getSingleQubitMatrix(inner);
        if (!innerMat)
            return failure();

        // Only fuse when inner's result is exclusively used by outer —
        // otherwise erasing inner would leave dangling users.
        if (!mid.hasOneUse())
            return failure();

        Mat2 combined = (*outerMat) * (*innerMat);
        Location loc  = outer->getLoc();
        Value originalIn = inner->getOperand(0);

        if (combined.isIdentityUpToPhase()) {
            rewriter.replaceOp(outer, originalIn);
        } else {
            double theta, phi, lambda;
            combined.toU3(theta, phi, lambda);
            auto f64 = [&](double v) { return rewriter.getF64FloatAttr(v); };
            auto u3 = rewriter.create<U3_Op>(
                loc, outer->getResult(0).getType(), originalIn,
                f64(theta), f64(phi), f64(lambda));
            rewriter.replaceOp(outer, u3.getResult());
        }

        rewriter.eraseOp(inner);
        return success();
    }
};

struct QECFuse1QPass : PassWrapper<QECFuse1QPass, OperationPass<ModuleOp>> {
    MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(QECFuse1QPass)

    StringRef getArgument() const final { return "qec-fuse-1q"; }
    StringRef getDescription() const final {
        return "Fuse chains of single-qubit gates into one qec.u3";
    }

    void runOnOperation() override {
        RewritePatternSet patterns(&getContext());
        patterns.add<FuseAdjacent1Q>(&getContext());
        if (failed(applyPatternsAndFoldGreedily(getOperation(),
                                                std::move(patterns))))
            signalPassFailure();
    }
};

} // namespace

void mlir::qec::registerQECFuse1QPass() {
    PassRegistration<QECFuse1QPass>();
}
