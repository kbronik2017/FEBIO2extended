#pragma once
#include "FEElasticShellDomain.h"

//-----------------------------------------------------------------------------
//! domain class for 3D rigid shell elements
//!
class FERigidShellDomain : public FEElasticShellDomain
{
public:
	//! constructor
	FERigidShellDomain(FEMesh* pm, FEMaterial* pmat) : FEElasticShellDomain(pm, pmat) { m_ntype = FE_RIGID_SHELL_DOMAIN; }

	//! Initialize
	bool Initialize(FEModel& fem);

	//! reset data
	void Reset();

public:
	//! calculates the global stiffness matrix for this domain
	void StiffnessMatrix(FESolver* psolver);

	//! calculates the internal forces (nothing to do)
	void InternalForces(FEGlobalVector& R);

	// update stresses
	void UpdateStresses(FEModel& fem);
};
