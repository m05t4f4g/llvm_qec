// lib/QECCNOTConjugation.cpp
#include "QEC/QECPasses.h"
#include "QEC/QECOps.h"
#include "QEC/QECTypes.h"

#include "mlir/IR/BuiltinOps.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

using namespace mlir;
using namespace mlir::qec;

namespace {

    enum class PauliType { None, X, Y, Z };

    static PauliType getPauliType(Operation *op) {
        if (!op) return PauliType::None;
        if (isa<X_Op>(op)) return PauliType::X;
        if (isa<Y_Op>(op)) return PauliType::Y;
        if (isa<Z_Op>(op)) return PauliType::Z;
        return PauliType::None;
    }

    static Value buildPauli(PatternRewriter &rewriter, Location loc,
                            Value in, PauliType p, Type ty) {
        switch (p) {
            case PauliType::X: return rewriter.create<X_Op>(loc, ty, in);
            case PauliType::Y: return rewriter.create<Y_Op>(loc, ty, in);
            case PauliType::Z: return rewriter.create<Z_Op>(loc, ty, in);
            default:           return in;
        }
    }

//   CNOT · (P⊗I) · CNOT     or     CNOT · (I⊗P) · CNOT
//        ↳ rewrite to the Pauli(s) on the *outer* wires.
    struct ConjugatePauliThroughCNOT : public OpRewritePattern<CNOT_Op> {
        using OpRewritePattern::OpRewritePattern;

        LogicalResult matchAndRewrite(CNOT_Op outer,
                                      PatternRewriter &rewriter) const override {
            if (succeeded(tryControl(outer, rewriter)))
                return success();
            return tryTarget(outer, rewriter);
        }

    private:
        //   %c1, %t1 = CNOT %c, %t
        //   %m       = P %c1              (P on CONTROL of the first CNOT)
        //   %c2, %t2 = CNOT %m, %t1
        LogicalResult tryControl(CNOT_Op outer, PatternRewriter &rewriter) const {
            Value midC = outer.getControl();
            Value t1   = outer.getTarget();

            Operation *midOp = midC.getDefiningOp();
            PauliType pC     = getPauliType(midOp);
            if (pC == PauliType::None)
                return failure();

            Value c1     = midOp->getOperand(0);
            auto  inner  = dyn_cast_or_null<CNOT_Op>(c1.getDefiningOp());
            if (!inner)
                return failure();
            if (inner.getTargetOut() != t1)
                return failure();

            // Every intermediate must have exactly one user, otherwise we'd
            // strand an SSA value when erasing.
            if (!c1.hasOneUse() || !t1.hasOneUse() || !midC.hasOneUse())
                return failure();

            Location loc = outer.getLoc();
            Value origC  = inner.getControl();
            Value origT  = inner.getTarget();
            Type  ty     = outer.getControlOut().getType();

            Value newC, newT;
            switch (pC) {
                case PauliType::X:  // X⊗X
                    newC = buildPauli(rewriter, loc, origC, PauliType::X, ty);
                    newT = buildPauli(rewriter, loc, origT, PauliType::X, ty);
                    break;
                case PauliType::Y:  // Y⊗X
                    newC = buildPauli(rewriter, loc, origC, PauliType::Y, ty);
                    newT = buildPauli(rewriter, loc, origT, PauliType::X, ty);
                    break;
                case PauliType::Z:  // Z⊗I
                    newC = buildPauli(rewriter, loc, origC, PauliType::Z, ty);
                    newT = origT;
                    break;
                default: return failure();
            }

            rewriter.replaceOp(outer, {newC, newT});
            rewriter.eraseOp(midOp);
            rewriter.eraseOp(inner);
            return success();
        }

        //   %c1, %t1 = CNOT %c, %t
        //   %m       = P %t1              (P on TARGET of the first CNOT)
        //   %c2, %t2 = CNOT %c1, %m
        LogicalResult tryTarget(CNOT_Op outer, PatternRewriter &rewriter) const {
            Value c1   = outer.getControl();
            Value midT = outer.getTarget();

            Operation *midOp = midT.getDefiningOp();
            PauliType pT     = getPauliType(midOp);
            if (pT == PauliType::None)
                return failure();

            Value t1    = midOp->getOperand(0);
            auto  inner = dyn_cast_or_null<CNOT_Op>(t1.getDefiningOp());
            if (!inner)
                return failure();
            if (inner.getControlOut() != c1)
                return failure();

            if (!c1.hasOneUse() || !t1.hasOneUse() || !midT.hasOneUse())
                return failure();

            Location loc = outer.getLoc();
            Value origC  = inner.getControl();
            Value origT  = inner.getTarget();
            Type  ty     = outer.getTargetOut().getType();

            Value newC, newT;
            switch (pT) {
                case PauliType::X:  // I⊗X
                    newC = origC;
                    newT = buildPauli(rewriter, loc, origT, PauliType::X, ty);
                    break;
                case PauliType::Y:  // Z⊗Y
                    newC = buildPauli(rewriter, loc, origC, PauliType::Z, ty);
                    newT = buildPauli(rewriter, loc, origT, PauliType::Y, ty);
                    break;
                case PauliType::Z:  // Z⊗Z
                    newC = buildPauli(rewriter, loc, origC, PauliType::Z, ty);
                    newT = buildPauli(rewriter, loc, origT, PauliType::Z, ty);
                    break;
                default: return failure();
            }

            rewriter.replaceOp(outer, {newC, newT});
            rewriter.eraseOp(midOp);
            rewriter.eraseOp(inner);
            return success();
        }
    };

    struct QECCNOTConjugationPass
            : PassWrapper<QECCNOTConjugationPass, OperationPass<ModuleOp>> {
    MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(QECCNOTConjugationPass)

    StringRef getArgument() const final { return "qec-cnot-conjugation"; }
StringRef getDescription() const final {
return "Propagate Paulis through adjacent CNOT pairs";
}

void runOnOperation() override {
RewritePatternSet patterns(&getContext());
patterns.add<ConjugatePauliThroughCNOT>(&getContext());
if (failed(applyPatternsAndFoldGreedily(getOperation(),
                                        std::move(patterns))))
signalPassFailure();
}
};

} // namespace

void mlir::qec::registerQECCNOTConjugationPass() {
    PassRegistration<QECCNOTConjugationPass>();
}