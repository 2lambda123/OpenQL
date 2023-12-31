import openql as ql
import os

from config import json_visualizer_dir, output_dir


# ql.set_option('log_level', 'LOG_DEBUG')
# ql.set_option('log_level', 'LOG_INFO')
ql.set_option('unique_output', 'yes')

c = ql.Compiler("testCompiler")
# c.append_pass("sch.Schedule", "scheduler", {"scheduler_heuristic": "asap"})
c.append_pass("ana.visualize.Interaction", "visualizer",
              {"config": os.path.join(json_visualizer_dir, "config_example_1.json"), "interactive": "yes"})
c.set_option("**.output_prefix", output_dir + "/%N")

platformCustomGates = ql.Platform('starmon',  os.path.join(json_visualizer_dir, 'config_cc_light_2.json'))
num_qubits = 2
p = ql.Program("Deutsch", platformCustomGates, num_qubits, 0)
k = ql.Kernel("kernel", platformCustomGates, num_qubits, 0)
for q in range(num_qubits):
    k.gate('prepz', [q])
k.gate('x', [1])
for q in range(num_qubits):
    k.gate('h', [q])
# cr = random()
cr = 0.23
if cr < 0.25:
    print("Balanced : IDENTITY")
    k.gate('cnot', [0, 1])
elif cr < 0.50:
    print("Balanced : NOT")
    k.gate('x', [0])
    k.gate('cnot', [0, 1])
    k.gate('x', [0])
elif cr < 0.75:
    print("Unbalanced : RESET")
else:
    print("Unbalanced : SET")
    k.gate('x', [1])
k.gate('h', [0])
for q in range(num_qubits):
    k.gate('measure', [q])
p.add_kernel(k)
c.compile(p)
