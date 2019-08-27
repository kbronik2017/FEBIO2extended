// FEStVenantKirchhoff.cpp: implementation of the FEStVenantKirchhoff class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FEStVenantKirchhoff.h"

// define the material parameters
BEGIN_PARAMETER_LIST(FEStVenantKirchhoff, FEElasticMaterial)
	ADD_PARAMETER(m_E, FE_PARAM_DOUBLE, "E");
	ADD_PARAMETER(m_v, FE_PARAM_DOUBLE, "v");
END_PARAMETER_LIST();

//////////////////////////////////////////////////////////////////////
// FEStVenantKirchhoff
//////////////////////////////////////////////////////////////////////

void FEStVenantKirchhoff::Init()
{
	// intialize base class
	FEElasticMaterial::Init();

	if (m_E <= 0) throw MaterialError("Invalid value for E");
	if (!IN_RIGHT_OPEN_RANGE(m_v, -1.0, 0.5)) throw MaterialError("Invalid value for v");
}

//-----------------------------------------------------------------------------
mat3ds FEStVenantKirchhoff::Stress(FEMaterialPoint& mp)
{
	FEElasticMaterialPoint& pt = *mp.ExtractData<FEElasticMaterialPoint>();

	// lame parameters
	double lam = m_v*m_E/((1+m_v)*(1-2*m_v));
	double mu  = 0.5*m_E/(1+m_v);

	// calculate left Cauchy-Green tensor (ie. b-matrix)
	mat3ds b = pt.LeftCauchyGreen();
	mat3ds b2 = b*b;

	// calculate trace of Green-Lagrance strain tensor
	double trE = 0.5*(b.tr()-3);

	// inverse jacobian
	double Ji = 1.0 / pt.m_J;

	// calculate stress
	// s = (lam*trE*b - mu*(b2 - b))/J;
	return b*(lam*trE*Ji) + (b2 - b)*(mu*Ji);
}

//-----------------------------------------------------------------------------
tens4ds FEStVenantKirchhoff::Tangent(FEMaterialPoint& mp)
{
	FEElasticMaterialPoint& pt = *mp.ExtractData<FEElasticMaterialPoint>();

	// jacobian
	double J = pt.m_J;

	// lame parameters
	double lam = m_v*m_E/((1+m_v)*(1-2*m_v));
	double mu  = 0.5*m_E/(1+m_v);

	double lam1 = lam / J;
	double mu1  = mu / J;

	// left cauchy-green matrix (i.e. the 'b' matrix)
	mat3ds b = pt.LeftCauchyGreen();

	tens4ds c = dyad1s(b)*lam1 + dyad4s(b)*(2.0*mu1);

	return c;
}
