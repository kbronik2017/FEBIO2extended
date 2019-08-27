// FEMesh.h: interface for the FEMesh class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FEMESH_H__81ABA97F_AD5F_4F1D_8EE9_95B67EBA448E__INCLUDED_)
#define AFX_FEMESH_H__81ABA97F_AD5F_4F1D_8EE9_95B67EBA448E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FEDomain.h"
#include "DumpFile.h"
#include "FENodeElemList.h"
#include "DumpStream.h"

//-----------------------------------------------------------------------------
//  This class stores the coordinates of a bounding box
//
struct FE_BOUNDING_BOX
{
	vec3d	r0, r1; // coordinates of opposite corners

	// center of box
	vec3d center() { return (r0+r1)*0.5; }

	// dimensions of box
	double width () { return (r1.x-r0.x); }
	double height() { return (r1.y-r0.y); }
	double depth () { return (r1.z-r0.z); }

	// max dimension
	double radius() 
	{ 
		double w = width();
		double h = height();
		double d = depth();

		if ((w>=d) && (w>=h)) return w;
		if ((h>=w) && (h>=d)) return h;
		
		return d;
	}

	void operator += (vec3d r)
	{
		if (r.x < r0.x) r0.x = r.x;
		if (r.y < r0.y) r0.y = r.y;
		if (r.z < r0.z) r0.z = r.z;
		if (r.x > r1.x) r1.x = r.x;
		if (r.y > r1.y) r1.y = r.y;
		if (r.z > r1.z) r1.z = r.z;
	}

	void inflate(double dx, double dy, double dz)
	{
		r0.x -= dx; r1.x += dx;
		r0.y -= dy; r1.y += dy;
		r0.z -= dz; r1.z += dz;
	}

	// check whether a point is inside or not
	bool IsInside(vec3d r) 
	{ 
		return ((r.x>=r0.x) && (r.y>=r0.y) && (r.z>=r0.z) && (r.x<=r1.x) && (r.y<=r1.y) && (r.z<=r1.z));
	}
};

//-----------------------------------------------------------------------------
//! This class defines a finite element node

//! It stores nodal positions and nodal equations numbers and more.
//!
//! The m_BC array stores the status of the degrees of freedom. For a given
//! dof, the value can be 0 = free or -1 = fixed. The array
//! is initialized to 0 and then the different analysis classes
//! will open the degrees of freedom they need to solve the analysis. 
//!
//! The m_ID array will store the equation number for the corresponding
//! degree of freedom. Its values can be (a) non-negative (0 or higher) which
//! gives the equation number in the linear system of equations, (b) -1 if the
//! dof is fixed, and (c) < -1 if the dof corresponds to a prescribed dof. In
//! that case the corresponding equation number is given by -ID-2.

class FENode
{
public:
	FENode();
	
public:
	// geometry data
	vec3d	m_r0;	//!< initial position
	vec3d	m_v0;	//!< initial velocity

	vec3d	m_rt;	//!< current position
	vec3d	m_vt;	//!< nodal velocity
	vec3d	m_at;	//!< nodal acceleration

	vec3d	m_rp;	//!< position of node at previous time step
	vec3d	m_vp;	//!< previous velocity
	vec3d	m_ap;	//!< previous acceleration

	vec3d	m_Fr;	//!< nodal reaction forces

	// shell data
	vec3d	m_D0;	//!< initial director
	vec3d	m_Dt;	//!< current director

	// poroelasticity-data
	double	m_p0;	//!< initial pressure
	double	m_pt;	//!< current pressure

	// heat-conduction data
	double	m_T;	//!< temperature

	// solute-data
	vector<double>	m_c0;	//!< initial effective concentration
	vector<double>	m_ct;	//!< current effective concentration
	vector<double>	m_cp;	//!< effective concentration at previous time step
	
	// rigid body data
	int		m_rid;	//!< rigid body number
	bool	m_bshell;	//!< does this node belong to a non-rigid shell element?

public:
	vector<int>		m_ID;	//!< nodal equation numbers
	vector<int>		m_BC;	//!< boundary condition
};

//-----------------------------------------------------------------------------
// Forward declaration of FEMesh class.
class FEMesh;

//-----------------------------------------------------------------------------
//! Defines a node set
//
class FENodeSet
{
public:
	FENodeSet(FEMesh* pm);

	void create(int n);

	int size() { return m_Node.size(); }

	int& operator [] (int i) { return m_Node[i]; }

	void SetID(int n) { m_nID = n; }
	int GetID() { return m_nID; }

	void SetName(const char* sz);
	const char* GetName() { return m_szname; }

protected:
	FEMesh*	m_pmesh;
	vector<int>	m_Node;		//!< list of nodes

	int	m_nID;
	char	m_szname[256];
};

//-----------------------------------------------------------------------------
//! This class defines a set of facets. This can be used in the creation of
//! surfaces.
class FEFacetSet
{
public:
	struct FACET
	{
		int	node[FEElement::MAX_NODES];
		int	ntype;	//	3=tri3, 4=quad4, 6=tri6, 7=tri7, 8=quad8
	};

public:
	FEFacetSet();
	const char* GetName() { return m_szname; }
	void SetName(const char* sz);

	void Create(int n);

	int Faces() { return (int) m_Face.size(); }
	FACET& Face(int i);

public:
	vector<FACET>	m_Face;
	char			m_szname[256];
};

//-----------------------------------------------------------------------------
//! Class that defines a finite element part, that is, a list of elements.

//! Note that this class is only a base class defining a common interface
//! to all derived part classes. Part classes need to be specialized depending
//! on the element type (solid, shell, truss, ...)
class FEPart
{
public:
	//! constructor
	FEPart(FEMesh* pm) : m_pmesh(pm) {}

