# Generated by OpenQL 0.9.0 for program test_mapper_allNN
version 1.2

pragma @ql.name("test_mapper_allNN")


.kernel_allNN
    { # start at cycle 0
        x q[0]
        ym90 q[3]
    }
    cz q[0], q[3]
    x q[1]
    cz q[1], q[3]
    skip 1
    y90 q[3]
    { # start at cycle 6
        ym90 q[5]
        x q[3]
    }
    { # start at cycle 7
        ym90 q[2]
        cz q[3], q[5]
    }
    cz q[0], q[2]
    { # start at cycle 9
        ym90 q[4]
        ym90 q[6]
    }
    { # start at cycle 10
        cz q[1], q[4]
        y90 q[2]
        cz q[3], q[6]
    }
    { # start at cycle 11
        x q[2]
        ym90 q[0]
    }
    { # start at cycle 12
        y90 q[4]
        cz q[3], q[0]
    }
    { # start at cycle 13
        cz q[2], q[5]
        x q[4]
        ym90 q[1]
    }
    cz q[3], q[1]
    { # start at cycle 15
        cz q[4], q[6]
        y90 q[5]
    }
    { # start at cycle 16
        x q[5]
        ym90 q[3]
    }
    cz q[5], q[3]
    skip 1
    y90 q[6]
    { # start at cycle 20
        cz q[2], q[0]
        cz q[4], q[1]
        x q[6]
    }
    cz q[6], q[3]
    { # start at cycle 22
        ym90 q[2]
        ym90 q[4]
    }
    { # start at cycle 23
        cz q[5], q[2]
        cz q[6], q[4]
    }
    skip 1
    { # start at cycle 25
        y90 q[0]
        y90 q[1]
        y90 q[2]
        y90 q[3]
        y90 q[4]
    }
    { # start at cycle 26
        x q[0]
        x q[1]
        x q[2]
        x q[3]
        x q[4]
        x q[5]
        x q[6]
    }
