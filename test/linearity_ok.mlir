// A linearity-clean Bell-pair preparation + measurement.
// Run with:  qec-opt --qec-linearity test/linearity_ok.mlir

func.func @bell() -> (i1, i1) {
  %q0 = qec.alloc : !qec.qubit
  %q1 = qec.alloc : !qec.qubit

  %h = qec.h %q0 : !qec.qubit -> !qec.qubit

  %c, %t = qec.cnot %h, %q1
      : !qec.qubit, !qec.qubit -> !qec.qubit, !qec.qubit

  %bc = qec.measure %c : !qec.qubit
  %bt = qec.measure %t : !qec.qubit
  return %bc, %bt : i1, i1
}