/**
 * @file    codegen_cc.cc
 * @date    201810xx
 * @author  Wouter Vlothuizen (wouter.vlothuizen@tno.nl)
 * @brief   code generator backend for the Central Controller
 * @note    here we don't check whether the sequence of calling code generator
 *          functions is correct
 */

// constants:
#define CC_BACKEND_VERSION_STRING       "0.2.5"


#include "codegen_cc.h"
#include "eqasm_backend_cc.h"

#include <version.h>
#include <options.h>

#ifdef _MSC_VER // MS Visual C++ does not know about ssize_t
  #include <type_traits>
  typedef std::make_signed<size_t>::type ssize_t;
#endif

/************************************************************************\
| Generic
\************************************************************************/

void codegen_cc::init(const ql::quantum_platform &platform)
{
    // NB: a new eqasm_backend_cc is instantiated per call to compile, and
    // as a result also a codegen_cc, so we don't need to cleanup
    this->platform = &platform;
    load_backend_settings();

    // optionally preload codewordTable
    std::string map_input_file = ql::options::get("backend_cc_map_input_file");
    if(map_input_file != "") {
        DOUT("loading map_input_file='" << map_input_file << "'");
        json map = ql::load_json(map_input_file);
        codewordTable = map["codeword_table"];      // FIXME: use json_get
        mapPreloaded = true;
    }
}

std::string codegen_cc::getCode()
{
    return cccode.str();
}

std::string codegen_cc::getMap()
{
    json map;

    map["note"] = "generated by OpenQL CC backend version " CC_BACKEND_VERSION_STRING;
    map["codeword_table"] = codewordTable;
//    map["inputLut_table"] = inputLutTable;
    return SS2S(std::setw(4) << map << std::endl);
}

void codegen_cc::program_start(const std::string &progName)
{
    // emit program header
    cccode << std::left;    // assumed by emit()
    cccode << "# Program: '" << progName << "'" << std::endl;   // NB: put on top so it shows up in internal CC logging
    cccode << "# CC_BACKEND_VERSION " << CC_BACKEND_VERSION_STRING << std::endl;
    cccode << "# OPENQL_VERSION " << OPENQL_VERSION_STRING << std::endl;
    cccode << "# Note:    generated by OpenQL Central Controller backend" << std::endl;
    cccode << "#" << std::endl;

    latencyCompensation();  // FIXME: does not support measuring yet

#if OPT_VCD_OUTPUT
    // define header
    vcd.start();

    // define kernel variable
    vcd.scope(vcd.ST_MODULE, "kernel");
    vcdVarKernel = vcd.registerVar("kernel", Vcd::VT_STRING);
    vcd.upscope();

    // define qubit variables
    vcd.scope(vcd.ST_MODULE, "qubits");
    vcdVarQubit.resize(platform->qubit_number);
    for(size_t q=0; q<platform->qubit_number; q++) {
        std::string name = "q"+std::to_string(q);
        vcdVarQubit[q] = vcd.registerVar(name, Vcd::VT_STRING);
    }
    vcd.upscope();

    // define signal variables
    size_t instrsUsed = jsonInstruments->size();
    vcd.scope(vcd.ST_MODULE, "signals");
    vcdVarSignal.assign(instrsUsed, std::vector<int>(MAX_GROUPS, {0}));
    for(size_t instrIdx=0; instrIdx<instrsUsed; instrIdx++) {
        const json &instrument = (*jsonInstruments)[instrIdx];                  // NB: always exists
        std::string instrumentPath = SS2S("instruments["<<instrIdx<<"]");       // for JSON error reporting
        std::string instrumentName = json_get<std::string>(instrument, "name", instrumentPath);
        const json qubits = json_get<const json>(instrument, "qubits", instrumentPath);
        for(size_t group=0; group<qubits.size(); group++) {
            std::string name = instrumentName+"-"+std::to_string(group);
            vcdVarSignal[instrIdx][group] = vcd.registerVar(name, Vcd::VT_STRING);
        }
    }
    vcd.upscope();

    // define codeword variables
    vcd.scope(vcd.ST_MODULE, "codewords");
    vcdVarCodeword.resize(platform->qubit_number);
    for(size_t instrIdx=0; instrIdx<instrsUsed; instrIdx++) {
        const json &instrument = (*jsonInstruments)[instrIdx];                  // NB: always exists
        std::string instrumentPath = SS2S("instruments["<<instrIdx<<"]");       // for JSON error reporting
        std::string instrumentName = json_get<std::string>(instrument, "name", instrumentPath);
        vcdVarCodeword[instrIdx] = vcd.registerVar(instrumentName, Vcd::VT_STRING);
    }
    vcd.upscope();
#endif
}

