// lib/QECSchedule.cpp
#include "QEC/QECPasses.h"
#include "QEC/QECOps.h"

#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/Pass/Pass.h"

#include "llvm/ADT/SmallVector.h"

using namespace mlir;
using namespace mlir::qec;

namespace {

    struct QECSchedulePass : PassWrapper<QECSchedulePass, OperationPass<ModuleOp>> {
    MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(QECSchedulePass)

    StringRef getArgument() const final { return "qec-schedule"; }
StringRef getDescription() const final {
return "Insert qec.barrier after every multi-qubit gate";
}

void runOnOperation() override {
ModuleOp module = getOperation();

// Collect first — mutating the IR while walking a module is unsafe.
SmallVector<CNOT_Op> cnots;
module.walk([&](CNOT_Op c) { cnots.push_back(c); });

OpBuilder builder(module.getContext());

for (CNOT_Op cnot : cnots) {
Value c = cnot.getControlOut();
Value t = cnot.getTargetOut();

builder.setInsertionPointAfter(cnot);
auto barrier = builder.create<Barrier_Op>(
        cnot.getLoc(), c.getType(), t.getType(), c, t);

// Everything downstream of the CNOT must now consume the
// barrier outputs, so that the CNOT's own uses drop to zero
// (except the barrier itself) and linearity is preserved.
c.replaceAllUsesExcept(barrier.getOut0(), barrier);
t.replaceAllUsesExcept(barrier.getOut1(), barrier);
}
}
};

} // namespace

void mlir::qec::registerQECSchedulePass() {
    PassRegistration<QECSchedulePass>();
}