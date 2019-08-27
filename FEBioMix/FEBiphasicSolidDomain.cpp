#include "FEBiphasicSolidDomain.h"
#include "FEBiphasic.h"
#include "FECore/log.h"

//-----------------------------------------------------------------------------
bool FEBiphasicSolidDomain::Initialize(FEModel &mdl)
{
	// initialize base class
	FEElasticSolidDomain::Initialize(mdl);

	// get the biphasic material
	FEBiphasic* pmb = dynamic_cast<FEBiphasic*>(GetMaterial());
	assert(pmb);

	const int NE = FEElement::MAX_NODES;
    double p0[NE];
	FEMesh& m = *GetMesh();
    
	// initialize all element data
	for (int i=0; i<(int) m_Elem.size(); ++i)
	{
		// get the solid element
		FESolidElement& el = m_Elem[i];
		
        // get the number of nodes
        int neln = el.Nodes();
        // get initial values of fluid pressure and solute concentrations
		for (int i=0; i<neln; ++i)
			p0[i] = m.Node(el.m_node[i]).m_p0;
        
		// get the number of integration points
		int nint = el.GaussPoints();
		
		// loop over the integration points
		for (int n=0; n<nint; ++n)
		{
			FEMaterialPoint& mp = *el.m_State[n];
            FEElasticMaterialPoint& pm = *(mp.ExtractData<FEElasticMaterialPoint>());
			FEBiphasicMaterialPoint& pt = *(mp.ExtractData<FEBiphasicMaterialPoint>());
			
            // initialize effective fluid pressure, its gradient, and fluid flux
            pt.m_p = el.Evaluate(p0, n);
            pt.m_gradp = gradient(el, p0, n);
            pt.m_w = pmb->Flux(mp);
            pt.m_pa = pmb->Pressure(mp);
            pm.m_s = pmb->Stress(mp);
           
			// initialize referential solid volume fraction
			pt.m_phi0 = pmb->m_phi0;
		}
	}
	
	return true;
}


//-----------------------------------------------------------------------------
void FEBiphasicSolidDomain::Reset()
{
	// reset base class data
	FEElasticSolidDomain::Reset();

	// get the biphasic material
	FEBiphasic* pmb = dynamic_cast<FEBiphasic*>(GetMaterial());
	assert(pmb);

	// initialize all element data
	for (int i=0; i<(int) m_Elem.size(); ++i)
	{
		// get the solid element
		FESolidElement& el = m_Elem[i];
		
		// get the number of integration points
		int nint = el.GaussPoints();
		
		// loop over the integration points
		for (int n=0; n<nint; ++n)
		{
			FEMaterialPoint& mp = *el.m_State[n];
			FEBiphasicMaterialPoint& pt = *(mp.ExtractData<FEBiphasicMaterialPoint>());
			
			// initialize referential solid volume fraction
			pt.m_phi0 = pmb->m_phi0;
		}
	}
}

/*
//-----------------------------------------------------------------------------
void FEBiphasicSolidDomain::Residual(FESolver* psolver, vector<double>& R)
{
	int i, j;
	
	FEM& fem = dynamic_cast<FEM&>(psolver->GetFEModel());
	double dt = fem.GetCurrentStep()->m_dt;
	
	// element force vector
	vector<double> fe;

	vector<int> elm;
	
	int NE = m_Elem.size();

	if (fem.GetCurrentStep()->m_nanalysis == FE_STEADY_STATE) {
		for (i=0; i<NE; ++i)
		{
			// get the element
			FESolidElement& el = m_Elem[i];
		
			// get the element force vector and initialize it to zero
			int ndof = 3*el.Nodes();
			fe.assign(ndof, 0);
			
			// calculate internal force vector
			// (This function is inherited from FEElasticSolidDomain)
			FEElasticSolidDomain::ElementInternalForce(el, fe);
			
			// assemble element 'fe'-vector into global R vector
			UnpackLM(el, elm);
			psolver->AssembleResidual(el.m_node, elm, fe, R);
		
			// do biphasic forces
			FEMaterial* pm = m_pMat;
			assert(dynamic_cast<FEBiphasic*>(pm) != 0);
			
			// calculate fluid internal work
			ElementInternalFluidWorkSS(el, fe, dt);
			
			// add fluid work to global residual
			int neln = el.Nodes();
			int J;
			for (j=0; j<neln; ++j)
			{
				J = elm[3*neln+j];
				if (J >= 0) R[J] += fe[j];
			}
		}
	} else {
		for (i=0; i<NE; ++i)
		{
			// get the element
			FESolidElement& el = m_Elem[i];
			
			// unpack the element
			UnpackLM(el, elm);
			
			// get the element force vector and initialize it to zero
			int ndof = 3*el.Nodes();
			fe.assign(ndof, 0);
			
			// calculate internal force vector
			// (This function is inherited from FEElasticSolidDomain)
			FEElasticSolidDomain::ElementInternalForce(el, fe);
			
			// assemble element 'fe'-vector into global R vector
			psolver->AssembleResidual(el.m_node, elm, fe, R);
			
			// do biphasic forces
			FEMaterial* pm = m_pMat;
			assert(dynamic_cast<FEBiphasic*>(pm) != 0);
			
			// calculate fluid internal work
			ElementInternalFluidWork(el, fe, dt);
			
			// add fluid work to global residual
			int neln = el.Nodes();
			int J;
			for (j=0; j<neln; ++j)
			{
				J = elm[3*neln+j];
				if (J >= 0) R[J] += fe[j];
			}
		}
	}
}
*/

