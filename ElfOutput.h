#ifndef __ELFOUTPUT_H__
#define __ELFOUTPUT_H__

#include "Output.h"
#include "Settings.h"

bool GenerateElfFile(OutputBlock* output, const Settings& settings, OutputBlock* codeSection,
    OutputBlock* dataSection);
uint64_t AdjustBaseForElfFile(uint64_t fileBase, const Settings& settings);
uint64_t AdjustDataSectionBaseForElfFile(uint64_t base);

#endif
