// lib/QECToQIR.cpp
#include "QEC/QECToQIR.h"
#include "QEC/QECOps.h"
#include "QEC/QECTypes.h"

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/Tools/mlir-translate/Translation.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/raw_ostream.h"

#include <sstream>
#include <string>

using namespace mlir;
using namespace mlir::qec;

namespace {

// ---------------------------------------------------------------------------
// Per-function QIR builder.
// ---------------------------------------------------------------------------
    struct QirFuncBuilder {
        raw_ostream &os;
        DenseMap<Value, unsigned> qubitIndex;
        DenseMap<Value, unsigned> resultIndex;
        DenseSet<Value> allocatedQubits;
        unsigned nextQubit  = 0;
        unsigned nextResult = 0;

        explicit QirFuncBuilder(raw_ostream &out) : os(out) {}

        // A !qec.qubit function argument arrives as a %Qubit* parameter.
        void bindArgument(Value arg) {
            if (isa<QubitType>(arg.getType()))
                qubitIndex[arg] = nextQubit++;
        }

        std::string qubitRef(Value v) const {
            return "%q" + std::to_string(qubitIndex.lookup(v));
        }

        // Shared helper for single-qubit gate ops.
        LogicalResult aliasAndCall(Value in, Value out, StringRef callee) {
            unsigned idx = qubitIndex.lookup(in);
            qubitIndex[out] = idx;
            os << "  call void @" << callee << "(%Qubit* %q" << idx << ")\n";
            return success();
        }

