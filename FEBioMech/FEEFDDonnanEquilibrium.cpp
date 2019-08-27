#include "stdafx.h"
#include "FEEFDDonnanEquilibrium.h"

// define the material parameters
BEGIN_PARAMETER_LIST(FEEFDDonnanEquilibrium, FEElasticMaterial)
	ADD_PARAMETER(m_DEQ.m_phiwr, FE_PARAM_DOUBLE, "phiw0");
	ADD_PARAMETER(m_DEQ.m_cFr, FE_PARAM_DOUBLE, "cF0");
	ADD_PARAMETER(m_DEQ.m_bosm, FE_PARAM_DOUBLE, "bosm");
	ADD_PARAMETERV(m_Fib.m_beta, FE_PARAM_DOUBLEV, 3, "beta");
	ADD_PARAMETERV(m_Fib.m_ksi , FE_PARAM_DOUBLEV, 3, "ksi" );
END_PARAMETER_LIST();

//-----------------------------------------------------------------------------
// FEEFDDonnanEquilibrium
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void FEEFDDonnanEquilibrium::Init()
{
	FEElasticMaterial::Init();
	m_DEQ.Init();
	m_Fib.Init();
}

//-----------------------------------------------------------------------------
void FEEFDDonnanEquilibrium::Serialize(DumpFile& ar)
{
	FEElasticMaterial::Serialize(ar);
	m_Fib.Serialize(ar);
	m_DEQ.Serialize(ar);
}

//-----------------------------------------------------------------------------
mat3ds FEEFDDonnanEquilibrium::Stress(FEMaterialPoint& mp)
{
	// --- M A T R I X   C O N T R I B U T I O N ---
	mat3ds s = m_DEQ.Stress(mp);
	
	// --- F I B E R   C O N T R I B U T I O N ---
	
	// evaluate stress and add it to matrix contribution
	s += m_Fib.Stress(mp);
		
	return s;
}

//-----------------------------------------------------------------------------
tens4ds FEEFDDonnanEquilibrium::Tangent(FEMaterialPoint& mp)
{
	// --- M A T R I X   C O N T R I B U T I O N ---
	tens4ds c = m_DEQ.Tangent(mp);
	
	// --- F I B E R   C O N T R I B U T I O N ---
	
	// evaluate stress and add it to matrix contribution
	c += m_Fib.Tangent(mp);

	return c;
}
