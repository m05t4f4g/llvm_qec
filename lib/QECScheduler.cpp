// lib/QECScheduler.cpp
#include "QEC/QECPasses.h"
#include "QEC/QECOps.h"

#include "mlir/IR/BuiltinOps.h"
#include "mlir/Pass/Pass.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/MapVector.h"
#include "llvm/ADT/SmallVector.h"

using namespace mlir;
using namespace mlir::qec;

namespace {

// A "wire root" is the SSA value that identifies the physical wire this value
// traces back to. Walk backwards through any aliasing gates (1-qubit gates,
// barriers) until we hit an alloc or a function argument.
    static Value wireRoot(Value v) {
        while (true) {
            Operation *def = v.getDefiningOp();
            if (!def)
                return v;                       // function argument
            if (isa<Alloc_Op>(def))
                return v;
            if (isa<H_Op>(def) || isa<X_Op>(def) || isa<Y_Op>(def) ||
                isa<Z_Op>(def) || isa<S_Op>(def) || isa<Sdg_Op>(def) ||
                isa<T_Op>(def) || isa<Tdg_Op>(def) || isa<U3_Op>(def)) {
                v = def->getOperand(0);
                continue;
            }
            if (isa<Barrier_Op>(def)) {
                v = (v == def->getResult(0)) ? def->getOperand(0)
                                             : def->getOperand(1);
                continue;
            }
            return v;                           // CNOT, measure, or opaque op
        }
    }

    static bool is1QGate(Operation *op) {
        return isa<H_Op>(op) || isa<X_Op>(op) || isa<Y_Op>(op) ||
               isa<Z_Op>(op) || isa<S_Op>(op) || isa<Sdg_Op>(op) ||
               isa<T_Op>(op) || isa<Tdg_Op>(op) || isa<U3_Op>(op);
    }

    struct QECSchedulerPass : PassWrapper<QECSchedulerPass, OperationPass<ModuleOp>> {
    MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(QECSchedulerPass)

    StringRef getArgument() const final { return "qec-scheduler"; }
StringRef getDescription() const final {
return "Reorder 1q gates in each consecutive run so same-wire gates are adjacent";
}

void runOnOperation() override {
ModuleOp module = getOperation();
module.walk([&](Operation *op) {
for (Region &region : op->getRegions())
for (Block &block : region)
scheduleBlock(block);
});
}

void scheduleBlock(Block &block) {
    // A "segment" is a maximal run of consecutive 1q gates. Terminate it
    // at any other op (or end of block).
    SmallVector<Operation *> segment;

    auto flush = [&]() {
        if (segment.size() < 2) {
            segment.clear();
            return;
        }

        // Identify the terminator: the op AFTER the segment (or end).
        Operation *sentinel = segment.back()->getNextNode();

        // Stable-sort by wire root — MapVector preserves first-seen order
        // for equal keys, so it's a *stable* partition by wire.
        llvm::MapVector<Value, SmallVector<Operation *>> byWire;
        for (Operation *g : segment)
            byWire[wireRoot(g->getOperand(0))].push_back(g);

        // Re-insert in sorted order at the sentinel position.
        for (auto &entry : byWire)
            for (Operation *g : entry.second)
                (sentinel ? g->moveBefore(sentinel) : g->moveBefore(&block,
                                                                    block.end()));

        segment.clear();
    };

    for (Operation &op : llvm::make_early_inc_range(block)) {
        if (is1QGate(&op)) {
            segment.push_back(&op);
        } else {
            flush();
        }
    }
    flush();
}
};

} // namespace

void mlir::qec::registerQECSchedulerPass() {
    PassRegistration<QECSchedulerPass>();
}