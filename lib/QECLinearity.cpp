#include "llvm/ADT/STLExtras.h"
#include "QEC/QECPasses.h"
#include "QEC/QECTypes.h"

#include "mlir/IR/BuiltinOps.h"
#include "mlir/Pass/Pass.h"

using namespace mlir;
using namespace mlir::qec;

namespace {

struct QECLinearityPass
    : public PassWrapper<QECLinearityPass, OperationPass<ModuleOp>> {
    MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(QECLinearityPass)

    StringRef getArgument() const final { return "qec-linearity"; }
    StringRef getDescription() const final {
        return "Verify that every !qec.qubit value is used exactly once";
    }

    void runOnOperation() override {
        ModuleOp module = getOperation();
        bool hadError = false;

        module.walk([&](Operation *op) {
            // (a) Every !qec.qubit result must have exactly one use.
            for (Value result : op->getResults()) {
                if (!isa<QubitType>(result.getType()))
                    continue;
                if (!result.hasOneUse()) {
                    if (result.use_empty()) {
                        op->emitError("dead qubit: result is never used");
                    } else {
                    op->emitError("fanout of qubit: result has ")
                    << llvm::range_size(result.getUses())
                    << " uses (quantum no-cloning violated)";
                    }
                    hadError = true;
                }
            }

            // (b) Every !qec.qubit block argument must also be used exactly once.
            //     This covers func.func parameters (qubit arguments are linear).
            for (Region &region : op->getRegions()) {
                for (Block &block : region) {
                    for (BlockArgument arg : block.getArguments()) {
                        if (!isa<QubitType>(arg.getType()))
                            continue;
                        if (!arg.hasOneUse()) {
                            if (arg.use_empty()) {
                                op->emitError("dead qubit: block argument #")
                                    << arg.getArgNumber() << " is never used";
                            } else {
                            op->emitError("fanout of qubit: block argument #")
                            << arg.getArgNumber() << " has "
                            << llvm::range_size(arg.getUses()) << " uses";
                            }
                            hadError = true;
                        }
                    }
                }
            }
        });

        if (hadError)
            signalPassFailure();
    }
};

} // namespace

void mlir::qec::registerQECLinearityPass() {
    PassRegistration<QECLinearityPass>();
}
