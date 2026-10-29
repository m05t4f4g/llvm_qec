// Run with:  qec-opt --qec-cancel-cnot test/cnot_cancel.mlir
//
// Expected after the pass:
//   @basic        → both CNOTs removed;  return %arg0, %arg1
//   @chain        → 3 CNOTs become 1;    net effect = one CNOT
//   @single       → unchanged (only one CNOT)
//   @swapped      → unchanged (CNOT(c,t) then CNOT(t,c) is not I)
//   @shared_inner → unchanged (inner's control_out has a 2nd user besides outer)

func.func @basic(%c: !qec.qubit, %t: !qec.qubit) -> (!qec.qubit, !qec.qubit) {
    %c1, %t1 = qec.cnot %c, %t
        : !qec.qubit, !qec.qubit -> !qec.qubit, !qec.qubit
    %c2, %t2 = qec.cnot %c1, %t1
        : !qec.qubit, !qec.qubit -> !qec.qubit, !qec.qubit
    return %c2, %t2 : !qec.qubit, !qec.qubit
}

func.func @chain(%c: !qec.qubit, %t: !qec.qubit) -> (!qec.qubit, !qec.qubit) {
    %c1, %t1 = qec.cnot %c, %t     : !qec.qubit, !qec.qubit -> !qec.qubit, !qec.qubit
    %c2, %t2 = qec.cnot %c1, %t1   : !qec.qubit, !qec.qubit -> !qec.qubit, !qec.qubit
    %c3, %t3 = qec.cnot %c2, %t2   : !qec.qubit, !qec.qubit -> !qec.qubit, !qec.qubit
    return %c3, %t3 : !qec.qubit, !qec.qubit
}

func.func @single(%c: !qec.qubit, %t: !qec.qubit) -> (!qec.qubit, !qec.qubit) {
    %c1, %t1 = qec.cnot %c, %t : !qec.qubit, !qec.qubit -> !qec.qubit, !qec.qubit
    return %c1, %t1 : !qec.qubit, !qec.qubit
}

func.func @swapped(%c: !qec.qubit, %t: !qec.qubit) -> (!qec.qubit, !qec.qubit) {
    %c1, %t1 = qec.cnot %c, %t     : !qec.qubit, !qec.qubit -> !qec.qubit, !qec.qubit
    %c2, %t2 = qec.cnot %t1, %c1   : !qec.qubit, !qec.qubit -> !qec.qubit, !qec.qubit
    return %c2, %t2 : !qec.qubit, !qec.qubit
}

func.func @shared_inner(%c: !qec.qubit, %t: !qec.qubit, %x: !qec.qubit)
        -> (!qec.qubit, !qec.qubit, !qec.qubit) {
    %c1, %t1 = qec.cnot %c, %t     : !qec.qubit, !qec.qubit -> !qec.qubit, !qec.qubit
    %c2, %t2 = qec.cnot %c1, %t1   : !qec.qubit, !qec.qubit -> !qec.qubit, !qec.qubit
    // c1 has a second use: NOT a safe cancellation
    %c3, %t3 = qec.cnot %c1, %x    : !qec.qubit, !qec.qubit -> !qec.qubit, !qec.qubit
    return %c2, %c3, %t2 : !qec.qubit, !qec.qubit, !qec.qubit
}