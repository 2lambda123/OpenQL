# Generated by OpenQL 0.9.0 for program test_cz_2R_any_commute
version 1.2

pragma @ql.name("test_cz_2R_any_commute")


.aKernel
    { # start at cycle 0
        cz q[0], q[3]
        cz q[3], q[6]
        t q[6]
        z q[6]
        cz q[1], q[3]
        t q[1]
        z q[1]
        t q[1]
        z q[1]
    }
    skip 2