//-----------------------------------------------------------------------------
void FEBiphasicSolidDomain::InternalFluidWorkSS(vector<double>& R, double dt)
{
	int NE = m_Elem.size();

	#pragma omp parallel for shared(NE)
	for (int i=0; i<NE; ++i)
	{
		// element force vector
		vector<double> fe;
		vector<int> elm;
		
		// get the element
		FESolidElement& el = m_Elem[i];
		
		// get the element force vector and initialize it to zero
		int ndof = 3*el.Nodes();
		fe.assign(ndof, 0);
			
		// assemble element 'fe'-vector into global R vector
		UnpackLM(el, elm);
		
		// calculate fluid internal work
		ElementInternalFluidWorkSS(el, fe, dt);
			
		#pragma omp critical
		{
			// add fluid work to global residual
			int neln = el.Nodes();
			for (int j=0; j<neln; ++j)
			{
				int J = elm[3*neln+j];
				if (J >= 0) R[J] += fe[j];
			}
		}
	}
}


//-----------------------------------------------------------------------------
// Calculate the work due to the internal fluid pressure
void FEBiphasicSolidDomain::InternalFluidWork(vector<double>& R, double dt)
{
	int NE = m_Elem.size();
    
    #pragma omp parallel for shared(NE)
	for (int i=0; i<NE; ++i)
	{
		// element force vector
		vector<double> fe;
		vector<int> elm;
		
		// get the element
		FESolidElement& el = m_Elem[i];
			
		// unpack the element
		UnpackLM(el, elm);
			
		// get the element force vector and initialize it to zero
		int ndof = 3*el.Nodes();
		fe.assign(ndof, 0);
			
		// calculate fluid internal work
		ElementInternalFluidWork(el, fe, dt);
			
        #pragma omp critical
        {
            // add fluid work to global residual
            int neln = el.Nodes();
            for (int j=0; j<neln; ++j)
            {
                int J = elm[3*neln+j];
                if (J >= 0) R[J] += fe[j];
            }
        }
	}
}

//-----------------------------------------------------------------------------
//! calculates the internal equivalent nodal forces due to the fluid work
//! Note that we only use the first n entries in fe, where n is the number
//! of nodes

bool FEBiphasicSolidDomain::ElementInternalFluidWork(FESolidElement& el, vector<double>& fe, double dt)
{
	int i, n;
	
	int nint = el.GaussPoints();
	int neln = el.Nodes();
	
	// jacobian
	double Ji[3][3], detJ, J0i[3][3];
	
	double *Gr, *Gs, *Gt, *H;
	double Gx, Gy, Gz, GX, GY, GZ;
	
	// Bp-matrix
	vector<double> B1(neln), B2(neln), B3(neln);
	
	// gauss-weights
	double* wg = el.GaussWeights();
	
	FEMesh& mesh = *GetMesh();
	
	vec3d rp[FEElement::MAX_NODES];
	for (i=0; i<neln; ++i) 
	{
		rp[i] = mesh.Node(el.m_node[i]).m_rp;
	}
	
	// get the element's material
	FEBiphasic* pm = dynamic_cast<FEBiphasic*>(GetMaterial());
	assert(pm);
	
	zero(fe);
	
	// loop over gauss-points
	for (n=0; n<nint; ++n)
	{
		FEMaterialPoint& mp = *el.m_State[n];
		FEElasticMaterialPoint& ept = *(mp.ExtractData<FEElasticMaterialPoint>());
		FEBiphasicMaterialPoint& pt = *(el.m_State[n]->ExtractData<FEBiphasicMaterialPoint>());
		
		// calculate jacobian
		detJ = invjact(el, Ji, n);
		
		// we need to calculate the divergence of v. To do this we use
		// the formula div(v) = 1/J*dJdt, where J = det(F)
		double temp = 0;
		temp = invjac0(el, J0i, n);
		
		// next we calculate the deformation gradient
		mat3d Fp;
		Fp.zero();
		
		Gr = el.Gr(n);
		Gs = el.Gs(n);
		Gt = el.Gt(n);
		
		H = el.H(n);
		
		for (i=0; i<neln; ++i)
		{
			// calculate global gradient of shape functions
			// note that we need the transposed of Ji, not Ji itself !
			Gx = Ji[0][0]*Gr[i]+Ji[1][0]*Gs[i]+Ji[2][0]*Gt[i];
			Gy = Ji[0][1]*Gr[i]+Ji[1][1]*Gs[i]+Ji[2][1]*Gt[i];
			Gz = Ji[0][2]*Gr[i]+Ji[1][2]*Gs[i]+Ji[2][2]*Gt[i];
			
			GX = J0i[0][0]*Gr[i]+J0i[1][0]*Gs[i]+J0i[2][0]*Gt[i];
			GY = J0i[0][1]*Gr[i]+J0i[1][1]*Gs[i]+J0i[2][1]*Gt[i];
			GZ = J0i[0][2]*Gr[i]+J0i[1][2]*Gs[i]+J0i[2][2]*Gt[i];
			
			Fp[0][0] += rp[i].x*GX; Fp[1][0] += rp[i].y*GX; Fp[2][0] += rp[i].z*GX;
			Fp[0][1] += rp[i].x*GY; Fp[1][1] += rp[i].y*GY; Fp[2][1] += rp[i].z*GY;
			Fp[0][2] += rp[i].x*GZ; Fp[1][2] += rp[i].y*GZ; Fp[2][2] += rp[i].z*GZ;
			
			// calculate Bp matrix
			B1[i] = Gx;
			B2[i] = Gy;
			B3[i] = Gz;
		}
		
		// next we get the determinant
		double Jp = Fp.det();
		double J = ept.m_J;
		
		// and then finally
		double divv = ((J-Jp)/dt)/J;
		
		// get the flux
		vec3d& w = pt.m_w;

		// get the solvent supply
		double phiwhat = pm->SolventSupply(mp);
		
		// update force vector
		for (i=0; i<neln; ++i)
		{
			fe[i] -= dt*(B1[i]*w.x+B2[i]*w.y+B3[i]*w.z + (phiwhat - divv)*H[i])*detJ*wg[n];
		}
	}
	
	return true;
}

