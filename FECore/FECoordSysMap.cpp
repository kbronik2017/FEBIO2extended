// FECoordSysMap.cpp: implementation of the FECoordSysMap class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FECoordSysMap.h"
#include "FEMesh.h"
#include "FEModel.h"

//=============================================================================
// FELocalMap
//-----------------------------------------------------------------------------

BEGIN_PARAMETER_LIST(FELocalMap, FECoordSysMap)
	ADD_PARAMETERV(m_n, FE_PARAM_INTV, 3, "local");
END_PARAMETER_LIST();

//-----------------------------------------------------------------------------
FELocalMap::FELocalMap(FEModel* pfem) : FECoordSysMap(FE_MAP_LOCAL), m_mesh(pfem->GetMesh())
{
	m_n[0] = -1;
	m_n[1] = -1;
	m_n[2] = -1;
}

//-----------------------------------------------------------------------------
void FELocalMap::Init()
{
	if ((m_n[0]==-1)&&(m_n[1]==-1)&&(m_n[2]==-1)) { m_n[0] = 0; m_n[1] = 1; m_n[2] = 3; }
	if (m_n[2] == -1) m_n[2] = m_n[1];
}

//-----------------------------------------------------------------------------
void FELocalMap::SetLocalNodes(int n1, int n2, int n3)
{
	m_n[0] = n1;
	m_n[1] = n2;
	m_n[2] = n3;
}

//-----------------------------------------------------------------------------
mat3d FELocalMap::LocalElementCoord(FEElement& el, int n)
{
	vec3d r0[FEElement::MAX_NODES];
	for (int i=0; i<el.Nodes(); ++i) r0[i] = m_mesh.Node(el.m_node[i]).m_r0;

	vec3d a, b, c, d;
	mat3d Q;

	a = r0[m_n[1]] - r0[m_n[0]];
	a.unit();

	if (m_n[2] != m_n[1])
	{
		d = r0[m_n[2]] - r0[m_n[0]];
	}
	else
	{
		d = vec3d(0,1,0);
		if (fabs(d*a) > 0.999) d = vec3d(1,0,0);
	}

	c = a^d;
	b = c^a;

	b.unit();
	c.unit();

	Q[0][0] = a.x; Q[0][1] = b.x; Q[0][2] = c.x;
	Q[1][0] = a.y; Q[1][1] = b.y; Q[1][2] = c.y;
	Q[2][0] = a.z; Q[2][1] = b.z; Q[2][2] = c.z;

	return Q;
}

//-----------------------------------------------------------------------------
void FELocalMap::Serialize(DumpFile& ar)
{
	if (ar.IsSaving())
	{
		ar << m_n[0] << m_n[1] << m_n[2];
	}
	else
	{
		ar >> m_n[0] >> m_n[1] >> m_n[2];
	}
}

//=============================================================================
// FESphericalMap
//-----------------------------------------------------------------------------

BEGIN_PARAMETER_LIST(FESphericalMap, FECoordSysMap)
	ADD_PARAMETER(m_c, FE_PARAM_VEC3D, "spherical");
END_PARAMETER_LIST();

//-----------------------------------------------------------------------------
FESphericalMap::FESphericalMap(FEModel* pfem): FECoordSysMap(FE_MAP_SPHERE), m_mesh(pfem->GetMesh())
{

}

//-----------------------------------------------------------------------------
void FESphericalMap::Init()
{

}

//-----------------------------------------------------------------------------
mat3d FESphericalMap::LocalElementCoord(FEElement& el, int n)
{
	vec3d r0[FEElement::MAX_NODES];
	for (int i=0; i<el.Nodes(); ++i) r0[i] = m_mesh.Node(el.m_node[i]).m_r0;

	double* H = el.H(n);
	vec3d a;
	for (int i=0; i<el.Nodes(); ++i) a += r0[i]*H[i];
	a -= m_c;
	a.unit();

	vec3d d = r0[1] - r0[0];
	d.unit();
	if (fabs(a*d) > .99) 
	{
		d = r0[2] - r0[1];
		d.unit();
	}

	vec3d c = a^d;
	vec3d b = c^a;

	a.unit();
	b.unit();
	c.unit();

	mat3d Q;
	Q[0][0] = a.x; Q[0][1] = b.x; Q[0][2] = c.x;
	Q[1][0] = a.y; Q[1][1] = b.y; Q[1][2] = c.y;
	Q[2][0] = a.z; Q[2][1] = b.z; Q[2][2] = c.z;

	return Q;
}

