// FETransIsoVerondaWestmann.h: interface for the FETransIsoVerondaWestmann class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FETRANSISOVERONDAWESTMANN_H__0FDCFE28_F8ED_4E54_A70E_A8877038CE15__INCLUDED_)
#define AFX_FETRANSISOVERONDAWESTMANN_H__0FDCFE28_F8ED_4E54_A70E_A8877038CE15__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FETransverselyIsotropic.h"

//-----------------------------------------------------------------------------
//! Transversely Isotropic Veronda-Westmann material

//! This material has an isotopric Veronda-Westmann basis and single preferred
//! fiber direction.

class FETransIsoVerondaWestmann : public FETransverselyIsotropic
{
public:
	FETransIsoVerondaWestmann (FEModel* pfem) : FETransverselyIsotropic(pfem) {}

public:
	double	m_c1;	//!< Veronda-Westmann coefficient C1
	double	m_c2;	//!< Veronda-Westmann coefficient C2

public:
	//! calculate deviatoric stress at material point
	virtual mat3ds DevStress(FEMaterialPoint& pt);

	//! calculate deviatoric tangent stiffness at material point
	virtual tens4ds DevTangent(FEMaterialPoint& pt);

	// declare parameter list
	DECLARE_PARAMETER_LIST();
};

#endif // !defined(AFX_FETRANSISOVERONDAWESTMANN_H__0FDCFE28_F8ED_4E54_A70E_A8877038CE15__INCLUDED_)