void codegen_cc::program_finish(const std::string &progName)
{
#if OPT_RUN_ONCE   // program runs once only
        emit("", "stop");
#else   // CC-light emulation: loop indefinitely
    emit("",      // no CCIO selector
         "jmp",
         "@mainLoop",
         "# loop indefinitely");
#endif

#if OPT_VCD_OUTPUT
    // generate VCD
    vcd.finish();

    // write VCD to file
    std::string file_name(ql::options::get("output_dir") + "/" + progName + ".vcd");
    IOUT("Writing Value Change Dump to " << file_name);
    ql::utils::write_file(file_name, vcd.getVcd());
#endif
}

void codegen_cc::kernel_start()
{
    ql::utils::zero(lastEndCycle);       // FIXME: actually, bundle.startCycle starts counting at 1
}

void codegen_cc::kernel_finish(const std::string &kernelName, size_t durationInCycles)
{
#if OPT_VCD_OUTPUT
    // NB: timing starts anew for every kernel
    size_t durationInNs = durationInCycles*platform->cycle_time;
    vcd.change(vcdVarKernel, kernelStartTime, kernelName);     // start of kernel
    vcd.change(vcdVarKernel, kernelStartTime + durationInNs, "");               // end of kernel
    kernelStartTime += durationInNs;
#endif
}

// bundle_start: clear groupInfo, which maintains the work that needs to be performed for bundle
void codegen_cc::bundle_start(const std::string &cmnt)
{
    size_t slotsUsed = jsonInstruments->size();   // FIXME: assuming all instruments use a slot
    groupInfo.assign(slotsUsed, std::vector<tGroupInfo>(MAX_GROUPS, {"", 0, -1}));

    comment(cmnt);
}

