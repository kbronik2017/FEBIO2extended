#include "FEVonMisesPlasticity.h"

//-----------------------------------------------------------------------------
// define the material parameters
BEGIN_PARAMETER_LIST(FEVonMisesPlasticity, FEElasticMaterial)
	ADD_PARAMETER(m_E, FE_PARAM_DOUBLE, "E");
	ADD_PARAMETER(m_v, FE_PARAM_DOUBLE, "v");
	ADD_PARAMETER(m_Y, FE_PARAM_DOUBLE, "Y");
	ADD_PARAMETER(m_H, FE_PARAM_DOUBLE, "H");
END_PARAMETER_LIST();


//-----------------------------------------------------------------------------
FEVonMisesPlasticity::FEVonMisesPlasticity(FEModel* pfem) : FEElasticMaterial(pfem)
{
	m_E = m_v = m_Y = m_H = 0;
	m_K = m_G = 0;
}

//-----------------------------------------------------------------------------
void FEVonMisesPlasticity::Init()
{
	if (m_E <= 0) throw MaterialError("E must be postive number");
	if ((m_v < -1)||(m_v >= 0.5)) throw MaterialError("v must be in the range [-1, 0.5]");
	if (m_Y <= 0) throw MaterialError("Y must be postitive number");
	if (m_H <  0) throw MaterialError("H must be postitive number");

	m_K = m_E/(3.0*(1.0 - 2*m_v));
	m_G = m_E/(2.0*(1.0 +   m_v));
}

//-----------------------------------------------------------------------------
mat3ds FEVonMisesPlasticity::Stress(FEMaterialPoint &mp)
{
	FEElasticMaterialPoint& pt = *mp.ExtractData<FEElasticMaterialPoint>();
	FEJ2PlasticMaterialPoint& pp = *mp.ExtractData<FEJ2PlasticMaterialPoint>();
	mat3d& F = pt.m_F;
	
	// get the current strain
	mat3ds e = F.sym() - mat3dd(1.0);

	// calculate strain increment
	mat3ds de = (e - pp.e0);

	// get the trial stress
	mat3ds strial = pp.sn + (de.dev()*(2.0*m_G) + de.iso()*(3.0*m_K));
	mat3ds dev_strial = strial.dev();
	double devs_norm = dev_strial.norm();

	// get current yield strenght
	double Y = pp.Y0;

	double k = Y / sqrt(3.0);
	double fac = devs_norm / (sqrt(2.0)*k);

	mat3ds s;
	if (fac<=1)
	{
		s = strial;
		pp.b = false;
	}
	else
	{
		// calculate plastic strain rate
		double L = (devs_norm - sqrt(2.0/3.0)*Y)/(2*m_G + m_H);

		// update yield strength
		pp.Y1 = Y + m_H*L/sqrt(2.0/3.0);

		// update stress
		s = strial.iso() + dev_strial*(1.0 - 2.0*m_G*L/devs_norm);
		pp.b = true;
	}

	// store the current strain measure
	pp.e1 = e;

	return s;
}

//-----------------------------------------------------------------------------
tens4ds FEVonMisesPlasticity::Tangent(FEMaterialPoint &mp)
{
	FEElasticMaterialPoint& pt = *mp.ExtractData<FEElasticMaterialPoint>();
	FEJ2PlasticMaterialPoint& pp = *mp.ExtractData<FEJ2PlasticMaterialPoint>();

	// lame parameters
	double lam = m_K - m_G*2.0/3.0;
	double mu  = m_G;

	double D[6][6] = {0};
	D[0][0] = lam+2.*mu; D[0][1] = lam      ; D[0][2] = lam      ;
	D[1][0] = lam      ; D[1][1] = lam+2.*mu; D[1][2] = lam      ;
	D[2][0] = lam      ; D[2][1] = lam      ; D[2][2] = lam+2.*mu;
	D[3][3] = mu;
	D[4][4] = mu;
	D[5][5] = mu;
	tens4ds C(D);

	// see if we are in plastic flow mode
	if (pp.b)
	{
		// get the stress
		mat3ds s = pt.m_s;
		mat3ds n = s.dev()*2.0;

		mat3ds A = C.dot(n);
		double G = n.dotdot(A) + m_H;

		C -= dyad4s(A)/G;
	}

	return C;
}
