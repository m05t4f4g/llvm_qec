// Run:  qec-opt --qec-fuse-1q test/fuse_1q.mlir
//
// Expected after the pass:
//   @hh        → no ops (H·H = I)
//   @ss        → one qec.u3 (S·S = Z = U3(0,0,π))
//   @ttdg      → no ops (T·T† = I)
//   @hsh       → one qec.u3 (H·S·H, general phase gate)
//   @xyz       → no ops (X·Y·Z = i·I, identity up to phase)
//   @single    → unchanged (only one op)
//   @h_not_1q  → unchanged (H feeds CNOT, not another 1q op)

func.func @hh(%q: !qec.qubit) -> !qec.qubit {
    %a = qec.h %q : !qec.qubit -> !qec.qubit
    %b = qec.h %a : !qec.qubit -> !qec.qubit
    return %b : !qec.qubit
}

func.func @ss(%q: !qec.qubit) -> !qec.qubit {
    %a = qec.s %q : !qec.qubit -> !qec.qubit
    %b = qec.s %a : !qec.qubit -> !qec.qubit
    return %b : !qec.qubit
}

func.func @ttdg(%q: !qec.qubit) -> !qec.qubit {
    %a = qec.t   %q : !qec.qubit -> !qec.qubit
    %b = qec.tdg %a : !qec.qubit -> !qec.qubit
    return %b : !qec.qubit
}

func.func @hsh(%q: !qec.qubit) -> !qec.qubit {
    %a = qec.h %q : !qec.qubit -> !qec.qubit
    %b = qec.s %a : !qec.qubit -> !qec.qubit
    %c = qec.h %b : !qec.qubit -> !qec.qubit
    return %c : !qec.qubit
}

func.func @xyz(%q: !qec.qubit) -> !qec.qubit {
    %a = qec.x %q : !qec.qubit -> !qec.qubit
    %b = qec.y %a : !qec.qubit -> !qec.qubit
    %c = qec.z %b : !qec.qubit -> !qec.qubit
    return %c : !qec.qubit
}

func.func @single(%q: !qec.qubit) -> !qec.qubit {
    %a = qec.h %q : !qec.qubit -> !qec.qubit
    return %a : !qec.qubit
}

func.func @h_not_1q(%q: !qec.qubit, %r: !qec.qubit) -> (!qec.qubit, !qec.qubit) {
    %a = qec.h %q : !qec.qubit -> !qec.qubit
    %c, %t = qec.cnot %a, %r
        : !qec.qubit, !qec.qubit -> !qec.qubit, !qec.qubit
    return %c, %t : !qec.qubit, !qec.qubit
}