        LogicalResult emitOp(Operation &op) {
            // -- 1. qec.alloc ---------------------------------------------------
            if (auto alloc = dyn_cast<Alloc_Op>(op)) {
                unsigned idx = nextQubit++;
                qubitIndex[alloc.getOutput()] = idx;
                allocatedQubits.insert(alloc.getOutput());
                os << "  %q" << idx
                   << " = call %Qubit* @__quantum__rt__qubit_allocate()\n";
                return success();
            }

            // -- 2. Named single-qubit gates -----------------------------------
            if (isa<H_Op>(op))
                return aliasAndCall(op.getOperand(0), op.getResult(0),
                                    "__quantum__qis__h__body");
            if (isa<X_Op>(op))
                return aliasAndCall(op.getOperand(0), op.getResult(0),
                                    "__quantum__qis__x__body");
            if (isa<Y_Op>(op))
                return aliasAndCall(op.getOperand(0), op.getResult(0),
                                    "__quantum__qis__y__body");
            if (isa<Z_Op>(op))
                return aliasAndCall(op.getOperand(0), op.getResult(0),
                                    "__quantum__qis__z__body");
            if (isa<S_Op>(op))
                return aliasAndCall(op.getOperand(0), op.getResult(0),
                                    "__quantum__qis__s__body");
            if (isa<Sdg_Op>(op))
                return aliasAndCall(op.getOperand(0), op.getResult(0),
                                    "__quantum__qis__s__adj");
            if (isa<T_Op>(op))
                return aliasAndCall(op.getOperand(0), op.getResult(0),
                                    "__quantum__qis__t__body");
            if (isa<Tdg_Op>(op))
                return aliasAndCall(op.getOperand(0), op.getResult(0),
                                    "__quantum__qis__t__adj");

            // -- 3. U3 ---------------------------------------------------------
            if (auto u3 = dyn_cast<U3_Op>(op)) {
                unsigned idx = qubitIndex.lookup(u3.getInput());
                qubitIndex[u3.getOutput()] = idx;
                os << "  call void @__quantum__qis__u3__body(%Qubit* %q" << idx
                   << ", double " << u3.getTheta().convertToDouble()
                   << ", double " << u3.getPhi().convertToDouble()
                   << ", double " << u3.getLambda().convertToDouble() << ")\n";
                return success();
            }

            // -- 4. CNOT -------------------------------------------------------
            if (auto cnot = dyn_cast<CNOT_Op>(op)) {
                unsigned ci = qubitIndex.lookup(cnot.getControl());
                unsigned ti = qubitIndex.lookup(cnot.getTarget());
                qubitIndex[cnot.getControlOut()] = ci;
                qubitIndex[cnot.getTargetOut()]  = ti;
                os << "  call void @__quantum__qis__cnot__body(%Qubit* %q" << ci
                   << ", %Qubit* %q" << ti << ")\n";
                return success();
            }

            // -- 5. Barrier ----------------------------------------------------
            // Not a runtime instruction: just thread the qubit indices through
            // so downstream ops see the correct wire numbers.
            if (auto barrier = dyn_cast<Barrier_Op>(op)) {
                qubitIndex[barrier.getOut0()] = qubitIndex.lookup(barrier.getIn0());
                qubitIndex[barrier.getOut1()] = qubitIndex.lookup(barrier.getIn1());
                os << "  ; barrier\n";
                return success();
            }

            // -- 6. Measure ----------------------------------------------------
            if (auto meas = dyn_cast<Measure_Op>(op)) {
                unsigned qi = qubitIndex.lookup(meas.getInput());
                unsigned ri = nextResult++;
                os << "  %r" << ri
                   << " = call %Result* @__quantum__rt__result_allocate()\n";
                os << "  call void @__quantum__qis__mz__body(%Qubit* %q" << qi
                   << ", %Result* %r" << ri << ")\n";
                os << "  %v" << ri
                   << " = call i1 @__quantum__rt__read_result(%Result* %r"
                   << ri << ")\n";
                os << "  call void @__quantum__rt__result_release(%Result* %r"
                   << ri << ")\n";
                resultIndex[meas.getResult()] = ri;
                return success();
            }

            // -- 7. func.return -------------------------------------------------
            if (auto ret = dyn_cast<func::ReturnOp>(op)) {
                // Release every allocated qubit that is still live.
                for (Value v : allocatedQubits)
                    os << "  call void @__quantum__rt__qubit_release(%Qubit* "
                       << qubitRef(v) << ")\n";

                if (ret.getNumOperands() == 0) {
                    os << "  ret void\n";
                    return success();
                }

                std::string retType, retVals;
                for (unsigned i = 0; i < ret.getNumOperands(); ++i) {
                    Value v = ret.getOperand(i);
                    if (i > 0) { retType += ", "; retVals += ", "; }
                    if (isa<IntegerType>(v.getType()) && v.getType().isInteger(1)) {
                        unsigned ri = resultIndex.lookup(v);
                        retType += "i1";
                        retVals += "%v" + std::to_string(ri);
                    } else if (isa<QubitType>(v.getType())) {
                        retType += "%Qubit*";
                        retVals += qubitRef(v);
                    } else {
                        return op.emitError("qec-to-qir: unsupported return type");
                    }
                }

                if (ret.getNumOperands() > 1)
                    retType = "{ " + retType + " }";
                os << "  ret " << retType << " " << retVals << "\n";
                return success();
            }

            return op.emitError("qec-to-qir: unsupported op '")
                    << op.getName() << "'";
        }
    };

// ---------------------------------------------------------------------------
// Emit the QIR text for a single func.func.
// ---------------------------------------------------------------------------
    LogicalResult emitQIRForFunc(func::FuncOp func, raw_ostream &os) {
        auto fnTy = func.getFunctionType();

        // Return type.
        std::string retTy;
        if (fnTy.getNumResults() == 0) {
            retTy = "void";
        } else if (fnTy.getNumResults() == 1) {
            Type rt = fnTy.getResult(0);
            if (isa<QubitType>(rt))         retTy = "%Qubit*";
            else if (rt.isInteger(1))       retTy = "i1";
            else return func.emitError("qec-to-qir: unsupported return type");
        } else {
            retTy = "{ ";
            for (unsigned i = 0; i < fnTy.getNumResults(); ++i) {
                if (i) retTy += ", ";
                Type rt = fnTy.getResult(i);
                if (isa<QubitType>(rt))     retTy += "%Qubit*";
                else if (rt.isInteger(1))   retTy += "i1";
                else return func.emitError("qec-to-qir: unsupported return type");
            }
            retTy += " }";
        }

        // Argument types.
        std::string argTys;
        for (unsigned i = 0; i < fnTy.getNumInputs(); ++i) {
            if (i) argTys += ", ";
            Type at = fnTy.getInput(i);
            if (isa<QubitType>(at))         argTys += "%Qubit*";
            else return func.emitError("qec-to-qir: unsupported arg type");
        }

        os << "define " << retTy << " @" << func.getSymName()
           << "(" << argTys << ") {\n";

        QirFuncBuilder b(os);
        for (BlockArgument arg : func.getArguments())
            b.bindArgument(arg);

        for (Block &block : func.getBody())
            for (Operation &op : block)
                if (failed(b.emitOp(op)))
                    return failure();

        os << "}\n\n";
        return success();
    }

// ---------------------------------------------------------------------------
// Whole-module translation.
// ---------------------------------------------------------------------------
    LogicalResult translateToQIR(Operation *op, raw_ostream &os) {
        auto module = dyn_cast<ModuleOp>(op);
        if (!module)
            return op->emitError("qec-to-qir expects a module");

        os << R"qir(; QIR (Quantum Intermediate Representation) module
; Emitted from the QEC dialect by qec-to-qir.

%Qubit  = type opaque
%Result = type opaque

; --- runtime intrinsics ---
declare %Qubit*  @__quantum__rt__qubit_allocate()
declare void     @__quantum__rt__qubit_release(%Qubit*)
declare %Result* @__quantum__rt__result_allocate()
declare void     @__quantum__rt__result_release(%Result*)
declare i1       @__quantum__rt__read_result(%Result*)

; --- quantum intrinsics ---
declare void @__quantum__qis__h__body(%Qubit*)
declare void @__quantum__qis__x__body(%Qubit*)
declare void @__quantum__qis__y__body(%Qubit*)
declare void @__quantum__qis__z__body(%Qubit*)
declare void @__quantum__qis__s__body(%Qubit*)
declare void @__quantum__qis__s__adj(%Qubit*)
declare void @__quantum__qis__t__body(%Qubit*)
declare void @__quantum__qis__t__adj(%Qubit*)
declare void @__quantum__qis__u3__body(%Qubit*, double, double, double)
declare void @__quantum__qis__cnot__body(%Qubit*, %Qubit*)
declare void @__quantum__qis__mz__body(%Qubit*, %Result*)

)qir";

        for (auto func : module.getOps<func::FuncOp>())
            if (failed(emitQIRForFunc(func, os)))
                return failure();
        return success();
    }

} // namespace

// ---------------------------------------------------------------------------
// Registration.
// ---------------------------------------------------------------------------
void mlir::qec::registerQECToQIRTranslation() {
    static TranslateFromMLIRRegistration registration(
            "qec-to-qir",
            "Emit QIR (LLVM IR with __quantum__qis__ intrinsics) from the QEC dialect",
            [](Operation *op, raw_ostream &output) {
                return translateToQIR(op, output);
            },
            [](DialectRegistry &registry) {
                registry.insert<QECDialect,
                        func::FuncDialect,
                        arith::ArithDialect>();
            });
}