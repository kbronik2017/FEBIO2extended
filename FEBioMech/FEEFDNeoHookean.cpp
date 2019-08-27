#include "stdafx.h"
#include "FEEFDNeoHookean.h"

// define the material parameters
BEGIN_PARAMETER_LIST(FEEFDNeoHookean, FEElasticMaterial)
	ADD_PARAMETER(m_NH.m_E, FE_PARAM_DOUBLE, "E");
	ADD_PARAMETER(m_NH.m_v, FE_PARAM_DOUBLE, "v");
	ADD_PARAMETERV(m_EFD.m_beta, FE_PARAM_DOUBLEV, 3, "beta");
	ADD_PARAMETERV(m_EFD.m_ksi , FE_PARAM_DOUBLEV, 3, "ksi" );
END_PARAMETER_LIST();

//////////////////////////////////////////////////////////////////////
// FEEFDNeoHookean
//////////////////////////////////////////////////////////////////////

void FEEFDNeoHookean::Init()
{
	FEElasticMaterial::Init();

	m_NH.Init();
	m_EFD.Init();
}

mat3ds FEEFDNeoHookean::Stress(FEMaterialPoint& mp)
{
	// --- M A T R I X   C O N T R I B U T I O N ---
	mat3ds s = m_NH.Stress(mp);
	
	// --- F I B E R   C O N T R I B U T I O N ---
	
	// evaluate stress and add it to matrix contribution
	s += m_EFD.Stress(mp);
	
	return s;
}

tens4ds FEEFDNeoHookean::Tangent(FEMaterialPoint& mp)
{
	// --- M A T R I X   C O N T R I B U T I O N ---
	tens4ds c = m_NH.Tangent(mp);
	
	// --- F I B E R   C O N T R I B U T I O N ---
	
	// evaluate stress and add it to matrix contribution
	c += m_EFD.Tangent(mp);
	
	return c;
}

//////////////////////////////////////////////////////////////////////
// FEEFDNeoHookeanOld
//////////////////////////////////////////////////////////////////////

// define the material parameters
BEGIN_PARAMETER_LIST(FEEFDNeoHookeanOld, FEElasticMaterial)
	ADD_PARAMETER(m_NH.m_E, FE_PARAM_DOUBLE, "E");
	ADD_PARAMETER(m_NH.m_v, FE_PARAM_DOUBLE, "v");
	ADD_PARAMETERV(m_EFD.m_beta, FE_PARAM_DOUBLEV, 3, "beta");
	ADD_PARAMETERV(m_EFD.m_ksi , FE_PARAM_DOUBLEV, 3, "ksi" );
END_PARAMETER_LIST();

void FEEFDNeoHookeanOld::Init()
{
	FEElasticMaterial::Init();

	m_NH.Init();
	m_EFD.Init();
}

mat3ds FEEFDNeoHookeanOld::Stress(FEMaterialPoint& mp)
{
	// --- M A T R I X   C O N T R I B U T I O N ---
	mat3ds s = m_NH.Stress(mp);
	
	// --- F I B E R   C O N T R I B U T I O N ---
	
	// evaluate stress and add it to matrix contribution
	s += m_EFD.Stress(mp);
	
	return s;
}

tens4ds FEEFDNeoHookeanOld::Tangent(FEMaterialPoint& mp)
{
	// --- M A T R I X   C O N T R I B U T I O N ---
	tens4ds c = m_NH.Tangent(mp);
	
	// --- F I B E R   C O N T R I B U T I O N ---
	
	// evaluate stress and add it to matrix contribution
	c += m_EFD.Tangent(mp);
	
	return c;
}
