# Generated by OpenQL 0.9.0 for program test_cnot_2R_non_commute_RAD
version 1.2

pragma @ql.name("test_cnot_2R_non_commute_RAD")


.aKernel
    cnot q[0], q[3]
    skip 3
    cnot q[3], q[5]
    skip 3
    { # start at cycle 8
        t q[5]
        z q[5]
    }
    skip 2