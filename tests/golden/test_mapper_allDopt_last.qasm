# Generated by OpenQL 0.9.0 for program test_mapper_allDopt
version 1.2

pragma @ql.name("test_mapper_allDopt")


.kernel_allDopt
    { # start at cycle 0
        x q[0]
        ym90 q[3]
    }
    cz q[0], q[3]
    skip 1
    y90 q[3]
    { # start at cycle 4
        x q[3]
        ym90 q[1]
    }
    cz q[3], q[1]
    ym90 q[0]
    cz q[3], q[0]
    y90 q[1]
    { # start at cycle 9
        ym90 q[4]
        x q[5]
        ym90 q[2]
        x q[1]
        ym90 q[3]
    }
    { # start at cycle 10
        x q[6]
        cz q[5], q[2]
        cz q[1], q[3]
    }
    cz q[6], q[4]
    { # start at cycle 12
        ym90 q[5]
        y90 q[3]
    }
    { # start at cycle 13
        y90 q[4]
        y90 q[2]
        cz q[3], q[5]
    }
    x q[2]
    { # start at cycle 15
        x q[4]
        ym90 q[6]
        cz q[2], q[5]
    }
    cz q[4], q[6]
    { # start at cycle 17
        y90 q[5]
        ym90 q[3]
    }
    { # start at cycle 18
        cz q[2], q[0]
        cz q[5], q[3]
    }
    skip 1
    { # start at cycle 20
        ym90 q[4]
        y90 q[6]
        y90 q[0]
        ym90 q[2]
    }
    { # start at cycle 21
        cz q[1], q[4]
        cz q[6], q[3]
        cz q[0], q[2]
    }
    skip 1
    { # start at cycle 23
        y90 q[3]
        ym90 q[0]
    }
    cz q[3], q[0]
    { # start at cycle 25
        y90 q[4]
        ym90 q[1]
        ym90 q[6]
    }
    { # start at cycle 26
        cz q[4], q[1]
        cz q[3], q[6]
    }
    skip 1
    { # start at cycle 28
        y90 q[0]
        ym90 q[3]
    }
    cz q[0], q[3]
    skip 1
    y90 q[3]
    cz q[3], q[1]
    ym90 q[0]
    cz q[3], q[0]
    skip 1
    { # start at cycle 36
        y90 q[1]
        ym90 q[3]
    }
    cz q[1], q[3]
    skip 1
    { # start at cycle 39
        y90 q[3]
        ym90 q[5]
    }
    cz q[3], q[5]
    skip 1
    { # start at cycle 42
        y90 q[6]
        ym90 q[4]
    }
    cz q[6], q[4]
    { # start at cycle 44
        y90 q[5]
        ym90 q[3]
    }
    cz q[5], q[3]
    skip 1
    { # start at cycle 47
        y90 q[4]
        ym90 q[6]
    }
    cz q[4], q[6]
    { # start at cycle 49
        y90 q[3]
        ym90 q[5]
    }
    cz q[3], q[5]
    skip 1
    { # start at cycle 52
        y90 q[6]
        ym90 q[4]
    }
    cz q[6], q[4]
    ym90 q[3]
    { # start at cycle 55
        cz q[1], q[4]
        cz q[6], q[3]
    }
    skip 1
    { # start at cycle 57
        y90 q[3]
        ym90 q[6]
    }
    cz q[3], q[6]
    y90 q[4]
    { # start at cycle 60
        ym90 q[1]
        y90 q[6]
        ym90 q[3]
    }
    { # start at cycle 61
        cz q[4], q[1]
        cz q[6], q[3]
    }
    y90 q[0]
    { # start at cycle 63
        ym90 q[6]
        cz q[0], q[3]
    }
    cz q[4], q[6]
    { # start at cycle 65
        y90 q[3]
        ym90 q[0]
    }
    { # start at cycle 66
        y90 q[6]
        ym90 q[4]
        cz q[3], q[0]
    }
    cz q[6], q[4]
    ym90 q[3]
    cz q[6], q[3]
    skip 1
    { # start at cycle 71
        y90 q[3]
        ym90 q[6]
    }
    cz q[3], q[6]
    skip 1
    { # start at cycle 74
        y90 q[6]
        ym90 q[3]
    }
    cz q[6], q[3]
    skip 1
    y90 q[3]
    cz q[3], q[5]
    skip 1
    { # start at cycle 80
        y90 q[5]
        ym90 q[3]
    }
    cz q[5], q[3]
    y90 q[1]
    cz q[1], q[3]
    skip 1
    { # start at cycle 85
        y90 q[3]
        ym90 q[1]
    }
    cz q[3], q[1]
    skip 1
    { # start at cycle 88
        ym90 q[5]
        y90 q[1]
    }
    { # start at cycle 89
        cz q[3], q[5]
        cz q[1], q[4]
    }
    skip 1
    ym90 q[3]
    { # start at cycle 92
        y90 q[5]
        y90 q[4]
        ym90 q[1]
    }
    { # start at cycle 93
        cz q[5], q[3]
        cz q[4], q[1]
    }
    skip 1
    y90 q[3]
    { # start at cycle 96
        y90 q[1]
        ym90 q[4]
    }
    { # start at cycle 97
        y90 q[0]
        ym90 q[5]
        cz q[1], q[4]
    }
    { # start at cycle 98
        cz q[0], q[2]
        cz q[3], q[5]
    }
    ym90 q[1]
    cz q[3], q[1]
    skip 1
    { # start at cycle 102
        y90 q[1]
        ym90 q[3]
    }
    cz q[1], q[3]
    skip 1
    { # start at cycle 105
        y90 q[2]
        ym90 q[0]
        y90 q[3]
        ym90 q[6]
    }
    { # start at cycle 106
        cz q[2], q[0]
        cz q[3], q[6]
    }
    skip 1
    { # start at cycle 108
        y90 q[5]
        ym90 q[2]
    }
    cz q[5], q[2]
    { # start at cycle 110
        y90 q[6]
        ym90 q[3]
    }
    cz q[6], q[3]
    skip 1
    { # start at cycle 113
        y90 q[3]
        y90 q[2]
    }
    { # start at cycle 114
        x q[3]
        ym90 q[1]
        ym90 q[5]
    }
    { # start at cycle 115
        cz q[3], q[1]
        cz q[2], q[5]
    }
    skip 1
    { # start at cycle 117
        y90 q[1]
        ym90 q[3]
        y90 q[5]
        ym90 q[2]
    }
    { # start at cycle 118
        cz q[1], q[3]
        cz q[5], q[2]
    }
    skip 1
    { # start at cycle 120
        y90 q[3]
        ym90 q[5]
    }
    cz q[3], q[5]
    ym90 q[1]
    cz q[3], q[1]
    skip 1
    { # start at cycle 125
        y90 q[5]
        ym90 q[3]
        y90 q[1]
    }
    { # start at cycle 126
        cz q[5], q[3]
        cz q[1], q[4]
    }
    skip 1
    { # start at cycle 128
        y90 q[3]
        y90 q[4]
    }
    { # start at cycle 129
        x q[3]
        ym90 q[5]
        ym90 q[1]
    }
    { # start at cycle 130
        cz q[3], q[5]
        cz q[4], q[1]
    }
    skip 1
    { # start at cycle 132
        y90 q[5]
        ym90 q[3]
        y90 q[1]
        ym90 q[4]
    }
    { # start at cycle 133
        cz q[5], q[3]
        cz q[1], q[4]
    }
    skip 1
    { # start at cycle 135
        y90 q[3]
        ym90 q[1]
    }
    cz q[3], q[1]
    ym90 q[5]
    cz q[3], q[5]
    skip 1
    { # start at cycle 140
        y90 q[1]
        ym90 q[3]
    }
    cz q[1], q[3]
    x q[0]
    { # start at cycle 143
        y90 q[2]
        y q[0]
        y90 q[3]
        ym90 q[6]
    }
    { # start at cycle 144
        cz q[2], q[0]
        cz q[3], q[6]
    }
    skip 1
    { # start at cycle 146
        y90 q[0]
        ym90 q[2]
        y90 q[6]
        ym90 q[3]
    }
    { # start at cycle 147
        cz q[0], q[2]
        cz q[6], q[3]
    }
    skip 1
    { # start at cycle 149
        y90 q[2]
        ym90 q[0]
        y90 q[3]
        ym90 q[6]
    }
    { # start at cycle 150
        cz q[2], q[0]
        cz q[3], q[6]
    }
    skip 1
    { # start at cycle 152
        y90 q[0]
        ym90 q[3]
    }
    cz q[0], q[3]
    skip 1
    { # start at cycle 155
        y90 q[3]
        ym90 q[0]
    }
    cz q[3], q[0]
    skip 1
    { # start at cycle 158
        y90 q[6]
        ym90 q[3]
    }
    cz q[6], q[3]
    skip 1
    { # start at cycle 161
        y90 q[3]
        ym90 q[6]
    }
    cz q[3], q[6]
    skip 1
    { # start at cycle 164
        y90 q[0]
        y90 q[4]
        y90 q[5]
        y90 q[6]
    }
    { # start at cycle 165
        x q[0]
        x q[1]
        x q[3]
        x q[6]
    }