// bundle_finish: generate code for bundle from information collected in groupInfo (which may be empty if no classical gates are present in bundle)
void codegen_cc::bundle_finish(size_t startCycle, size_t durationInCycles, bool isLastBundle)
{
    if(isLastBundle) {
        comment(SS2S(" # last bundle of kernel, will pad outputs to match durations"));
    }

    // iterate over instruments
    for(size_t instrIdx=0; instrIdx<jsonInstruments->size(); instrIdx++) {
        const json &instrument = (*jsonInstruments)[instrIdx];          // NB: always exists
        std::string instrumentName = json_get<std::string>(instrument, "name", SS2S("instruments["<<instrIdx<<"]"));
        JSON_ASSERT(instrument, "controller", instrumentName);          // first check intermediate node
        int slot = json_get<int>(instrument["controller"], "slot", instrumentName+"/controller");    // FIXME: assuming controller being cc

        // collect info for all groups within slot, i.e. one connected instrument
        // FIXME: the term 'group' is used in a diffused way: 1) index of signal vectors, 2) ...
        bool isSlotUsed = false;
        uint32_t digOut = 0;
        uint32_t digIn = 0;
        uint32_t slotDurationInCycles = 0;                                  // maximum duration over groups that are used
        size_t nrGroups = groupInfo[instrIdx].size();
        for(size_t group=0; group<nrGroups; group++) {                      // iterate over groups used within slot
            if(groupInfo[instrIdx][group].signalValue != "") {              // signal defined, i.e.: we need to output something
                isSlotUsed = true;

                // find control mode & bits for instrument&group
                std::string refControlMode = json_get<std::string>(instrument, "ref_control_mode", instrumentName);
                const json controlMode = json_get<const json>(*jsonControlModes, refControlMode, "control_modes");     // the control mode definition for our instrument
                size_t nrControlBitsGroups = controlMode["control_bits"].size();    // how many groups of control bits does the control mode specify
                // determine which group to use
                size_t controlModeGroup = -1;
                if(nrControlBitsGroups == 0) {
                    FATAL("'control_bits' not defined in 'control_modes/" << refControlMode <<"'");
                } else if(nrControlBitsGroups == 1) {                       // vector mode: group addresses channel within vector
                    controlModeGroup = 0;
                } else if(group < nrControlBitsGroups) {                    // normal mode: group selects control group
                    controlModeGroup = group;
                } else {
                    FATAL("instrument '" << instrumentName
                          << "' uses " << nrGroups
                          << " groups, but control mode '" << refControlMode
                          << "' only defines " << nrControlBitsGroups
                          << " groups in 'control_bits'");
                }
                // FIXME: check array size
                const json groupControlBits = controlMode["control_bits"][controlModeGroup];


                // find or create codeword/mask fragment for this group
                DOUT("instrumentName=" << instrumentName
                     << ", slot=" << slot
                     << ", control mode group=" << controlModeGroup
                     << ", group control bits: " << groupControlBits);
                size_t nrGroupControlBits = groupControlBits.size();
                uint32_t groupDigOut = 0;
                if(nrGroupControlBits == 1) {      // single bit, implying this is a mask (not code word)
                    groupDigOut |= 1<<(int)groupControlBits[0];     // NB: we assume the mask is active high, which is correct for VSM and UHF-QC
                    // FIXME: check controlModeGroup vs group
                } else {                // > 1 bit, implying code word
                    // FIXME allow single code word for vector of groups. Requires looking at all signals before assigning code word
                    if(group != controlModeGroup) {
                        FATAL("vector mode not yet supported");
                    }

                    // find or assign code word
                    uint32_t codeword = 0;
                    bool codewordOverriden = false;
#if OPT_SUPPORT_STATIC_CODEWORDS    // FIXME: this does not only provide support, but actually requires static codewords
                    int staticCodewordOverride = groupInfo[instrIdx][group].staticCodewordOverride;
                    codeword = staticCodewordOverride;
                    codewordOverriden = true;
#else
                    codeword = assignCodeword(instrumentName, instrIdx, group);
#endif

                    // convert codeword to digOut
                    for(size_t idx=0; idx<nrGroupControlBits; idx++) {
                        int codeWordBit = nrGroupControlBits-1-idx;    // NB: groupControlBits defines MSB..LSB
                        if(codeword & (1<<codeWordBit)) groupDigOut |= 1<<(int)groupControlBits[idx];
                    }

                    comment(SS2S("  # slot=" << slot
                            << ", instrument='" << instrumentName << "'"
                            << ", group=" << group
                            << ": codeword=" << codeword
                            << std::string(codewordOverriden ? " (static override)" : "")
                            << ": groupDigOut=0x" << std::hex << std::setfill('0') << std::setw(8) << groupDigOut
                            ));
                }

                digOut |= groupDigOut;

                // add trigger to digOut
                size_t nrTriggerBits = controlMode["trigger_bits"].size();
                if(nrTriggerBits == 0) {         // no trigger
                    // do nothing
                } else if(nrTriggerBits == 1) {  // single trigger for all groups
                    digOut |= 1 << (int)controlMode["trigger_bits"][0];
                } else {                        // trigger per group
                    digOut |= 1 << (int)controlMode["trigger_bits"][group];
                    // FIXME: check validity of nrTriggerBits
                }

                // compute slot duration
                size_t durationInCycles = platform->time_to_cycles(groupInfo[instrIdx][group].durationInNs);
                if(durationInCycles > slotDurationInCycles) slotDurationInCycles = durationInCycles;

                // handle readout
                // NB: this does not allow for readout without signal generation (by the same instrument), which might be needed in the future
                // FIXME: move out of this loop
                // FIXME: test for groupInfo[instrIdx][group].readoutCop >= 0
                if(JSON_EXISTS(controlMode, "result_bits")) {                               // this instrument mode produces results
                    const json &resultBits = controlMode["result_bits"][group];
                    size_t nrResultBits = resultBits.size();
                    if(nrResultBits == 1) {                     // single bit
                        digIn |= 1<<(int)resultBits[0];         // NB: we assume the result is active high, which is correct for UHF-QC

#if 0   // FIXME: WIP on measurement
                        // FIXME: save groupInfo[instrIdx][group].readoutCop in inputLut
                        if(JSON_EXISTS(inputLutTable, instrumentName) &&                    // instrument exists
                                        inputLutTable[instrumentName].size() > group) {     // group exists
                        } else {    // new instrument or group
                            codeword = 1;
                            inputLutTable[instrumentName][group][0] = "";                   // code word 0 is empty
                            inputLutTable[instrumentName][group][codeword] = signalValue;   // NB: structure created on demand
                        }
#endif
                    } else {    // NB: nrResultBits==0 will not arrive at this point
                        std::string controlModeName = controlMode;                      // convert to string
                        FATAL("JSON key '" << controlModeName << "/result_bits' must have 1 bit per group");
                    }
                }
#if OPT_VCD_OUTPUT
                // generate signal output for group
                size_t startTime = kernelStartTime + startCycle*platform->cycle_time;
                size_t durationInNs = groupInfo[instrIdx][group].durationInNs;
                std::string signalValue = groupInfo[instrIdx][group].signalValue;
                int var = vcdVarSignal[instrIdx][group];
                std::string val = SS2S(groupDigOut) + "=" + signalValue;
                vcd.change(var, startTime, val);                // start of signal
                vcd.change(var, startTime+durationInNs, "");     // end of signal
#endif
            } // if(signal defined)
        } // for(group)

#if OPT_VCD_OUTPUT
        // generate codeword output for instrument
        size_t startTime = kernelStartTime + startCycle*platform->cycle_time;
        size_t durationInNs = slotDurationInCycles*platform->cycle_time;
        int var = vcdVarCodeword[instrIdx];
        std::string val = SS2S("0x" << std::hex << std::setfill('0') << std::setw(8) << digOut);
        vcd.change(var, startTime, val);                // start of signal
        vcd.change(var, startTime+durationInNs, "");     // end of signal
#endif


        // generate code for slot
        if(isSlotUsed) {
            comment(SS2S("  # slot=" << slot
                    << ", instrument='" << instrumentName << "'"
//                        << ", group=" << group
                    << ": lastEndCycle=" << lastEndCycle[instrIdx]
                    << ", startCycle=" << startCycle
                    << ", slotDurationInCycles=" << slotDurationInCycles
                    ));

            padToCycle(lastEndCycle[instrIdx], startCycle, slot, instrumentName);

            // emit code for slot
            emit(SS2S("[" << slot << "]").c_str(),      // CCIO selector
                 "seq_out",
                 SS2S("0x" << std::hex << std::setfill('0') << std::setw(8) << digOut << std::dec <<
                      "," << slotDurationInCycles),
                 SS2S("# cycle " << startCycle << "-" << startCycle+slotDurationInCycles << ": code word/mask on '" << instrumentName+"'").c_str());

            // update lastEndCycle
            lastEndCycle[instrIdx] = startCycle + slotDurationInCycles;

            if(digIn) { // FIXME
                comment(SS2S("# digIn=" << digIn));
                // get qop
                // get cop
                // get/assign LUT
                // seq_in_sm
            }
        } else {
            // nothing to do, we delay emitting till a slot is used or kernel finishes (i.e. isLastBundle just below)
        }

        // pad end of bundle to align durations
        if(isLastBundle) {
            padToCycle(lastEndCycle[instrIdx], startCycle+durationInCycles, slot, instrumentName);
            // FIXME: also pad latency differences, here or in kernel_finish?
        }
    } // for(instrIdx)

    comment("");    // blank line to separate bundles
}

