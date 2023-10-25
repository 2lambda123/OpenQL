import openql as ql
import os

from config import json_visualizer_dir, output_dir


ql.initialize()
ql.set_option('output_dir', output_dir)
ql.set_option('optimize', 'no')
ql.set_option('scheduler', 'ASAP')
# ql.set_option('log_level', 'LOG_DEBUG')
ql.set_option('log_level', 'LOG_INFO')
ql.set_option('unique_output', 'yes')
ql.set_option('write_qasm_files', 'no')
ql.set_option('write_report_files', 'no')

platformCustomGates = ql.Platform('starmon',  os.path.join(json_visualizer_dir, 'config_cc_light.json'))
num_qubits = 4
p = ql.Program("testProgram1", platformCustomGates, num_qubits, 0)
k = ql.Kernel("aKernel1", platformCustomGates, num_qubits, 0)
k.gate('x', [0])
for i in range(num_qubits):
    k.gate('prepz', [i])
k.gate('wait', [1], 40)
k.gate('wait', [2], 40)
k.gate('wait', [3], 40)
k.gate('x', [0])
k.gate('x', [0])
k.gate('x', [0])
k.gate('wait', [2], 40)
k.gate('h', [2])
k.gate('cz', [3, 1])
k.gate('cz', [2, 0])
k.gate('cz', [2, 0])
k.gate('wait', [3], 40)
k.gate('measure', [3])
k.gate('measure', [0])
k.gate('measure', [1])
k.gate('measure', [2])
# k.gate('measure', [3])
p.add_kernel(k)

# p.get_compiler().append_pass(
#     'ana.visualize.Interaction',
#     'visualize_interaction',
#     {
#         'output_prefix': output_dir + '/%N_interaction',
#         'config': os.path.join(cur_dir, "config_example_2.json"),
#         'interactive': 'yes'
#     }
# )

p.get_compiler().append_pass(
    'ana.visualize.Circuit',
    'visualize_circuit',
    {
        'output_prefix': output_dir + '/%N_circuit',
        'config': os.path.join(json_visualizer_dir, "config_example_2.json"),
        'waveform_mapping': os.path.join(json_visualizer_dir, "waveform_mapping.json"),
        'interactive': 'yes'
    }
)

p.compile()
