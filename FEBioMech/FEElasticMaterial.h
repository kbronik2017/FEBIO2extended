#pragma once
#include "FESolidMaterial.h"
#include <algorithm>
//-----------------------------------------------------------------------------
//! This class defines material point data for elastic materials.
class FEElasticMaterialPoint : public FEMaterialPoint
{
public:
	//! constructor
	FEElasticMaterialPoint();

	//! Initialize material point data
	void Init(bool bflag);

	//! create a shallow copy
	FEMaterialPoint* Copy();

	//! serialize material point data
	void Serialize(DumpFile& ar);

	//! stream material point data
	void ShallowCopy(DumpStream& dmp, bool bsave);

public:
	mat3ds Strain();
	mat3ds SmallStrain();

	mat3ds RightCauchyGreen();
	mat3ds LeftCauchyGreen ();

	mat3ds DevRightCauchyGreen();
	mat3ds DevLeftCauchyGreen ();

	mat3ds pull_back(const mat3ds& A);
	mat3ds push_forward(const mat3ds& A);

	tens4ds pull_back(const tens4ds& C);
	tens4ds push_forward(const tens4ds& C);

public:
	// position 
	vec3d	m_r0;	//!< material position
	vec3d	m_rt;	//!< spatial position

	// deformation data
	mat3d	m_F;	//!< deformation gradient
	double	m_J;	//!< determinant of F
	mat3d	m_Q;	//!< local material orientation

	// solid material data
	mat3ds		m_s;		//!< Cauchy stress
	mat3ds		m_s0;		//!< Initial stress (only used by linear solid solver)
};

//-----------------------------------------------------------------------------
//! Base class for (hyper-)elastic materials

class FEElasticMaterial : public FESolidMaterial
{
public:
	//! constructor 
	FEElasticMaterial(FEModel* pfem);

	//! destructor
	~FEElasticMaterial();

	//! Initialization
	void Init();

	//! Serialization
	void Serialize(DumpFile& ar);

	//! create material point data for this material
	virtual FEMaterialPoint* CreateMaterialPointData() { return new FEElasticMaterialPoint; }

	//! Get the elastic component
	FEElasticMaterial* GetElasticMaterial() { return this; }

public:
	bool SetAttribute(const char* szatt, const char* szval);

	DECLARE_PARAMETER_LIST();
};