void codegen_cc::comment(const std::string &c)
{
    if(verboseCode) emit(c.c_str());
}

/************************************************************************\
| Quantum instructions
\************************************************************************/

// single/two/N qubit gate, including readout
void codegen_cc::custom_gate(
        const std::string &iname,
        const std::vector<size_t> &qops,
        const std::vector<size_t> &cops,
        double angle, size_t startCycle, size_t durationInNs)
{
#if 1   // FIXME: test for angle parameter
    if(angle != 0.0) {
        DOUT("iname=" << iname << ", angle=" << angle);
    }
#endif
    bool isReadout = false;

    if("readout" == platform->find_instruction_type(iname))          // handle readout
    /* FIXME: we only use the "readout" instruction_type and don't care about the rest because the terms "mw" and "flux" don't fully
     * cover gate functionality. It would be nice if custom gates could mimic ql::gate_type_t
    */
    {
        isReadout = true;

        if(cops.size() == 0) {      // NB: existing code uses empty cops: measurement results can also be read from the readout device
            // FIXME: define meaning: no classical target, or implied target (classical register matching qubit)
            comment(SS2S(" # READOUT: " << iname << "(q" << qops[0] << ")"));
        } else if(cops.size() != 1) {
            FATAL("Readout instruction requires 0 or 1 classical operands, not " << cops.size());
        } else {
            comment(SS2S(" # READOUT: " << iname << "(c" << cops[0] << ",q" << qops[0] << ")"));
        }

    } else { // handle all other instruction types
        // generate comment. NB: we don't have a particular limit for the number of operands
        std::stringstream cmnt;
        cmnt << " # gate '" << iname << " ";
        for(size_t i=0; i<qops.size(); i++) {
            cmnt << qops[i];
            if(i<qops.size()-1) cmnt << ",";
        }
        cmnt << "'";
        comment(cmnt.str());
    }

    const json &instruction = platform->find_instruction(iname);

#if OPT_VCD_OUTPUT
    // generate qubit output
    size_t startTime = kernelStartTime + startCycle*platform->cycle_time;
    for(size_t i=0; i<qops.size(); i++) {
        // FIXME: improve name for 2q gates
        int var = vcdVarQubit[qops[i]];
        vcd.change(var, startTime, iname);             // start of instruction
        vcd.change(var, startTime+durationInNs, "");    // end of instruction
    }
#endif

#if OPT_SUPPORT_STATIC_CODEWORDS
    // look for optional codeword override
    int staticCodewordOverride = -1;    // -1 means unused
    if(JSON_EXISTS(instruction["cc"], "static_codeword_override")) {
        staticCodewordOverride = instruction["cc"]["static_codeword_override"];
        DOUT("Found static_codeword_override=" << staticCodewordOverride <<
             " for instruction '" << iname << "'");
    }
 #if 1 // FIXME: require override
    if(staticCodewordOverride < 0) {
        FATAL("No static codeword defined for instruction '" << iname <<
            "' (we currently require it because automatic assignment is disabled)");
    }
 #endif
#endif

    // find signal definition for iname
    tJsonNodeInfo nodeInfo = findSignalDefinition(instruction, iname);
    const json signal = nodeInfo.node;
    std::string signalPath = nodeInfo.path;

    // iterate over signals defined for instruction
    for(size_t s=0; s<signal.size(); s++) {
        // get the qubit to work on
        std::string signalSPath = SS2S(signalPath<<"["<<s<<"]");        // for JSON error reporting
        size_t operandIdx = json_get<size_t>(signal[s], "operand_idx", signalSPath);

        if(operandIdx >= qops.size()) {
            FATAL("Error in JSON definition of instruction '" << iname <<
                  "': illegal operand number " << operandIdx <<
                  "' exceeds expected maximum of " << qops.size()-1)
        }
        size_t qubit = qops[operandIdx];


        // get the instrument and group that generates the signal
        std::string instructionSignalType = json_get<std::string>(signal[s], "type", signalSPath);
        JSON_ASSERT(signal[s], "value", signalSPath);                   // NB: json_get<const json&> unavailable
        const json &instructionSignalValue = signal[s]["value"];

        tSignalInfo si = findSignalInfoForQubit(instructionSignalType, qubit);
        const json &instrument = (*jsonInstruments)[si.instrIdx];       // should exist
        std::string instrumentName = json_get<std::string>(instrument, "name", SS2S("instruments["<<si.instrIdx<<"]"));
        JSON_ASSERT(instrument, "controller", instrumentName);          // first check intermediate node
        int slot = json_get<int>(instrument["controller"], "slot", instrumentName+"/controller");    // FIXME: assuming controller being cc


        // expand macros in signalValue
        std::string signalValueString = SS2S(instructionSignalValue);   // serialize instructionSignalValue into std::string
        ql::utils::replace(signalValueString, std::string("\""), std::string(""));   // get rid of quotes
        ql::utils::replace(signalValueString, std::string("{gateName}"), iname);
        ql::utils::replace(signalValueString, std::string("{instrumentName}"), instrumentName);
        ql::utils::replace(signalValueString, std::string("{instrumentGroup}"), std::to_string(si.group));
        // FIXME: allow using all qubits involved (in same signalType?, or refer to signal: qubitOfSignal[n]), e.g. qubit[0], qubit[1], qubit[2]
        ql::utils::replace(signalValueString, std::string("{qubit}"), std::to_string(qubit));

        comment(SS2S("  # slot=" << slot
                << ", instrument='" << instrumentName << "'"
                << ", group=" << si.group
                << "': signal='" << signalValueString << "'"
                ));

        // check and store signal value
        if(groupInfo[si.instrIdx][si.group].signalValue == "") {                         // signal not yet used
            groupInfo[si.instrIdx][si.group].signalValue = signalValueString;
#if OPT_SUPPORT_STATIC_CODEWORDS
            groupInfo[si.instrIdx][si.group].staticCodewordOverride = staticCodewordOverride;   // NB: -1 means unused
#endif
        } else if(groupInfo[si.instrIdx][si.group].signalValue == signalValueString) {   // signal unchanged
            // do nothing
        } else {
            EOUT("Code so far:\n" << cccode.str());                    // FIXME: provide context to help finding reason
            FATAL("Signal conflict on instrument='" << instrumentName <<
                  "', group=" << si.group <<
                  ", between '" << groupInfo[si.instrIdx][si.group].signalValue <<
                  "' and '" << signalValueString << "'");
        }

        if(isReadout) {
            // remind the classical operand used
            int cop = cops.size()>0 ? cops[0] : -1;
            groupInfo[si.instrIdx][si.group].readoutCop = cop;
        }

        groupInfo[si.instrIdx][si.group].durationInNs = durationInNs;

        DOUT("custom_gate(): iname='" << iname <<
             "', duration=" << durationInNs <<
             "[ns], si.instrIdx=" << si.instrIdx <<
             ", si.group=" << si.group);

        // NB: code is generated in bundle_finish()
    }   // for(signal)
}

