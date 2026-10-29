// Tests for --qec-cancel-hh. Run with:
//   qec-opt --qec-cancel-hh test/hh_cancel.mlir
//
// Expected after the pass:
//   @basic    →  return %arg0                       (both H removed)
//   @chain4   →  return %arg0                       (all four H removed)
//   @odd      →  unchanged (single H)
//   @h_not_h  →  unchanged (H followed by CNOT, not H)

func.func @basic(%q: !qec.qubit) -> !qec.qubit {
    %a = qec.h %q : !qec.qubit -> !qec.qubit
    %b = qec.h %a : !qec.qubit -> !qec.qubit
    return %b : !qec.qubit
}

func.func @chain4(%q: !qec.qubit) -> !qec.qubit {
    %a = qec.h %q : !qec.qubit -> !qec.qubit
    %b = qec.h %a : !qec.qubit -> !qec.qubit
    %c = qec.h %b : !qec.qubit -> !qec.qubit
    %d = qec.h %c : !qec.qubit -> !qec.qubit
    return %d : !qec.qubit
}

func.func @odd(%q: !qec.qubit) -> !qec.qubit {
    %a = qec.h %q : !qec.qubit -> !qec.qubit
    return %a : !qec.qubit
}

func.func @h_not_h(%q: !qec.qubit, %r: !qec.qubit) -> (!qec.qubit, !qec.qubit) {
    %a = qec.h %q : !qec.qubit -> !qec.qubit
    %c, %t = qec.cnot %a, %r
        : !qec.qubit, !qec.qubit -> !qec.qubit, !qec.qubit
    return %c, %t : !qec.qubit, !qec.qubit
}