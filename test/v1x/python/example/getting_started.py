import openql as ql
import os

from config import json_dir, output_dir


ql.set_option('output_dir', output_dir)
ql.set_option('optimize', 'no')
ql.set_option('scheduler', 'ASAP')
ql.set_option('log_level', 'LOG_WARNING')
ql.set_option('write_qasm_files', 'yes')


def hello_openql():
    # if you copy this example somewhere else, make sure to provide
    # correct path of configuration file copy the configuration file
    # to the same directory and update the path
    config_fn = os.path.join(json_dir, 'config_cc_light.json')
    platform = ql.Platform("myPlatform", config_fn)

    num_qubits = 3

    # create a program
    p = ql.Program("aProgram", platform, num_qubits)

    # create a kernel
    k = ql.Kernel("aKernel", platform, num_qubits)

    # populate kernel using default and custom gates
    for i in range(num_qubits):
        k.gate('prepz', [i])

    k.gate('x', [0])
    k.gate('h', [1])
    k.gate('cz', [2, 0])
    k.gate('measure', [0])
    k.gate('measure', [1])

    # add the kernel to the program
    p.add_kernel(k)

    # compile the program
    p.compile()


if __name__ == '__main__':
    hello_openql()