	//! Get the mesh this part belongs to
	FEMesh* GetMesh() { return m_pmesh; }

	//! Create a part
	virtual void Create(int n) = 0;

	//! Number of elements in domain
	virtual int Elements() = 0;

	//! return reference to an element
	virtual FEElement& ElementRef(int n) = 0;

	//! Find an element based on its ID
	FEElement* FindElementFromID(int nid);

protected:
	FEMesh*		m_pmesh;
};

//-----------------------------------------------------------------------------
template <class T> class FEPart_T : public FEPart
{
public:
	FEPart_T(FEMesh* pm) : FEPart(pm) {}

	void Create(int n) { m_Elem.resize(n); }
	int Elements() { return (int) m_Elem.size(); }
	FEElement& ElementRef(int n) { return m_Elem[n]; }

protected:
	vector<T>	m_Elem;
};

//-----------------------------------------------------------------------------
typedef FEPart_T<FESolidElement>		FESolidPart;
typedef FEPart_T<FEShellElement>		FEShellPart;
typedef FEPart_T<FETrussElement>		FETrussPart;
typedef FEPart_T<FEDiscreteElement>		FEDiscretePart;

//-----------------------------------------------------------------------------
//! Defines a finite element mesh

//! All the geometry data is stored in this class. 

class FEMesh  
{
public:
	//! constructor
	FEMesh();

	//! destructor
	virtual ~FEMesh();

	//! stream mesh data
	void ShallowCopy(DumpStream& dmp, bool bsave);

	//! initialize mesh
	bool Init();

	//! allocate storage for mesh data
	void CreateNodes(int nodes);
	void AddNodes(int nodes);

	//! return number of nodes
	int Nodes() { return m_Node.size(); }

	//! return total nr of elements
	int Elements();

	//! return the total nr of solid elements
	int SolidElements();

	//! return the total nr of shell elements
	int ShellElements();

	//! return the total nr of truss elements
	int TrussElements();

	//! return the total nr of discrete elements
	int DiscreteElements();

	//! return reference to a node
	FENode& Node(int i) { return m_Node[i]; }

	//! update bounding box
	void UpdateBox();

	//! retrieve the bounding box
	FE_BOUNDING_BOX& GetBoundingBox() { return m_box; }

	//! remove isolated vertices
	int RemoveIsolatedVertices();

	//! Reset the mesh data
	void Reset();

	//! Calculates an elements volume
	double ElementVolume(FEElement& el);

	//! adds a node set to the mesh
	void AddNodeSet(FENodeSet* pns) { m_NodeSet.push_back(pns); }

	//! Find a nodeset by ID
	FENodeSet* FindNodeSet(int nid);

	//! Find a nodeset by name
	FENodeSet* FindNodeSet(const char* szname);

	//! Get the face nodes from a given element
	int GetFace(FEElement& el, int n, int nf[8]);

	//! return the nr of faces an element has
	int Faces(FEElement& el);

	//! Finds an element from a given ID
	FEElement* FindElementFromID(int nid);

	//! Finds the solid element in which y lies
	FESolidElement* FindSolidElement(vec3d y, double r[3]);

	FENodeElemList& NodeElementList()
	{
		if (m_NEL.Size() != m_Node.size()) m_NEL.Create(*this);
		return m_NEL;
	}

	// --- PARTS ---
	int Parts() { return (int) m_Part.size(); }
	FEPart& Part(int n) { return *m_Part[n]; }
	void AddPart(FEPart* pg) { m_Part.push_back(pg); }

	// --- DOMAINS ---
	int Domains() { return (int) m_Domain.size(); }
	FEDomain& Domain(int n) { return *m_Domain[n]; }

	void AddDomain(FEDomain* pd) { m_Domain.push_back(pd); }

	//! get a list of domains that belong to a specific material
	void DomainListFromMaterial(vector<int>& lmat, vector<int>& ldom);

	// --- SURFACES ---
	int Surfaces() { return (int) m_Surf.size(); }
	FESurface& Surface(int n) { return *m_Surf[n]; }
	void AddSurface(FESurface* ps) { m_Surf.push_back(ps); }

	// --- FACETSETS ---
	int FacetSets() { return (int) m_FaceSet.size(); }
	FEFacetSet& FacetSet(int n) { return *m_FaceSet[n]; }
	void AddFacetSet(FEFacetSet* ps) { m_FaceSet.push_back(ps); }

protected:
	double SolidElementVolume(FESolidElement& el);
	double ShellElementVolume(FEShellElement& el);

	//! Look for any inverted elements
	int FindInvertedElements();
	
	//! Initialize shell normals
	void InitShellNormals();

protected:
	void ClearDomains();

protected:
	vector<FENode>		m_Node;		//!< nodes
	vector<FEDomain*>	m_Domain;	//!< list of domains
	vector<FESurface*>	m_Surf;		//!< surfaces

	vector<FEPart*>		m_Part;		//!< parts
	vector<FENodeSet*>	m_NodeSet;	//!< node sets
	vector<FEFacetSet*>	m_FaceSet;	//!< facet sets

	FE_BOUNDING_BOX		m_box;	//!< bounding box

	FENodeElemList	m_NEL;

private:
	//! hide the copy constructor
	FEMesh(FEMesh& m){}

	//! hide assignment operator
	void operator =(FEMesh& m) {}
};

#endif // !defined(AFX_FEMESH_H__81ABA97F_AD5F_4F1D_8EE9_95B67EBA448E__INCLUDED_)
