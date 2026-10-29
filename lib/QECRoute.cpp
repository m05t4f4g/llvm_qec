// lib/QECRoute.cpp
#include "QEC/QECPasses.h"
#include "QEC/QECOps.h"
#include "QEC/QECTypes.h"

#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallVector.h"

#include <cstdlib>

using namespace mlir;
using namespace mlir::qec;

namespace {

    struct QECRoutePass : PassWrapper<QECRoutePass, OperationPass<ModuleOp>> {
    MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(QECRoutePass)

    StringRef getArgument() const final { return "qec-route"; }
StringRef getDescription() const final {
return "Insert SWAP chains so CNOTs act on adjacent physical qubits (linear topology)";
}

void runOnOperation() override {
ModuleOp module = getOperation();
module.walk([&](func::FuncOp func) { routeFunc(func); });
}

// -- per-function routing ----------------------------------------------
void routeFunc(func::FuncOp func) {
    // 1. Count total physical slots (qubit args + allocs anywhere).
    unsigned numSlots = 0;
    for (BlockArgument arg : func.getArguments())
        if (isa<QubitType>(arg.getType()))
            ++numSlots;
    func.walk([&](Alloc_Op) { ++numSlots; });

    if (numSlots < 2)
        return;   // nothing to route

    DenseMap<Value, unsigned> posOf;
    DenseMap<unsigned, Value> qubitAtPos;

    unsigned nextPos = 0;
    for (BlockArgument arg : func.getArguments()) {
        if (!isa<QubitType>(arg.getType()))
            continue;
        posOf[arg] = nextPos;
        qubitAtPos[nextPos] = arg;
        ++nextPos;
    }

    OpBuilder builder(func.getContext());

    for (Block &block : func.getBody()) {
        for (Operation &op : llvm::make_early_inc_range(block)) {
            // -- alloc: fresh physical slot.
            if (auto alloc = dyn_cast<Alloc_Op>(op)) {
                unsigned p = nextPos++;
                posOf[alloc.getOutput()] = p;
                qubitAtPos[p] = alloc.getOutput();
                continue;
            }

            // -- 1q gate: forward position to output.
            if (is1QGate(&op)) {
                unsigned p = posOf.lookup(op.getOperand(0));
                posOf[op.getResult(0)] = p;
                qubitAtPos[p] = op.getResult(0);
                continue;
            }

            // -- barrier: forward both positions.
            if (auto bar = dyn_cast<Barrier_Op>(op)) {
                unsigned p0 = posOf.lookup(bar.getIn0());
                unsigned p1 = posOf.lookup(bar.getIn1());
                posOf[bar.getOut0()] = p0;
                posOf[bar.getOut1()] = p1;
                qubitAtPos[p0] = bar.getOut0();
                qubitAtPos[p1] = bar.getOut1();
                continue;
            }

            // -- CNOT: route if needed, then forward positions.
            if (auto cnot = dyn_cast<CNOT_Op>(op)) {
                routeCNOT(cnot, posOf, qubitAtPos, builder);
                unsigned pc = posOf.lookup(cnot.getControl());
                unsigned pt = posOf.lookup(cnot.getTarget());
                posOf[cnot.getControlOut()] = pc;
                posOf[cnot.getTargetOut()]  = pt;
                qubitAtPos[pc] = cnot.getControlOut();
                qubitAtPos[pt] = cnot.getTargetOut();
                continue;
            }

            // -- measure / return: nothing to track.
        }
    }
}

// -- helper: bubble one operand toward the other -----------------------
static bool is1QGate(Operation *op) {
    return isa<H_Op>(op) || isa<X_Op>(op) || isa<Y_Op>(op) ||
           isa<Z_Op>(op) || isa<S_Op>(op) || isa<Sdg_Op>(op) ||
           isa<T_Op>(op) || isa<Tdg_Op>(op) || isa<U3_Op>(op);
}

void routeCNOT(CNOT_Op cnot,
               DenseMap<Value, unsigned> &posOf,
               DenseMap<unsigned, Value> &qubitAtPos,
               OpBuilder &builder) {
    Value c = cnot.getControl();
    Value t = cnot.getTarget();
    unsigned pc = posOf.lookup(c);
    unsigned pt = posOf.lookup(t);

    int dist = static_cast<int>(pc) - static_cast<int>(pt);
    if (std::abs(dist) == 1)
        return;   // already adjacent

    if (pt > pc + 1) {
        // Bubble target left, down toward control.
        for (unsigned i = pt - 1; i > pc; --i) {
            Value other = qubitAtPos.lookup(i);
            builder.setInsertionPoint(cnot);
            auto sw = builder.create<Swap_Op>(cnot.getLoc(), t, other);
            // t moves to position i; other moves to t's old position pt.
            unsigned tOld = posOf.lookup(t);
            posOf[sw.getOut0()] = i;
            posOf[sw.getOut1()] = tOld;
            qubitAtPos[i] = sw.getOut0();
            qubitAtPos[tOld] = sw.getOut1();
            t.replaceAllUsesExcept(sw.getOut0(), sw);
            other.replaceAllUsesExcept(sw.getOut1(), sw);
            t = sw.getOut0();
            pt = i;
        }
    } else {
        // pt < pc - 1: bubble control right, up toward target.
        for (unsigned i = pc - 1; i > pt; --i) {
            Value other = qubitAtPos.lookup(i);
            builder.setInsertionPoint(cnot);
            auto sw = builder.create<Swap_Op>(cnot.getLoc(), c, other);
            unsigned cOld = posOf.lookup(c);
            posOf[sw.getOut0()] = i;
            posOf[sw.getOut1()] = cOld;
            qubitAtPos[i] = sw.getOut0();
            qubitAtPos[cOld] = sw.getOut1();
            c.replaceAllUsesExcept(sw.getOut0(), sw);
            other.replaceAllUsesExcept(sw.getOut1(), sw);
            c = sw.getOut0();
            pc = i;
        }
    }
}
};

} // namespace

void mlir::qec::registerQECRoutePass() {
    PassRegistration<QECRoutePass>();
}