void codegen_cc::nop_gate()
{
    comment("# NOP gate");
    FATAL("FIXME: NOP gate not implemented");
}

/************************************************************************\
| Classical operations on kernels
\************************************************************************/

void codegen_cc::if_start(size_t op0, const std::string &opName, size_t op1)
{
    comment(SS2S("# IF_START(R" << op0 << " " << opName << " R" << op1 << ")"));
    FATAL("FIXME: not implemented");
}

void codegen_cc::else_start(size_t op0, const std::string &opName, size_t op1)
{
    comment(SS2S("# ELSE_START(R" << op0 << " " << opName << " R" << op1 << ")"));
    FATAL("FIXME: not implemented");
}

void codegen_cc::for_start(const std::string &label, int iterations)
{
    comment(SS2S("# FOR_START(" << iterations << ")"));
    // FIXME: reserve register
    emit((label+":").c_str(), "move", SS2S(iterations << ",R63"), "# R63 is the 'for loop counter'");        // FIXME: fixed reg, no nested loops
}

void codegen_cc::for_end(const std::string &label)
{
    comment("# FOR_END");
    // FIXME: free register
    emit("", "loop", SS2S("R63,@" << label), "# R63 is the 'for loop counter'");        // FIXME: fixed reg, no nested loops
}

void codegen_cc::do_while_start(const std::string &label)
{
    comment("# DO_WHILE_START");
    emit((label+":").c_str(), "", SS2S(""), "# ");        // FIXME: just a label
}

