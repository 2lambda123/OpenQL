import openql as ql
import os
import unittest

from config import cq_golden_dir, output_dir
from utils import file_compare


class TestInstructionDecomposition(unittest.TestCase):
    @classmethod
    def setUp(cls):
        ql.initialize()
        ql.set_option('output_dir', output_dir)
        ql.set_option('log_level', 'LOG_WARNING')

    @staticmethod
    def get_test_program(name):
        platform = ql.Platform.from_json('test_platform', {
            "hardware_settings": {
                "qubit_number": 3
            },
            "instructions": {
                "y90": {
                    "prototype": ["U:qubit"],
                    "duration_cycles": 1
                },
                "ym90": {
                    "prototype": ["U:qubit"],
                    "duration_cycles": 1
                },
                "cz": {
                    "prototype": ["U:qubit", "U:qubit"],
                    "duration_cycles": 2
                },
                "cnot": {
                    "prototype": ["U:qubit", "U:qubit"],
                    "duration_cycles": 4,
                    "decomposition": {
                        "name": "to_cz",
                        "into": "ym90 op(1); cz op(0), op(1); skip 1; y90 op(1)"
                    }
                }
            }
        })
        p = ql.Program(name, platform)
        k = ql.Kernel('test', platform)
        k.cnot(0, 1)
        k.cnot(1, 2)
        p.add_kernel(k)
        return p

    def test_decompose_before_schedule(self):
        name = 'decompose_before_schedule'
        p = self.get_test_program(name)
        c = p.get_compiler()
        c.clear_passes()
        c.append_pass('io.cqasm.Report', '', {'output_prefix': output_dir + '/%N_in'})
        c.append_pass('dec.Instructions', '', {})
        c.append_pass('io.cqasm.Report', '', {'output_prefix': output_dir + '/%N_dec'})
        c.append_pass('sch.ListSchedule', '', {})
        c.append_pass('io.cqasm.Report', '', {'output_prefix': output_dir + '/%N_sch'})
        p.compile()
        for suffix in ['in', 'dec', 'sch']:
            self.assertTrue(file_compare(
                os.path.join(output_dir, name + '_' + suffix + '.cq'),
                os.path.join(cq_golden_dir, name + '_' + suffix + '.cq')
            ))

    def test_decompose_after_schedule(self):
        name = 'decompose_after_schedule'
        p = self.get_test_program(name)
        c = p.get_compiler()
        c.clear_passes()
        c.append_pass('io.cqasm.Report', '', {'output_prefix': output_dir + '/%N_in'})
        c.append_pass('sch.ListSchedule', '', {})
        c.append_pass('io.cqasm.Report', '', {'output_prefix': output_dir + '/%N_sch'})
        c.append_pass('dec.Instructions', '', {'ignore_schedule': 'no'})
        c.append_pass('io.cqasm.Report', '', {'output_prefix': output_dir + '/%N_dec'})
        p.compile()
        for suffix in ['in', 'sch', 'dec']:
            self.assertTrue(file_compare(
                os.path.join(output_dir, name + '_' + suffix + '.cq'),
                os.path.join(cq_golden_dir, name + '_' + suffix + '.cq')
            ))


if __name__ == '__main__':
    unittest.main()
