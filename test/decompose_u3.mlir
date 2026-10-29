// Run:  qec-opt --qec-decompose-u3 test/decompose_u3.mlir
//
//  @u3_h        → qec.h
//  @u3_x        → qec.x
//  @u3_z        → qec.z
//  @u3_t        → qec.t
//  @u3_identity → removed (return %arg0)
//  @u3_general  → unchanged (params don't match a standard gate)
//
//  Round-trip test:
//    fuse_1q of S·S produces U3(0,0,π)=Z. Decomposing it gives Z back.

func.func @u3_h(%q: !qec.qubit) -> !qec.qubit {
    %0 = qec.u3 %q[1.5707963267948966, 0.0, 3.1415926535897931]
        : !qec.qubit -> !qec.qubit
    return %0 : !qec.qubit
}

func.func @u3_x(%q: !qec.qubit) -> !qec.qubit {
    %0 = qec.u3 %q[3.1415926535897931, 0.0, 3.1415926535897931]
        : !qec.qubit -> !qec.qubit
    return %0 : !qec.qubit
}

func.func @u3_z(%q: !qec.qubit) -> !qec.qubit {
    %0 = qec.u3 %q[0.0, 0.0, 3.1415926535897931]
        : !qec.qubit -> !qec.qubit
    return %0 : !qec.qubit
}

func.func @u3_t(%q: !qec.qubit) -> !qec.qubit {
    %0 = qec.u3 %q[0.0, 0.0, 0.7853981633974483]
        : !qec.qubit -> !qec.qubit
    return %0 : !qec.qubit
}

func.func @u3_identity(%q: !qec.qubit) -> !qec.qubit {
    %0 = qec.u3 %q[0.0, 0.0, 0.0] : !qec.qubit -> !qec.qubit
    return %0 : !qec.qubit
}

func.func @u3_general(%q: !qec.qubit) -> !qec.qubit {
    %0 = qec.u3 %q[1.0, 2.0, 3.0] : !qec.qubit -> !qec.qubit
    return %0 : !qec.qubit
}