//-----------------------------------------------------------------------------
//! calculates the internal equivalent nodal forces due to the fluid work
//! for a steady-state analysis (solid velocity = 0)

bool FEBiphasicSolidDomain::ElementInternalFluidWorkSS(FESolidElement& el, vector<double>& fe, double dt)
{
	int i, n;
	
	int nint = el.GaussPoints();
	int neln = el.Nodes();
	
	// jacobian
	double Ji[3][3], detJ;
	
	double *Gr, *Gs, *Gt, *H;
	double Gx, Gy, Gz;
	
	// Bp-matrix
	vector<double> B1(neln), B2(neln), B3(neln);
	
	// gauss-weights
	double* wg = el.GaussWeights();
		
	// get the element's material
	FEBiphasic* pm = dynamic_cast<FEBiphasic*>(GetMaterial()); 
	assert(pm);
	
	zero(fe);
	
	// loop over gauss-points
	for (n=0; n<nint; ++n)
	{
		FEMaterialPoint& mp = *el.m_State[n];
		FEElasticMaterialPoint& ept = *(mp.ExtractData<FEElasticMaterialPoint>());
		FEBiphasicMaterialPoint& pt = *(el.m_State[n]->ExtractData<FEBiphasicMaterialPoint>());
		
		// calculate jacobian
		detJ = invjact(el, Ji, n);
		
		Gr = el.Gr(n);
		Gs = el.Gs(n);
		Gt = el.Gt(n);
		
		H = el.H(n);
		
		for (i=0; i<neln; ++i)
		{
			// calculate global gradient of shape functions
			// note that we need the transposed of Ji, not Ji itself !
			Gx = Ji[0][0]*Gr[i]+Ji[1][0]*Gs[i]+Ji[2][0]*Gt[i];
			Gy = Ji[0][1]*Gr[i]+Ji[1][1]*Gs[i]+Ji[2][1]*Gt[i];
			Gz = Ji[0][2]*Gr[i]+Ji[1][2]*Gs[i]+Ji[2][2]*Gt[i];
			
			// calculate Bp matrix
			B1[i] = Gx;
			B2[i] = Gy;
			B3[i] = Gz;
		}
		
		// get the solvent flux
		vec3d& w = pt.m_w;

		// get the solvent supply
		double phiwhat = pm->SolventSupply(mp);

		// update force vector
		for (i=0; i<neln; ++i)
		{
			fe[i] -= dt*(B1[i]*w.x+B2[i]*w.y+B3[i]*w.z + H[i]*phiwhat)*detJ*wg[n];
		}
	}
	
	return true;
}


