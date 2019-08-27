#include "stdafx.h"
#include "FEElasticSolidDomain.h"
#include "FETransverselyIsotropic.h"
#include "FEViscoElasticMaterial.h"
#include "FEUncoupledViscoElasticMaterial.h"
#include "FEUncoupledElasticMixture.h"
#include "FECore/log.h"
#include "FECore/DOFS.h"
//#include "FEBioHeat/FEHeatTransferMaterial.h"

#ifdef WIN32
extern "C" int __cdecl omp_get_num_threads(void);
extern "C" int __cdecl omp_get_thread_num(void);
#else
extern "C" int omp_get_num_threads(void);
extern "C" int omp_get_thread_num(void);
#endif


//-----------------------------------------------------------------------------
// Reset data
void FEElasticSolidDomain::Reset()
{
	for (int i=0; i<(int) m_Elem.size(); ++i) m_Elem[i].Init(true);
}

//-----------------------------------------------------------------------------
void FEElasticSolidDomain::SetLocalCoordinateSystem(FEElement& el, int n, FEMaterialPoint& mp, FEElasticMaterial* pme)
{
	// get the material's coordinate system (if defined)
	FECoordSysMap* pmap = pme->GetCoordinateSystemMap();

	// set the local element coordinates
	if (pmap)
	{
		FEElasticMaterialPoint& pt = *(mp.ExtractData<FEElasticMaterialPoint>());
	
		// compound the local map with the global material axes
		mat3d Qlocal = pmap->LocalElementCoord(el, n);
		pt.m_Q = Qlocal*pt.m_Q;
	}

	// check if this is also a viscoelastic material
	FEViscoElasticMaterial* pve = dynamic_cast<FEViscoElasticMaterial*> (pme);
	if (pve)
	{
		FEElasticMaterial* pme2 = pve->GetBaseMaterial();
		SetLocalCoordinateSystem(el, n, mp, pme2);
	}

	// check if this is also an uncoupled viscoelastic material
	FEUncoupledViscoElasticMaterial* puve = dynamic_cast<FEUncoupledViscoElasticMaterial*> (pme);
	if (puve)
	{
		// check if the nested elastic material has local material axes specified
		FEElasticMaterial* pme2 = puve->GetBaseMaterial();
		SetLocalCoordinateSystem(el, n, mp, pme2);
	}

	// check the uncoupled elastic mixture
	FEUncoupledElasticMixture* pumix = dynamic_cast<FEUncoupledElasticMixture*>(pme);
	if (pumix)
	{
		// check the local coordinate systems for each component
		for (int j=0; j<pumix->Materials(); ++j)
		{
			FEElasticMaterial* pmj = pumix->GetMaterial(j)->GetElasticMaterial();
			FEMaterialPoint& mpj = *mp.GetPointData(j);
			SetLocalCoordinateSystem(el, n, mpj, pmj);
		}
	}
}

//-----------------------------------------------------------------------------
//! \todo The material point initialization needs to move to the base class.
bool FEElasticSolidDomain::Initialize(FEModel &fem)
{
	// initialize base class
	FESolidDomain::Initialize(fem);

	// get the elements material
	FEElasticMaterial* pme = m_pMat->GetElasticMaterial();


	for (size_t i=0; i<m_Elem.size(); ++i)
	{
		FESolidElement& el = m_Elem[i];
		for (int n=0; n<el.GaussPoints(); ++n) SetLocalCoordinateSystem(el, n, *(el.m_State[n]), pme);
	}

	return true;
}

//-----------------------------------------------------------------------------
//! Initialize element data
void FEElasticSolidDomain::InitElements()
{
	const int NE = FEElement::MAX_NODES;
	vec3d x0[NE], xt[NE], r0(0, 0, 0), rt(0, 0, 0);
	for (int l = 0; l<NE; l++)
	{
		x0[l] = (0, 0, 0);
		xt[l] = (0, 0, 0);
	}
	
	FEMesh& m = *GetMesh();
	for (size_t i=0; i<m_Elem.size(); ++i)
	{
		FESolidElement& el = m_Elem[i];
		int neln = el.Nodes();
		for (int i=0; i<neln; ++i)
		{
			x0[i] = m.Node(el.m_node[i]).m_r0;
			xt[i] = m.Node(el.m_node[i]).m_rt;
		}

		int n = el.GaussPoints();
		for (int j=0; j<n; ++j) 
		{
			r0 = el.Evaluate(x0, j);
			rt = el.Evaluate(xt, j);

			FEMaterialPoint& mp = *el.m_State[j];
			FEElasticMaterialPoint& pt = *mp.ExtractData<FEElasticMaterialPoint>();
			pt.m_r0 = r0;
			pt.m_rt = rt;

			pt.m_J = defgrad(el, pt.m_F, j);

			el.m_State[j]->Init(false);
		}
	}
}


/*
//-----------------------------------------------------------------------------
void FEElasticSolidDomain::Residual(FESolver *psolver, vector<double>& R)
{
	FEModel& fem = psolver->GetFEModel();

	// element force vector
	vector<double> fe;

	vector<int> lm;

	int NE = m_Elem.size();
	for (int i=0; i<NE; ++i)
	{
		// get the element
		FESolidElement& el = m_Elem[i];

		// get the element force vector and initialize it to zero
		int ndof = 3*el.Nodes();
		fe.assign(ndof, 0);

		// calculate internal force vector
		ElementInternalForce(el, fe);

		// apply body forces
		if (fem.HasBodyForces()) ElementBodyForce(fem, el, fe);

		// get the element's LM vector
		UnpackLM(el, lm);

		// assemble element 'fe'-vector into global R vector
		psolver->AssembleResidual(el.m_node, lm, fe, R);
	}
}
*/

