// lib/QECDeadGateElim.cpp
#include "QEC/QECPasses.h"
#include "QEC/QECOps.h"

#include "mlir/IR/BuiltinOps.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

using namespace mlir;
using namespace mlir::qec;

namespace {

// Cancel `1q_gate · 1q_gate` when both are the same self-inverse gate.
    template <typename OpTy>
    struct CancelSelfPair : OpRewritePattern<OpTy> {
        using OpRewritePattern<OpTy>::OpRewritePattern;

        LogicalResult matchAndRewrite(OpTy outer,
                                      PatternRewriter &rewriter) const override {
            auto inner = outer.getInput().template getDefiningOp<OpTy>();
            if (!inner)
                return failure();
            if (!inner.getResult().hasOneUse())
                return failure();

            rewriter.replaceOp(outer, inner.getInput());
            rewriter.eraseOp(inner);
            return success();
        }
    };

// Cancel `CNOT · CNOT` on the same (control, target) pair.
    struct CancelCNOTPair : OpRewritePattern<CNOT_Op> {
        using OpRewritePattern::OpRewritePattern;

        LogicalResult matchAndRewrite(CNOT_Op outer,
                                      PatternRewriter &rewriter) const override {
            Value c = outer.getControl();
            Value t = outer.getTarget();
            Operation *def = c.getDefiningOp();
            if (!def || def != t.getDefiningOp())
                return failure();
            auto inner = dyn_cast<CNOT_Op>(def);
            if (!inner)
                return failure();
            if (inner.getControlOut() != c || inner.getTargetOut() != t)
                return failure();
            if (!inner.getControlOut().hasOneUse() ||
                !inner.getTargetOut().hasOneUse())
                return failure();

            rewriter.replaceOp(outer, {inner.getControl(), inner.getTarget()});
            rewriter.eraseOp(inner);
            return success();
        }
    };

    struct QECDeadGateElimPass
            : PassWrapper<QECDeadGateElimPass, OperationPass<ModuleOp>> {
    MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(QECDeadGateElimPass)

    StringRef getArgument() const final { return "qec-dead-gate-elim"; }
StringRef getDescription() const final {
return "Cancel self-inverse 1q gate pairs (H/X/Y/Z) and CNOT pairs";
}

void runOnOperation() override {
RewritePatternSet patterns(&getContext());
patterns.add<CancelSelfPair<H_Op>,
        CancelSelfPair<X_Op>,
        CancelSelfPair<Y_Op>,
        CancelSelfPair<Z_Op>,
        CancelCNOTPair>(&getContext());

if (failed(applyPatternsAndFoldGreedily(getOperation(),
                                        std::move(patterns))))
signalPassFailure();
}
};

} // namespace

void mlir::qec::registerQECDeadGateElimPass() {
    PassRegistration<QECDeadGateElimPass>();
}