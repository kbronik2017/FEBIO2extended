// FEVonMisesFibers.h: interface for the FEVonMisesFibers class.
//
//////////////////////////////////////////////////////////////////////
 //en gros, les lignes suivantes veulent dire: si le current file n'a pas encore été inclus, définis et compile-le, sinon passe ton chemin
#if !defined(AFX_FEVonMisesFibers_H__E918D89B_4CCD_44B9_9731_19CEC4EDF406__INCLUDED_)
#define AFX_FEVonMisesFibers_H__E918D89B_4CCD_44B9_9731_19CEC4EDF406__INCLUDED_

//si la version de visual basic est suffisamment récente, on peut utiliser pragma once pour s'assurer qu'on inclut le fichier une seule fois:
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FETransverselyIsotropic.h"

//-----------------------------------------------------------------------------
//! Transversely Isotropic Multiple material

//! This material has an isotopric Multiple basis and single preferred
//! fiber direction.

class FEMRVonMisesFibers: public FETransverselyIsotropic
{
public:
	FEMRVonMisesFibers (FEModel* pfem) : FETransverselyIsotropic(pfem) {}

public:
	double	c1;	//!< Mooney-Rivlin coefficient C1
	double	c2;	//!< Mooney-Rivlin coefficient C2
	double	kf;	//!< Fiber Concentration Factor
	double	tp;	//!< Preferred Fiber Orientation IN RADIANS
	int	gipt;	//!< Gauss Integration Points
	int	vmc;	//!< Choice of von Mises distribution 
	double var_n;	//!< Exponent for the constrained von Mises distribution

public:
	//! calculate stress at material point
	virtual mat3ds DevStress(FEMaterialPoint& pt);

	//! calculate tangent stiffness at material point
	virtual tens4ds DevTangent(FEMaterialPoint& pt);

	// declare parameter list
	DECLARE_PARAMETER_LIST();
};


#endif // !defined(AFX_FEVonMisesFibers_H__E918D89B_4CCD_44B9_9731_19CEC4EDF406__INCLUDED_)

