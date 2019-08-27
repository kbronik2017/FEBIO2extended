#pragma once
#include "FEBioMech/FEElasticMaterial.h"
#include "FEHydraulicPermeability.h"
#include "FESolventSupply.h"

//-----------------------------------------------------------------------------
//! Biphasic material point class.
//
class FEBiphasicMaterialPoint : public FEMaterialPoint
{
public:
	//! constructor
	FEBiphasicMaterialPoint(FEMaterialPoint* ppt);

	//! create a shallow copy
	FEMaterialPoint* Copy();

	//! data serialization
	void Serialize(DumpFile& ar);

	//! shallow copy
	void ShallowCopy(DumpStream& dmp, bool bsave);

	//! Data initialization
	void Init(bool bflag);

public:
	// poro-elastic material data
	// The actual fluid pressure is the same as the effective fluid pressure
	// in a poroelastic material without solute(s).  The actual fluid pressure
	// is included here so that models that include both poroelastic and
	// solute-poroelastic domains produce plotfiles with consistent fluid
	// pressure fields.
	double		m_p;		//!< fluid pressure
	vec3d		m_gradp;	//!< spatial gradient of p
	vec3d		m_w;		//!< fluid flux
	double		m_pa;		//!< actual fluid pressure
	double		m_phi0;		//!< referential solid volume fraction at current time
	double		m_phi0p;	//!< referential solid volume fraction at previous time
	double		m_phi0hat;	//!< referential solid volume fraction supply at current time
};

//-----------------------------------------------------------------------------
//! Base class for biphasic materials.

class FEBiphasic : public FEMaterial
{
public:
	FEBiphasic(FEModel* pfem);
	
	// returns a pointer to a new material point object
	FEMaterialPoint* CreateMaterialPointData();

	// Get the elastic component (overridden from FEMaterial)
	FEElasticMaterial* GetElasticMaterial() { return m_pSolid->GetElasticMaterial(); }

	// find a material parameter
	FEParam* GetParameter(const ParamString& s);
	
public:
	void Init();
	
	//! calculate stress at material point
	mat3ds Stress(FEMaterialPoint& pt);
	
	//! calculate tangent stiffness at material point
	tens4ds Tangent(FEMaterialPoint& pt);

	//! return the permeability tensor as a matrix
	void Permeability(double k[3][3], FEMaterialPoint& pt);

	//! return the permeability as a tensor
	mat3ds Permeability(FEMaterialPoint& pt);

	//! return the permeability property
	FEHydraulicPermeability* GetPermeability() { return m_pPerm; }
	
	//! calculate fluid flux
	vec3d Flux(FEMaterialPoint& pt);
	
	//! calculate actual fluid pressure
	double Pressure(FEMaterialPoint& pt);

	//! porosity
	double Porosity(FEMaterialPoint& pt);
	
	//! fluid density
	double FluidDensity() { return m_rhoTw; } 

	//! get the solvent supply
	double SolventSupply(FEMaterialPoint& mp) { return (m_pSupp? m_pSupp->Supply(mp) : 0); }

	//! get the solvent supply property
	FESolventSupply* GetSolventSupply() { return m_pSupp; }

	//! Serialization
	void Serialize(DumpFile& ar);

public:
	//! return number of material properties
	int Properties();

	//! return a material property
	FECoreBase* GetProperty(int n);

	//! find a material property index ( returns <0 for error)
	int FindPropertyIndex(const char* szname);

	//! set a material property (returns false on error)
	bool SetProperty(int i, FECoreBase* pm);

public: // material parameters
	double						m_rhoTw;	//!< true fluid density
	double						m_phi0;		//!< solid volume fraction in reference configuration

private: // material properties
	FEElasticMaterial*			m_pSolid;	//!< pointer to elastic solid material
	FEHydraulicPermeability*	m_pPerm;	//!< pointer to permeability material
	FESolventSupply*			m_pSupp;	//!< pointer to solvent supply
	
	DECLARE_PARAMETER_LIST();
};
