// Run:  qec-translate --qec-to-qasm test/qasm_emit.mlir
//
// Expected output:
//   OPENQASM 2.0;
//   include "qelib1.inc";
//   qreg q[2];
//   creg c[2];
//   h q[0];
//   cx q[0],q[1];
//   measure q[0] -> c[0];
//   measure q[1] -> c[1];
//
//   // === with_args ===
//   OPENQASM 2.0;
//   include "qelib1.inc";
//   qreg q[2];
//   cx q[0],q[1];

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

func.func @with_args(%q0: !qec.qubit, %q1: !qec.qubit)
        -> (!qec.qubit, !qec.qubit) {
    %c, %t = qec.cnot %q0, %q1
        : !qec.qubit, !qec.qubit -> !qec.qubit, !qec.qubit
    return %c, %t : !qec.qubit, !qec.qubit
}