void codegen_cc::do_while_end(const std::string &label, size_t op0, const std::string &opName, size_t op1)
{
    comment(SS2S("# DO_WHILE_END(R" << op0 << " " << opName << " R" << op1 << ")"));
    emit("", "jmp", SS2S("@" << label), "# endless loop'");        // FIXME: just endless loop
}

/************************************************************************\
| Classical arithmetic instructions
\************************************************************************/

void codegen_cc::add() {
    FATAL("FIXME: not implemented");
}

/************************************************************************\
| Some helpers to ease nice assembly formatting
\************************************************************************/

void codegen_cc::emit(const char *labelOrComment, const char *instr)
{
    if(!labelOrComment || strlen(labelOrComment)==0) {  // no label
        cccode << "        " << instr << std::endl;
    } else if(strlen(labelOrComment)<8) {               // label fits before instr
        cccode << std::setw(8) << labelOrComment << instr << std::endl;
    } else if(strlen(instr)==0) {                       // no instr
        cccode << labelOrComment << std::endl;
    } else {
        cccode << labelOrComment << std::endl << "        " << instr << std::endl;
    }
}

void codegen_cc::emit(const char *label, const char *instr, const std::string &qops, const char *comment)
{
    cccode << std::setw(16) << label << std::setw(16) << instr << std::setw(24) << qops << comment << std::endl;
}
// FIXME: assure space between fields!
// FIXME: also provide the above with std::string parameters


/************************************************************************\
| helpers
\************************************************************************/

void codegen_cc::latencyCompensation()
{
    comment("# synchronous start and latency compensation");

#if OPT_CALCULATE_LATENCIES    // fixed compensation based on instrument latencies
    std::map<int, int> slotLatencies;   // maps slot to latency

    // get latencies per slot, iterating over instruments
    for(size_t instrIdx=0; instrIdx<jsonInstruments->size(); instrIdx++) {
        const json &instrument = (*jsonInstruments)[instrIdx];                  // NB: always exists
        std::string instrumentRef = instrument["ref_instrument_definition"];    // FIXME: use json_get
        int slot = instrument["controller"]["slot"];    // FIXME: assuming controller being cc, use json_get

        // find latency
        const json &id = findInstrumentDefinition(instrumentRef);
        int latency = id["latency"];    // FIXME: json_get
        DOUT("latency of '" << instrumentRef << "' in slot " << slot << " is " << latency);
        slotLatencies.insert(std::make_pair(slot, latency));
    }

    // find max latency
    int maxLatency = 0;
    for(auto it=slotLatencies.begin(); it!=slotLatencies.end(); it++) {
        int latency = it->second;
        if(latency > maxLatency) maxLatency = latency;
    }
    DOUT("maxLatency = " << maxLatency);

    // align latencies
    for(auto it=slotLatencies.begin(); it!=slotLatencies.end(); it++) {
        int slot = it->first;
        int latency = it->second;
        const int minDelay = 1;     // min value for seq_bar
        int delayInCycles = minDelay + platform->time_to_cycles(maxLatency-latency);

        emit(SS2S("[" << slot << "]").c_str(),      // CCIO selector
            "seq_bar",
            SS2S(delayInCycles),    // FIXME: old semantics
            SS2S("# latency compensation").c_str());    // FIXME: add instrumentName/instrumentRef/latency
    }
#else   // user settable delay via register
 #if OPT_OLD_SEQBAR_SEMANTICS  // original seq_bar semantics
        // FIXME: is 'seq_bar 1' safe in the sense that we will never get an empty queue?
        emit("",                "add",      "R63,1,R0",         "# R63 externally set by user, prevent 0 value which would wrap counter");
        emit("",                "seq_bar",  "20",               "# synchronization");
        emit("syncLoop:",       "seq_out",  "0x00000000,1",     "# 20 ns delay");
        emit("",                "loop",     "R0,@syncLoop",     "# ");
        emit("mainLoop:",       "",         "",                 "# ");
 #else  // new seq_bar semantics (firmware from 20191219 onwards)
        emit("",                "seq_bar",  "",                 "# synchronization, delay set externally through SET_SEQ_BAR_CNT");
        emit("mainLoop:",       "",         "",                 "# ");
 #endif
#endif
}

