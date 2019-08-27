#pragma once
#include "FEBodyForce.h"
#include "FECore/FESolver.h"
#include "FECore/FEModel.h"
#include <vector>
using namespace std;

//-----------------------------------------------------------------------------
//! Abstract interface class for elastic domains.

//! An elastic domain is used by the structural mechanics solver.
//! This interface defines the functions that have to be implemented by an
//! elastic domain. There are basically two categories: residual functions
//! that contribute to the global residual vector. And stiffness matrix 
//! function that calculate contributions to the global stiffness matrix.
class FEElasticDomain
{
public:
	virtual ~FEElasticDomain(){}

	//! Updates the element stresses
	virtual void UpdateStresses(FEModel& fem) = 0;

	// --- R E S I D U A L ---

	// calculate total residual (TODO: remove this)
//	virtual void Residual(FESolver* psolver, vector<double>& R) = 0;

	//! calculate the internal forces
	virtual void InternalForces(FEGlobalVector& R) = 0;

	//! Calculate the body force vector
	virtual void BodyForce(FEGlobalVector& R, FEBodyForce& bf) = 0;

	//! calculate the interial forces (for dynamic problems)
	virtual void InertialForces(FEGlobalVector& R, vector<double>& F) = 0;

	virtual  void ConductionMatrix(FESolver* pnls) = 0;
	virtual  void  CapacitanceMatrix(FESolver* pnls, double dt) = 0;
	virtual void HeatResidual(FEGlobalVector& R)=0;
	// --- S T I F F N E S S   M A T R I X ---

	//! Calculate global stiffness matrix (only contribution from internal force derivative)
	//! \todo maybe I should rename this the InternalStiffness matrix?
	virtual void StiffnessMatrix   (FESolver* psolver) = 0;

	//! Calculate stiffness contribution of body forces
	virtual void BodyForceStiffness(FESolver* psolver, FEBodyForce& bf) = 0;
	virtual  void ConvectiveHeatStiffnessMatrix(FESolver* psolver)=0;
	//! calculate the mass matrix (for dynamic problems)
	virtual void MassMatrix(FESolver* psolver, double scale) = 0;
};
