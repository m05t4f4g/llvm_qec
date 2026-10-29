// RUN: qec-opt %s
//
// Allocate two qubits, apply H to the first, CNOT them together,
// then measure the control. (Not a verifier-checked semantics test,
// just a parse/print/round-trip exercise for the dialect.)

func.func @bell() -> i1 {
  %q0 = qec.alloc : !qec.qubit
  %q1 = qec.alloc : !qec.qubit

  %h = qec.h %q0 : !qec.qubit -> !qec.qubit

  %c, %t = qec.cnot %h, %q1 : !qec.qubit, !qec.qubit -> !qec.qubit, !qec.qubit

  %bit = qec.measure %c : !qec.qubit
  return %bit : i1
}