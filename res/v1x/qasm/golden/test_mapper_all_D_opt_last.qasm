# Generated by OpenQL 0.11.1 for program test_mapper_all_D_opt
version 1.2

pragma @ql.name("test_mapper_all_D_opt")


.kernel_all_D_opt
    { # start at cycle 0
        x q[0]
        x q[3]
    }
    cnot q[0], q[3]
    skip 3
    cnot q[3], q[0]
    skip 3
    { # start at cycle 9
        x q[1]
        x q[6]
        x q[4]
        x q[5]
        x q[2]
    }
    { # start at cycle 10
        cnot q[3], q[1]
        cnot q[6], q[4]
        cnot q[5], q[2]
    }
    skip 3
    { # start at cycle 14
        cnot q[1], q[3]
        cnot q[4], q[6]
        cnot q[2], q[5]
    }
    skip 3
    cnot q[3], q[5]
    skip 3
    cnot q[5], q[3]
    skip 3
    { # start at cycle 26
        cnot q[1], q[4]
        cnot q[6], q[3]
    }
    skip 3
    { # start at cycle 30
        cnot q[4], q[1]
        cnot q[3], q[6]
    }
    skip 3
    swap q[3], q[1]
    skip 9
    cnot q[2], q[0]
    skip 3
    cnot q[0], q[2]
    skip 3
    cnot q[0], q[3]
    skip 3
    cnot q[3], q[0]
    skip 3
    cnot q[3], q[6]
    skip 3
    cnot q[6], q[3]
    skip 3
    swap q[3], q[5]
    skip 9
    cnot q[6], q[3]
    skip 3
    cnot q[3], q[6]
    skip 3
    cnot q[3], q[0]
    skip 3
    cnot q[0], q[3]
    skip 3
    cnot q[5], q[3]
    skip 3
    cnot q[3], q[5]
    skip 3
    swap q[3], q[6]
    skip 9
    swap q[5], q[2]
    skip 1
    cnot q[0], q[3]
    skip 3
    cnot q[3], q[0]
    skip 3
    swap q[3], q[5]
    skip 1
    cnot q[1], q[4]
    skip 3
    cnot q[4], q[1]
    skip 3
    cnot q[1], q[3]
    skip 3
    cnot q[3], q[1]
    skip 3
    x q[1]
    swap q[1], q[4]
    skip 1
    cnot q[5], q[3]
    skip 3
    cnot q[3], q[5]
    skip 3
    swap q[3], q[0]
    skip 9
    cnot q[3], q[1]
    skip 3
    cnot q[0], q[2]
    skip 3
    cnot q[1], q[3]
    skip 3
    cnot q[2], q[0]
    skip 3
    x q[3]
    { # start at cycle 178
        swap q[4], q[6]
        swap q[3], q[0]
    }
    skip 9
    cnot q[4], q[1]
    skip 3
    cnot q[1], q[4]
    skip 3
    cnot q[3], q[1]
    skip 3
    cnot q[1], q[3]
    skip 3
    { # start at cycle 204
        x q[1]
        x q[2]
        x q[3]
        x q[4]
        x q[5]
    }
