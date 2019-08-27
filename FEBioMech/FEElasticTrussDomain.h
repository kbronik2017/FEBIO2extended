#pragma once
#include "FECore/FETrussDomain.h"
#include "FEElasticDomain.h"

//-----------------------------------------------------------------------------
//! Domain described by 3D truss elements
class FEElasticTrussDomain : public FETrussDomain, public FEElasticDomain
{
public:
	//! Constructor
	FEElasticTrussDomain(FEMesh* pm, FEMaterial* pmat) : FETrussDomain(FE_TRUSS_DOMAIN, pm, pmat) {}

	//! copy operator
	FEElasticTrussDomain& operator = (FEElasticTrussDomain& d) { m_Elem = d.m_Elem; m_pMesh = d.m_pMesh; return (*this); }

	//! Reset data
	void Reset();

	//! Initialize elements
	void InitElements();

	//! Unpack truss element data
	void UnpackLM(FEElement& el, vector<int>& lm);

public: // overloads from FEElasticDomain

	//! update the truss stresses
	void UpdateStresses(FEModel& fem);

	//! calculates the residual
//	void Residual(FEGlobalVector& R);

	//! internal stress forces
	void InternalForces(FEGlobalVector& R);

	//! calculate body force \todo implement this
	void BodyForce(FEGlobalVector& R, FEBodyForce& bf) { assert(false); }

	//! Calculates inertial forces for dynamic problems
	void InertialForces(FEGlobalVector& R, vector<double>& F) { assert(false); }

	//! calculates the global stiffness matrix for this domain
	void StiffnessMatrix(FESolver* psolver);

	void ConductionMatrix(FESolver* pnls){};
	void  CapacitanceMatrix(FESolver* pnls, double dt) {};
	void HeatResidual(FEGlobalVector& R) {};
	void ConvectiveHeatStiffnessMatrix(FESolver* psolver) {};
	//! intertial stiffness matrix \todo implement this
	void MassMatrix(FESolver* psolver, double scale) { assert(false); }

	//! body force stiffness matrix \todo implement this
	void BodyForceStiffness(FESolver* psolver, FEBodyForce& bf) { assert(false); }

protected:
	//! calculates the truss element stiffness matrix
	void ElementStiffness(int iel, matrix& ke);

	//! Calculates the internal stress vector for solid elements
	void ElementInternalForces(FETrussElement& el, vector<double>& fe);
};