//-----------------------------------------------------------------------------
void FEElasticSolidDomain::InternalForces(FEGlobalVector& R)
{
	int NE = m_Elem.size();
	#pragma omp parallel for
	for (int i=0; i<NE; ++i)
	{
		// element force vector
		vector<double> fe;
		vector<int> lm;
		
		// get the element
		FESolidElement& el = m_Elem[i];

		// get the element force vector and initialize it to zero
		int ndof = 3*el.Nodes();
		fe.assign(ndof, 0);

		// calculate internal force vector
		ElementInternalForce(el, fe);

		// get the element's LM vector
		UnpackLM(el, lm);

		// assemble element 'fe'-vector into global R vector
		#pragma omp critical
		R.Assemble(el.m_node, lm, fe);
	}
}

//-----------------------------------------------------------------------------
//! calculates the internal equivalent nodal forces for solid elements

void FEElasticSolidDomain::ElementInternalForce(FESolidElement& el, vector<double>& fe)
{
	int i, n;

	// jacobian matrix, inverse jacobian matrix and determinants
	double Ji[3][3], detJt;

	double Gx, Gy, Gz;
	mat3ds s;

	const double* Gr, *Gs, *Gt;

	int nint = el.GaussPoints();
	int neln = el.Nodes();

	double*	gw = el.GaussWeights();

	// repeat for all integration points
	for (n=0; n<nint; ++n)
	{
		FEMaterialPoint& mp = *el.m_State[n];
		FEElasticMaterialPoint& pt = *(mp.ExtractData<FEElasticMaterialPoint>());

		// calculate the jacobian
		detJt = invjact(el, Ji, n);

		detJt *= gw[n];

		// get the stress vector for this integration point
		s = pt.m_s;

		Gr = el.Gr(n);
		Gs = el.Gs(n);
		Gt = el.Gt(n);

		for (i=0; i<neln; ++i)
		{
			// calculate global gradient of shape functions
			// note that we need the transposed of Ji, not Ji itself !
			Gx = Ji[0][0]*Gr[i]+Ji[1][0]*Gs[i]+Ji[2][0]*Gt[i];
			Gy = Ji[0][1]*Gr[i]+Ji[1][1]*Gs[i]+Ji[2][1]*Gt[i];
			Gz = Ji[0][2]*Gr[i]+Ji[1][2]*Gs[i]+Ji[2][2]*Gt[i];

			// calculate internal force
			// the '-' sign is so that the internal forces get subtracted
			// from the global residual vector
			fe[3*i  ] -= ( Gx*s.xx() +
				           Gy*s.xy() +
					       Gz*s.xz() )*detJt;

			fe[3*i+1] -= ( Gy*s.yy() +
				           Gx*s.xy() +
					       Gz*s.yz() )*detJt;

			fe[3*i+2] -= ( Gz*s.zz() +
				           Gy*s.yz() +
					       Gx*s.xz() )*detJt;
		}
	}
}

////////////
//-----------------------------------------------------------------------------
void FEElasticSolidDomain::HeatElementResidual(FESolidElement& el, vector<double>& fe)
{
	zero(fe);
	double* w = el.GaussWeights();
	int ne = el.Nodes();
	int ni = el.GaussPoints();
	for (int n = 0; n<ni; ++n)
	{
		double* H = el.H(n);
		double J = detJt(el, n);
		for (int i = 0; i<ne; ++i)
		{
			fe[i] += m_Q_H*H[i] * J*w[n];
		}
	}
}

/////////////////////////////////
void FEElasticSolidDomain::HeatResidual(FEGlobalVector& R)
{
	// get the mesh
	//FEMesh& mesh = GetFEModel()->GetMesh();

	int NE = m_Elem.size();
#pragma omp parallel for
	for (int i = 0; i<NE; ++i)
	{
		// element force vector
		vector<double> fe;
		vector<int> lm;

		// get the element
		FESolidElement& el = m_Elem[i];

		// get the element force vector and initialize it to zero
		int ndof = 3 * el.Nodes();
		fe.assign(ndof, 0);

		// calculate internal force vector
		HeatElementResidual(el, fe);

		// get the element's LM vector
		UnpackLM(el, lm);

		// assemble element 'fe'-vector into global R vector
#pragma omp critical
		R.Assemble(el.m_node, lm, fe);
	}
}




//-----------------------------------------------------------------------------
void FEElasticSolidDomain::BodyForce(FEGlobalVector& R, FEBodyForce& BF)
{
	// element force vector
	vector<double> fe;

	vector<int> lm;

	int NE = m_Elem.size();
	for (int i=0; i<NE; ++i)
	{
		// get the element
		FESolidElement& el = m_Elem[i];

		// get the element force vector and initialize it to zero
		int ndof = 3*el.Nodes();
		fe.assign(ndof, 0);

		// apply body forces
		ElementBodyForce(BF, el, fe);

		// get the element's LM vector
		UnpackLM(el, lm);

		// assemble element 'fe'-vector into global R vector
		R.Assemble(el.m_node, lm, fe);
	}
}

//-----------------------------------------------------------------------------
//! calculates the body forces

void FEElasticSolidDomain::ElementBodyForce(FEBodyForce& BF, FESolidElement& el, vector<double>& fe)
{
	// don't forget to multiply with the density
	FESolidMaterial* pme = dynamic_cast<FESolidMaterial*>(m_pMat);
	double dens = pme->Density();

	// jacobian
	double detJ;
	double *H;
	double* gw = el.GaussWeights();
	vec3d f;

	// number of nodes
	int neln = el.Nodes();

	// nodal coordinates
	vec3d r0[FEElement::MAX_NODES], rt[FEElement::MAX_NODES];
	for (int i=0; i<neln; ++i)
	{
		r0[i] = m_pMesh->Node(el.m_node[i]).m_r0;
		rt[i] = m_pMesh->Node(el.m_node[i]).m_rt;
	}
	
	// loop over integration points
	int nint = el.GaussPoints();
	for (int n=0; n<nint; ++n)
	{
		FEMaterialPoint& mp = *el.m_State[n];
		FEElasticMaterialPoint& pt = *mp.ExtractData<FEElasticMaterialPoint>();
		pt.m_r0 = el.Evaluate(r0, n);
		pt.m_rt = el.Evaluate(rt, n);

		detJ = detJ0(el, n)*gw[n];

		// get the force
		f = BF.force(mp);

		H = el.H(n);

		for (int i=0; i<neln; ++i)
		{
			fe[3*i  ] -= H[i]*dens*f.x*detJ;
			fe[3*i+1] -= H[i]*dens*f.y*detJ;
			fe[3*i+2] -= H[i]*dens*f.z*detJ;
		}						
	}
}


