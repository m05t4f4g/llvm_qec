// lib/QECDecomposeU3.cpp
#include "QEC/QECPasses.h"
#include "QEC/QECOps.h"

#include "mlir/IR/BuiltinOps.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

#include <cmath>

using namespace mlir;
using namespace mlir::qec;

namespace {

    constexpr double kPi  = 3.14159265358979323846;
    constexpr double kTol = 1e-6;

    static bool near(double a, double b) { return std::abs(a - b) < kTol; }

// Canonical U3(θ, φ, λ) values for standard gates (up to global phase).
// When we match, rewrite U3 to the named gate.
    struct CanonicalizeU3 : OpRewritePattern<U3_Op> {
        using OpRewritePattern::OpRewritePattern;

        LogicalResult matchAndRewrite(U3_Op u3,
                                      PatternRewriter &rewriter) const override {
            double th = u3.getTheta().convertToDouble();
            double ph = u3.getPhi().convertToDouble();
            double la = u3.getLambda().convertToDouble();
            Location loc = u3.getLoc();
            Type ty = u3.getOutput().getType();
            Value in = u3.getInput();

            auto replace = [&](Operation *g) {
                rewriter.replaceOp(u3, g->getResult(0));
                return success();
            };

            // Identity → remove.
            if (near(th, 0) && near(ph, 0) && near(la, 0)) {
                rewriter.replaceOp(u3, in);
                return success();
            }
            // Z
            if (near(th, 0) && near(ph, 0) && near(la, kPi))
                return replace(rewriter.create<Z_Op>(loc, ty, in));
            // S
            if (near(th, 0) && near(ph, 0) && near(la, kPi / 2))
                return replace(rewriter.create<S_Op>(loc, ty, in));
            // Sdg
            if (near(th, 0) && near(ph, 0) && near(la, -kPi / 2))
                return replace(rewriter.create<Sdg_Op>(loc, ty, in));
            // T
            if (near(th, 0) && near(ph, 0) && near(la, kPi / 4))
                return replace(rewriter.create<T_Op>(loc, ty, in));
            // Tdg
            if (near(th, 0) && near(ph, 0) && near(la, -kPi / 4))
                return replace(rewriter.create<Tdg_Op>(loc, ty, in));
            // H
            if (near(th, kPi / 2) && near(ph, 0) && near(la, kPi))
                return replace(rewriter.create<H_Op>(loc, ty, in));
            // X
            if (near(th, kPi) && near(ph, 0) && near(la, kPi))
                return replace(rewriter.create<X_Op>(loc, ty, in));
            // Y
            if (near(th, kPi) && near(ph, kPi / 2) && near(la, kPi / 2))
                return replace(rewriter.create<Y_Op>(loc, ty, in));

            return failure();   // non-canonical U3: leave as-is
        }
    };

    struct QECDecomposeU3Pass
            : PassWrapper<QECDecomposeU3Pass, OperationPass<ModuleOp>> {
    MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(QECDecomposeU3Pass)

    StringRef getArgument() const final { return "qec-decompose-u3"; }
StringRef getDescription() const final {
return "Rewrite canonical U3(θ,φ,λ) values to named gates (H, X, Y, Z, S, T, ...)";
}

void runOnOperation() override {
RewritePatternSet patterns(&getContext());
patterns.add<CanonicalizeU3>(&getContext());
if (failed(applyPatternsAndFoldGreedily(getOperation(),
                                        std::move(patterns))))
signalPassFailure();
}
};

} // namespace

void mlir::qec::registerQECDecomposeU3Pass() {
    PassRegistration<QECDecomposeU3Pass>();
}