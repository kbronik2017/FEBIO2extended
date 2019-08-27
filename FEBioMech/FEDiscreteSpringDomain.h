#pragma once
#include "FECore/FEDiscreteDomain.h"
#include "FEElasticDomain.h"

//-----------------------------------------------------------------------------
//! domain for discrete elements
class FEDiscreteSpringDomain : public FEDiscreteDomain, public FEElasticDomain
{
public:
	//! constructor
	FEDiscreteSpringDomain(FEMesh* pm, FEMaterial* pmat) : FEDiscreteDomain(FE_DISCRETE_DOMAIN, pm, pmat) {}

	//! Unpack LM data
	void UnpackLM(FEElement& el, vector<int>& lm);

public: // overridden from FEElasticDomain

	//! calculate stiffness matrix
	void StiffnessMatrix(FESolver* psolver);
	void MassMatrix(FESolver* psolver, double scale) {}
	void BodyForceStiffness  (FESolver* psolver, FEBodyForce& bf) {}

	//! Calculates inertial forces for dynamic problems
	void InertialForces(FEGlobalVector& R, vector<double>& F) { assert(false); }

	//! calculate residual
//	void Residual(FEGlobalVector& R);

	//! update stresses (not used for discrete springs)
	void UpdateStresses(FEModel& fem){}	
	void ConductionMatrix(FESolver* pnls){}
	void  CapacitanceMatrix(FESolver* pnls, double dt) {}
	void HeatResidual(FEGlobalVector& R) {}
	void ConvectiveHeatStiffnessMatrix(FESolver* psolver) {}
	//void ConvectiveHeatElementStiffness(FESurfaceElement& el, matrix& ke, double hc){}
	//! internal stress forces
	void InternalForces(FEGlobalVector& R);

	//! calculate bodyforces (not used since springs are considered mass-less)
	void BodyForce(FEGlobalVector& R, FEBodyForce& bf) {}
};
