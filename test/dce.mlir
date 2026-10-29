// Run:  qec-opt --qec-dce test/dce.mlir
//
//  @unused_alloc     → q2 allocation removed
//  @dead_gate_chain  → h and x removed (dead chain)
//  @measure_kept     → alloc + measure stay (measure has side effect)
//  @partial_cnot     → cnot stays (one of its two results is still live)

// q2 is allocated but never used → DCE removes it.
func.func @unused_alloc() -> i1 {
    %q1 = qec.alloc : !qec.qubit
    %q2 = qec.alloc : !qec.qubit                 // dead
    %m  = qec.measure %q1 : !qec.qubit
    return %m : i1
}

// h and x form a dead branch off q → both removed.
func.func @dead_gate_chain() -> i1 {
    %q  = qec.alloc : !qec.qubit
    %h  = qec.h %q : !qec.qubit -> !qec.qubit    // dead (result unused)
    %x  = qec.x %h : !qec.qubit -> !qec.qubit    // dead (result unused)
    %m  = qec.measure %q : !qec.qubit
    return %m : i1
}

// measurement has a side effect, must NOT be removed even though %m is unused.
func.func @measure_kept() {
    %q = qec.alloc : !qec.qubit
    %m = qec.measure %q : !qec.qubit             // side effect: keep
    return
}

// only %c of the CNOT is used; the op cannot be fully removed.
func.func @partial_cnot(%a: !qec.qubit, %b: !qec.qubit) -> i1 {
    %c, %t = qec.cnot %a, %b
        : !qec.qubit, !qec.qubit -> !qec.qubit, !qec.qubit
    %m = qec.measure %c : !qec.qubit
    return %m : i1
}