//-----------------------------------------------------------------------------
/*
void FEBiphasicSolidDomain::StiffnessMatrix(FESolver* psolver)
{
	FEM& fem = dynamic_cast<FEM&>(psolver->GetFEModel());
	
	// element stiffness matrix
	matrix ke;

	// repeat over all solid elements
	int NE = m_Elem.size();
	if (fem.GetCurrentStep()->m_nanalysis == FE_STEADY_STATE) {
		for (int iel=0; iel<NE; ++iel)
		{
			FESolidElement& el = m_Elem[iel];
			
			// get the elements material
			FEMaterial* pmat = m_pMat;
			assert(dynamic_cast<FEBiphasic*>(pmat) != 0);
			
			// allocate stiffness matrix
			int neln = el.Nodes();
			int ndof = neln*4;
			ke.resize(ndof, ndof);
			
			// calculate the element stiffness matrix
			ElementBiphasicStiffnessSS(fem, el, ke);
			
			// TODO: the problem here is that the LM array that is returned by the UnpackLM
			// function does not give the equation numbers in the right order. For this reason we
			// have to create a new lm array and place the equation numbers in the right order.
			// What we really ought to do is fix the UnpackLM function so that it returns
			// the LM vector in the right order for poroelastic elements.
			vector<int> elm;
			UnpackLM(el, elm);

			vector<int> lm(ndof);
			for (int i=0; i<neln; ++i)
			{
				lm[4*i  ] = elm[3*i];
				lm[4*i+1] = elm[3*i+1];
				lm[4*i+2] = elm[3*i+2];
				lm[4*i+3] = elm[3*neln+i];
			}
			
			// assemble element matrix in global stiffness matrix
			psolver->AssembleStiffness(el.m_node, lm, ke);
		}
	} else {
		for (int iel=0; iel<NE; ++iel)
		{
			FESolidElement& el = m_Elem[iel];
			
			// get the elements material
			FEMaterial* pmat = m_pMat;
			assert(dynamic_cast<FEBiphasic*>(pmat) != 0);
			
			// allocate stiffness matrix
			int neln = el.Nodes();
			int ndof = neln*4;
			ke.resize(ndof, ndof);
			
			// calculate the element stiffness matrix
			ElementBiphasicStiffness(fem, el, ke);
			
			// TODO: the problem here is that the LM array that is returned by the UnpackLM
			// function does not give the equation numbers in the right order. For this reason we
			// have to create a new lm array and place the equation numbers in the right order.
			// What we really ought to do is fix the UnpackLM function so that it returns
			// the LM vector in the right order for poroelastic elements.
			vector<int> elm;
			UnpackLM(el, elm);

			vector<int> lm(ndof);
			for (int i=0; i<neln; ++i)
			{
				lm[4*i  ] = elm[3*i];
				lm[4*i+1] = elm[3*i+1];
				lm[4*i+2] = elm[3*i+2];
				lm[4*i+3] = elm[3*neln+i];
			}
			
			// assemble element matrix in global stiffness matrix
			psolver->AssembleStiffness(el.m_node, lm, ke);
		}
	}
}
*/

//-----------------------------------------------------------------------------
void FEBiphasicSolidDomain::StiffnessMatrix(FESolver* psolver, bool bsymm, double dt)
{
	// repeat over all solid elements
	int NE = m_Elem.size();
    
    #pragma omp parallel for shared(NE)
	for (int iel=0; iel<NE; ++iel)
	{
		// element stiffness matrix
		matrix ke;
		vector<int> elm;
		
		FESolidElement& el = m_Elem[iel];
		
		// allocate stiffness matrix
		int neln = el.Nodes();
		int ndof = neln*4;
		ke.resize(ndof, ndof);
		
		// calculate the element stiffness matrix
		ElementBiphasicStiffness(el, ke, bsymm, dt);
		
		// TODO: the problem here is that the LM array that is returned by the UnpackLM
		// function does not give the equation numbers in the right order. For this reason we
		// have to create a new lm array and place the equation numbers in the right order.
		// What we really ought to do is fix the UnpackLM function so that it returns
		// the LM vector in the right order for poroelastic elements.
		UnpackLM(el, elm);

        vector<int> lm(ndof);
        for (int i=0; i<neln; ++i)
        {
            lm[4*i  ] = elm[3*i];
            lm[4*i+1] = elm[3*i+1];
            lm[4*i+2] = elm[3*i+2];
            lm[4*i+3] = elm[3*neln+i];
        }
        
        // assemble element matrix in global stiffness matrix
        #pragma omp critical
        psolver->AssembleStiffness(el.m_node, lm, ke);
	}
}

//-----------------------------------------------------------------------------
void FEBiphasicSolidDomain::StiffnessMatrixSS(FESolver* psolver, bool bsymm, double dt)
{
	// repeat over all solid elements
	int NE = m_Elem.size();

	#pragma omp parallel for shared(NE)
	for (int iel=0; iel<NE; ++iel)
	{
		// element stiffness matrix
		matrix ke;
		vector<int> elm;
		
		FESolidElement& el = m_Elem[iel];
		
		// allocate stiffness matrix
		int neln = el.Nodes();
		int ndof = neln*4;
		ke.resize(ndof, ndof);
		
		// calculate the element stiffness matrix
		ElementBiphasicStiffnessSS(el, ke, bsymm, dt);
		
		// TODO: the problem here is that the LM array that is returned by the UnpackLM
		// function does not give the equation numbers in the right order. For this reason we
		// have to create a new lm array and place the equation numbers in the right order.
		// What we really ought to do is fix the UnpackLM function so that it returns
		// the LM vector in the right order for poroelastic elements.
		UnpackLM(el, elm);

		vector<int> lm(ndof);
		for (int i=0; i<neln; ++i)
		{
			lm[4*i  ] = elm[3*i];
			lm[4*i+1] = elm[3*i+1];
			lm[4*i+2] = elm[3*i+2];
			lm[4*i+3] = elm[3*neln+i];
		}
		
		// assemble element matrix in global stiffness matrix
		#pragma omp critical
		psolver->AssembleStiffness(el.m_node, lm, ke);
	}
}

