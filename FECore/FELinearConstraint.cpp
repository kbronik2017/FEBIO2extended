#include "stdafx.h"
#include "FELinearConstraint.h"

//-----------------------------------------------------------------------------
FELinearConstraint::FELinearConstraint(const FELinearConstraint& LC)
{
	master = LC.master;
	int n = (int) LC.slave.size();
	list<SlaveDOF>::const_iterator it = LC.slave.begin();
	for (int i=0; i<n; ++i) slave.push_back(*it);
}

//-----------------------------------------------------------------------------
double FELinearConstraint::FindDOF(int n)
{
	int N = slave.size();
	list<SlaveDOF>::iterator it = slave.begin();
	for (int i=0; i<N; ++i, ++it) if (it->neq == n) return it->val;

	return 0;
}

//-----------------------------------------------------------------------------
void FELinearConstraint::Serialize(DumpFile& ar)
{
	if (ar.IsSaving())
	{
		ar.write(&master, sizeof(DOF), 1);
		int n = (int) slave.size();
		ar << n;
		list<SlaveDOF>::iterator it = slave.begin();
		for (int i=0; i<n; ++i, ++it) ar << it->val << it->node << it->bc << it->neq;
	}
	else
	{
		slave.clear();
		ar.read(&master, sizeof(DOF), 1);
		int n;
		ar >> n;
		for (int i=0; i<n; ++i)
		{
			SlaveDOF dof;
			ar >> dof.val >> dof.node >> dof.bc >> dof.neq;
			slave.push_back(dof);
		}
	}
}
