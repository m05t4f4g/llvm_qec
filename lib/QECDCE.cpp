// lib/QECDCE.cpp
#include "QEC/QECPasses.h"
#include "QEC/QECOps.h"

#include "mlir/IR/BuiltinOps.h"
#include "mlir/Pass/Pass.h"

#include "llvm/ADT/SmallVector.h"

using namespace mlir;
using namespace mlir::qec;

namespace {

// Ops that are safe to remove when their results are unused.
// Excludes Measure_Op (has a side effect: collapses the qubit) and
// func::FuncOp / func::ReturnOp (structural, not quantum ops).
    static bool isPureQuantumOp(Operation *op) {
        return isa<Alloc_Op, H_Op, X_Op, Y_Op, Z_Op, S_Op, Sdg_Op,
                T_Op, Tdg_Op, U3_Op, CNOT_Op, Swap_Op, Barrier_Op>(op);
    }

    static bool allResultsUnused(Operation *op) {
        for (Value r : op->getResults())
            if (!r.use_empty())
                return false;
        return true;
    }

    struct QECDCEPass : PassWrapper<QECDCEPass, OperationPass<ModuleOp>> {
    MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(QECDCEPass)

    StringRef getArgument() const final { return "qec-dce"; }
StringRef getDescription() const final {
return "Remove dead quantum ops (results all unused); preserves measure";
}

void runOnOperation() override {
ModuleOp module = getOperation();

// Iterate to fixpoint: removing a dead op can make its operands dead.
bool changed = true;
while (changed) {
changed = false;

SmallVector<Operation *> toErase;
module.walk([&](Operation *op) {
if (isPureQuantumOp(op) && allResultsUnused(op))
toErase.push_back(op);
});

for (Operation *op : toErase) {
op->erase();
changed = true;
}
}
}
};

} // namespace

void mlir::qec::registerQECDCEPass() {
    PassRegistration<QECDCEPass>();
}