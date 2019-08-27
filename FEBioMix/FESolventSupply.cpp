#include "FESolventSupply.h"

//-----------------------------------------------------------------------------
// Material parameters for FESolventSupply
void FESolventSupply::Init()
{
	FEMaterial::Init();
}

//-----------------------------------------------------------------------------
// Derivative of supply w.r.t. solute concentration at material point
// Set this to zero by default because biphasic problems do not require it
double FESolventSupply::Tangent_Supply_Concentration(FEMaterialPoint& pt, const int isol)
{
	return 0;
}
