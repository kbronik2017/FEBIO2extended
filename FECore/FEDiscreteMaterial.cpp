#include "stdafx.h"
#include "FEDiscreteMaterial.h"

//=============================================================================
FEMaterialPoint* FEDiscreteMaterialPoint::Copy()
{
	FEDiscreteMaterialPoint* pt = new FEDiscreteMaterialPoint(*this);
	if (m_pt) pt->m_pt = m_pt->Copy();
	return pt;
}

//-----------------------------------------------------------------------------
void FEDiscreteMaterialPoint::Serialize(DumpFile& ar)
{
	if (m_pt) m_pt->Serialize(ar);
}

//-----------------------------------------------------------------------------
void FEDiscreteMaterialPoint::ShallowCopy(DumpStream& dmp, bool bsave)
{
	if (m_pt) m_pt->ShallowCopy(dmp, bsave);
}

//-----------------------------------------------------------------------------
void FEDiscreteMaterialPoint::Init(bool bflag)
{
	if (m_pt) m_pt->Init(bflag);
}

//=============================================================================
FEDiscreteMaterial::FEDiscreteMaterial(FEModel* pfem) : FEMaterial(pfem) {}

//-----------------------------------------------------------------------------
FEMaterialPoint* FEDiscreteMaterial::CreateMaterialPointData() { return new FEDiscreteMaterialPoint; }