//-----------------------------------------------------------------------------
//! calculates element stiffness matrix for element iel
//!
bool FEBiphasicSolidDomain::ElementBiphasicStiffness(FESolidElement& el, matrix& ke, bool bsymm, double dt)
{
	int i, j, n;
	
	int nint = el.GaussPoints();
	int neln = el.Nodes();
	
	double *Gr, *Gs, *Gt, *H;
	double Gx, Gy, Gz, GX, GY, GZ;
	
	// jacobian
	double Ji[3][3], detJ, J0i[3][3];
	
	// Bp-matrix
	vector<double> B1(neln), B2(neln), B3(neln);
	vector<vec3d> gradN(neln);
	double tmp;
	
	// gauss-weights
	double* gw = el.GaussWeights();
	
	FEMesh& mesh = *GetMesh();
	
	const int NE = FEElement::MAX_NODES;
	vec3d r0[NE], rt[NE], rp[NE], v[NE];
	for (i=0; i<neln; ++i) 
	{
		r0[i] = mesh.Node(el.m_node[i]).m_r0;
		rt[i] = mesh.Node(el.m_node[i]).m_rt;
		rp[i] = mesh.Node(el.m_node[i]).m_rp;
		v[i]  = mesh.Node(el.m_node[i]).m_vt;
	}
	
	// zero stiffness matrix
	ke.zero();
	
	// calculate solid stiffness matrix
	int ndof = 3*el.Nodes();
	matrix ks(ndof, ndof); ks.zero();
	SolidElementStiffness(el, ks);
	
	// copy solid stiffness matrix into ke
	for (i=0; i<neln; ++i)
		for (j=0; j<neln; ++j)
		{
			ke[4*i  ][4*j] = ks[3*i  ][3*j  ]; ke[4*i  ][4*j+1] = ks[3*i  ][3*j+1]; ke[4*i  ][4*j+2] = ks[3*i  ][3*j+2];
			ke[4*i+1][4*j] = ks[3*i+1][3*j  ]; ke[4*i+1][4*j+1] = ks[3*i+1][3*j+1]; ke[4*i+1][4*j+2] = ks[3*i+1][3*j+2];
			ke[4*i+2][4*j] = ks[3*i+2][3*j  ]; ke[4*i+2][4*j+1] = ks[3*i+2][3*j+1]; ke[4*i+2][4*j+2] = ks[3*i+2][3*j+2];
		}
	
	// get the element's material
	FEBiphasic* pm = dynamic_cast<FEBiphasic*>(GetMaterial());
	assert(pm);
	
	// loop over gauss-points
	for (n=0; n<nint; ++n)
	{
		FEMaterialPoint& mp = *el.m_State[n];
		FEElasticMaterialPoint& ept = *(mp.ExtractData<FEElasticMaterialPoint>());
		FEBiphasicMaterialPoint& pt = *(el.m_State[n]->ExtractData<FEBiphasicMaterialPoint>());
		
		// calculate jacobian
		detJ = invjact(el, Ji, n);

		vec3d g1(Ji[0][0],Ji[0][1],Ji[0][2]);
		vec3d g2(Ji[1][0],Ji[1][1],Ji[1][2]);
		vec3d g3(Ji[2][0],Ji[2][1],Ji[2][2]);
		
		// we need to calculate the divergence of v. To do this we use
		// the formula div(v) = 1/J*dJdt, where J = det(F)
		double temp = 0;
		temp = invjac0(el, J0i, n);
		vec3d G1(J0i[0][0],J0i[0][1],J0i[0][2]);
		vec3d G2(J0i[1][0],J0i[1][1],J0i[1][2]);
		vec3d G3(J0i[2][0],J0i[2][1],J0i[2][2]);
		
		// next we calculate the deformation gradient and the solid velocity
		mat3d Fp, gradv;
		Fp.zero();
		gradv.zero();
		vec3d vs(0);
		
		Gr = el.Gr(n);
		Gs = el.Gs(n);
		Gt = el.Gt(n);
		
		H = el.H(n);
		
		for (i=0; i<neln; ++i)
		{
			// calculate global gradient of shape functions
			// note that we need the transposed of Ji, not Ji itself !
			Gx = Ji[0][0]*Gr[i]+Ji[1][0]*Gs[i]+Ji[2][0]*Gt[i];
			Gy = Ji[0][1]*Gr[i]+Ji[1][1]*Gs[i]+Ji[2][1]*Gt[i];
			Gz = Ji[0][2]*Gr[i]+Ji[1][2]*Gs[i]+Ji[2][2]*Gt[i];
			
			GX = J0i[0][0]*Gr[i]+J0i[1][0]*Gs[i]+J0i[2][0]*Gt[i];
			GY = J0i[0][1]*Gr[i]+J0i[1][1]*Gs[i]+J0i[2][1]*Gt[i];
			GZ = J0i[0][2]*Gr[i]+J0i[1][2]*Gs[i]+J0i[2][2]*Gt[i];
			
			Fp[0][0] += rp[i].x*GX; Fp[1][0] += rp[i].y*GX; Fp[2][0] += rp[i].z*GX;
			Fp[0][1] += rp[i].x*GY; Fp[1][1] += rp[i].y*GY; Fp[2][1] += rp[i].z*GY;
			Fp[0][2] += rp[i].x*GZ; Fp[1][2] += rp[i].y*GZ; Fp[2][2] += rp[i].z*GZ;
			
			// calculate solid velocity and its gradient
			vs += v[i]*H[i];
			gradv[0][0] += v[i].x*Gx; gradv[1][0] += v[i].y*Gx; gradv[2][0] += v[i].z*Gx;
			gradv[0][1] += v[i].x*Gy; gradv[1][1] += v[i].y*Gy; gradv[2][1] += v[i].z*Gy;
			gradv[0][2] += v[i].x*Gz; gradv[1][2] += v[i].y*Gz; gradv[2][2] += v[i].z*Gz;
			
			// calculate Bp matrix
			B1[i] = Gx;
			B2[i] = Gy;
			B3[i] = Gz;
			gradN[i] = vec3d(Gx,Gy,Gz);
			
		}
		
		// next we get the determinant
		double Jp = Fp.det();
		double J = ept.m_J;
		
		// and then finally
		double dJdt = (J-Jp)/dt;
		double divv = dJdt/J;
		
		// get the fluid flux and pressure gradient
		vec3d w = pt.m_w;
		vec3d gradp = pt.m_gradp;
		
		// evaluate the permeability and its derivatives
		mat3ds K = pm->Permeability(mp);
		tens4ds dKdE = pm->GetPermeability()->Tangent_Permeability_Strain(mp);

		// evaluate the solvent supply and its derivatives
		double phiwhat = 0;
		mat3ds Phie; Phie.zero();
		double Phip = 0;
		if (pm->GetSolventSupply()) {
			phiwhat = pm->GetSolventSupply()->Supply(mp);
			Phie = pm->GetSolventSupply()->Tangent_Supply_Strain(mp);
			Phip = pm->GetSolventSupply()->Tangent_Supply_Pressure(mp);
		}
		
		// Miscellaneous constants
		mat3dd I(1);
		
		// calculate the kpp matrix
		//		tmp = detJ*gw[n];
		tmp = detJ*gw[n]*dt;
		for (i=0; i<neln; ++i)
			for (j=0; j<neln; ++j)
			{
				ke[4*i+3][4*j+3] += (H[i]*H[j]*Phip - gradN[i]*(K*gradN[j]))*tmp;
			}
		
		if (!bsymm) {
			// calculate the kup matrix
			for (i=0; i<neln; ++i) {
				for (j=0; j<neln; ++j)
				{
					tmp = detJ*gw[n]*H[j];
					ke[4*i  ][4*j+3] -= tmp*gradN[i].x;
					ke[4*i+1][4*j+3] -= tmp*gradN[i].y;
					ke[4*i+2][4*j+3] -= tmp*gradN[i].z;
				}
			}
			
			// calculate the kpu matrix
			//			tmp = detJ*gw[n];
			tmp = detJ*gw[n]*dt;
			for (i=0; i<neln; ++i) {
				for (j=0; j<neln; ++j)
				{
					vec3d vt = (-(vdotTdotv(gradN[i], dKdE, gradN[j])*(gradp))
								-(I*(divv+1./dt) - gradv.transpose() - Phie)*gradN[j]*H[i])*tmp;
					ke[4*i+3][4*j  ] += vt.x;
					ke[4*i+3][4*j+1] += vt.y;
					ke[4*i+3][4*j+2] += vt.z;
				}
			}
			
		} else {
			// calculate the kup matrix and let kpu be its symmetric part
			tmp = detJ*gw[n];
			for (i=0; i<neln; ++i) {
				for (j=0; j<neln; ++j)
				{
					ke[4*i  ][4*j+3] -= tmp*H[j]*gradN[i].x;
					ke[4*i+1][4*j+3] -= tmp*H[j]*gradN[i].y;
					ke[4*i+2][4*j+3] -= tmp*H[j]*gradN[i].z;
					
					ke[4*i+3][4*j  ] -= tmp*H[i]*gradN[j].x;
					ke[4*i+3][4*j+1] -= tmp*H[i]*gradN[j].y;
					ke[4*i+3][4*j+2] -= tmp*H[i]*gradN[j].z;
				}
			}
		}
	}
	return true;
}