//-----------------------------------------------------------------------------
//! calculates the body forces
/*
void FEElasticSolidDomain::ElementBodyForce(FEModel& fem, FESolidElement& el, vector<double>& fe)
{
	int NF = fem.BodyForces();
	for (int nf = 0; nf < NF; ++nf)
	{
		FEBodyForce& BF = *fem.GetBodyForce(nf);

		// don't forget to multiply with the density
		FESolidMaterial* pme = dynamic_cast<FESolidMaterial*>(m_pMat);
		double dens = pme->Density();

		// TODO: I don't like this but for now I'll hard-code the modification of the
		//       force center position
		if (dynamic_cast<FEPointBodyForce*>(&BF))
		{
			FEPointBodyForce* pf = dynamic_cast<FEPointBodyForce*>(&BF);
			if (pf->m_rlc[0] >= 0) pf->m_rc.x = fem.GetLoadCurve(pf->m_rlc[0])->Value();
			if (pf->m_rlc[1] >= 0) pf->m_rc.y = fem.GetLoadCurve(pf->m_rlc[1])->Value();
			if (pf->m_rlc[2] >= 0) pf->m_rc.z = fem.GetLoadCurve(pf->m_rlc[2])->Value();
		}

		// jacobian
		double detJ;
		double *H;
		double* gw = el.GaussWeights();
		vec3d f;

		// number of nodes
		int neln = el.Nodes();

		// nodal coordinates
		vec3d r0[FEElement::MAX_NODES], rt[FEElement::MAX_NODES];
		for (int i=0; i<neln; ++i)
		{
			r0[i] = m_pMesh->Node(el.m_node[i]).m_r0;
			rt[i] = m_pMesh->Node(el.m_node[i]).m_rt;
		}

		// loop over integration points
		int nint = el.GaussPoints();
		for (int n=0; n<nint; ++n)
		{
			FEMaterialPoint& mp = *el.m_State[n];
			FEElasticMaterialPoint& pt = *mp.ExtractData<FEElasticMaterialPoint>();
			pt.r0 = el.Evaluate(r0, n);
			pt.rt = el.Evaluate(rt, n);

			detJ = detJ0(el, n)*gw[n];

			// get the force
			f = BF.force(mp);

			H = el.H(n);

			for (int i=0; i<neln; ++i)
			{
				fe[3*i  ] -= H[i]*dens*f.x*detJ;
				fe[3*i+1] -= H[i]*dens*f.y*detJ;
				fe[3*i+2] -= H[i]*dens*f.z*detJ;
			}						
		}
	}
}
*/
//-----------------------------------------------------------------------------
//! This function calculates the stiffness due to body forces
void FEElasticSolidDomain::ElementBodyForceStiffness(FEBodyForce& BF, FESolidElement &el, matrix &ke)
{
	int neln = el.Nodes();
	int ndof = ke.columns()/neln;

	// don't forget to multiply with the density
	FESolidMaterial* pme = dynamic_cast<FESolidMaterial*>(m_pMat);
	double dens = pme->Density();

	// jacobian
	double detJ;
	double *H;
	double* gw = el.GaussWeights();
	mat3ds K;

	// loop over integration points
	int nint = el.GaussPoints();
	for (int n=0; n<nint; ++n)
	{
		FEMaterialPoint& mp = *el.m_State[n];
		detJ = detJ0(el, n)*gw[n];

		// get the stiffness
		K = BF.stiffness(mp);

		H = el.H(n);

		for (int i=0; i<neln; ++i)
			for (int j=0; j<neln; ++j)
			{
				ke[ndof*i  ][ndof*j  ] -= H[i]*H[j]*dens*K(0,0)*detJ;
				ke[ndof*i  ][ndof*j+1] -= H[i]*H[j]*dens*K(0,1)*detJ;
				ke[ndof*i  ][ndof*j+2] -= H[i]*H[j]*dens*K(0,2)*detJ;

				ke[ndof*i+1][ndof*j  ] -= H[i]*H[j]*dens*K(1,0)*detJ;
				ke[ndof*i+1][ndof*j+1] -= H[i]*H[j]*dens*K(1,1)*detJ;
				ke[ndof*i+1][ndof*j+2] -= H[i]*H[j]*dens*K(1,2)*detJ;

				ke[ndof*i+2][ndof*j  ] -= H[i]*H[j]*dens*K(2,0)*detJ;
				ke[ndof*i+2][ndof*j+1] -= H[i]*H[j]*dens*K(2,1)*detJ;
				ke[ndof*i+2][ndof*j+2] -= H[i]*H[j]*dens*K(2,2)*detJ;
			}
	}	
}

//-----------------------------------------------------------------------------
//! calculates element's geometrical stiffness component for integration point n

