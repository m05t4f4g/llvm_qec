// Run:  qec-opt --qec-dead-gate-elim test/dead_gate_elim.mlir
//
//  @xx, @yy, @zz, @hh     → each collapses to `return %arg0`
//  @cnot_pair             → collapses to `return %arg0, %arg1`
//  @hx_not_cancelled      → unchanged (H·X ≠ I)

func.func @xx(%q: !qec.qubit) -> !qec.qubit {
    %a = qec.x %q : !qec.qubit -> !qec.qubit
    %b = qec.x %a : !qec.qubit -> !qec.qubit
    return %b : !qec.qubit
}

func.func @yy(%q: !qec.qubit) -> !qec.qubit {
    %a = qec.y %q : !qec.qubit -> !qec.qubit
    %b = qec.y %a : !qec.qubit -> !qec.qubit
    return %b : !qec.qubit
}

func.func @zz(%q: !qec.qubit) -> !qec.qubit {
    %a = qec.z %q : !qec.qubit -> !qec.qubit
    %b = qec.z %a : !qec.qubit -> !qec.qubit
    return %b : !qec.qubit
}

func.func @hh(%q: !qec.qubit) -> !qec.qubit {
    %a = qec.h %q : !qec.qubit -> !qec.qubit
    %b = qec.h %a : !qec.qubit -> !qec.qubit
    return %b : !qec.qubit
}

func.func @cnot_pair(%c: !qec.qubit, %t: !qec.qubit)
        -> (!qec.qubit, !qec.qubit) {
    %c1, %t1 = qec.cnot %c, %t : !qec.qubit, !qec.qubit -> !qec.qubit, !qec.qubit
    %c2, %t2 = qec.cnot %c1, %t1 : !qec.qubit, !qec.qubit -> !qec.qubit, !qec.qubit
    return %c2, %t2 : !qec.qubit, !qec.qubit
}

func.func @hx_not_cancelled(%q: !qec.qubit) -> !qec.qubit {
    %a = qec.h %q : !qec.qubit -> !qec.qubit
    %b = qec.x %a : !qec.qubit -> !qec.qubit
    return %b : !qec.qubit
}