//-----------------------------------------------------------------------------
//! calculates element stiffness matrix for element iel
//! for the steady-state response (zero solid velocity)
//!
bool FEBiphasicSolidDomain::ElementBiphasicStiffnessSS(FESolidElement& el, matrix& ke, bool bsymm, double dt)
{
	int i, j, n;
	
	int nint = el.GaussPoints();
	int neln = el.Nodes();
	
	double *Gr, *Gs, *Gt, *H;
	double Gx, Gy, Gz;
	
	// jacobian
	double Ji[3][3], detJ;
	
	// Bp-matrix
	vector<double> B1(neln), B2(neln), B3(neln);
	vector<vec3d> gradN(neln);
	double tmp;
	
	// gauss-weights
	double* gw = el.GaussWeights();
	
	// zero stiffness matrix
	ke.zero();
	
	// calculate solid stiffness matrix
	int ndof = 3*el.Nodes();
	matrix ks(ndof, ndof); ks.zero();
	SolidElementStiffness(el, ks);
	
	// copy solid stiffness matrix into ke
	for (i=0; i<neln; ++i)
		for (j=0; j<neln; ++j)
		{
			ke[4*i  ][4*j] = ks[3*i  ][3*j  ]; ke[4*i  ][4*j+1] = ks[3*i  ][3*j+1]; ke[4*i  ][4*j+2] = ks[3*i  ][3*j+2];
			ke[4*i+1][4*j] = ks[3*i+1][3*j  ]; ke[4*i+1][4*j+1] = ks[3*i+1][3*j+1]; ke[4*i+1][4*j+2] = ks[3*i+1][3*j+2];
			ke[4*i+2][4*j] = ks[3*i+2][3*j  ]; ke[4*i+2][4*j+1] = ks[3*i+2][3*j+1]; ke[4*i+2][4*j+2] = ks[3*i+2][3*j+2];
		}
	
	// get the element's material
	FEBiphasic* pm = dynamic_cast<FEBiphasic*>(GetMaterial());
	assert(pm);
	
	// loop over gauss-points
	for (n=0; n<nint; ++n)
	{
		FEMaterialPoint& mp = *el.m_State[n];
		FEBiphasicMaterialPoint& pt = *(el.m_State[n]->ExtractData<FEBiphasicMaterialPoint>());
		
		// calculate jacobian
		detJ = invjact(el, Ji, n);

		vec3d g1(Ji[0][0],Ji[0][1],Ji[0][2]);
		vec3d g2(Ji[1][0],Ji[1][1],Ji[1][2]);
		vec3d g3(Ji[2][0],Ji[2][1],Ji[2][2]);
		
		Gr = el.Gr(n);
		Gs = el.Gs(n);
		Gt = el.Gt(n);
		
		H = el.H(n);
		
		for (i=0; i<neln; ++i)
		{
			// calculate global gradient of shape functions
			// note that we need the transposed of Ji, not Ji itself !
			Gx = Ji[0][0]*Gr[i]+Ji[1][0]*Gs[i]+Ji[2][0]*Gt[i];
			Gy = Ji[0][1]*Gr[i]+Ji[1][1]*Gs[i]+Ji[2][1]*Gt[i];
			Gz = Ji[0][2]*Gr[i]+Ji[1][2]*Gs[i]+Ji[2][2]*Gt[i];
			
			// calculate Bp matrix
			B1[i] = Gx;
			B2[i] = Gy;
			B3[i] = Gz;
			gradN[i] = vec3d(Gx,Gy,Gz);
			
		}
		
		// get the fluid flux and pressure gradient
		vec3d w = pt.m_w;
		vec3d gradp = pt.m_gradp;
		
		// evaluate the permeability and its derivatives
		mat3ds K = pm->Permeability(mp);
		tens4ds dKdE = pm->GetPermeability()->Tangent_Permeability_Strain(mp);

		// evaluate the solvent supply and its derivatives
		double phiwhat = 0;
		mat3ds Phie; Phie.zero();
		double Phip = 0;
		if (pm->GetSolventSupply()) {
			phiwhat = pm->GetSolventSupply()->Supply(mp);
			Phie = pm->GetSolventSupply()->Tangent_Supply_Strain(mp);
			Phip = pm->GetSolventSupply()->Tangent_Supply_Pressure(mp);
		}

		// Miscellaneous constants
		mat3dd I(1);
		
		// calculate the kpp matrix
		//		tmp = detJ*gw[n];
		tmp = detJ*gw[n]*dt;
		for (i=0; i<neln; ++i)
			for (j=0; j<neln; ++j)
			{
				ke[4*i+3][4*j+3] += (H[i]*H[j]*Phip - gradN[i]*(K*gradN[j]))*tmp;
			}
		
		if (!bsymm) {
			// calculate the kup matrix
			for (i=0; i<neln; ++i) {
				for (j=0; j<neln; ++j)
				{
					tmp = detJ*gw[n]*H[j];
					ke[4*i  ][4*j+3] -= tmp*gradN[i].x;
					ke[4*i+1][4*j+3] -= tmp*gradN[i].y;
					ke[4*i+2][4*j+3] -= tmp*gradN[i].z;
				}
			}
			
			// calculate the kpu matrix
			//			tmp = detJ*gw[n];
			tmp = detJ*gw[n]*dt;
			for (i=0; i<neln; ++i) {
				for (j=0; j<neln; ++j)
				{
					vec3d vt = (-(vdotTdotv(gradN[i], dKdE, gradN[j])*(gradp))
								+Phie*gradN[j]*H[i])*tmp;
					ke[4*i+3][4*j  ] += vt.x;
					ke[4*i+3][4*j+1] += vt.y;
					ke[4*i+3][4*j+2] += vt.z;
				}
			}
			
		} else {
			// calculate the kup matrix and let kpu be its symmetric part
			tmp = detJ*gw[n];
			for (i=0; i<neln; ++i) {
				for (j=0; j<neln; ++j)
				{
					ke[4*i  ][4*j+3] -= tmp*H[j]*gradN[i].x;
					ke[4*i+1][4*j+3] -= tmp*H[j]*gradN[i].y;
					ke[4*i+2][4*j+3] -= tmp*H[j]*gradN[i].z;
					
					ke[4*i+3][4*j  ] -= tmp*H[i]*gradN[j].x;
					ke[4*i+3][4*j+1] -= tmp*H[i]*gradN[j].y;
					ke[4*i+3][4*j+2] -= tmp*H[i]*gradN[j].z;
				}
			}
		}
	}
	return true;
}