void FEElasticSolidDomain::ElementGeometricalStiffness(FESolidElement &el, matrix &ke)
{
	int n, i, j;

	double Gx[FEElement::MAX_NODES];
	double Gy[FEElement::MAX_NODES];
	double Gz[FEElement::MAX_NODES];
	double *Grn, *Gsn, *Gtn;
	double Gr, Gs, Gt;

	// nr of nodes
	int neln = el.Nodes();

	// nr of integration points
	int nint = el.GaussPoints();

	// jacobian
	double Ji[3][3], detJt;

	// weights at gauss points
	const double *gw = el.GaussWeights();

	// stiffness component for the initial stress component of stiffness matrix
	double kab;

	// calculate geometrical element stiffness matrix
	for (n=0; n<nint; ++n)
	{
		// calculate jacobian
		detJt = invjact(el, Ji, n)*gw[n];

		Grn = el.Gr(n);
		Gsn = el.Gs(n);
		Gtn = el.Gt(n);

		for (i=0; i<neln; ++i)
		{
			Gr = Grn[i];
			Gs = Gsn[i];
			Gt = Gtn[i];

			// calculate global gradient of shape functions
			// note that we need the transposed of Ji, not Ji itself !
			Gx[i] = Ji[0][0]*Gr+Ji[1][0]*Gs+Ji[2][0]*Gt;
			Gy[i] = Ji[0][1]*Gr+Ji[1][1]*Gs+Ji[2][1]*Gt;
			Gz[i] = Ji[0][2]*Gr+Ji[1][2]*Gs+Ji[2][2]*Gt;
		}

		// get the material point data
		FEMaterialPoint& mp = *el.m_State[n];
		FEElasticMaterialPoint& pt = *(mp.ExtractData<FEElasticMaterialPoint>());

		// element's Cauchy-stress tensor at gauss point n
		// s is the voight vector
		mat3ds& s = pt.m_s;

		for (i=0; i<neln; ++i)
			for (j=i; j<neln; ++j)
			{
				kab = (Gx[i]*(s.xx()*Gx[j]+s.xy()*Gy[j]+s.xz()*Gz[j]) +
					   Gy[i]*(s.xy()*Gx[j]+s.yy()*Gy[j]+s.yz()*Gz[j]) + 
					   Gz[i]*(s.xz()*Gx[j]+s.yz()*Gy[j]+s.zz()*Gz[j]))*detJt;

				ke[3*i  ][3*j  ] += kab;
				ke[3*i+1][3*j+1] += kab;
				ke[3*i+2][3*j+2] += kab;
			}
	}
}


//-----------------------------------------------------------------------------
//! Calculates element material stiffness element matrix

void FEElasticSolidDomain::ElementMaterialStiffness(FESolidElement &el, matrix &ke)
{
	int i, i3, j, j3, n;

	// Get the current element's data
	const int nint = el.GaussPoints();
	const int neln = el.Nodes();
	const int ndof = 3*neln;

	// global derivatives of shape functions
	// Gx = dH/dx
	double Gx[FEElement::MAX_NODES];
	double Gy[FEElement::MAX_NODES];
	double Gz[FEElement::MAX_NODES];

	double Gxi, Gyi, Gzi;
	double Gxj, Gyj, Gzj;

	// The 'D' matrix
	double D[6][6] = {0};	// The 'D' matrix

	// The 'D*BL' matrix
	double DBL[6][3];

	double *Grn, *Gsn, *Gtn;
	double Gr, Gs, Gt;

	// jacobian
	double Ji[3][3], detJt;
	
	// weights at gauss points
	const double *gw = el.GaussWeights();

	FESolidMaterial* pmat = dynamic_cast<FESolidMaterial*>(m_pMat);

	// calculate element stiffness matrix
	for (n=0; n<nint; ++n)
	{
		// calculate jacobian
		detJt = invjact(el, Ji, n)*gw[n];

		Grn = el.Gr(n);
		Gsn = el.Gs(n);
		Gtn = el.Gt(n);

		// setup the material point
		// NOTE: deformation gradient and determinant have already been evaluated in the stress routine
		FEMaterialPoint& mp = *el.m_State[n];
		FEElasticMaterialPoint& pt = *(mp.ExtractData<FEElasticMaterialPoint>());

		// get the 'D' matrix
		tens4ds C = pmat->Tangent(mp);
		C.extract(D);

		for (i=0; i<neln; ++i)
		{
			Gr = Grn[i];
			Gs = Gsn[i];
			Gt = Gtn[i];

			// calculate global gradient of shape functions
			// note that we need the transposed of Ji, not Ji itself !
			Gx[i] = Ji[0][0]*Gr+Ji[1][0]*Gs+Ji[2][0]*Gt;
			Gy[i] = Ji[0][1]*Gr+Ji[1][1]*Gs+Ji[2][1]*Gt;
			Gz[i] = Ji[0][2]*Gr+Ji[1][2]*Gs+Ji[2][2]*Gt;
		}

		// we only calculate the upper triangular part
		// since ke is symmetric. The other part is
		// determined below using this symmetry.
		for (i=0, i3=0; i<neln; ++i, i3 += 3)
		{
			Gxi = Gx[i];
			Gyi = Gy[i];
			Gzi = Gz[i];

			for (j=i, j3 = i3; j<neln; ++j, j3 += 3)
			{
				Gxj = Gx[j];
				Gyj = Gy[j];
				Gzj = Gz[j];

				// calculate D*BL matrices
				DBL[0][0] = (D[0][0]*Gxj+D[0][3]*Gyj+D[0][5]*Gzj);
				DBL[0][1] = (D[0][1]*Gyj+D[0][3]*Gxj+D[0][4]*Gzj);
				DBL[0][2] = (D[0][2]*Gzj+D[0][4]*Gyj+D[0][5]*Gxj);

				DBL[1][0] = (D[1][0]*Gxj+D[1][3]*Gyj+D[1][5]*Gzj);
				DBL[1][1] = (D[1][1]*Gyj+D[1][3]*Gxj+D[1][4]*Gzj);
				DBL[1][2] = (D[1][2]*Gzj+D[1][4]*Gyj+D[1][5]*Gxj);

				DBL[2][0] = (D[2][0]*Gxj+D[2][3]*Gyj+D[2][5]*Gzj);
				DBL[2][1] = (D[2][1]*Gyj+D[2][3]*Gxj+D[2][4]*Gzj);
				DBL[2][2] = (D[2][2]*Gzj+D[2][4]*Gyj+D[2][5]*Gxj);

				DBL[3][0] = (D[3][0]*Gxj+D[3][3]*Gyj+D[3][5]*Gzj);
				DBL[3][1] = (D[3][1]*Gyj+D[3][3]*Gxj+D[3][4]*Gzj);
				DBL[3][2] = (D[3][2]*Gzj+D[3][4]*Gyj+D[3][5]*Gxj);

				DBL[4][0] = (D[4][0]*Gxj+D[4][3]*Gyj+D[4][5]*Gzj);
				DBL[4][1] = (D[4][1]*Gyj+D[4][3]*Gxj+D[4][4]*Gzj);
				DBL[4][2] = (D[4][2]*Gzj+D[4][4]*Gyj+D[4][5]*Gxj);

				DBL[5][0] = (D[5][0]*Gxj+D[5][3]*Gyj+D[5][5]*Gzj);
				DBL[5][1] = (D[5][1]*Gyj+D[5][3]*Gxj+D[5][4]*Gzj);
				DBL[5][2] = (D[5][2]*Gzj+D[5][4]*Gyj+D[5][5]*Gxj);

				ke[i3  ][j3  ] += (Gxi*DBL[0][0] + Gyi*DBL[3][0] + Gzi*DBL[5][0] )*detJt;
				ke[i3  ][j3+1] += (Gxi*DBL[0][1] + Gyi*DBL[3][1] + Gzi*DBL[5][1] )*detJt;
				ke[i3  ][j3+2] += (Gxi*DBL[0][2] + Gyi*DBL[3][2] + Gzi*DBL[5][2] )*detJt;

				ke[i3+1][j3  ] += (Gyi*DBL[1][0] + Gxi*DBL[3][0] + Gzi*DBL[4][0] )*detJt;
				ke[i3+1][j3+1] += (Gyi*DBL[1][1] + Gxi*DBL[3][1] + Gzi*DBL[4][1] )*detJt;
				ke[i3+1][j3+2] += (Gyi*DBL[1][2] + Gxi*DBL[3][2] + Gzi*DBL[4][2] )*detJt;

				ke[i3+2][j3  ] += (Gzi*DBL[2][0] + Gyi*DBL[4][0] + Gxi*DBL[5][0] )*detJt;
				ke[i3+2][j3+1] += (Gzi*DBL[2][1] + Gyi*DBL[4][1] + Gxi*DBL[5][1] )*detJt;
				ke[i3+2][j3+2] += (Gzi*DBL[2][2] + Gyi*DBL[4][2] + Gxi*DBL[5][2] )*detJt;
			}
		}
	}
}

