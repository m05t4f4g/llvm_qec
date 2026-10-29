// Run:  qec-opt --qec-scheduler test/scheduler.mlir
// Then: qec-opt --qec-scheduler --qec-fuse-1q test/scheduler.mlir   (fusion now fires)
//
//  @interleaved: [H_a, H_b, H_a, H_b]  →  [H_a, H_a, H_b, H_b]  →  fuse → [I, I] = [] or 1 u3 per wire
//  @already_sorted: unchanged (already grouped)

func.func @interleaved(%a: !qec.qubit, %b: !qec.qubit) -> (!qec.qubit, !qec.qubit) {
    %a1 = qec.h %a : !qec.qubit -> !qec.qubit
    %b1 = qec.h %b : !qec.qubit -> !qec.qubit
    %a2 = qec.h %a1 : !qec.qubit -> !qec.qubit
    %b2 = qec.h %b1 : !qec.qubit -> !qec.qubit
    return %a2, %b2 : !qec.qubit, !qec.qubit
}

func.func @already_sorted(%a: !qec.qubit, %b: !qec.qubit) -> (!qec.qubit, !qec.qubit) {
    %a1 = qec.h %a : !qec.qubit -> !qec.qubit
    %a2 = qec.s %a1 : !qec.qubit -> !qec.qubit
    %b1 = qec.h %b : !qec.qubit -> !qec.qubit
    %b2 = qec.s %b1 : !qec.qubit -> !qec.qubit
    return %a2, %b2 : !qec.qubit, !qec.qubit
}