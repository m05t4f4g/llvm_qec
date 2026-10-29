// lib/QECHHCancel.cpp
#include "QEC/QECPasses.h"
#include "QEC/QECOps.h"

#include "mlir/IR/BuiltinOps.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

using namespace mlir;
using namespace mlir::qec;

namespace {

//   %a = qec.h %q : !qec.qubit -> !qec.qubit
//   %b = qec.h %a : !qec.qubit -> !qec.qubit
//   ... users of %b ...
//
// => %b is replaced by %q, and both qec.h ops disappear.
    struct CancelHH : OpRewritePattern<H_Op> {
        using OpRewritePattern::OpRewritePattern;

        LogicalResult matchAndRewrite(H_Op outer,
                                      PatternRewriter &rewriter) const override {
            auto inner = outer.getInput().getDefiningOp<H_Op>();
            if (!inner)
                return failure();

            // The inner H's result must be consumed only by the outer H,
            // otherwise folding would change the semantics for other users.
            if (!inner.getResult().hasOneUse())
                return failure();

            Value src = inner.getInput();
            rewriter.replaceOp(outer, src);
            rewriter.eraseOp(inner);
            return success();
        }
    };

    struct QECCancelHHPass
            : PassWrapper<QECCancelHHPass, OperationPass<ModuleOp>> {
    MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(QECCancelHHPass)

    StringRef getArgument() const final { return "qec-cancel-hh"; }
StringRef getDescription() const final {
return "Cancel adjacent qec.h ops (H·H = I)";
}

void runOnOperation() override {
RewritePatternSet patterns(&getContext());
patterns.add<CancelHH>(&getContext());

if (failed(applyPatternsAndFoldGreedily(getOperation(),
                                        std::move(patterns))))
signalPassFailure();
}
};

} // namespace

void mlir::qec::registerQECCancelHHPass() {
    PassRegistration<QECCancelHHPass>();
}