//-----------------------------------------------------------------------------
/*
void FEElasticSolidDomain::StiffnessMatrix(FESolver* psolver)
{
	FEM& fem = dynamic_cast<FEM&>(psolver->GetFEModel());

	// element stiffness matrix
	matrix ke;

	vector<int> lm;

	// repeat over all solid elements
	int NE = m_Elem.size();
	for (int iel=0; iel<NE; ++iel)
	{
		FESolidElement& el = m_Elem[iel];

		// create the element's stiffness matrix
		int ndof = 3*el.Nodes();
		ke.resize(ndof, ndof);
		ke.zero();

		// calculate the element stiffness matrix
		ElementStiffness(fem, iel, ke);

		// add the inertial stiffness for dynamics
		if (fem.GetCurrentStep()->m_nanalysis == FE_DYNAMIC) ElementInertialStiffness(fem, el, ke);

		// add body force stiffness
		if (fem.HasBodyForces()) ElementBodyForceStiffness(fem, el, ke);

		// get the element's LM vector
		UnpackLM(el, lm);

		// assemble element matrix in global stiffness matrix
		psolver->AssembleStiffness(el.m_node, lm, ke);
	}
}
*/

//-----------------------------------------------------------------------------

void FEElasticSolidDomain::StiffnessMatrix(FESolver* psolver)
{
	// repeat over all solid elements
	int NE = m_Elem.size();
	
	#pragma omp parallel for
	for (int iel=0; iel<NE; ++iel)
	{
		// element stiffness matrix
		matrix ke;
		vector<int> lm;
		
		FESolidElement& el = m_Elem[iel];

		// create the element's stiffness matrix
		int ndof = 3*el.Nodes();
		ke.resize(ndof, ndof);
		ke.zero();

		// calculate geometrical stiffness
		ElementGeometricalStiffness(el, ke);

		// calculate material stiffness
		ElementMaterialStiffness(el, ke);

		// assign symmetic parts
		// TODO: Can this be omitted by changing the Assemble routine so that it only
		// grabs elements from the upper diagonal matrix?
		for (int i=0; i<ndof; ++i)
			for (int j=i+1; j<ndof; ++j)
				ke[j][i] = ke[i][j];

		// get the element's LM vector
		UnpackLM(el, lm);

		// assemble element matrix in global stiffness matrix
		#pragma omp critical
		psolver->AssembleStiffness(el.m_node, lm, ke);
	}
}

//-----------------------------------------------------------------------------

void FEElasticSolidDomain::MassMatrix(FESolver* psolver, double scale)
{
	FEModel& fem = psolver->GetFEModel();

	// get the analysis
	FEAnalysis* pstep = fem.GetCurrentStep();

	// element stiffness matrix
	matrix ke;
	vector<int> lm;

	// repeat over all solid elements
	int NE = m_Elem.size();
	for (int iel=0; iel<NE; ++iel)
	{
		FESolidElement& el = m_Elem[iel];

		// create the element's stiffness matrix
		int ndof = 3*el.Nodes();
		ke.resize(ndof, ndof);
		ke.zero();

		// calculate inertial stiffness
		ElementMassMatrix(el, ke, scale);

		// get the element's LM vector
		UnpackLM(el, lm);

		// assemble element matrix in global stiffness matrix
		psolver->AssembleStiffness(el.m_node, lm, ke);
	}
}