//-----------------------------------------------------------------------------
void FESphericalMap::Serialize(DumpFile& ar)
{
	if (ar.IsSaving())
	{
		ar << m_c;
	}
	else
	{
		ar >> m_c;
	}
}


//=============================================================================
// FECylindricalMap
//-----------------------------------------------------------------------------

BEGIN_PARAMETER_LIST(FECylindricalMap, FECoordSysMap)
	ADD_PARAMETER(m_c, FE_PARAM_VEC3D, "center");
	ADD_PARAMETER(m_a, FE_PARAM_VEC3D, "axis"  );
	ADD_PARAMETER(m_r, FE_PARAM_VEC3D, "vector");
END_PARAMETER_LIST();

//-----------------------------------------------------------------------------
FECylindricalMap::FECylindricalMap(FEModel* pfem) : FECoordSysMap(FE_MAP_CYLINDER), m_mesh(pfem->GetMesh())
{
	m_c = vec3d(0,0,0);
	m_a = vec3d(0,0,1);
	m_r = vec3d(1,0,0);
}

//-----------------------------------------------------------------------------
void FECylindricalMap::Init()
{
	m_a.unit();
	m_r.unit();
}

//-----------------------------------------------------------------------------
mat3d FECylindricalMap::LocalElementCoord(FEElement& el, int n)
{
	// get the element nodes
	vec3d r0[FEElement::MAX_NODES];
	for (int i=0; i<el.Nodes(); ++i) r0[i] = m_mesh.Node(el.m_node[i]).m_r0;

	// find the nodal position of the integration point n
	vec3d p = el.Evaluate(r0, n);

	// find the vector to the axis
	vec3d b = (p - m_c) - m_a*(m_a*(p - m_c)); b.unit();

	// setup the rotation vector
	vec3d x_unit(vec3d(1,0,0));
	quatd q(x_unit, b);

	// rotate the reference vector
	vec3d r(m_r); r.unit();
	q.RotateVector(r);

	// setup a local coordinate system with r as the x-axis
	vec3d d(vec3d(0,1,0));
	q.RotateVector(d);
	if (fabs(d*r) > 0.99)
	{
		d = vec3d(0,0,1);
		q.RotateVector(d);
	}

	// find basis vectors
	vec3d e1 = r;
	vec3d e3 = (e1 ^ d); e3.unit();
	vec3d e2 = e3 ^ e1;

	// setup rotation matrix
	mat3d Q;
	Q[0][0] = e1.x; Q[0][1] = e2.x; Q[0][2] = e3.x;
	Q[1][0] = e1.y; Q[1][1] = e2.y; Q[1][2] = e3.y;
	Q[2][0] = e1.z; Q[2][1] = e2.z; Q[2][2] = e3.z;

	return Q;
}

//-----------------------------------------------------------------------------
void FECylindricalMap::Serialize(DumpFile& ar)
{
	if (ar.IsSaving())
	{
		ar << m_c << m_a << m_r;
	}
	else
	{
		ar >> m_c >> m_a >> m_r;
	}
}

//=============================================================================
// FEVectorMap
//-----------------------------------------------------------------------------

BEGIN_PARAMETER_LIST(FEVectorMap, FECoordSysMap)
	ADD_PARAMETER(m_a, FE_PARAM_VEC3D, "vector");
END_PARAMETER_LIST();

//-----------------------------------------------------------------------------
void FEVectorMap::Init()
{
	m_a.unit();
	m_d = vec3d(1,0,0);
	if (m_a*m_d > .999) m_d = vec3d(0,1,0);
}

//-----------------------------------------------------------------------------
mat3d FEVectorMap::LocalElementCoord(FEElement& el, int n)
{
	vec3d a = m_a;
	vec3d d = m_d;

	vec3d c = a^d;
	vec3d b = c^a;

	a.unit();
	b.unit();
	c.unit();

	mat3d Q;
	Q[0][0] = a.x; Q[0][1] = b.x; Q[0][2] = c.x;
	Q[1][0] = a.y; Q[1][1] = b.y; Q[1][2] = c.y;
	Q[2][0] = a.z; Q[2][1] = b.z; Q[2][2] = c.z;

	return Q;
}

//-----------------------------------------------------------------------------
void FEVectorMap::Serialize(DumpFile &ar)
{
	if (ar.IsSaving())
	{
		ar << m_a << m_d;
	}
	else
	{
		ar >> m_a >> m_d;
	}
}
