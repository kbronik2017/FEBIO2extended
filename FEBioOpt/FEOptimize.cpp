#include "stdafx.h"
#include "FEOptimize.h"
#include "FEOptimizer.h"
#include "FECore/FECoreKernel.h"
#include "FECore/log.h"

//-----------------------------------------------------------------------------
FEOptimize::FEOptimize(FEModel* pfem) : FECoreTask(pfem)
{

}

//-----------------------------------------------------------------------------
bool FEOptimize::Run(const char* szfile)
{
	// TODO: the logfile has not been opened yet, so this will only get printed to the screen
	felog.printbox("P A R A M E T E R   O P T I M I Z A T I O N   M O D U L E", "version 0.1");

	// initialize FE model
	if (m_pfem->Init() == false) return false;

	// create an optimizer object
	FEOptimizeData opt(*m_pfem);

	// read the data from the xml input file
	if (opt.Input(szfile) == false) return false;

	// do initialization
	if (opt.Init() == false) return false;

	// solve the problem
	bool bret = opt.Solve();

	if (bret)
		felog.printf(" N O R M A L   T E R M I N A T I O N\n\n");
	else 
		felog.printf(" E R R O R   T E R M I N A T I O N\n\n");

	return bret;
}
