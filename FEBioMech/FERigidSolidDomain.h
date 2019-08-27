#pragma once
#include "FEElasticSolidDomain.h"

//-----------------------------------------------------------------------------
//! domain class for 3D rigid elements
//!
class FERigidSolidDomain : public FEElasticSolidDomain
{
public:
	//! constructor
	FERigidSolidDomain(FEMesh* pm, FEMaterial* pmat) : FEElasticSolidDomain(pm, pmat) { m_ntype = FE_RIGID_SOLID_DOMAIN; }

	//! Initialize
	bool Initialize(FEModel& fem);

	//! reset data
	void Reset();

public:

	//! calculates the global stiffness matrix for this domain
	void StiffnessMatrix(FESolver* psolver);

	//! calculates the residual (nothing to do)
	void InternalForces(FESolver* psolver, vector<double>& R);

	// update stresses (nothing to do)
	void UpdateStresses(FEModel& fem);
};