void codegen_cc::padToCycle(size_t lastEndCycle, size_t startCycle, int slot, const std::string &instrumentName)
{
    // compute prePadding: time to bridge to align timing
    ssize_t prePadding = startCycle - lastEndCycle;
    if(prePadding < 0) {
        EOUT(getCode());        // show what we made
        FATAL("Inconsistency detected in bundle contents: time travel not yet possible in this version: prePadding=" << prePadding <<
              ", startCycle=" << startCycle <<
              ", lastEndCycle=" << lastEndCycle <<
              ", instrumentName='" << instrumentName << "'");
    }

    if(prePadding > 0) {     // we need to align
        emit(SS2S("[" << slot << "]").c_str(),      // CCIO selector
            "seq_out",
            SS2S("0x00000000," << prePadding),
            SS2S("# cycle " << lastEndCycle << "-" << startCycle << ": padding on '" << instrumentName+"'").c_str());
    }
}

#if !OPT_SUPPORT_STATIC_CODEWORDS
uint32_t codegen_cc::assignCodeword(const std::string &instrumentName, int instrIdx, int group)
{
    uint32_t codeword;
    std::string signalValue = groupInfo[instrIdx][group].signalValue;

    if(JSON_EXISTS(codewordTable, instrumentName) &&                    // instrument exists
                    codewordTable[instrumentName].size() > group) {     // group exists
        bool cwFound = false;
        // try to find signalValue
        json &myCodewordArray = codewordTable[instrumentName][group];
        for(codeword=0; codeword<myCodewordArray.size() && !cwFound; codeword++) {   // NB: JSON find() doesn't work for arrays
            if(myCodewordArray[codeword] == signalValue) {
                DOUT("signal value found at cw=" << codeword);
                cwFound = true;
            }
        }
        if(!cwFound) {
            std::string msg = SS2S("signal value '" << signalValue
                    << "' not found in group " << group
                    << ", which contains " << myCodewordArray);
            if(mapPreloaded) {
                FATAL("mismatch between preloaded 'backend_cc_map_input_file' and program requirements:" << msg)
            } else {
                DOUT(msg);
                // NB: codeword already contains last used value + 1
                // FIXME: check that number is available
                myCodewordArray[codeword] = signalValue;                    // NB: structure created on demand
            }
        }
    } else {    // new instrument or group
        if(mapPreloaded) {
            FATAL("mismatch between preloaded 'backend_cc_map_input_file' and program requirements: instrument '"
                  << instrumentName << "', group "
                  << group
                  << " not present in file");
        } else {
            codeword = 1;
            codewordTable[instrumentName][group][0] = "";                   // code word 0 is empty
            codewordTable[instrumentName][group][codeword] = signalValue;   // NB: structure created on demand
        }
    }
    return codeword;
}
#endif

/************************************************************************\
| Functions processing JSON
\************************************************************************/

void codegen_cc::load_backend_settings()
{
    // remind some main JSON areas
    JSON_ASSERT(platform->hardware_settings, "eqasm_backend_cc", "hardware_settings");  // NB: json_get<const json &> unavailable
    const json &jsonBackendSettings = platform->hardware_settings["eqasm_backend_cc"];

    JSON_ASSERT(jsonBackendSettings, "instrument_definitions", "eqasm_backend_cc");
    jsonInstrumentDefinitions = &jsonBackendSettings["instrument_definitions"];

    JSON_ASSERT(jsonBackendSettings, "control_modes", "eqasm_backend_cc");
    jsonControlModes = &jsonBackendSettings["control_modes"];

    JSON_ASSERT(jsonBackendSettings, "instruments", "eqasm_backend_cc");
    jsonInstruments = &jsonBackendSettings["instruments"];

    JSON_ASSERT(jsonBackendSettings, "signals", "eqasm_backend_cc");
    jsonSignals = &jsonBackendSettings["signals"];

#if 0   // FIXME: print some info, which also helps detecting errors early on
    // read instrument definitions
    // FIXME: the following requires json>v3.1.0:  for(auto& id : jsonInstrumentDefinitions->items()) {
    for(size_t i=0; i<jsonInstrumentDefinitions->size(); i++) {
        std::string idName = jsonInstrumentDefinitions[i];        // NB: uses type conversion to get node value
        DOUT("found instrument definition: '" << idName <<"'");
    }

    // read control modes
    for(size_t i=0; i<jsonControlModes->size(); i++)
    {
        const json &name = (*jsonControlModes)[i]["name"];
        DOUT("found control mode '" << name <<"'");
    }

    // read instruments
    const json &ccSetupType = jsonCcSetup["type"];

    // CC specific
    const json &ccSetupSlots = jsonCcSetup["slots"];                      // FIXME: check against jsonInstrumentDefinitions
    for(size_t slot=0; slot<ccSetupSlots.size(); slot++) {
        const json &instrument = ccSetupSlots[slot]["instrument"];
        std::string instrumentName = instrument["name"];
        std::string signalType = instrument["signal_type"];

        DOUT("found instrument: name='" << instrumentName << "', signal type='" << signalType << "'");
    }
#endif
}


