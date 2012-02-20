#ifndef __OPTIMIZE_H__
#define __OPTIMIZE_H__

#include "Function.h"

class Optimize
{
	Settings m_settings;

public:
	Optimize(const Settings& settings);

	void PerformControlFlowAnalysis(Function* func);
	bool ConsolidateBasicBlocks(Function* func);

	void OptimizeFunction(Function* func);
};

#endif

