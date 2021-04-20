/** \file
 * Platform header for target-specific compilation.
 */

#pragma once

#include "ql/utils/num.h"
#include "ql/utils/str.h"
#include "ql/utils/opt.h"
#include "ql/utils/json.h"
#include "ql/utils/tree.h"
#include "ql/plat/hardware_configuration.h"
#include "ql/plat/topology.h"
#include "ql/arch/architecture.h"

namespace ql {
namespace plat {

/**
 * Platform configuration structure. Represents everything we know about the
 * target qubit chip, simulator, control architecture, etc.
 *
 * TODO: this still needs a lot of work. As much JSON parsing as possible should
 *  be done while loading, and we need different classes for gate instances and
 *  types.
 */
class Platform : public utils::Node {
public:

    /**
     * User-specified name for the platform.
     */
    utils::Str name;

    /**
     * The total number of physical qubits supported by the platform.
     */
    utils::UInt qubit_count;

    /**
     * The total number of 32-bit general-purpose classical registers supported
     * by the platform.
     */
    utils::UInt creg_count;

    /**
     * Historically, creg count was not specified in the platform description
     * file, and was instead implicitly taken from the amount allocated for the
     * program constructed from it. Setting this models this old behavior to
     * some extent: creg_count will then be increased whenever a program is
     * created with more than creg_count creg declarations.
     */
    utils::Bool compat_implicit_creg_count;

    /**
     * The total number of single-bit condition/measurement result registers
     * supported by the platform.
     */
    utils::UInt breg_count;

    /**
     * Same as compat_implicit_creg_count, but for bregs.
     */
    utils::Bool compat_implicit_breg_count;

    /**
     * Cycle time in nanoseconds.
     *
     * FIXME: why is this a UInt? Non-integer-nanosecond cycle times are not
     *  supported...? At least use picoseconds or femtoseconds as a unit if it
     *  needs to be fixed-point, 64-bit is plenty for that.
     */
    utils::UInt cycle_time;

    /**
     * Path to the JSON file that was used to configure this platform.
     *
     * FIXME: it's wrong that things are using this. Once constructed, the
     *  filename should be don't care (in theory, there doesn't even need to be
     *  a file).
     */
    utils::Str configuration_file_name;

    /**
     * The gate/instruction set supported by this platform.
     */
    InstructionMap instruction_map;

    /**
     * Settings for the compiler. This can be:
     *  - an empty string, if no eqasm_compiler key is specified;
     *  - a recognized string (none, qx, cc_light_compiler, or
     *    eqasm_backend_cc);
     *  - a JSON object representing the compiler configuration structure,
     *    which may or may not have a strategy.architecture key set to cc or
     *    cc_light.
     *
     * NOTE: while it's nasty that this is here as a raw JSON object, we can't
     *  construct it into a pass manager until program.compile, because
     *  construction may use global options in compatibility mode...
     */
    utils::Json compiler_settings;

    /**
     * Raw instruction setting data for use by the eqasm backend, corresponding
     * to the `"instructions"` key in the root JSON object.
     *
     * FIXME: this shouldn't be here. Extra data should be part of the gate
     *  types (but there are no gate types yet, of course).
     */
    utils::Json instruction_settings;

    /**
     * Additional hardware settings (to use by the eqasm backend), corresponding
     * to the `"hardware_settings"` key in the root JSON object.
     */
    utils::Json hardware_settings;

    /**
     * Scheduling resource description (representing e.g. instrument/control
     * constraints), corresponding to the `"resources"` key in the root JSON
     * object.
     *
     * FIXME: this shouldn't be here as a raw JSON object.
     */
    utils::Json resources;

    /**
     * Topology/qubit grid description, corresponding to the `"topology"` key
     * in the root of the JSON object.
     *
     * FIXME: this shouldn't be here as a raw JSON object.
     */
    utils::Json topology;

    /**
     * Parsed topology/qubit grid information.
     */
    utils::Opt<Grid> grid;

    /**
     * Constructs a platform from the given configuration filename.
     */
    Platform(
        const utils::Str &name,
        const utils::Str &platform_config_file_name,
        const utils::Str &compiler_config_file_name = ""
    );

    /**
     * Dumps some basic info about the platform to the given stream.
     */
    void dump_info(std::ostream &os = std::cout, utils::Str line_prefix = "") const;

    /**
     * Returns the architecture corresponding to this platform.
     */
    arch::Architecture get_architecture() const;

    /**
     * Returns the JSON data for a custom gate, throwing a semi-useful
     * exception if the instruction is not found.
     */
    const utils::Json &find_instruction(const utils::Str &iname) const;

    /**
     * Converts the given time in nanoseconds to cycles.
     */
    utils::UInt time_to_cycles(utils::Real time_ns) const;

};

/**
 * Smart pointer reference to a platform.
 */
using PlatformRef = utils::One<Platform>;

} // namespace plat
} // namespace ql
