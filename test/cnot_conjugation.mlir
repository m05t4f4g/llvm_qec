// Run with:  qec-opt --qec-cnot-conjugation test/cnot_conjugation.mlir
//
// Expected results (see each `// =>` comment):
//
//   @x_on_control  →  %c2 = qec.x %c ;  %t2 = qec.x %t
//   @y_on_control  →  %c2 = qec.y %c ;  %t2 = qec.x %t
//   @z_on_control  →  %c2 = qec.z %c ;  %t2 = %t
//   @x_on_target   →  %c2 = %c     ;  %t2 = qec.x %t
//   @y_on_target   →  %c2 = qec.z %c ;  %t2 = qec.y %t
//   @z_on_target   →  %c2 = qec.z %c ;  %t2 = qec.z %t
//   @h_middle      →  unchanged (H does not commute through CNOT)
//   @swapped_cnot  →  unchanged (second CNOT uses swapped wires)

func.func @x_on_control(%c: !qec.qubit, %t: !qec.qubit) -> (!qec.qubit, !qec.qubit) {
    %c1, %t1 = qec.cnot %c, %t : !qec.qubit, !qec.qubit -> !qec.qubit, !qec.qubit
    %m       = qec.x  %c1     : !qec.qubit -> !qec.qubit
    %c2, %t2 = qec.cnot %m, %t1 : !qec.qubit, !qec.qubit -> !qec.qubit, !qec.qubit
    return %c2, %t2 : !qec.qubit, !qec.qubit
}

func.func @y_on_control(%c: !qec.qubit, %t: !qec.qubit) -> (!qec.qubit, !qec.qubit) {
    %c1, %t1 = qec.cnot %c, %t : !qec.qubit, !qec.qubit -> !qec.qubit, !qec.qubit
    %m       = qec.y  %c1     : !qec.qubit -> !qec.qubit
    %c2, %t2 = qec.cnot %m, %t1 : !qec.qubit, !qec.qubit -> !qec.qubit, !qec.qubit
    return %c2, %t2 : !qec.qubit, !qec.qubit
}

func.func @z_on_control(%c: !qec.qubit, %t: !qec.qubit) -> (!qec.qubit, !qec.qubit) {
    %c1, %t1 = qec.cnot %c, %t : !qec.qubit, !qec.qubit -> !qec.qubit, !qec.qubit
    %m       = qec.z  %c1     : !qec.qubit -> !qec.qubit
    %c2, %t2 = qec.cnot %m, %t1 : !qec.qubit, !qec.qubit -> !qec.qubit, !qec.qubit
    return %c2, %t2 : !qec.qubit, !qec.qubit
}

func.func @x_on_target(%c: !qec.qubit, %t: !qec.qubit) -> (!qec.qubit, !qec.qubit) {
    %c1, %t1 = qec.cnot %c, %t : !qec.qubit, !qec.qubit -> !qec.qubit, !qec.qubit
    %m       = qec.x  %t1     : !qec.qubit -> !qec.qubit
    %c2, %t2 = qec.cnot %c1, %m : !qec.qubit, !qec.qubit -> !qec.qubit, !qec.qubit
    return %c2, %t2 : !qec.qubit, !qec.qubit
}

func.func @y_on_target(%c: !qec.qubit, %t: !qec.qubit) -> (!qec.qubit, !qec.qubit) {
    %c1, %t1 = qec.cnot %c, %t : !qec.qubit, !qec.qubit -> !qec.qubit, !qec.qubit
    %m       = qec.y  %t1     : !qec.qubit -> !qec.qubit
    %c2, %t2 = qec.cnot %c1, %m : !qec.qubit, !qec.qubit -> !qec.qubit, !qec.qubit
    return %c2, %t2 : !qec.qubit, !qec.qubit
}

func.func @z_on_target(%c: !qec.qubit, %t: !qec.qubit) -> (!qec.qubit, !qec.qubit) {
    %c1, %t1 = qec.cnot %c, %t : !qec.qubit, !qec.qubit -> !qec.qubit, !qec.qubit
    %m       = qec.z  %t1     : !qec.qubit -> !qec.qubit
    %c2, %t2 = qec.cnot %c1, %m : !qec.qubit, !qec.qubit -> !qec.qubit, !qec.qubit
    return %c2, %t2 : !qec.qubit, !qec.qubit
}

// Negative: H is not a Pauli, must be left untouched.
func.func @h_middle(%c: !qec.qubit, %t: !qec.qubit) -> (!qec.qubit, !qec.qubit) {
    %c1, %t1 = qec.cnot %c, %t : !qec.qubit, !qec.qubit -> !qec.qubit, !qec.qubit
    %m       = qec.h  %c1     : !qec.qubit -> !qec.qubit
    %c2, %t2 = qec.cnot %m, %t1 : !qec.qubit, !qec.qubit -> !qec.qubit, !qec.qubit
    return %c2, %t2 : !qec.qubit, !qec.qubit
}

// Negative: outer CNOT uses swapped wires (control/target exchanged).
// CNOT·(X⊗I)·CNOT†-like, not the same op.
func.func @swapped_cnot(%c: !qec.qubit, %t: !qec.qubit) -> (!qec.qubit, !qec.qubit) {
    %c1, %t1 = qec.cnot %c, %t : !qec.qubit, !qec.qubit -> !qec.qubit, !qec.qubit
    %m       = qec.x  %c1     : !qec.qubit -> !qec.qubit
    %c2, %t2 = qec.cnot %t1, %m : !qec.qubit, !qec.qubit -> !qec.qubit, !qec.qubit
    return %c2, %t2 : !qec.qubit, !qec.qubit
}