//-----------------------------------------------------------------------------

void FEElasticSolidDomain::BodyForceStiffness(FESolver* psolver, FEBodyForce& bf)
{
	// element stiffness matrix
	matrix ke;
	vector<int> lm;

	// repeat over all solid elements
	int NE = m_Elem.size();
	for (int iel=0; iel<NE; ++iel)
	{
		FESolidElement& el = m_Elem[iel];

		// create the element's stiffness matrix
		int ndof = 3*el.Nodes();
		ke.resize(ndof, ndof);
		ke.zero();

		// calculate inertial stiffness
		ElementBodyForceStiffness(bf, el, ke);

		// get the element's LM vector
		UnpackLM(el, lm);

		// assemble element matrix in global stiffness matrix
		psolver->AssembleStiffness(el.m_node, lm, ke);
	}
}

//-----------------------------------------------------------------------------
//! This function calculates the element stiffness matrix. It calls the material
//! stiffness function, the geometrical stiffness function and, if necessary, the
//! dilatational stiffness function. Note that these three functions only calculate
//! the upper diagonal matrix due to the symmetry of the element stiffness matrix
//! The last section of this function fills the rest of the element stiffness matrix.

void FEElasticSolidDomain::ElementStiffness(FEModel& fem, int iel, matrix& ke)
{
	FESolidElement& el = Element(iel);

	// calculate material stiffness (i.e. constitutive component)
	ElementMaterialStiffness(el, ke);

	// calculate geometrical stiffness
	ElementGeometricalStiffness(el, ke);

	// assign symmetic parts
	// TODO: Can this be omitted by changing the Assemble routine so that it only
	// grabs elements from the upper diagonal matrix?
	int ndof = 3*el.Nodes();
	int i, j;
	for (i=0; i<ndof; ++i)
		for (j=i+1; j<ndof; ++j)
			ke[j][i] = ke[i][j];
}

//-----------------------------------------------------------------------------
//! calculates element inertial stiffness matrix
void FEElasticSolidDomain::ElementMassMatrix(FESolidElement& el, matrix& ke, double a)
{
	// get the material's density
	FESolidMaterial* pm = dynamic_cast<FESolidMaterial*>(m_pMat);
	double d = pm->Density();

	// Get the current element's data
	const int nint = el.GaussPoints();
	const int neln = el.Nodes();
	const int ndof = 3*neln;

	// weights at gauss points
	const double *gw = el.GaussWeights();

	// calculate element mass matrix
	for (int n=0; n<nint; ++n)
	{
		// shape functions
		double* H = el.H(n);

		// jacobian
		double J0 = detJ0(el, n)*gw[n];

		// loop over nodes
		for (int i=0; i<neln; ++i)
			for (int j=i; j<neln; ++j)
			{
				double kab = a*H[i]*H[j]*J0*d;
				ke[3*i  ][3*j  ] += kab;
				ke[3*i+1][3*j+1] += kab;
				ke[3*i+2][3*j+2] += kab;
			}	
	}

	// assign symmetic parts
	// TODO: Can this be omitted by changing the Assemble routine so that it only
	// grabs elements from the upper diagonal matrix?
	for (int i=0; i<ndof; ++i)
		for (int j=i+1; j<ndof; ++j)
			ke[j][i] = ke[i][j];

}

//-----------------------------------------------------------------------------
void FEElasticSolidDomain::UpdateStresses(FEModel &fem)
{
	double dt = fem.GetCurrentStep()->m_dt;
	
	bool berr = false;
	int NE = (int) m_Elem.size();
	#pragma omp parallel for shared(NE, berr)
	for (int i=0; i<NE; ++i)
	{
		try
		{
			UpdateElementStress(i, dt);
		}
		catch (NegativeJacobian e)
		{
			// A negative jacobian was detected
			felog.printbox("ERROR","Negative jacobian was detected at element %d at gauss point %d\njacobian = %lg\n", e.m_iel, e.m_ng+1, e.m_vol);
			#pragma omp critical
			berr = true;
		}
	}

	// if we encountered an error, we request a running restart
	if (berr) throw DoRunningRestart();
}

//-----------------------------------------------------------------------------
//! Update element state data (mostly stresses, but some other stuff as well)
//! \todo Remove the remodeling solid stuff
void FEElasticSolidDomain::UpdateElementStress(int iel, double dt)
{
	// get the solid element
	FESolidElement& el = m_Elem[iel];

	// get the number of integration points
	int nint = el.GaussPoints();

	// number of nodes
	int neln = el.Nodes();

	// nodal coordinates
	vec3d r0[FEElement::MAX_NODES];
	vec3d rt[FEElement::MAX_NODES];
	for (int j=0; j<neln; ++j)
	{
		r0[j] = m_pMesh->Node(el.m_node[j]).m_r0;
		rt[j] = m_pMesh->Node(el.m_node[j]).m_rt;
	}

	// get the integration weights
	double* gw = el.GaussWeights();

	// get the material
	FESolidMaterial* pm = dynamic_cast<FESolidMaterial*>(m_pMat);
	assert(pm);

	// loop over the integration points and calculate
	// the stress at the integration point
	for (int n=0; n<nint; ++n)
	{
		FEMaterialPoint& mp = *el.m_State[n];
		FEElasticMaterialPoint& pt = *(mp.ExtractData<FEElasticMaterialPoint>());

		// material point coordinates
		// TODO: I'm not entirly happy with this solution
		//		 since the material point coordinates are used by most materials.
		pt.m_r0 = el.Evaluate(r0, n);
		pt.m_rt = el.Evaluate(rt, n);

		// get the deformation gradient and determinant
		pt.m_J = defgrad(el, pt.m_F, n);

		// calculate the stress at this material point
		pt.m_s = pm->Stress(mp);
	}
}

