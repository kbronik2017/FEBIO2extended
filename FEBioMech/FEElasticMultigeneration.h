#pragma once
#include "FEElasticMaterial.h"

//-----------------------------------------------------------------------------
//! Material defining a single generation of a multi-generation material
class FEGenerationMaterial : public FEElasticMaterial
{
public:
	FEGenerationMaterial(FEModel* pfem) : FEElasticMaterial(pfem){}

	//! initialization
	void Init();

	//! calculate stress at material point
	mat3ds Stress(FEMaterialPoint& pt);
		
	//! calculate tangent stiffness at material point
	tens4ds Tangent(FEMaterialPoint& pt);

public:
	// get the number of properties
	int Properties() { return 1; }

	//! return a material property
	FECoreBase* GetProperty(int i) { return m_pMat; }

	//! find a material property index ( returns <0 for error)
	int FindPropertyIndex(const char* szname);

	//! set a material property (returns false on error)
	bool SetProperty(int i, FECoreBase* pm);

public:
	double	btime;	//!< generation birth time

	FEElasticMaterial*	m_pMat;	//!< pointer to elastic material

	DECLARE_PARAMETER_LIST();
};

//-----------------------------------------------------------------------------
// forward declaration of material class
class FEElasticMultigeneration;

//-----------------------------------------------------------------------------
//! Multigenerational material point.
//! First generation exists at t=0. Second, third, etc. generations appear at t>0.
//! This material point stores the inverse of the relative deformation gradient of
//! second, third, etc. generations.  These relate the reference configuration of 
//! each generation relative to the first generation.

class FEMultigenerationMaterialPoint : public FEMaterialPoint
{
public:
	FEMultigenerationMaterialPoint(FEElasticMultigeneration* pm, FEMaterialPoint* pt) : m_pmat(pm), FEMaterialPoint(pt) { m_tgen = 0.0; }
		
	FEMaterialPoint* Copy();
		
	void Serialize(DumpFile& ar);

	void ShallowCopy(DumpStream& dmp, bool bsave);
		
	void Init(bool bflag);
		
public:
	// multigenerational material data
	vector <mat3d> Fi;	//!< inverse of relative deformation gradient
	vector <double> Ji;	//!< determinant of Fi (store for efficiency)
	double	m_tgen;		//!< last generation time

private:
	FEElasticMultigeneration*	m_pmat;
};

//-----------------------------------------------------------------------------
//! Multigenerational solid

class FEElasticMultigeneration : public FEElasticMaterial
{
public:
	FEElasticMultigeneration(FEModel* pfem) : FEElasticMaterial(pfem) {}
		
	// returns a pointer to a new material point object
	FEMaterialPoint* CreateMaterialPointData() 
	{ 
		// use the zero-th generation material point as the base elastic material point
		return new FEMultigenerationMaterialPoint(this, m_MG[0]->CreateMaterialPointData());
	}

	void AddMaterial(FEElasticMaterial* pmat);
	
public:
	//! return material properties
	int Properties() { return (int) m_MG.size(); }

	//! return a material property
	FECoreBase* GetProperty(int i) { return m_MG[i]; }

	//! find a material property index ( returns <0 for error)
	int FindPropertyIndex(const char* szname);

	//! set a material property (returns false on error)
	bool SetProperty(int i, FECoreBase* pm);

public:
	//! calculate stress at material point
	mat3ds Stress(FEMaterialPoint& pt);
		
	//! calculate tangent stiffness at material point
	tens4ds Tangent(FEMaterialPoint& pt);
		
	//! data initialization and checking
	void Init();
		
	int CheckGeneration(const double t);

public:
	vector<FEGenerationMaterial*>	m_MG;		//!< multigeneration data

	// declare the parameter list
//	DECLARE_PARAMETER_LIST();
};
