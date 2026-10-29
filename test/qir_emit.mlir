// Run:  qec-to-qir --qec-to-qir test/qir_emit.mlir
//
// Expected: a QIR module with __quantum__qis__h__body / cnot / mz calls.
//
//   @bell         → Bell-pair preparation + measure, returns (i1, i1)
//   @u3_gate      → single U3 gate + measure, returns i1
//   @with_barrier → CNOT followed by a barrier, returns two qubits

func.func @bell() -> (i1, i1) {
    %q0 = qec.alloc : !qec.qubit
    %q1 = qec.alloc : !qec.qubit
    %h  = qec.h %q0 : !qec.qubit -> !qec.qubit
    %c, %t = qec.cnot %h, %q1
        : !qec.qubit, !qec.qubit -> !qec.qubit, !qec.qubit
    %bc = qec.measure %c : !qec.qubit
    %bt = qec.measure %t : !qec.qubit
    return %bc, %bt : i1, i1
}

func.func @u3_gate() -> i1 {
    %q = qec.alloc : !qec.qubit
    %u = qec.u3 %q[1.5707963267948966, 0.0, 3.1415926535897931]
        : !qec.qubit -> !qec.qubit
    %r = qec.measure %u : !qec.qubit
    return %r : i1
}

func.func @with_barrier(%a: !qec.qubit, %b: !qec.qubit)
        -> (!qec.qubit, !qec.qubit) {
    %c, %t = qec.cnot %a, %b
        : !qec.qubit, !qec.qubit -> !qec.qubit, !qec.qubit
    %cb, %tb = qec.barrier %c, %t
        : !qec.qubit, !qec.qubit
    return %cb, %tb : !qec.qubit, !qec.qubit
}