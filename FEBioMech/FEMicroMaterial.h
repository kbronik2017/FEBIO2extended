#pragma once
#include "FEElasticMaterial.h"
#include "FECore/FEModel.h"
#include "FECore/FEMaterial.h"

//-----------------------------------------------------------------------------
//! The micro-material implements material homogenization. The stress and tangents
//! are calculated by solving a micro-structural RVE problem and return the
//! averaged stress and condensed tangents.
//!
class FEMicroMaterial :	public FEElasticMaterial
{
public:
	FEMicroMaterial(FEModel* pfem);
	~FEMicroMaterial(void);

public:
	char	m_szrve[256];	//!< filename for RVE file

protected:
	FEModel		m_rve;	//!< the RVE (Representive Volume Element)
	bool		m_brve;	//!< flag indicating whether RVE was read in
	double		m_V0;	//!< initial volume of RVE

public:
	//! calculate stress at material point
	virtual mat3ds Stress(FEMaterialPoint& pt);

	//! calculate tangent stiffness at material point
	virtual tens4ds Tangent(FEMaterialPoint& pt);

	//! data initialization
	void Init();

protected:
	void PrepRVE();
	mat3ds AveragedStress(FEMaterialPoint& pt);

public:
	// declare the parameter list
	DECLARE_PARAMETER_LIST();
};
