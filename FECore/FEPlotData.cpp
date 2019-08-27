#include "stdafx.h"
#include "FEPlotData.h"
#include "FEModel.h"


//-----------------------------------------------------------------------------
int FEPlotData::VarSize(Var_Type t)
{
	int ndata = 0;
	switch (DataType())
	{
	case PLT_FLOAT: ndata = 1; break;
	case PLT_VEC3F: ndata = 3; break;
	case PLT_MAT3FS: ndata = 6; break;
	case PLT_MAT3FD: ndata = 3; break;
    case PLT_TENS4FS: ndata = 21; break;
	}
	assert(ndata);
	return ndata;
}

//-----------------------------------------------------------------------------
void FENodeData::Save(FEModel &fem, Archive& ar)
{
	// loop over all node sets
	// write now there is only one, namely the master node set
	// so we just pass the mesh
	int ndata = VarSize(DataType());

	int N = fem.GetMesh().Nodes();
	vector<float> a; a.reserve(ndata*N);
	if (Save(fem.GetMesh(), a))
	{
		assert(a.size() == N*ndata);
		ar.WriteChunk(0, a);
	}
}

//-----------------------------------------------------------------------------
void FEDomainData::Save(FEModel &fem, Archive& ar)
{
	FEMesh& m = fem.GetMesh();
	int ND = m.Domains();

	// if the item list is empty, store all domains
	if (m_item.empty())
	{
		for (int i=0; i<ND; ++i) m_item.push_back(i);
	}

	// loop over all domains in the item list
	int N = m_item.size();
	for (int i=0; i<ND; ++i)
	{
		// get the domain
		FEDomain& D = m.Domain(m_item[i]);

		// calculate the size of the data vector
		int nsize = VarSize(DataType());
		switch (m_sfmt)
		{
		case FMT_NODE: nsize *= D.Nodes(); break;
		case FMT_ITEM: nsize *= D.Elements(); break;
		case FMT_MULT:
			{
				// since all elements have the same type within a domain
				// we just grab the number of nodes of the first element 
				// to figure out how much storage we need
				FEElement& e = D.ElementRef(0);
				int n = e.Nodes();
				nsize *= n*D.Elements();
			}
			break;
		default:
			assert(false);
		}
		assert(nsize > 0);

		// fill data vector and save
		vector<float> a; 
		a.reserve(nsize);
		if (Save(D, a))
		{
			assert(a.size() == nsize);
			ar.WriteChunk(m_item[i]+1, a);
		}
	}
}

//-----------------------------------------------------------------------------
//! Save surface data
//! \todo For the FMT_MULT option we are assuming 8 values per facet. I need to
//! make sure that the FEBioPlot assumes as many values.
void FESurfaceData::Save(FEModel &fem, Archive& ar)
{
	// loop over all surfaces
	FEMesh& m = fem.GetMesh();
	int NS = m.Surfaces();
	for (int i=0; i<NS; ++i)
	{
		FESurface& S = m.Surface(i);

		// Determine data size.
		// Note that for the FMT_MULT case we are 
		// assuming 8 data entries per facet
		// regardless of the nr of nodes a facet really has
		// this is because for surfaces, all elements are not
		// necessarily of the same type
		// TODO: Fix the assumption of the FMT_MULT
		int nsize = VarSize(DataType());
		switch (m_sfmt)
		{
		case FMT_NODE: nsize *= S.Nodes(); break;
		case FMT_ITEM: nsize *= S.Elements(); break;
		case FMT_MULT: nsize *= 8*S.Elements(); break;
		default:
			assert(false);
		}

		// save data
		vector<float> a; a.reserve(nsize);
		if (Save(S, a))
		{
			assert(a.size() == nsize);
			ar.WriteChunk(i+1, a);
		}
	}
}
