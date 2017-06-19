/**
 * @file   openql.i
 * @author Imran Ashraf
 * @brief  swig interface file
 */
%define DOCSTRING
"`OpenQL` is a C++ framework for high-level quantum programming. The framework provide a compiler for compiling and optimizing quantum code. The compiler produce the intermediate quantum assembly language and the compiled micro-code for various target platforms. While the microcode is platform-specific, the quantum assembly code (qasm) is hardware-agnostic and can be simulated on the QX simulator."
%enddef

%module(docstring=DOCSTRING) openql

%include "std_string.i"
%include "std_vector.i"

namespace std {
   %template(vectori) vector<int>;
   %template(vectorui) vector<size_t>;
   %template(vectorf) vector<float>;
   %template(vectord) vector<double>;
};

%{
#include "openql.h"
%}




%feature("docstring") set_instruction_map_file
""" Sets instruction map file.

Parameters
----------
arg1 : str
    Releative path of instruction map file, default is instructions.map in current directory.
"""

%feature("docstring") get_instruction_map_file
""" Returns the path of current set instruction map file.

Parameters
----------
None

Returns
-------
str
    Path of instruction map file. """


%feature("docstring") init
""" Initializes OpenQL environment.

Parameters
----------
None 
"""


%feature("docstring") Kernel
""" Kernel class which contains various quantum instructions."""


%feature("docstring") Kernel::Kernel
""" Constructs a kernel object.

Parameters
----------
arg1 : str
    name of the kernel
"""


%feature("docstring") Kernel::identity
""" Applies identity on the qubit specified in argument.

Parameters
----------
arg1 : int
    target qubit
"""


%feature("docstring") Kernel::hadamard
""" Applies hadamard on the qubit specified in argument.

Parameters
----------
arg1 : int
    target qubit
"""

%feature("docstring") Kernel::s
""" Applies s on the qubit specified in argument.

Parameters
----------
arg1 : int
    target qubit
"""

%feature("docstring") Kernel::sdag
""" Applies sdag on the qubit specified in argument.

Parameters
----------
arg1 : int
    target qubit
"""

%feature("docstring") Kernel::s
""" Applies x on the qubit specified in argument.

Parameters
----------
arg1 : int
    target qubit
"""

%feature("docstring") Kernel::y
""" Applies y on the qubit specified in argument.

Parameters
----------
arg1 : int
    target qubit
"""

%feature("docstring") Kernel::z
""" Applies z on the qubit specified in argument.

Parameters
----------
arg1 : int
    target qubit
"""

%feature("docstring") Kernel::rx90
""" Applies rx90 on the qubit specified in argument.

Parameters
----------
arg1 : int
    target qubit
"""


%feature("docstring") Kernel::mrx90
""" Applies mrx90 on the qubit specified in argument.

Parameters
----------
arg1 : int
    target qubit
"""

%feature("docstring") Kernel::rx180
""" Applies rx180 on the qubit specified in argument.

Parameters
----------
arg1 : int
    target qubit
"""

%feature("docstring") Kernel::ry180
""" Applies ry180 on the qubit specified in argument.

Parameters
----------
arg1 : int
    target qubit
"""

%feature("docstring") Kernel::measure
""" measures input qubit.

Parameters
----------
arg1 : int
    input qubit
"""


%feature("docstring") Kernel::cnot
""" Applies controlled-not operation.

Parameters
----------
arg1 : int
    control qubit
arg2 : int
    target qubit
"""

%feature("docstring") Kernel::cphase
""" Applies controlled-phase operation.

Parameters
----------
arg1 : int
    control qubit
arg2 : int
    target qubit
"""


%feature("docstring") Kernel::toffoli
""" Applies controlled-controlled-not operation.

Parameters
----------
arg1 : int
    control qubit
arg2 : int
    control qubit
arg3 : int
    target qubit
"""


%feature("docstring") Kernel::clifford
""" Applies clifford operation of the specified id on the qubit.

Parameters
----------
arg1 : int
    clifford operation id
arg2 : int
    target qubit

The ids and the corresponding operations are:
0 : ['I']
1 : ['Y90', 'X90']
2 : ['mX90', 'mY90']
3 : ['X180']
4 : ['mY90', 'mX90']
5 : ['X90', 'mY90']
6 : ['Y180']
7 : ['mY90', 'X90']
8 : ['X90', 'Y90']
9 : ['X180', 'Y180']
10: ['Y90', 'mX90']
11: ['mX90', 'Y90']
12: ['Y90', 'X180']
13: ['mX90']
14: ['X90', 'mY90', 'mX90']
15: ['mY90']
16: ['X90']
17: ['X90', 'Y90', 'X90']
18: ['mY90', 'X180']
19: ['X90', 'Y180']
20: ['X90', 'mY90', 'X90']
21: ['Y90']
22: ['mX90', 'Y180']
23: ['X90', 'Y90', 'mX90']
"""

%feature("docstring") Kernel::load_custom_instructions
""" Loads the JSON file describing custom instructions.

Parameters
----------
arg1 : str
    Path to JSON file, default instructions.json in the current directory.
"""


%feature("docstring") Kernel::print_custom_instructions
""" Prints the available custom instructions.

Parameters
----------
None
"""

%feature("docstring") Kernel::toffoli
""" Applies custom gate on specified qubits.

Parameters
----------
arg1 : str
    custom gate name
arg2 : []
    list of qubits involved in custom gate.
"""


%feature("docstring") Program
""" Program class which contains one or more Kernels."""


%feature("docstring") Program::Program
""" Constructs a Program object.

Parameters
----------
arg1 : str
    name of the Program
arg1 : int
	number of qubits the program will use
"""

%feature("docstring") Program::set_sweep_points
""" Sets sweep points for an experiment.

Parameters
----------
arg1 : []
    list of sweep points
arg1 : int
	number of circuits
"""

%feature("docstring") Program::add_kernel
""" Adds specified Kernel to Program.

Parameters
----------
arg1 : Kernel
    Kernel to be added
"""

%feature("docstring") Program::compile
""" Compiles the Program.

Parameters
----------
arg1 : bool
    optimize, default is False
arg2 : bool
    verbose, default is False
"""

%feature("docstring") Program::schedule
""" Schedules the Program.

Parameters
----------
arg1 : string
    scheduler which can be ASAP or ALAP, default is ASAP
arg2 : bool
    verbose, default is False
"""


%feature("docstring") Program::qasm
""" Returns Program QASM
Parameters
----------
None

Returns
-------
str
    qasm """

%feature("docstring") Program::microcode
""" Returns Program microcode
Parameters
----------
None

Returns
-------
str
    microcode """




// Include the header file with above prototypes
%include "openql.h"