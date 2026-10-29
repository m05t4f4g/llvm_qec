// lib/QECCNOTCancel.cpp
#include "QEC/QECPasses.h"
#include "QEC/QECOps.h"

#include "mlir/IR/BuiltinOps.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

using namespace mlir;
using namespace mlir::qec;

namespace {

//   %c1, %t1 = qec.cnot %c, %t : ...
//   %c2, %t2 = qec.cnot %c1, %t1 : ...     ← must match (c1, t1), not (t1, c1)
//   ... users of %c2, %t2 ...
//
// => %c2 replaced with %c, %t2 replaced with %t; both qec.cnot ops disappear.
    struct CancelCNOT : OpRewritePattern<CNOT_Op> {
        using OpRewritePattern::OpRewritePattern;

        LogicalResult matchAndRewrite(CNOT_Op outer,
                                      PatternRewriter &rewriter) const override {
            Value outerCtrl = outer.getControl();
            Value outerTgt  = outer.getTarget();

            // Both operands must come from the same defining op.
            Operation *ctrlDef = outerCtrl.getDefiningOp();
            Operation *tgtDef  = outerTgt.getDefiningOp();
            if (!ctrlDef || ctrlDef != tgtDef)
                return failure();

            auto inner = dyn_cast<CNOT_Op>(ctrlDef);
            if (!inner)
                return failure();

            // The outer op must consume exactly inner's (control_out, target_out),
            // in that order. If it were (target_out, control_out), that would be
            // CNOT(t,c) — a different gate — and must NOT be cancelled.
            if (inner.getControlOut() != outerCtrl ||
                inner.getTargetOut()  != outerTgt)
                return failure();

            // Safe to erase inner only if those two results have no other users.
            if (!inner.getControlOut().hasOneUse() ||
                !inner.getTargetOut().hasOneUse())
                return failure();

            // outer's results -> inner's operands.
            rewriter.replaceOp(outer, {inner.getControl(), inner.getTarget()});
            rewriter.eraseOp(inner);
            return success();
        }
    };

    struct QECCancelCNOTPass
            : PassWrapper<QECCancelCNOTPass, OperationPass<ModuleOp>> {
    MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(QECCancelCNOTPass)

    StringRef getArgument() const final { return "qec-cancel-cnot"; }
StringRef getDescription() const final {
return "Cancel adjacent qec.cnot ops on the same control/target pair";
}

void runOnOperation() override {
RewritePatternSet patterns(&getContext());
patterns.add<CancelCNOT>(&getContext());

if (failed(applyPatternsAndFoldGreedily(getOperation(),
                                        std::move(patterns))))
signalPassFailure();
}
};

} // namespace

void mlir::qec::registerQECCancelCNOTPass() {
    PassRegistration<QECCancelCNOTPass>();
}