//-----------------------------------------------------------------------------
//! Unpack the element LM data. 
void FEElasticSolidDomain::UnpackLM(FEElement& el, vector<int>& lm)
{
    // get nodal DOFS
    DOFS& fedofs = *DOFS::GetInstance();
    int MAX_NDOFS = fedofs.GetNDOFS();
    int MAX_CDOFS = fedofs.GetCDOFS();
    
	int N = el.Nodes();
	lm.resize(N*MAX_NDOFS);
	
	for (int i=0; i<N; ++i)
	{
		int n = el.m_node[i];
		FENode& node = m_pMesh->Node(n);

		vector<int>& id = node.m_ID;

		// first the displacement dofs
		lm[3*i  ] = id[0];
		lm[3*i+1] = id[1];
		lm[3*i+2] = id[2];

		// now the pressure dofs
		lm[3*N+i] = id[6];

		// rigid rotational dofs
		lm[4*N + 3*i  ] = id[7];
		lm[4*N + 3*i+1] = id[8];
		lm[4*N + 3*i+2] = id[9];

		// fill the rest with -1
		lm[7*N + 3*i  ] = -1;
		lm[7*N + 3*i+1] = -1;
		lm[7*N + 3*i+2] = -1;

		lm[10*N + i] = id[10];
		
		// concentration dofs
		for (int k=0; k<MAX_CDOFS; ++k)
			lm[(11+k)*N + i] = id[11+k];
	}
}

//-----------------------------------------------------------------------------
// Calculate inertial forces
void FEElasticSolidDomain::InertialForces(FEGlobalVector& R, vector<double>& F)
{
	FESolidMaterial* pme = dynamic_cast<FESolidMaterial*>(m_pMat); assert(pme);
	double d = pme->Density();

	// element mass matrix
	matrix ke;

	// element force vector
	vector<double> fe;

	// element's LM vector
	vector<int> lm;

	// loop over all elements
	int NE = Elements();
	for (int iel=0; iel<NE; ++iel)
	{
		FESolidElement& el = Element(iel);

		int nint = el.GaussPoints();
		int neln = el.Nodes();

		ke.resize(3*neln, 3*neln);
		ke.zero();

		fe.resize(3*neln);
		
		// create the element mass matrix
		for (int n=0; n<nint; ++n)
		{
			double J0 = detJ0(el, n)*el.GaussWeights()[n];

			double* H = el.H(n);
			for (int i=0; i<neln; ++i)
				for (int j=0; j<neln; ++j)
				{
					double kab = H[i]*H[j]*J0*d;
					ke[3*i  ][3*j  ] += kab;
					ke[3*i+1][3*j+1] += kab;
					ke[3*i+2][3*j+2] += kab;
				}	
		}

		// now, multiply M with F and add to R
		int* en = &el.m_node[0];
		for (int i=0; i<3*neln; ++i)
		{
			fe[i] = 0;
			for (int j=0; j<3*neln; ++j)
			{
				fe[i] -= ke[i][j]*F[3*(en[j/3]) + j%3];
			}
		}

		// get the element degrees of freedom
		UnpackLM(el, lm);

		// assemble fe into R
		R.Assemble(el.m_node, lm, fe);
	}
}




/////////////////////////////


// Calculate the heat conduction matrix
void FEElasticSolidDomain::ConductionMatrix(FESolver* psolver)
{
	vector<int> lm;

	// loop over all elements in domain
	for (int i = 0; i<(int)m_Elem.size(); ++i)
	{
		FESolidElement& el = m_Elem[i];
		int ne = el.Nodes();

		// build the element stiffness matrix
		matrix ke(ne, ne);
		ElementConduction(el, ke);

		// set up the LM matrix
		UnpackLM(el, lm);

		// assemble into global matrix
		psolver->AssembleStiffness(el.m_node, lm, ke);
	}
}

//-----------------------------------------------------------------------------
// Calculate the capacitance matrix
void FEElasticSolidDomain::CapacitanceMatrix(FESolver* psolver, double dt)
{
	vector<int> lm;

	// loop over all elements in domain
	for (int i = 0; i<(int)m_Elem.size(); ++i)
	{
		FESolidElement& el = m_Elem[i];
		int ne = el.Nodes();

		// element capacitance matrix
		matrix kc(ne, ne);
		ElementCapacitance(el, kc, dt);

		// set up the LM matrix
		UnpackLM(el, lm);

		// assemble into global matrix
		psolver->AssembleStiffness(el.m_node, lm, kc);
	}
}
//-----------------------------------------------------------------------------
//! This function calculates the element stiffness matrix for a particular
//! element.
//!
void FEElasticSolidDomain::Conductivity(double D[3][3])
{
	D[0][0] = D[1][1] = D[2][2] = m_k;
	D[0][1] = D[1][0] = 0;
	D[0][2] = D[2][0] = 0;
	D[1][2] = D[2][1] = 0;
}

