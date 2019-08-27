// FERigid.h: interface for the FERigid class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FERIGID_H__42385DA7_ECE1_4862_B6E1_EFE5B4D4CC4B__INCLUDED_)
#define AFX_FERIGID_H__42385DA7_ECE1_4862_B6E1_EFE5B4D4CC4B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FEElasticMaterial.h"

//-----------------------------------------------------------------------------
//! Rigd body material data

//! Since rigid elements are skipped during the stiffness and residual calculations
//! we don't implement the Stress and Tangent functions
//! \todo make the m_rc a parameter
//! \todo Can I remove the m_bc variable?

class FERigidMaterial : public FESolidMaterial
{
public:
	FERigidMaterial(FEModel* pfem);

public:
	double	m_E;		//!< Young's modulus
	double	m_v;		//!< Poisson's ratio
	int		m_pmid;		//!< parent material ID

public:
	int		m_com;	//!< center of mass input flag
	vec3d	m_rc;	//!< center of mass

public:
	// inherited from FEMaterial
	virtual bool IsRigid() { return true; }

	// override this function to set the COM logic
	void SetParameter(FEParam& p);

public:
	//! Create a rigid material point
	FEMaterialPoint* CreateMaterialPointData() { return new FEElasticMaterialPoint(); }

	//! calculate stress at material point
	virtual mat3ds Stress(FEMaterialPoint& pt){ return mat3ds(); }

	//! calculate tangent stiffness at material point
	virtual tens4ds Tangent(FEMaterialPoint& pt) { return tens4ds(); }

	//! data initialization
	void Init();

	//! serialization
	void Serialize(DumpFile& ar);

	// declare a parameter list
	DECLARE_PARAMETER_LIST();

private:
	bool	m_binit;	//!< flag for first initialization
};

#endif // !defined(AFX_FERIGID_H__42385DA7_ECE1_4862_B6E1_EFE5B4D4CC4B__INCLUDED_)
