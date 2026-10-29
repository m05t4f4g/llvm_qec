#ifndef QEC_DIALECT_TD
#define QEC_DIALECT_TD

include "mlir/IR/OpBase.td"
include "mlir/IR/AttrTypeBase.td"
include "mlir/IR/OpAsmInterface.td"
include "mlir/IR/Properties.td"

//===----------------------------------------------------------------------===//
// Dialect
//===----------------------------------------------------------------------===//
def QEC_Dialect : Dialect {
  let name = "qec";
  let summary = "Quantum Error Correction Dialect";
  let cppNamespace = "::mlir::qec";

  // Generate QECDialect::parseType / printType that dispatch to
  // generatedTypeParser / generatedTypePrinter from QECTypes.cpp.inc.
  let useDefaultTypePrinterParser = 1;
}

// Base class for all QEC ops.
class QEC_Op<string mnemonic, list<Trait> traits = []>
    : Op<QEC_Dialect, mnemonic, traits>;

//===----------------------------------------------------------------------===//
// Qubit type  (concrete syntax: !qec.qubit)
//===----------------------------------------------------------------------===//
def QEC_Qubit : TypeDef<QEC_Dialect, "Qubit"> {
  let summary = "A quantum bit type";
  let mnemonic = "qubit";
  let assemblyFormat = "";        // parameterless type
}

//===----------------------------------------------------------------------===//
// Ops
//===----------------------------------------------------------------------===//

// qec.alloc : produces a fresh |0> qubit.
//   %q = qec.alloc : !qec.qubit
def QEC_Alloc_Op : QEC_Op<"alloc"> {
  let results = (outs QEC_Qubit:$output);
  let assemblyFormat = "attr-dict `:` type($output)";
}

// qec.h : Hadamard on a single qubit.
//   %out = qec.h %in : !qec.qubit -> !qec.qubit
def QEC_H_Op : QEC_Op<"h"> {
  let arguments = (ins  QEC_Qubit:$input);
  let results   = (outs QEC_Qubit:$output);
  let assemblyFormat = "$input attr-dict `:` type($input) `->` type($output)";
}

// qec.cnot : two-qubit CNOT.
//   %co, %to = qec.cnot %c, %t : !qec.qubit, !qec.qubit -> !qec.qubit, !qec.qubit
def QEC_CNOT_Op : QEC_Op<"cnot"> {
  let arguments = (ins  QEC_Qubit:$control,     QEC_Qubit:$target);
  let results   = (outs QEC_Qubit:$control_out, QEC_Qubit:$target_out);
  let assemblyFormat = "$control `,` $target attr-dict "
                       "`:` type($control) `,` type($target) "
                       "`->` type($control_out) `,` type($target_out)";
}

// qec.measure : measurement in the Z basis, produces i1.
//   %bit = qec.measure %q : !qec.qubit
def QEC_Measure_Op : QEC_Op<"measure"> {
  let arguments = (ins  QEC_Qubit:$input);
  let results   = (outs I1:$result);
  let assemblyFormat = "$input attr-dict `:` type($input)";
}

//===----------------------------------------------------------------------===//
// Single-qubit named gates (all are U3 specialisations)
//===----------------------------------------------------------------------===//
class QEC_SingleQubitOp<string mnemonic, list<Trait> traits = []>
    : QEC_Op<mnemonic, traits> {
  let arguments = (ins  QEC_Qubit:$input);
  let results   = (outs QEC_Qubit:$output);
  let assemblyFormat = "$input attr-dict `:` type($input) `->` type($output)";
}

def QEC_S_Op   : QEC_SingleQubitOp<"s">    { let summary = "S gate (phase π/2)"; }
def QEC_Sdg_Op : QEC_SingleQubitOp<"sdg">  { let summary = "S† gate"; }
def QEC_T_Op   : QEC_SingleQubitOp<"t">    { let summary = "T gate (phase π/4)"; }
def QEC_Tdg_Op : QEC_SingleQubitOp<"tdg">  { let summary = "T† gate"; }
def QEC_X_Op   : QEC_SingleQubitOp<"x">    { let summary = "Pauli X"; }
def QEC_Y_Op   : QEC_SingleQubitOp<"y">    { let summary = "Pauli Y"; }
def QEC_Z_Op   : QEC_SingleQubitOp<"z">    { let summary = "Pauli Z"; }

// Universal 1-qubit gate:  U3(θ,φ,λ)
//   [[cos(θ/2),            -e^{iλ}·sin(θ/2)],
//    [e^{iφ}·sin(θ/2),  e^{i(φ+λ)}·cos(θ/2)]]
// Syntax:  qec.u3 %q[1.57, 0.0, 3.14] : !qec.qubit -> !qec.qubit
def QEC_U3_Op : QEC_Op<"u3"> {
  let summary = "Universal single-qubit gate U3(θ, φ, λ)";
  let arguments = (ins  QEC_Qubit:$input,
                        F64Attr:$theta, F64Attr:$phi, F64Attr:$lambda);
  let results   = (outs QEC_Qubit:$output);
  let assemblyFormat = "$input `[` $theta `,` $phi `,` $lambda `]` attr-dict "
                       "`:` type($input) `->` type($output)";
}

// A scheduling fence: the two qubits are synchronized at this point.
// Outputs alias inputs (threading ensures linearity is preserved).
def QEC_Barrier_Op : QEC_Op<"barrier"> {
  let summary = "Scheduling barrier between two qubits";
  let arguments = (ins QEC_Qubit:$in0, QEC_Qubit:$in1);
  let results   = (outs QEC_Qubit:$out0, QEC_Qubit:$out1);
  let assemblyFormat = "$in0 `,` $in1 attr-dict `:` type($in0) `,` type($in1)";
}
#endif // QEC_DIALECT_TD