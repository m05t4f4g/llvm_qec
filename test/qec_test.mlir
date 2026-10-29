func.func @cnot(%q0: !qec.qubit, %q1: !qec.qubit) -> (!qec.qubit, !qec.qubit) {
  %c, %t = qec.cnot %q0, %q1 : !qec.qubit, !qec.qubit -> !qec.qubit, !qec.qubit
  return %c, %t : !qec.qubit, !qec.qubit
}