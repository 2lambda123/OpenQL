# Generated by OpenQL 0.11.1 for program test_mapper_max_cut
version 1.2

pragma @ql.name("test_mapper_max_cut")


.kernel_max_cut
    { # start at cycle 0
        x q[7]
        x q[6]
    }
    cz q[6], q[7]
    skip 1
    { # start at cycle 3
        x q[4]
        x q[3]
    }
    { # start at cycle 4
        cz q[4], q[7]
        x q[5]
        swap q[3], q[0]
    }
    { # start at cycle 5
        cz q[5], q[6]
        x q[1]
    }
    cz q[1], q[0]
    swap q[4], q[2]
    x q[6]
    { # start at cycle 9
        cz q[1], q[2]
        swap q[6], q[7]
    }
    skip 1
    swap q[6], q[5]
    swap q[1], q[2]
    swap q[6], q[7]
    cz q[0], q[1]
    { # start at cycle 15
        swap q[5], q[3]
        swap q[7], q[4]
    }
    skip 1
    { # start at cycle 17
        cz q[0], q[3]
        cz q[2], q[4]
    }
    skip 2
    { # start at cycle 20
        x q[0]
        x q[1]
        x q[2]
        x q[3]
        x q[4]
    }
