#pragma once

#include "ql/ir/ir.h"

#include "types.h"
#include "operands.h"
#include "datapath.h"
#include "q1helpers.h"

namespace ql {
namespace arch {
namespace cc {
namespace pass {
namespace gen {
namespace vq1asm {
namespace detail {

class Codegen;  // FIXME, to prevent recursive include loop

class Functions {
public:
    explicit Functions(OperandContext &operandContext, Datapath &dp, Codegen &cg);
    ~Functions() = default;

    /*
     * FIXME: add comment
     */
    void dispatch(const ir::ExpressionRef &lhs, const ir::FunctionCall *fn, const Str &describe);

    /*
     * Cast bregs to bits in REG_TMP0, i.e. transfer them from DSM to processor.
     * Returns a mask of bits set in REG_TMP0. FIXME: provides no knowledge about relation with DSM bits
     *
     * FIXME: We don't have a matching quantum instruction for this cast (formerly, we had 'if_1_break' etc), but do
     *  take up 'quantum' time, so the timeline is silently shifted
     */
    UInt emit_bin_cast(utils::Vec<utils::UInt> bregs, Int expOpCnt);


private:  // types
    struct  OpArgs {
        Operands ops;
        UInt dest_reg;
        Str label_if_false;
        Str operation;  // from function table
        Str describe; // to generate comments
    };

    typedef void (Functions::*tOpFunc)(const OpArgs &a);

    struct FuncInfo {
        tOpFunc func;
        Str operation;
    };


private:    // vars
    // Object instances needed
    OperandContext &operandContext;                             // context for Operand processing
    Datapath &dp;                                               // handling of CC datapath
    Codegen &cg;                                                // code generation, used for helpers

    // map name to function, see register_functions()
    std::map<Str, FuncInfo> func_map;

private:    // methods
    /*
     * operator functions for code generation in expressions
     *
     * Naming conventions:
     * - functions handling a single operator have naming inspired by libqasm's func_gen::Function::unique_name
     * - functions handling a group of operators are named "op_grp_<groupName>"
     * - the suffix defines the argument profile(s), using the naming from get_operand_type()
     */

    // bitwise inversion
    void op_binv_C(const OpArgs &a);

    // logical inversion
    void op_linv_B(const OpArgs &a);

    // int arithmetic, 2 operands: "+", "-", "&", "|", "^"
    void op_grp_int_2op_CC(const OpArgs &a);
    void op_grp_int_2op_Ci_iC(const OpArgs &a);
    void op_sub_iC(const OpArgs &a);    // special case

    // bit arithmetic, 2 operands: "&&", "||", "^^"
    void op_grp_bit_2op_BB(const OpArgs &a);

    // relop, group 1: "==", "!="
    void op_grp_rel1_tail(const OpArgs &a); // common tail for functions below
    void op_grp_rel1_CC(const OpArgs &a);
    void op_grp_rel1_Ci_iC(const OpArgs &a);

    // relop, group 2: ">=", "<"
    void op_grp_rel2_CC(const OpArgs &a);
    void op_grp_rel2_Ci_iC(const OpArgs &a);

    // relop, group 3: ">", "<="
    void op_gt_CC(const OpArgs &a);
    void op_gt_Ci(const OpArgs &a);
    void op_gt_iC(const OpArgs &a);

    /*
     * other functions for code generation in expressions
     */
#if OPT_CC_USER_FUNCTIONS
    void rnd_seed_C(const OpArgs &a);
    void rnd_seed_i(const OpArgs &a);
    void rnd(const OpArgs &a);
#endif

    /*
     * Register the functions supported by the CC backend
     */
    void register_functions();

    /*
     * Get type of single function operand. Encoding:
     * - 'b': bit literal
     * - 'i': int literal
     * - 'B': breg reference
     * - 'C': creg reference
     *
     * Inspired by func_gen::Function::generate_impl_footer and cqasm::types::from_spec, but notice that we add 'C' and
     * have sightly different purpose and interpretation
     */
    Str get_operand_type(const ir::ExpressionRef &op);

    // FIXME: rename to emit_operation_..
    void emit_mnem2args(const OpArgs &a, const Str &arg0, const Str &arg1, const Str &target=REG_TMP0);

    Int creg2reg(const ir::Reference &ref);
};

} // namespace detail
} // namespace vq1asm
} // namespace gen
} // namespace pass
} // namespace cc
} // namespace arch
} // namespace ql
