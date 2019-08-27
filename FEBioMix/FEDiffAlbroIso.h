//
//  FEDiffAlbroIso.h
//  FEBio
//
//  Created by Gerard Ateshian on 12/17/13.
//  Copyright (c) 2013 febio.org. All rights reserved.
//

#pragma once
#include "FEBiphasicSolute.h"

//-----------------------------------------------------------------------------
// This class implements a material that has a porosity and concentration
// dependent diffusivity which is isotropic, according to the constitutive relation
// of Albro et al (CMBE 2009)

class FEDiffAlbroIso : public FESoluteDiffusivity
{
public:
    //! constructor
    FEDiffAlbroIso(FEModel* pfem);
    
    //! free diffusivity
    double Free_Diffusivity(FEMaterialPoint& pt);
    
    //! Tangent of free diffusivity with respect to concentration
    double Tangent_Free_Diffusivity_Concentration(FEMaterialPoint& pt, const int isol);
    
    //! diffusivity
    mat3ds Diffusivity(FEMaterialPoint& pt);
    
    //! Tangent of diffusivity with respect to strain
    tens4ds Tangent_Diffusivity_Strain(FEMaterialPoint& mp);
    
    //! Tangent of diffusivity with respect to concentration
    mat3ds Tangent_Diffusivity_Concentration(FEMaterialPoint& mp, const int isol=0);
    
    //! data initialization and checking
    void Init();
    
public:
    double	m_diff0;        //!< free diffusivity
    double	m_cdinv;		//!< inverse of characteristic concentration c_D
    double	m_alphad;		//!< non-dimensional coefficient for porosity term
    int     m_lsol;         //!< local ID of solute
    
    // declare parameter list
    DECLARE_PARAMETER_LIST();
};
