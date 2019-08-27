#include "stdafx.h"
#include "FEElemElemList.h"
#include "FENodeElemList.h"
#include "FESolidDomain.h"

//-----------------------------------------------------------------------------
FEElemElemList::FEElemElemList(void)
{
}

//-----------------------------------------------------------------------------
FEElemElemList::~FEElemElemList(void)
{
}

//-----------------------------------------------------------------------------
void FEElemElemList::Init()
{
	FEMesh& m = *m_pmesh;

	// allocate storage
	int NE = m.Elements();
	m_ref.resize(NE);

	// count nr of neighbors
	int NN = 0, n = 0, nf;
	m_ref[0] = 0;
	for (int i=0; i<m.Domains(); ++i)
	{
		FEDomain& dom = m.Domain(i);
		for (int j=0; j<dom.Elements(); ++j, ++n)
		{
			FEElement& el = dom.ElementRef(j);
			nf = m.Faces(el);
			if (n != 0) m_ref[n] = m_ref[n-1] + nf;
			NN += nf;
		}
	}

	m_pel.resize(NN);

	// TODO: do this for shells as well (if we have to)
}

//-----------------------------------------------------------------------------
void FEElemElemList::Create(FEMesh* pmesh)
{
	// store a pointer to the mesh
	m_pmesh = pmesh;
	FEMesh& m = *m_pmesh;

	// initialize data structures
	Init();

	// create the node element list
	FENodeElemList NEL;
	NEL.Create(m);

	// loop over all solid elements first
	int en0[FEElement::MAX_NODES], en1[FEElement::MAX_NODES], n0, n1, M = 0;
	int nf0, nf1;
	for (int nd=0; nd<m.Domains(); ++nd)
	{
		FEDomain& dom = m.Domain(nd);
		for (int i=0; i<dom.Elements(); ++i)
		{
			FEElement& el = dom.ElementRef(i);
			
			// get the number of neighbors
			nf0 = m.Faces(el);

			// loop over all neighbors
			for (int j=0; j<nf0; ++j, ++M)
			{
				// get the face nodes
				n0 = m.GetFace(el, j, en0);

				// find the neighbor element
				m_pel[M] = 0;

				// loop over all possible candidates
				int nval = NEL.Valence(en0[0]);
				FEElement** pne = NEL.ElementList(en0[0]);
				for (int k=0; k<nval; ++k)
				{
					// make sure we don't compare the current element
					if (pne[k] != &el)
					{
						// get the number of faces
						nf1 = m.Faces(*pne[k]);

						// see if any of these faces match en0
						for (int l=0; l<nf1; ++l)
						{
							n1 = m.GetFace(*pne[k], l, en1);

							// make sure the faces have the same nr of nodes
							if (n1 == n0)
							{
								// check triangles
								if ((n0 == 3) || (n0 == 6))
								{
									if (((en0[0] == en1[0]) || (en0[0] == en1[1]) || (en0[0] == en1[2])) &&
										((en0[1] == en1[0]) || (en0[1] == en1[1]) || (en0[1] == en1[2])) &&
										((en0[2] == en1[0]) || (en0[2] == en1[1]) || (en0[2] == en1[2])))
									{
										// found it!
										m_pel[M] = pne[k];
										break;
									}
								}
								// check quads
								else if (n0 == 4)
								{
									if (((en0[0] == en1[0]) || (en0[0] == en1[1]) || (en0[0] == en1[2]) || (en0[0] == en1[3])) &&
										((en0[1] == en1[0]) || (en0[1] == en1[1]) || (en0[1] == en1[2]) || (en0[1] == en1[3])) &&
										((en0[2] == en1[0]) || (en0[2] == en1[1]) || (en0[2] == en1[2]) || (en0[2] == en1[3])) &&
										((en0[3] == en1[0]) || (en0[3] == en1[1]) || (en0[3] == en1[2]) || (en0[3] == en1[3])))
									{
										// found it!
										m_pel[M] = pne[k];
										break;
									}
								}
							}

							if (m_pel[M] != 0) break;
						}
					}
				}
			}
		}
	}

	// TODO: do the same for shells
}