void FEElasticSolidDomain::ElementConduction(FESolidElement& el, matrix& ke)
{
	int i, j, n;

	int ne = el.Nodes();
	int ni = el.GaussPoints();

	// global derivatives of shape functions
	// Gx = dH/dx
	const int EN = FEElement::MAX_NODES;
	double Gx[EN], Gy[EN], Gz[EN];

	double Gr, Gs, Gt;
	double Gi[3], Gj[3];
	double DB[3];

	// jacobian
	double Ji[3][3], detJt;

	// weights at gauss points
	const double *gw = el.GaussWeights();

	// zero stiffness matrix
	ke.zero();

	// conductivity matrix
	double D[3][3];

	//FEHeatTransferMaterial& mat = dynamic_cast<FEHeatTransferMaterial&>(*m_pMat);

	// loop over all integration points
	for (n = 0; n<ni; ++n)
	{
		// calculate jacobian
		detJt = invjact(el, Ji, n);

		// evaluate the conductivity
		Conductivity(D);

		for (i = 0; i<ne; ++i)
		{
			Gr = el.Gr(n)[i];
			Gs = el.Gs(n)[i];
			Gt = el.Gt(n)[i];

			// calculate global gradient of shape functions
			// note that we need the transposed of Ji, not Ji itself !
			Gx[i] = Ji[0][0] * Gr + Ji[1][0] * Gs + Ji[2][0] * Gt;
			Gy[i] = Ji[0][1] * Gr + Ji[1][1] * Gs + Ji[2][1] * Gt;
			Gz[i] = Ji[0][2] * Gr + Ji[1][2] * Gs + Ji[2][2] * Gt;
		}

		for (i = 0; i<ne; ++i)
		{
			Gi[0] = Gx[i];
			Gi[1] = Gy[i];
			Gi[2] = Gz[i];

			for (j = 0; j<ne; ++j)
			{
				Gj[0] = Gx[j];
				Gj[1] = Gy[j];
				Gj[2] = Gz[j];

				DB[0] = D[0][0] * Gj[0] + D[0][1] * Gj[1] + D[0][2] * Gj[2];
				DB[1] = D[1][0] * Gj[0] + D[1][1] * Gj[1] + D[1][2] * Gj[2];
				DB[2] = D[2][0] * Gj[0] + D[2][1] * Gj[1] + D[2][2] * Gj[2];

				ke[i][j] += (Gi[0] * DB[0] + Gi[1] * DB[1] + Gi[2] * DB[2])*detJt*gw[n];
			}
		}
	}
}

//-----------------------------------------------------------------------------
void FEElasticSolidDomain::ElementCapacitance(FESolidElement &el, matrix &ke, double dt)
{
	int i, j, n;
	// get the material's density
	FESolidMaterial* pm = dynamic_cast<FESolidMaterial*>(m_pMat);
	double d = pm->Density();
	//pm->GetFEModel()->

	int ne = el.Nodes();
	int ni = el.GaussPoints();

	// shape functions
	double* H;

	// jacobian
	double Ji[3][3], detJt;

	// weights at gauss points
	const double *gw = el.GaussWeights();

	// zero stiffness matrix
	ke.zero();

	//FEHeatTransferMaterial& mat = dynamic_cast<FEHeatTransferMaterial&>(*m_pMat);
	double c = Capacitance();
	//double d = mat.Density();
	double alpha = c*d / dt;

	// loop over all integration points
	for (n = 0; n<ni; ++n)
	{
		// calculate jacobian
		detJt = invjact(el, Ji, n);

		H = el.H(n);

		for (i = 0; i<ne; ++i)
		{
			for (j = 0; j<ne; ++j)
			{
				ke[i][j] += H[i] * H[j] * alpha*detJt*gw[n];
			}
		}
	}
}

/*

void FEElasticSolidDomain::ConvectiveHeatStiffnessMatrix(FESolver* psolver)
{
	FEModel& fem = psolver->GetFEModel();
	
	matrix ke;
	vector<int> elm;
	//FEMesh& mesh = *GetMesh();
	int npr = m_FC.size();
	for (int m = 0; m<npr; ++m)
	{
		LOAD& fc = m_FC[m];

		// get the surface element
		FESurfaceElement& el = m_pMesh->    m_psurf->Element(m);
		//mesh.
		// get the element stiffness matrix
		int neln = el.Nodes();
		int ndof = neln;
		ke.resize(ndof, ndof);

		// calculate pressure stiffness
		ConvectiveHeatElementStiffness(el, ke, fc.hc);

		// get the element's LM vector
		UnpackLM(el, elm);

		vector<int> lm(neln);
		for (int j = 0; j<neln; ++j) lm[j] = elm[neln * 10 + j];

		// assemble element matrix in global stiffness matrix
		psolver->AssembleStiffness(el.m_node, lm, ke);
	}
}

//-----------------------------------------------------------------------------
void FEElasticSolidDomain::ConvectiveHeatElementStiffness(FESurfaceElement& el, matrix& ke, double hc)
{
	// nr integration points
	int nint = el.GaussPoints();

	// nr of element nodes
	int neln = el.Nodes();

	// nodal coordinates
	vec3d rt[FEElement::MAX_NODES];
	//FEMesh& m = *GetMesh();
	//FESurface*	m_psurf;
	//m_pMesh->Surface(1);
	//for (int j = 0; j<neln; ++j) rt[j] = m_psurf->GetMesh()->Node(el.m_node[j]).m_rt;
	for (int j = 0; j<neln; ++j) rt[j] = m_pMesh->Node(el.m_node[j]).m_rt;
	//->Node(el.m_node[j]).m_r0;
	//m_pMesh->
	double* Gr, *Gs;
	double* N;
	double* w = el.GaussWeights();

	vec3d dxr, dxs;
	ke.zero();

	// repeat over integration points
	for (int n = 0; n<nint; ++n)
	{
		N = el.H(n);
		Gr = el.Gr(n);
		Gs = el.Gs(n);

		dxr = dxs = vec3d(0, 0, 0);
		for (int i = 0; i<neln; ++i)
		{
			dxr += rt[i] * Gr[i];
			dxs += rt[i] * Gs[i];
		}

		double J = (dxr ^ dxs).norm();

		for (int i = 0; i<neln; ++i)
		{
			for (int j = 0; j<neln; ++j)
			{
				double kij = hc*N[i] * N[j] * J*w[n];
				ke[i][j] += kij;
			}
		}
	}
}


*/