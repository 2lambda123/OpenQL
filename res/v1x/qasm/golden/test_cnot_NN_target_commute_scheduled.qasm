# Generated by OpenQL 0.9.0 for program test_cnot_NN_target_commute
version 1.2

pragma @ql.name("test_cnot_NN_target_commute")


.aKernel
    cnot q[0], q[3]
    skip 3
    cnot q[6], q[3]
    skip 3
    { # start at cycle 8
        t q[6]
        cnot q[1], q[3]
    }
    skip 2
    z q[6]
    t q[1]
    skip 2
    z q[1]
    skip 1
    t q[1]
    skip 2
    z q[1]
    skip 1
