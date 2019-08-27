#pragma once
#include "FECore/FENLConstraint.h"
#include "FECore/FEElement.h"
#include "FECore/FEGlobalVector.h"

//-----------------------------------------------------------------------------
//! This class implements a point constraint. That is, it forces a node of a 
//! mesh in the same relative position with respect to the element in which the 
//! node is located.
class FEPointConstraint : public FENLConstraint
{
public:
	//! constructor
	FEPointConstraint(FEModel* pfem);

	//! initialize data
	bool Init();

	//! serialize \todo Implement this
	void Serialize(DumpFile& ar) {}

	//! Calculate the constraint force
	void Residual(FEGlobalVector& R);

	//! calculate the constraint stiffness
	void StiffnessMatrix(FESolver* psolver);

	//! augmentations \todo implement this
	bool Augment(int naug) { return true; }

public:
	double		m_eps;		//!< penalty parameter
	int			m_node_id;	//!< id of master node

public:
	int					m_node;		//!< node to which the constraint is applied
	FESolidElement*		m_pel;		//!< element in which the node is located.
	double				m_rs[3];	//!< natural coordinates in element m_pel

	// declare parameter list
	DECLARE_PARAMETER_LIST();
};
