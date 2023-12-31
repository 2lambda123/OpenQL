/**
 * @file   vcd.cc
 * @date   20190515
 * @author Wouter Vlothuizen (wouter.vlothuizen@tno.nl)
 * @brief  generate Value Change Dump file for GTKWave viewer
 * @remark based on https://github.com/SanDisk-Open-Source/pyvcd/tree/master/vcd
 */

#define OPT_DEBUG_VCD   0


#include "ql/utils/vcd.h"

#include <iostream>

namespace ql {
namespace utils {

void Vcd::start() {
    vcd << "$date today $end" << std::endl;
    vcd << "$timescale 1 ns $end" << std::endl;
}


void Vcd::scope(Scope, const Str &name) {
    // FIXME: handle type
    vcd << "$scope " << "module" << " " << name << " $end" << std::endl;
}


int Vcd::registerVar(const Str &name, VarType, Scope) {
    // FIXME: incomplete
    const Int width = 20;

    vcd << "$var string " << width << " " << lastId << " " << name << " $end" << std::endl;

    return lastId++;
}


void Vcd::upscope() {
    vcd << "$upscope $end" << std::endl;
}


void Vcd::change(Int var, Int timestamp, const Str &value)
{
    auto tsIt = timestampMap.find(timestamp);
    if (tsIt != timestampMap.end()) {    // timestamp found
        VarChangeMap &vcm = tsIt->second;

        auto vcmIt = vcm.find(var);
        if (vcmIt != vcm.end()) {        // var found
#if OPT_DEBUG_VCD
            std::cout << "ts=" << tsIt->first
                << ", var " << vcmIt->first
                << " overwritten with '" << value << "'" << std::endl;
#endif
            Value &val = vcmIt->second;
            val.strVal = value;         // overwrite previous value. FIXME: only if it was empty?
        } else {                        // var not found
#if OPT_DEBUG_VCD
            std::cout << "ts=" << tsIt->first
                << ", var " << var
                << " not found, wrote value '" << value << "'" << std::endl;
#endif
            Value val;
            val.strVal = value;
            vcm.insert({var, val});
        }
    } else {                            // timestamp not found
#if OPT_DEBUG_VCD
        std::cout << "ts=" << timestamp
            << " not found, wrote var " << var
            << " with value '" << value << "'" << std::endl;
#endif
        Value val;
        val.strVal = value;

        VarChangeMap varChange{{var, val}};
        timestampMap.insert({timestamp, varChange});
    }
}


void Vcd::change(Int /* var */, Int /* timestamp */, Int /* value */) {
    throw Exception("not yet implemented");
    // FIXME
}


void Vcd::finish() {
    vcd << "$enddefinitions $end" << std::endl;

    for (auto &t: timestampMap) {
        vcd << "#" << t.first << std::endl;      // timestamp
        for (auto &v: t.second) {
            vcd << "s" << v.second.strVal << " " << v.first << std::endl;
        }
    }
}


std::string Vcd::getVcd() {
    return vcd.str();
}


} // namespace utils
} // namespace ql
