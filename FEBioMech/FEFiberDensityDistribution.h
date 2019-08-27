//
//  FEFiberDensityDistribution.h
//
//  Created by Gerard Ateshian on 11/16/13.
//

#pragma once

#include "FECore/FEMaterial.h"

//---------------------------------------------------------------------------
// Base class for fiber density distribution functions
//
class FEFiberDensityDistribution : public FEMaterial
{
public:
    FEFiberDensityDistribution(FEModel* pfem) : FEMaterial(pfem) { m_IFD = 1; }
    
    // Initialization function
    virtual void Init() = 0;
    
    // Evaluation of fiber density along n0
    virtual double FiberDensity(const vec3d n0) = 0;
    
public:
    double m_IFD;      // integrated fiber density
};

//---------------------------------------------------------------------------
// Spherical fiber density distribution
//
class FESphericalFiberDensityDistribution : public FEFiberDensityDistribution
{
public:
    FESphericalFiberDensityDistribution(FEModel* pfem) : FEFiberDensityDistribution(pfem) {}
    
    void Init() {}
    
    double FiberDensity(const vec3d n0) { return 1.0/m_IFD; }  
};

//---------------------------------------------------------------------------
// Ellipsoidal fiber density distribution
//
class FEEllipsodialFiberDensityDistribution : public FEFiberDensityDistribution
{
public:
    FEEllipsodialFiberDensityDistribution(FEModel* pfem) : FEFiberDensityDistribution(pfem) {}
    
    void Init();
    
    double FiberDensity(const vec3d n0);
    
public:
    double m_spa[3];    // semi-principal axes of ellipsoid
    
	// declare the parameter list
	DECLARE_PARAMETER_LIST();
};

//---------------------------------------------------------------------------
// 3D axisymmetric von Mises fiber density distribution
//
class FEVonMises3DFiberDensityDistribution : public FEFiberDensityDistribution
{
public:
    FEVonMises3DFiberDensityDistribution(FEModel* pfem) : FEFiberDensityDistribution(pfem) {}
    
    void Init();
    
    double FiberDensity(const vec3d n0);
    
public:
    double m_b;         // concentration parameter
    
	// declare the parameter list
	DECLARE_PARAMETER_LIST();
};

//---------------------------------------------------------------------------
// Circular fiber density distribution (2d)
//
class FECircularFiberDensityDistribution : public FEFiberDensityDistribution
{
public:
    FECircularFiberDensityDistribution(FEModel* pfem) : FEFiberDensityDistribution(pfem) {}
    
    void Init() {}
    
    double FiberDensity(const vec3d n0) { return 1.0/m_IFD; }
};

//---------------------------------------------------------------------------
// Elliptical fiber density distribution (2d)
//
class FEEllipticalFiberDensityDistribution : public FEFiberDensityDistribution
{
public:
    FEEllipticalFiberDensityDistribution(FEModel* pfem) : FEFiberDensityDistribution(pfem) {}
    
    void Init();
    
    double FiberDensity(const vec3d n0);
    
public:
    double m_spa[2];    // semi-principal axes of ellipse
    
	// declare the parameter list
	DECLARE_PARAMETER_LIST();
};

//---------------------------------------------------------------------------
// 2D axisymmetric von Mises fiber density distribution
//
class FEVonMises2DFiberDensityDistribution : public FEFiberDensityDistribution
{
public:
    FEVonMises2DFiberDensityDistribution(FEModel* pfem) : FEFiberDensityDistribution(pfem) {}
    
    void Init();
    
    double FiberDensity(const vec3d n0);
    
public:
    double m_b;         // concentration parameter
    
	// declare the parameter list
	DECLARE_PARAMETER_LIST();
};