//-----------------------------------------------------------------------------
//! This function calculates the element stiffness matrix. It calls the material
//! stiffness function, the geometrical stiffness function and, if necessary, the
//! dilatational stiffness function. Note that these three functions only calculate
//! the upper diagonal matrix due to the symmetry of the element stiffness matrix
//! The last section of this function fills the rest of the element stiffness matrix.

void FEBiphasicSolidDomain::SolidElementStiffness(FESolidElement& el, matrix& ke)
{
	// calculate material stiffness (i.e. constitutive component)
	ElementBiphasicMaterialStiffness(el, ke);
	
	// calculate geometrical stiffness (inherited from FEElasticSolidDomain)
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
//! Calculates element material stiffness element matrix

void FEBiphasicSolidDomain::ElementBiphasicMaterialStiffness(FESolidElement &el, matrix &ke)
{
	int i, i3, j, j3, n;
	
	// Get the current element's data
	const int nint = el.GaussPoints();
	const int neln = el.Nodes();
	
	// global derivatives of shape functions
	// NOTE: hard-coding of hex elements!
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
	
	// see if this is a biphasic material
	FEBiphasic* pmat = dynamic_cast<FEBiphasic*>(GetMaterial());
	assert(pmat);

	// nodal pressures
	double pn[FEElement::MAX_NODES];
	for (i=0; i<neln; ++i) pn[i] = m_pMesh->Node(el.m_node[i]).m_pt;
	
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

		// evaluate fluid pressure at gauss-point
		FEBiphasicMaterialPoint& ppt = *(mp.ExtractData<FEBiphasicMaterialPoint>());
		ppt.m_p = el.Evaluate(pn, n);
		
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
void FEBiphasicSolidDomain::UpdateStresses(FEModel &fem)
{
	bool berr = false;
	int NE = (int) m_Elem.size();
	#pragma omp parallel for shared(NE, berr)
	for (int i=0; i<NE; ++i)
	{
		try
		{
			UpdateElementStress(i);
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
void FEBiphasicSolidDomain::UpdateElementStress(int iel)
{
   // extract the elastic component
    FEElasticMaterial* pme = m_pMat->GetElasticMaterial();

	// get the solid element
	FESolidElement& el = m_Elem[iel];
		
	// get the number of integration points
	int nint = el.GaussPoints();
		
	// get the integration weights
	double* gw = el.GaussWeights();

	// get the number of nodes
	int neln = el.Nodes();

	// get the nodal data
	FEMesh& mesh = *m_pMesh;
	vec3d r0[FEElement::MAX_NODES];
	vec3d rt[FEElement::MAX_NODES];
	double pn[FEElement::MAX_NODES];
	for (int j=0; j<neln; ++j)
	{
		r0[j] = mesh.Node(el.m_node[j]).m_r0;
		rt[j] = mesh.Node(el.m_node[j]).m_rt;
		pn[j] = mesh.Node(el.m_node[j]).m_pt;
	}

	// get the biphasic material
	FEBiphasic* pmb = dynamic_cast<FEBiphasic*>(GetMaterial());
	assert(pmb);
		
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
			
		// poroelasticity data
		FEBiphasicMaterialPoint& ppt = *(mp.ExtractData<FEBiphasicMaterialPoint>());
			
		// evaluate fluid pressure at gauss-point
		ppt.m_p = el.Evaluate(pn, n);
			
		// calculate the gradient of p at gauss-point
		ppt.m_gradp = gradient(el, pn, n);
			
		// for biphasic materials also update the fluid flux
		ppt.m_w = pmb->Flux(mp);
		ppt.m_pa = pmb->Pressure(mp);
			
		// calculate the stress at this material point
		pt.m_s = pmb->Stress(mp);
	}
}
