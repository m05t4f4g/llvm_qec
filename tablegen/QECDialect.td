#ifndef QEC_DIALECT_TD
#define QEC_DIALECT_TD

include "mlir/IR/OpBase.td"
include "mlir/IR/AttrTypeBase.td"
include "mlir/IR/OpAsmInterface.td"
include "mlir/IR/Properties.td"

def QEC_Dialect : Dialect {
  let name = "qec";
  let summary = "Quantum Error Correction Dialect";
  let cppNamespace = "::mlir::qec";
  let useDefaultTypePrinterParser = 1;    // <-- ADD THIS
}

class QEC_Op<string mnemonic, list<Trait> traits = []>
    : Op<QEC_Dialect, mnemonic, traits>;

def QEC_Qubit : TypeDef<QEC_Dialect, "Qubit"> {
  let summary = "A quantum bit type";
  let mnemonic = "qubit";
  let assemblyFormat = "";          // <-- ADD THIS
}

def QEC_CNOT_Op : QEC_Op<"cnot"> {
  let arguments = (ins QEC_Qubit:$control, QEC_Qubit:$target);
  let results   = (outs QEC_Qubit:$control_out, QEC_Qubit:$target_out);
  let assemblyFormat = "$control `,` $target attr-dict "
                       "`:` type($control) `,` type($target) "
                       "`->` type($control_out) `,` type($target_out)";
}

#endif // QEC_DIALECT_TD