#include "stdafx.h"
#include "FEBioPlotFile.h"
#include "FECore/FECoreKernel.h"

//-----------------------------------------------------------------------------
bool FEBioPlotFile::Dictionary::AddVariable(FEModel* pfem, const char* szname, vector<int>& item)
{
	FECoreKernel& febio = FECoreKernel::GetInstance();

	FEPlotData* ps = fecore_new<FEPlotData>(FEPLOTDATA_ID, szname, pfem);
	if (ps)
	{
		ps->SetItemList(item);
		if      (dynamic_cast<FENodeData*   >(ps)) return AddNodalVariable  (ps, szname, item);
		else if (dynamic_cast<FEDomainData* >(ps)) return AddDomainVariable (ps, szname, item);
		else if (dynamic_cast<FESurfaceData*>(ps)) return AddSurfaceVariable(ps, szname, item);
	}
	return false;
}

//-----------------------------------------------------------------------------
bool FEBioPlotFile::Dictionary::AddGlobalVariable(FEPlotData* ps, const char* szname)
{
	return false;
}

//-----------------------------------------------------------------------------
bool FEBioPlotFile::Dictionary::AddMaterialVariable(FEPlotData* ps, const char* szname)
{
	return false;
}

//-----------------------------------------------------------------------------
bool FEBioPlotFile::Dictionary::AddNodalVariable(FEPlotData* ps, const char* szname, vector<int>& item)
{
	if (dynamic_cast<FENodeData*>(ps))
	{
		DICTIONARY_ITEM it;
		it.m_ntype = ps->DataType();
		it.m_nfmt  = ps->StorageFormat();
		it.m_psave = ps;
		strcpy(it.m_szname, szname);
		m_Node.push_back(it);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool FEBioPlotFile::Dictionary::AddDomainVariable(FEPlotData* ps, const char* szname, vector<int>& item)
{
	if (dynamic_cast<FEDomainData*>(ps))
	{
		DICTIONARY_ITEM it;
		it.m_ntype = ps->DataType();
		it.m_nfmt  = ps->StorageFormat();
		it.m_psave = ps;
		strcpy(it.m_szname, szname);
		m_Elem.push_back(it);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool FEBioPlotFile::Dictionary::AddSurfaceVariable(FEPlotData* ps, const char* szname, vector<int>& item)
{
	if (dynamic_cast<FESurfaceData*>(ps))
	{
		DICTIONARY_ITEM it;
		it.m_ntype = ps->DataType();
		it.m_nfmt  = ps->StorageFormat();
		it.m_psave = ps;
		strcpy(it.m_szname, szname);
		m_Face.push_back(it);
		return true;
	}
	return false;
}


//-----------------------------------------------------------------------------
void FEBioPlotFile::Dictionary::Defaults(FEModel& fem)
{
	// First we build the dictionary
	// get the mesh
	FEMesh& m = fem.GetMesh();

	// Define default variables
	if (m_Node.empty() && m_Elem.empty() && m_Face.empty())
	{
		vector<int> l; // empty list
		AddVariable(&fem, "displacement", l);
		AddVariable(&fem, "stress", l);
	}
}

//-----------------------------------------------------------------------------
void FEBioPlotFile::Dictionary::Clear()
{
	m_Glob.clear();
	m_Mat.clear();
	m_Node.clear();
	m_Elem.clear();
	m_Face.clear();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

FEBioPlotFile::FEBioPlotFile(FEModel& fem) : m_fem(fem)
{
}

FEBioPlotFile::~FEBioPlotFile(void)
{
	Close();
}

//-----------------------------------------------------------------------------
void FEBioPlotFile::Close()
{
	m_ar.Close();

	int i;
	list<DICTIONARY_ITEM>::iterator it = m_dic.m_Glob.begin();
	for (i=0; i<(int) m_dic.m_Glob.size(); ++i, ++it) delete it->m_psave;

	it = m_dic.m_Mat.begin();
	for (i=0; i<(int) m_dic.m_Mat.size(); ++i, ++it) delete it->m_psave;

	it = m_dic.m_Node.begin();
	for (i=0; i<(int) m_dic.m_Node.size(); ++i, ++it) delete it->m_psave;

	it = m_dic.m_Elem.begin();
	for (i=0; i<(int) m_dic.m_Elem.size(); ++i, ++it) delete it->m_psave;

	it = m_dic.m_Face.begin();
	for (i=0; i<(int) m_dic.m_Face.size(); ++i, ++it) delete it->m_psave;
}

//-----------------------------------------------------------------------------
bool FEBioPlotFile::Open(FEModel &fem, const char *szfile)
{
	// open the archive
	m_ar.Create(szfile);

	// write the root element
	try
	{
		if (WriteRoot(fem) == false) return false;
	}
	catch (...)
	{
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
bool FEBioPlotFile::WriteRoot(FEModel& fem)
{
	// write the root element
	m_ar.BeginChunk(PLT_ROOT);
	{
		// --- save the header file ---
		m_ar.BeginChunk(PLT_HEADER);
		{
			if (WriteHeader(fem) == false) return false;
		}
		m_ar.EndChunk();

		// --- save the dictionary ---
		m_ar.BeginChunk(PLT_DICTIONARY);
		{
			if (WriteDictionary(fem) == false) return false;
		}
		m_ar.EndChunk();

		// --- save the materials
		m_ar.BeginChunk(PLT_MATERIALS);
		{
			if (WriteMaterials(fem) == false) return false;
		}
		m_ar.EndChunk();

		// --- save the geometry ---
		m_ar.BeginChunk(PLT_GEOMETRY);
		{
			if (WriteGeometry(fem) == false) return false;
		}
		m_ar.EndChunk();
	}
	m_ar.EndChunk();

	return true;
}

//-----------------------------------------------------------------------------
bool FEBioPlotFile::WriteHeader(FEModel& fem)
{
	// setup the header
	unsigned int nversion = PLT_VERSION;

	// output header
	m_ar.WriteChunk(PLT_HDR_VERSION, nversion);

	int N = fem.GetMesh().Nodes();
	m_ar.WriteChunk(PLT_HDR_NODES, N);

	// max number of nodes per facet
	int n = (int) PLT_MAX_FACET_NODES;
	m_ar.WriteChunk(PLT_HDR_MAX_FACET_NODES, n);

	return true;
}

//-----------------------------------------------------------------------------
bool FEBioPlotFile::WriteDictionary(FEModel& fem)
{
	// setup defaults for the dictionary
	m_dic.Defaults(fem);

	// Next, we save the dictionary
	// Global variables
	if (!m_dic.m_Glob.empty())
	{
		m_ar.BeginChunk(PLT_DIC_GLOBAL);
		{
			WriteDicList(m_dic.m_Glob);
		}
		m_ar.EndChunk();
	}

	// store material variables
	if (!m_dic.m_Mat.empty())
	{
		m_ar.BeginChunk(PLT_DIC_MATERIAL);
		{
			WriteDicList(m_dic.m_Mat);
		}
		m_ar.EndChunk();
	}

	// store nodal variables
	if (!m_dic.m_Node.empty())
	{
		m_ar.BeginChunk(PLT_DIC_NODAL);
		{
			WriteDicList(m_dic.m_Node);
		}
		m_ar.EndChunk();
	}

	// store element variables
	if (!m_dic.m_Elem.empty())
	{
		m_ar.BeginChunk(PLT_DIC_DOMAIN);
		{
			WriteDicList(m_dic.m_Elem);
		}
		m_ar.EndChunk();
	}

	// store surface data
	if (!m_dic.m_Face.empty())
	{
		m_ar.BeginChunk(PLT_DIC_SURFACE);
		{
			WriteDicList(m_dic.m_Face);
		}
		m_ar.EndChunk();
	}

	return true;
}

//-----------------------------------------------------------------------------
void FEBioPlotFile::WriteDicList(list<FEBioPlotFile::DICTIONARY_ITEM>& dic)
{
	int N = (int) dic.size();
	list<DICTIONARY_ITEM>::iterator pi = dic.begin();
	for (int i=0; i<N; ++i, ++pi)
	{
		m_ar.BeginChunk(PLT_DIC_ITEM);
		{
			m_ar.WriteChunk(PLT_DIC_ITEM_TYPE, pi->m_ntype);
			m_ar.WriteChunk(PLT_DIC_ITEM_FMT , pi->m_nfmt);
			m_ar.WriteChunk(PLT_DIC_ITEM_NAME, pi->m_szname, STR_SIZE);
		}
		m_ar.EndChunk();
	}
}

//-----------------------------------------------------------------------------
bool FEBioPlotFile::WriteMaterials(FEModel& fem)
{
	int NMAT = fem.Materials();
	for (int i=0; i<NMAT; ++i)
	{
		FEMaterial* pm = fem.GetMaterial(i);
		m_ar.BeginChunk(PLT_MATERIAL);
		{
			unsigned int nid = (unsigned int) pm->GetID();
			char szname[STR_SIZE] = {0};
			strcpy(szname, pm->GetName());
			m_ar.WriteChunk(PLT_MAT_ID, nid);
			m_ar.WriteChunk(PLT_MAT_NAME, szname, STR_SIZE);
		}
		m_ar.EndChunk();
	}
	return true;
}

//-----------------------------------------------------------------------------
bool FEBioPlotFile::WriteGeometry(FEModel& fem)
{
	// get the mesh
	FEMesh& m = fem.GetMesh();

	// node section
	m_ar.BeginChunk(PLT_NODE_SECTION);
	{
		WriteNodeSection(m);
	}
	m_ar.EndChunk();

	// domain section
	m_ar.BeginChunk(PLT_DOMAIN_SECTION);
	{
		WriteDomainSection(m);
	}
	m_ar.EndChunk();

	// surface section
	if (m.Surfaces() > 0)
	{
		m_ar.BeginChunk(PLT_SURFACE_SECTION);
		{
			WriteSurfaceSection(m);
		}
		m_ar.EndChunk();
	}

	return true;
}

//-----------------------------------------------------------------------------
void FEBioPlotFile::WriteNodeSection(FEMesh& m)
{
	// write the material coordinates
	int NN = m.Nodes();
	vector<float> X(3*NN);
	for (int i=0; i<m.Nodes(); ++i)
	{
		FENode& node = m.Node(i);
		X[3*i  ] = (float) node.m_r0.x;
		X[3*i+1] = (float) node.m_r0.y;
		X[3*i+2] = (float) node.m_r0.z;
	}

	m_ar.WriteChunk(PLT_NODE_COORDS, X);
}

//-----------------------------------------------------------------------------
void FEBioPlotFile::WriteDomainSection(FEMesh& m)
{
	// write all domains
	for (int nd = 0; nd<m.Domains(); ++nd)
	{
		m_ar.BeginChunk(PLT_DOMAIN);
		{
			FESolidDomain* pbd = dynamic_cast<FESolidDomain*>(&m.Domain(nd));
			if (pbd) WriteSolidDomain(*pbd);

			FEShellDomain* psd = dynamic_cast<FEShellDomain*>(&m.Domain(nd));
			if (psd) WriteShellDomain(*psd);

			FETrussDomain* ptd = dynamic_cast<FETrussDomain*>(&m.Domain(nd));
			if (ptd) WriteTrussDomain(*ptd);

			FEDiscreteDomain* pdd = dynamic_cast<FEDiscreteDomain*>(&m.Domain(nd));
			if (pdd) WriteDiscreteDomain(*pdd);
		}
		m_ar.EndChunk();
	}
}

//-----------------------------------------------------------------------------
void FEBioPlotFile::WriteSolidDomain(FESolidDomain& dom)
{
	int mid = dom.GetMaterial()->GetID();
	assert(mid > 0);
	int etype = dom.GetElementType();

	int i, j;
	int NE = dom.Elements();

	// figure out element type
	int ne = 0;
	int dtype = 0;
	switch (etype)
	{
		case FE_HEX8G8  :
		case FE_HEX8RI  :
		case FE_HEX8G1  : ne = 8; dtype = PLT_ELEM_HEX; break;
		case FE_PENTA6G6: ne = 6; dtype = PLT_ELEM_PENTA; break;
		case FE_TET4G4  :
		case FE_TET4G1  : ne = 4; dtype = PLT_ELEM_TET; break;
		case FE_TET10G4 :
		case FE_TET10G8 : 
		case FE_TET10GL11: ne = 10; dtype = PLT_ELEM_TET10; break;
		case FE_TET15G8: ne = 15; dtype = PLT_ELEM_TET15; break;
		case FE_HEX20G27: ne = 20; dtype = PLT_ELEM_HEX20; break;
		default:
			assert(false);
	}

	// write the header
	m_ar.BeginChunk(PLT_DOMAIN_HDR);
	{
		m_ar.WriteChunk(PLT_DOM_ELEM_TYPE, dtype);
		m_ar.WriteChunk(PLT_DOM_MAT_ID   ,   mid);
		m_ar.WriteChunk(PLT_DOM_ELEMS    ,    NE);
	}
	m_ar.EndChunk();

	// write the element list
	int n[FEElement::MAX_NODES + 1];
	m_ar.BeginChunk(PLT_DOM_ELEM_LIST);
	{
		for (i=0; i<NE; ++i)
		{
			FESolidElement& el = dom.Element(i);
			n[0] = el.m_nID;
			for (j=0; j<ne; ++j) n[j+1] = el.m_node[j];
			m_ar.WriteChunk(PLT_ELEMENT, n, ne+1);
		}
	}
	m_ar.EndChunk();
}

//-----------------------------------------------------------------------------
void FEBioPlotFile::WriteShellDomain(FEShellDomain& dom)
{
	int mid = dom.GetMaterial()->GetID();
	assert(mid > 0);
	int etype = dom.GetElementType();

	int i, j;
	int NE = dom.Elements();

	// figure out element type
	int ne = 0;
	int dtype = 0;
	switch (etype)
	{
		case FE_SHELL_QUAD: ne = 4; dtype = PLT_ELEM_QUAD; break;
		case FE_SHELL_TRI : ne = 3; dtype = PLT_ELEM_TRI; break;
	}

	// write the header
	m_ar.BeginChunk(PLT_DOMAIN_HDR);
	{
		m_ar.WriteChunk(PLT_DOM_ELEM_TYPE, dtype);
		m_ar.WriteChunk(PLT_DOM_MAT_ID   ,   mid);
		m_ar.WriteChunk(PLT_DOM_ELEMS    ,    NE);
	}
	m_ar.EndChunk();

	// write the element list
	int n[5];
	m_ar.BeginChunk(PLT_DOM_ELEM_LIST);
	{
		for (i=0; i<NE; ++i)
		{
			FEShellElement& el = dom.Element(i);
			n[0] = el.m_nID;
			for (j=0; j<ne; ++j) n[j+1] = el.m_node[j];
			m_ar.WriteChunk(PLT_ELEMENT, n, ne+1);
		}
	}
	m_ar.EndChunk();
}

//-----------------------------------------------------------------------------
void FEBioPlotFile::WriteTrussDomain(FETrussDomain& dom)
{
	int mid = dom.GetMaterial()->GetID();
	assert(mid > 0);

	int i, j;
	int NE = dom.Elements();

	// figure out element type
	int ne = 2;
	int dtype = PLT_ELEM_TRUSS;

	// write the header
	m_ar.BeginChunk(PLT_DOMAIN_HDR);
	{
		m_ar.WriteChunk(PLT_DOM_ELEM_TYPE, dtype);
		m_ar.WriteChunk(PLT_DOM_MAT_ID   ,   mid);
		m_ar.WriteChunk(PLT_DOM_ELEMS    ,    NE);
	}
	m_ar.EndChunk();

	// write the element list
	int n[5];
	m_ar.BeginChunk(PLT_DOM_ELEM_LIST);
	{
		for (i=0; i<NE; ++i)
		{
			FETrussElement& el = dynamic_cast<FETrussElement&>(dom.ElementRef(i));
			n[0] = el.m_nID;
			for (j=0; j<ne; ++j) n[j+1] = el.m_node[j];
			m_ar.WriteChunk(PLT_ELEMENT, n, ne+1);
		}
	}
	m_ar.EndChunk();
}

//-----------------------------------------------------------------------------
void FEBioPlotFile::WriteDiscreteDomain(FEDiscreteDomain& dom)
{
	int mid = dom.GetMaterial()->GetID();
	assert(mid > 0);

	int i, j;
	int NE = dom.Elements();

	// figure out element type
	int ne = 2;
	int dtype = PLT_ELEM_TRUSS;

	// write the header
	m_ar.BeginChunk(PLT_DOMAIN_HDR);
	{
		m_ar.WriteChunk(PLT_DOM_ELEM_TYPE, dtype);
		m_ar.WriteChunk(PLT_DOM_MAT_ID   ,   mid);
		m_ar.WriteChunk(PLT_DOM_ELEMS    ,    NE);
	}
	m_ar.EndChunk();

	// write the element list
	int n[5];
	m_ar.BeginChunk(PLT_DOM_ELEM_LIST);
	{
		for (i=0; i<NE; ++i)
		{
			FEDiscreteElement& el = dynamic_cast<FEDiscreteElement&>(dom.ElementRef(i));
			n[0] = el.m_nID;
			for (j=0; j<ne; ++j) n[j+1] = el.m_node[j];
			m_ar.WriteChunk(PLT_ELEMENT, n, ne+1);
		}
	}
	m_ar.EndChunk();
}

//-----------------------------------------------------------------------------
void FEBioPlotFile::WriteSurfaceSection(FEMesh& m)
{
	for (int ns = 0; ns<m.Surfaces(); ++ns)
	{
		FESurface& s = m.Surface(ns);
		int NF = s.Elements();
		m_ar.BeginChunk(PLT_SURFACE);
		{
			m_ar.BeginChunk(PLT_SURFACE_HDR);
			{
				int sid = ns+1;
				m_ar.WriteChunk(PLT_SURFACE_ID, sid);
				m_ar.WriteChunk(PLT_SURFACE_FACES, NF);
			}
			m_ar.EndChunk();

			m_ar.BeginChunk(PLT_FACE_LIST);
			{
				int n[FEBioPlotFile::PLT_MAX_FACET_NODES + 2];
				for (int i=0; i<NF; ++i)
				{
					FESurfaceElement& f = s.Element(i);
					int nf = f.Nodes();
					n[0] = i+1;
					n[1] = nf;
					for (int i=0; i<nf; ++i) n[i+2] = f.m_node[i];
					m_ar.WriteChunk(PLT_FACE, n, FEBioPlotFile::PLT_MAX_FACET_NODES);
				}
			}
			m_ar.EndChunk();
		}
		m_ar.EndChunk();
	}
}

//-----------------------------------------------------------------------------
bool FEBioPlotFile::Write(FEModel &fem)
{
	// store the fem pointer
	m_pfem = &fem;

	m_ar.BeginChunk(PLT_STATE);
	{
		// state header
		m_ar.BeginChunk(PLT_STATE_HEADER);
		{
			float f = (float) fem.m_ftime;
			m_ar.WriteChunk(PLT_STATE_HDR_TIME, f);
		}
		m_ar.EndChunk();

		m_ar.BeginChunk(PLT_STATE_DATA);
		{
			// Global Data
			if (!m_dic.m_Glob.empty())
			{
				m_ar.BeginChunk(PLT_GLOBAL_DATA);
				{
					WriteGlobalData(fem);
				}
				m_ar.EndChunk();
			}

			// Material Data
			if (!m_dic.m_Mat.empty())
			{
				m_ar.BeginChunk(PLT_MATERIAL_DATA);
				{
					WriteMaterialData(fem);
				}
				m_ar.EndChunk();
			}

			// Node Data
			if (!m_dic.m_Node.empty())
			{
				m_ar.BeginChunk(PLT_NODE_DATA);
				{
					WriteNodeData(fem);
				}
				m_ar.EndChunk();
			}

			// Element Data
			if (!m_dic.m_Elem.empty())
			{
				m_ar.BeginChunk(PLT_ELEMENT_DATA);
				{
					WriteDomainData(fem);
				}
				m_ar.EndChunk();
			}

			// surface data
			if (!m_dic.m_Face.empty())
			{
				m_ar.BeginChunk(PLT_FACE_DATA);
				{
					WriteSurfaceData(fem);
				}
				m_ar.EndChunk();
			}
		}
		m_ar.EndChunk();
	}
	m_ar.EndChunk();

	return true;
}

//-----------------------------------------------------------------------------
void FEBioPlotFile::WriteGlobalData(FEModel& fem)
{

}

//-----------------------------------------------------------------------------
void FEBioPlotFile::WriteMaterialData(FEModel& fem)
{

}

//-----------------------------------------------------------------------------
void FEBioPlotFile::WriteNodeData(FEModel& fem)
{
	list<DICTIONARY_ITEM>::iterator it = m_dic.m_Node.begin();
	for (int i=0; i<(int) m_dic.m_Node.size(); ++i, ++it)
	{
		m_ar.BeginChunk(PLT_STATE_VARIABLE);
		{
			unsigned int nid = i+1;
			m_ar.WriteChunk(PLT_STATE_VAR_ID, nid);
			m_ar.BeginChunk(PLT_STATE_VAR_DATA);
			{
				if (it->m_psave) (it->m_psave)->Save(fem, m_ar);
			}
			m_ar.EndChunk();
		}
		m_ar.EndChunk();
	}
}

//-----------------------------------------------------------------------------
void FEBioPlotFile::WriteDomainData(FEModel& fem)
{
	list<DICTIONARY_ITEM>::iterator it = m_dic.m_Elem.begin();
	for (int i=0; i<(int) m_dic.m_Elem.size(); ++i, ++it)
	{
		m_ar.BeginChunk(PLT_STATE_VARIABLE);
		{
			unsigned int nid = i+1;
			m_ar.WriteChunk(PLT_STATE_VAR_ID, nid);
			m_ar.BeginChunk(PLT_STATE_VAR_DATA);
			{
				if (it->m_psave) (it->m_psave)->Save(fem, m_ar);
			}
			m_ar.EndChunk();
		}
		m_ar.EndChunk();
	}
}

//-----------------------------------------------------------------------------
void FEBioPlotFile::WriteSurfaceData(FEModel& fem)
{
	list<DICTIONARY_ITEM>::iterator it = m_dic.m_Face.begin();
	for (int i=0; i<(int) m_dic.m_Face.size(); ++i, ++it)
	{
		m_ar.BeginChunk(PLT_STATE_VARIABLE);
		{
			unsigned int nid = i+1;
			m_ar.WriteChunk(PLT_STATE_VAR_ID, nid);
			m_ar.BeginChunk(PLT_STATE_VAR_DATA);
			{
				if (it->m_psave) (it->m_psave)->Save(fem, m_ar);
			}
			m_ar.EndChunk();
		}
		m_ar.EndChunk();
	}
}

//-----------------------------------------------------------------------------
bool FEBioPlotFile::Append(FEModel& fem, const char *szfile)
{
	// try to open the file
	if (m_ar.Open(szfile) == false) return false;

	// open the root element
	m_ar.OpenChunk();
	unsigned int nid = m_ar.GetChunkID();
	if (nid != PLT_ROOT) return false;

	bool bok = false;
	while (m_ar.OpenChunk() == IO_OK)
	{
		nid = m_ar.GetChunkID();
		if (nid == PLT_DICTIONARY)
		{
			// read the dictionary
			bok = ReadDictionary();
			break;
		}
		m_ar.CloseChunk();
	}

	// close it again ...
	m_ar.Close();

	// ... and open for appending
	if (bok) return m_ar.Append(szfile);

	return false;
}

//-----------------------------------------------------------------------------
bool FEBioPlotFile::ReadDictionary()
{
	m_dic.Clear();

	while (m_ar.OpenChunk() == IO_OK)
	{
		unsigned int nid = m_ar.GetChunkID();
		switch (nid)
		{
		case PLT_DIC_GLOBAL: assert(false); return false;
		case PLT_DIC_MATERIAL: assert(false); return false; 
		case PLT_DIC_NODAL: ReadDicList(); break;
		case PLT_DIC_DOMAIN: ReadDicList(); break;
		case PLT_DIC_SURFACE: ReadDicList(); break;
		default:
			assert(false);
			return false;
		}
		m_ar.CloseChunk();
	}
	return true;
}

//-----------------------------------------------------------------------------
bool FEBioPlotFile::ReadDicList()
{
	vector<int> l; // empty item list
	while (m_ar.OpenChunk() == IO_OK)
	{
		unsigned int nid = m_ar.GetChunkID();
		if (nid == PLT_DIC_ITEM)
		{
			while (m_ar.OpenChunk() == IO_OK)
			{
				unsigned int nid = m_ar.GetChunkID();
				if (nid == PLT_DIC_ITEM_NAME)
				{
					char sz[STR_SIZE];
					m_ar.read(sz, STR_SIZE);
					AddVariable(sz, l);
				}
				m_ar.CloseChunk();
			}
		}
		else return false;
		m_ar.CloseChunk();
	}
	return true;
}
