#ifndef __OPTIMIZE_H__
#define __OPTIMIZE_H__

#include "Function.h"
#include "Linker.h"

class Optimize
{
	Linker* m_linker;
	Settings m_settings;

public:
	Optimize(Linker* linker);

	void PerformControlFlowAnalysis(Function* func);
	bool ConsolidateBasicBlocks(Function* func);
	bool OptimizeForNoReturnCalls(Function* func);

	void InlineFunction(Function* func, Function* target);

	void RemoveUnreferencedSymbols();
	void PerformGlobalOptimizations();
	bool OptimizeFunction(Function* func);
};

#endif

