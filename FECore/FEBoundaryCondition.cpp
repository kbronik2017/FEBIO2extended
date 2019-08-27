#include "stdafx.h"
#include "FEBoundaryCondition.h"
using namespace FECore;

int FEBoundaryCondition::m_ncount = 0;

FEBoundaryCondition::FEBoundaryCondition(SUPER_CLASS_ID sid, FEModel* pfem) : FEModelComponent(sid, pfem)
{
	m_nID = m_ncount++;
}

void FEBoundaryCondition::SetID(int nid)
{
	m_nID = nid;
	if (nid > m_ncount) m_ncount = nid+1;
}
