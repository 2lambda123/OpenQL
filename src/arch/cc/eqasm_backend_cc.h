/**
 * @file   eqasm_backend_cc.h
 * @date   201809xx
 * @author Wouter Vlothuizen (wouter.vlothuizen@tno.nl)
 * @brief  eqasm backend for the Central Controller
 * @remark based on cc_light_eqasm_compiler.h, commit f34c0d9
 */

#ifndef QL_ARCH_CC_EQASM_BACKEND_CC_H
#define QL_ARCH_CC_EQASM_BACKEND_CC_H

#include "codegen_cc.h"

#include <eqasm_compiler.h>
#include <kernel.h>
#include <platform.h>
#include <circuit.h>

#include <string>
#include <vector>

namespace ql
{
namespace arch
{

class eqasm_backend_cc : public eqasm_compiler
{
public:
    eqasm_backend_cc();
    ~eqasm_backend_cc();

    void compile(std::string prog_name, std::vector<quantum_kernel> kernels, const ql::quantum_platform &platform);
    void compile(std::string prog_name, ql::circuit& ckt, ql::quantum_platform& platform);

private:
    std::string kernelLabel(ql::quantum_kernel &k);
    void codegen_classical_instruction(ql::gate *classical_ins);
    void codegen_kernel_prologue(ql::quantum_kernel &k);
    void codegen_kernel_epilogue(ql::quantum_kernel &k);
    void codegen_bundles(ql::ir::bundles_t &bundles, const ql::quantum_platform &platform);
    void load_hw_settings(const ql::quantum_platform& platform);

private: // vars
    codegen_cc codegen;
    int bundleIdx;

    // parameters from JSON file:
#if 0   // FIXME: unused
    size_t buffer_matrix[__operation_types_num__][__operation_types_num__];
#endif
}; // class

} // arch
} // ql

#endif // QL_ARCH_CC_EQASM_BACKEND_CC_H

