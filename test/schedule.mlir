// Run with:  qec-opt --qec-schedule test/schedule.mlir
//
// Every qec.cnot gets a qec.barrier immediately after it. Downstream
// users pick up the barrier outputs so linearity is preserved.

func.func @bell() -> (i1, i1) {
    %q0 = qec.alloc : !qec.qubit
    %q1 = qec.alloc : !qec.qubit
    %h  = qec.h %q0 : !qec.qubit -> !qec.qubit
    %c, %t = qec.cnot %h, %q1 : !qec.qubit, !qec.qubit -> !qec.qubit, !qec.qubit
    %bc = qec.measure %c : !qec.qubit
    %bt = qec.measure %t : !qec.qubit
    return %bc, %bt : i1, i1
}

func.func @two_cnots(%a: !qec.qubit, %b: !qec.qubit) -> (!qec.qubit, !qec.qubit) {
    %c1, %t1 = qec.cnot %a, %b : !qec.qubit, !qec.qubit -> !qec.qubit, !qec.qubit
    %c2, %t2 = qec.cnot %c1, %t1 : !qec.qubit, !qec.qubit -> !qec.qubit, !qec.qubit
    return %c2, %t2 : !qec.qubit, !qec.qubit
}