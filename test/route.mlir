// Run:  qec-opt --qec-route test/route.mlir
//
//  @adjacent      → unchanged (already on adjacent slots 0,1)
//  @distance2     → one SWAP inserted, then CNOT on {0,1}
//  @distance3     → two SWAPs inserted, then CNOT on {0,1}
//
// Linear topology: alloc order = physical slot order.

func.func @adjacent() -> (!qec.qubit, !qec.qubit) {
    %q0 = qec.alloc : !qec.qubit
    %q1 = qec.alloc : !qec.qubit
    %c, %t = qec.cnot %q0, %q1 : !qec.qubit, !qec.qubit -> !qec.qubit, !qec.qubit
    return %c, %t : !qec.qubit, !qec.qubit
}

func.func @distance2() -> (!qec.qubit, !qec.qubit) {
    %q0 = qec.alloc : !qec.qubit
    %q1 = qec.alloc : !qec.qubit
    %q2 = qec.alloc : !qec.qubit
    // q0 at slot 0, q2 at slot 2 — distance 2. Needs one SWAP.
    %c, %t = qec.cnot %q0, %q2 : !qec.qubit, !qec.qubit -> !qec.qubit, !qec.qubit
    // keep q1 live so linearity holds
    %dummy = qec.h %q1 : !qec.qubit -> !qec.qubit
    return %c, %dummy : !qec.qubit, !qec.qubit
}

func.func @distance3() -> (!qec.qubit, !qec.qubit) {
    %q0 = qec.alloc : !qec.qubit
    %q1 = qec.alloc : !qec.qubit
    %q2 = qec.alloc : !qec.qubit
    %q3 = qec.alloc : !qec.qubit
    // q0 at slot 0, q3 at slot 3 — distance 3. Needs two SWAPs.
    %c, %t = qec.cnot %q0, %q3 : !qec.qubit, !qec.qubit -> !qec.qubit, !qec.qubit
    %d1 = qec.h %q1 : !qec.qubit -> !qec.qubit
    %d2 = qec.h %q2 : !qec.qubit -> !qec.qubit
    return %c, %d1 : !qec.qubit, !qec.qubit
    // NB: %d2 and %t are consumed by nothing here — this test file is for
    // SHAPE only. Run without --qec-linearity.
}