#if OPT_CALCULATE_LATENCIES
const json &codegen_cc::findInstrumentDefinition(const std::string &name)
{
    // FIXME: use json_get
    if JSON_EXISTS(*jsonInstrumentDefinitions, name) {
        return (*jsonInstrumentDefinitions)[name];
    } else {
        FATAL("Could not find key 'name'=" << name << "in JSON section 'instrument_definitions'");
    }
}
#endif


// find instrument/group providing instructionSignalType for qubit
codegen_cc::tSignalInfo codegen_cc::findSignalInfoForQubit(const std::string &instructionSignalType, size_t qubit)
{
    tSignalInfo ret = {-1, -1};
    bool signalTypeFound = false;
    bool qubitFound = false;

    // iterate over instruments
    for(size_t instrIdx=0; instrIdx<jsonInstruments->size(); instrIdx++) {
        const json &instrument = (*jsonInstruments)[instrIdx];                  // NB: always exists
        std::string instrumentPath = SS2S("instruments["<<instrIdx<<"]");       // for JSON error reporting
        std::string instrumentSignalType = json_get<std::string>(instrument, "signal_type", instrumentPath);
        if(instrumentSignalType == instructionSignalType) {
            signalTypeFound = true;
            std::string instrumentName = json_get<std::string>(instrument, "name", instrumentPath);
            const json qubits = json_get<const json>(instrument, "qubits", instrumentPath);   // NB: json_get<const json&> unavailable

            // FIXME: verify group size: qubits vs. control mode
            // FIXME: verify signal dimensions

            // anyone connected to qubit?
            for(size_t group=0; group<qubits.size() && !qubitFound; group++) {
                for(size_t idx=0; idx<qubits[group].size() && !qubitFound; idx++) {
                    if(qubits[group][idx] == qubit) {
                        qubitFound = true;

                        DOUT("qubit " << qubit
                             << " signal type '" << instructionSignalType
                             << "' driven by instrument '" << instrumentName
                             << "' group " << group
//                                 << " in CC slot " << ccSetupSlot["slot"]
                             );

                        ret.instrIdx = instrIdx;
                        ret.group = group;
                        // FIXME: stop searching?
                    }
                }
            }
        }
    }
    if(!signalTypeFound) {
        FATAL("No instruments found providing signal type '" << instructionSignalType << "'");     // FIXME: clarify for user
    }
    if(!qubitFound) {
        FATAL("No instruments found driving qubit " << qubit << " for signal type '" << instructionSignalType << "'");     // FIXME: clarify for user
    }

    return ret;
}


codegen_cc::tJsonNodeInfo codegen_cc::findSignalDefinition(const json &instruction, const std::string &iname) const
{
    tJsonNodeInfo nodeInfo;
    std::string instructionPath = "instructions/"+iname;
    JSON_ASSERT(instruction, "cc", instructionPath);
    if(JSON_EXISTS(instruction["cc"], "ref_signal")) {                          // optional syntax: "ref_signal"
        std::string refSignal = instruction["cc"]["ref_signal"];
        nodeInfo.node = (*jsonSignals)[refSignal];                              // poor man's JSON pointer
        if(nodeInfo.node.size() == 0) {
            FATAL("Error in JSON definition of instruction '" << iname <<
                  "': ref_signal '" << refSignal << "' does not resolve");
        }
        nodeInfo.path = "signals/"+refSignal;
    } else {                                                                    // alternative syntax: "signal"
        nodeInfo.node = json_get<json>(instruction["cc"], "signal", instructionPath+"/cc");
        DOUT("signal for '" << instruction << "': " << nodeInfo.node);
        nodeInfo.path = instructionPath+"/cc/signal";
    }
    return nodeInfo;
}
