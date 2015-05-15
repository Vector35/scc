#ifndef __PEOUTPUT_H__
#define __PEOUTPUT_H__

#include "Output.h"
#include "Settings.h"
#include "Linker.h"

bool GeneratePeFile(OutputBlock* output, const Settings& settings, OutputBlock* codeSection, OutputBlock* dataSection,
	std::map<std::string, ImportTable>& imports);
uint64_t AdjustBaseForPeFile(uint64_t fileBase, const Settings& settings);
uint64_t AdjustDataSectionBaseForPeFile(uint64_t base);

#endif

