func.func @phase_gate() -> i1 {
    %q  = qec.alloc : !qec.qubit
    %a  = qec.h  %q  : !qec.qubit -> !qec.qubit
    %b  = qec.s  %a  : !qec.qubit -> !qec.qubit
    %c  = qec.h  %b  : !qec.qubit -> !qec.qubit
    %r  = qec.measure %c : !qec.qubit
    return %r : i1
}