#ifndef __MACHOOUTPUT_H__
#define __MACHOOUTPUT_H__

#include "Output.h"
#include "Settings.h"

bool GenerateMachOFile(OutputBlock* output, const Settings& settings, OutputBlock* codeSection, OutputBlock* dataSection);
uint64_t AdjustBaseForMachOFile(uint64_t fileBase, const Settings& settings);
uint64_t AdjustDataSectionBaseForMachOFile(uint64_t base);

#endif

