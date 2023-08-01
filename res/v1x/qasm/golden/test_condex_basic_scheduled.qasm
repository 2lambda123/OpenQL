# Generated by OpenQL 0.9.0 for program test_condex_basic
version 1.2

pragma @ql.name("test_condex_basic")


.kernel_basic
    measure q[1], b[1]
    skip 14
    cond (b[1]) x q[1]
    { # start at cycle 16
        measure q[0], b[0]
        measure q[3], b[1]
    }
    skip 14
    { # start at cycle 31
        cond (b[0]) x q[0]
        x q[1]
        cond (b[1]) x q[3]
    }
    { # start at cycle 32
        measure q[0], b[0]
        measure q[1], b[1]
    }
    skip 3
    measure q[5], b[5]
    { # start at cycle 37
        measure q[2], b[2]
        measure q[4], b[4]
    }
    skip 9
    cond (b[0] && b[1]) x q[0]
    cond (!(b[0] && b[1])) x q[0]
    cond (b[0] || b[1]) x q[0]
    cond (!(b[0] || b[1])) x q[0]
    { # start at cycle 51
        x q[5]
        cond (b[0] != b[1]) x q[0]
    }
    { # start at cycle 52
        cond (b[2]) x q[2]
        cond (!b[4]) x q[4]
        cond (false) x q[5]
        cond (b[0] == b[1]) x q[0]
    }