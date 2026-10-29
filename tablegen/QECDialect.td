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

#endif // QEC_DIALECT_TD