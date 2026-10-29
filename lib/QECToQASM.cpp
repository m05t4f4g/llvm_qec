// lib/QECToQASM.cpp
#include <sstream>
#include "QEC/QECToQASM.h"
#include "QEC/QECOps.h"
#include "QEC/QECTypes.h"

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/Tools/mlir-translate/Translation.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/raw_ostream.h"

#include <string>

using namespace mlir;
using namespace mlir::qec;

namespace {

// Walk one func.func, emit a self-contained OpenQASM 2.0 program for it.
//
// Strategy: each !qec.qubit SSA value is mapped to a fixed q[...] index.
//   - qec.alloc         → allocate a fresh index
//   - func.func args    → allocate a fresh index (they're !qec.qubit too)
//   - qec.h    %in→%out → %out aliases %in
//   - qec.cnot %c,%t→%co,%to → %co aliases %c, %to aliases %t
//   - qec.measure %q    → allocate a fresh c[...] index
//
// Every other op is unsupported and errors out.
    LogicalResult emitQASMForFunc(func::FuncOp func, raw_ostream &os) {
        DenseMap<Value, unsigned> qasmIndex;
        unsigned nextQubit = 0;

        // Function arguments that are !qec.qubit get fresh QASM indices.
        for (BlockArgument arg : func.getArguments()) {
            if (isa<QubitType>(arg.getType()))
                qasmIndex[arg] = nextQubit++;
        }

        SmallVector<std::string> lines;
        unsigned numMeasures = 0;

        for (Block &block : func.getBody()) {
            for (Operation &op : block) {
                if (auto alloc = dyn_cast<Alloc_Op>(op)) {
                    qasmIndex[alloc.getOutput()] = nextQubit++;
                    continue;
                }
                if (auto h = dyn_cast<H_Op>(op)) {
                    unsigned idx = qasmIndex[h.getInput()];
                    qasmIndex[h.getOutput()] = idx;
                    lines.push_back("h q[" + std::to_string(idx) + "];");
                    continue;
                }
                if (auto cnot = dyn_cast<CNOT_Op>(op)) {
                    unsigned cIdx = qasmIndex[cnot.getControl()];
                    unsigned tIdx = qasmIndex[cnot.getTarget()];
                    qasmIndex[cnot.getControlOut()] = cIdx;
                    qasmIndex[cnot.getTargetOut()]  = tIdx;
                    lines.push_back("cx q[" + std::to_string(cIdx) + "],q["
                                    + std::to_string(tIdx) + "];");
                    continue;
                }
                if (auto u3 = dyn_cast<U3_Op>(op)) {
                    unsigned idx = qasmIndex[u3.getInput()];
                    qasmIndex[u3.getOutput()] = idx;

                    std::ostringstream oss;
                    oss << "u3(" << u3.getTheta().convertToDouble() << ","
                        << u3.getPhi().convertToDouble()   << ","
                        << u3.getLambda().convertToDouble() << ") q["
                        << idx << "];";
                    lines.push_back(oss.str());
                    continue;
                }
                if (auto meas = dyn_cast<Measure_Op>(op)) {
                    unsigned qIdx = qasmIndex[meas.getInput()];
                    unsigned cIdx = numMeasures++;
                    lines.push_back("measure q[" + std::to_string(qIdx)
                                    + "] -> c[" + std::to_string(cIdx) + "];");
                    continue;
                }
                if (isa<func::ReturnOp>(op))
                    continue;

                return op.emitError("qec-to-qasm: unsupported op '")
                        << op.getName() << "'";
            }
        }

        // Nothing quantum happened → nothing to emit.
        if (nextQubit == 0 && numMeasures == 0)
            return success();

        os << "OPENQASM 2.0;\n";
        os << "include \"qelib1.inc\";\n";
        os << "qreg q[" << nextQubit << "];\n";
        if (numMeasures > 0)
            os << "creg c[" << numMeasures << "];\n";
        for (const std::string &line : lines)
            os << line << "\n";

        return success();
    }

    LogicalResult translateToQASM(Operation *op, raw_ostream &output) {
        auto module = dyn_cast<ModuleOp>(op);
        if (!module)
            return op->emitError("qec-to-qasm expects a module");

        bool first = true;
        for (auto func : module.getOps<func::FuncOp>()) {
            if (!first)
                output << "\n// === " << func.getSymName() << " ===\n";
            first = false;
            if (failed(emitQASMForFunc(func, output)))
                return failure();
        }
        return success();
    }

} // namespace

void mlir::qec::registerQECToQASMTranslation() {
    static TranslateFromMLIRRegistration registration(
            "qec-to-qasm",
            "Emit OpenQASM 2.0 from the QEC dialect",
            [](Operation *op, raw_ostream &output) {
                return translateToQASM(op, output);
            },
            [](DialectRegistry &registry) {
                registry.insert<QECDialect,
                        func::FuncDialect,
                        arith::ArithDialect>();
            });
}