#pragma once
#include "FECore/DataStore.h"
#include "FECore/NodeDataRecord.h"
#include "FECore/ElementDataRecord.h"
#include "FECore/ObjectDataRecord.h"

//=============================================================================
// N O D E  D A T A
//=============================================================================

//-----------------------------------------------------------------------------
class FENodeXPos : public FENodeLogData
{ 
public: 
	FENodeXPos(FEModel* pfem) : FENodeLogData(pfem){} 
	double value(int node); 
};

//-----------------------------------------------------------------------------
class FENodeYPos : public FENodeLogData 
{ 
public: 
	FENodeYPos(FEModel* pfem) : FENodeLogData(pfem){} 
	double value(int node); 
};

//-----------------------------------------------------------------------------
class FENodeZPos : public FENodeLogData
{ 
public: 
	FENodeZPos(FEModel* pfem) : FENodeLogData(pfem){} 
	double value(int node); 
};

//-----------------------------------------------------------------------------
class FENodeXDisp : public FENodeLogData
{ 
public: 
	FENodeXDisp(FEModel* pfem) : FENodeLogData(pfem){} 
	double value(int node); 
};

//-----------------------------------------------------------------------------
class FENodeYDisp : public FENodeLogData
{ 
public: 
	FENodeYDisp(FEModel* pfem) : FENodeLogData(pfem){} 
	double value(int node); 
};

//-----------------------------------------------------------------------------
class FENodeZDisp : public FENodeLogData
{ 
public: 
	FENodeZDisp(FEModel* pfem) : FENodeLogData(pfem){} 
	double value(int node); 
};

//-----------------------------------------------------------------------------
class FENodeXVel : public FENodeLogData
{ 
public: 
	FENodeXVel(FEModel* pfem) : FENodeLogData(pfem){} 
	double value(int node); 
};

//-----------------------------------------------------------------------------
class FENodeYVel : public FENodeLogData
{ 
public: 
	FENodeYVel(FEModel* pfem) : FENodeLogData(pfem){} 
	double value(int node); 
};

//-----------------------------------------------------------------------------
class FENodeZVel : public FENodeLogData
{ 
public: 
	FENodeZVel(FEModel* pfem) : FENodeLogData(pfem){} 
	double value(int node); 
};

//-----------------------------------------------------------------------------
class FENodeForceX: public FENodeLogData
{ 
public: 
	FENodeForceX(FEModel* pfem) : FENodeLogData(pfem){} 
	double value(int node); 
};

//-----------------------------------------------------------------------------
class FENodeForceY: public FENodeLogData
{ 
public: 
	FENodeForceY(FEModel* pfem) : FENodeLogData(pfem){} 
	double value(int node); 
};

//-----------------------------------------------------------------------------
class FENodeForceZ: public FENodeLogData
{ 
public: 
	FENodeForceZ(FEModel* pfem) : FENodeLogData(pfem){} 
	double value(int node); 
};

//=============================================================================
// E L E M E N T   D A T A
//=============================================================================

//-----------------------------------------------------------------------------
class FELogElemPosX : public FELogElemData
{
public:
	FELogElemPosX(FEModel* pfem) : FELogElemData(pfem){}
	double value(FEElement& el);
};

//-----------------------------------------------------------------------------
class FELogElemPosY : public FELogElemData
{
public:
	FELogElemPosY(FEModel* pfem) : FELogElemData(pfem){}
	double value(FEElement& el);
};

//-----------------------------------------------------------------------------
class FELogElemPosZ : public FELogElemData
{
public:
	FELogElemPosZ(FEModel* pfem) : FELogElemData(pfem){}
	double value(FEElement& el);
};

//-----------------------------------------------------------------------------
class FELogElemJacobian : public FELogElemData
{
public:
	FELogElemJacobian(FEModel* pfem) : FELogElemData(pfem){}
	double value(FEElement& el);
};

//-----------------------------------------------------------------------------
class FELogElemStrainX : public FELogElemData
{
public:
	FELogElemStrainX(FEModel* pfem) : FELogElemData(pfem){}
	double value(FEElement& el);
};

//-----------------------------------------------------------------------------
class FELogElemStrainY : public FELogElemData
{
public:
	FELogElemStrainY(FEModel* pfem) : FELogElemData(pfem){}
	double value(FEElement& el);
};

//-----------------------------------------------------------------------------
class FELogElemStrainZ : public FELogElemData
{
public:
	FELogElemStrainZ(FEModel* pfem) : FELogElemData(pfem){}
	double value(FEElement& el);
};

//-----------------------------------------------------------------------------
class FELogElemStrainXY : public FELogElemData
{
public:
	FELogElemStrainXY(FEModel* pfem) : FELogElemData(pfem){}
	double value(FEElement& el);
};

//-----------------------------------------------------------------------------
class FELogElemStrainYZ : public FELogElemData
{
public:
	FELogElemStrainYZ(FEModel* pfem) : FELogElemData(pfem){}
	double value(FEElement& el);
};

//-----------------------------------------------------------------------------
class FELogElemStrainXZ : public FELogElemData
{
public:
	FELogElemStrainXZ(FEModel* pfem) : FELogElemData(pfem){}
	double value(FEElement& el);
};

//-----------------------------------------------------------------------------
class FELogElemStrain1 : public FELogElemData
{
public:
	FELogElemStrain1(FEModel* pfem) : FELogElemData(pfem){}
	double value(FEElement& el);
};

//-----------------------------------------------------------------------------
class FELogElemStrain2 : public FELogElemData
{
public:
	FELogElemStrain2(FEModel* pfem) : FELogElemData(pfem){}
	double value(FEElement& el);
};

//-----------------------------------------------------------------------------
class FELogElemStrain3 : public FELogElemData
{
public:
	FELogElemStrain3(FEModel* pfem) : FELogElemData(pfem){}
	double value(FEElement& el);
};

//-----------------------------------------------------------------------------
class FELogElemStressX : public FELogElemData
{
public:
	FELogElemStressX(FEModel* pfem) : FELogElemData(pfem){}
	double value(FEElement& el);
};

//-----------------------------------------------------------------------------
class FELogElemStressY : public FELogElemData
{
public:
	FELogElemStressY(FEModel* pfem) : FELogElemData(pfem){}
	double value(FEElement& el);
};

//-----------------------------------------------------------------------------
class FELogElemStressZ : public FELogElemData
{
public:
	FELogElemStressZ(FEModel* pfem) : FELogElemData(pfem){}
	double value(FEElement& el);
};

//-----------------------------------------------------------------------------
class FELogElemStressXY : public FELogElemData
{
public:
	FELogElemStressXY(FEModel* pfem) : FELogElemData(pfem){}
	double value(FEElement& el);
};

//-----------------------------------------------------------------------------
class FELogElemStressYZ : public FELogElemData
{
public:
	FELogElemStressYZ(FEModel* pfem) : FELogElemData(pfem){}
	double value(FEElement& el);
};

//-----------------------------------------------------------------------------
class FELogElemStressXZ : public FELogElemData
{
public:
	FELogElemStressXZ(FEModel* pfem) : FELogElemData(pfem){}
	double value(FEElement& el);
};

//-----------------------------------------------------------------------------
class FELogElemStress1 : public FELogElemData
{
public:
	FELogElemStress1(FEModel* pfem) : FELogElemData(pfem){}
	double value(FEElement& el);
};

//-----------------------------------------------------------------------------
class FELogElemStress2 : public FELogElemData
{
public:
	FELogElemStress2(FEModel* pfem) : FELogElemData(pfem){}
	double value(FEElement& el);
};

//-----------------------------------------------------------------------------
class FELogElemStress3 : public FELogElemData
{
public:
	FELogElemStress3(FEModel* pfem) : FELogElemData(pfem){}
	double value(FEElement& el);
};

//-----------------------------------------------------------------------------
class FELogElemDeformationGradientXX : public FELogElemData
{
public:
	FELogElemDeformationGradientXX(FEModel* pfem) : FELogElemData(pfem){}
	double value(FEElement& el);
};

//-----------------------------------------------------------------------------
class FELogElemDeformationGradientXY : public FELogElemData
{
public:
	FELogElemDeformationGradientXY(FEModel* pfem) : FELogElemData(pfem){}
	double value(FEElement& el);
};

//-----------------------------------------------------------------------------
class FELogElemDeformationGradientXZ : public FELogElemData
{
public:
	FELogElemDeformationGradientXZ(FEModel* pfem) : FELogElemData(pfem){}
	double value(FEElement& el);
};

//-----------------------------------------------------------------------------
class FELogElemDeformationGradientYX : public FELogElemData
{
public:
	FELogElemDeformationGradientYX(FEModel* pfem) : FELogElemData(pfem){}
	double value(FEElement& el);
};

//-----------------------------------------------------------------------------
class FELogElemDeformationGradientYY : public FELogElemData
{
public:
	FELogElemDeformationGradientYY(FEModel* pfem) : FELogElemData(pfem){}
	double value(FEElement& el);
};

//-----------------------------------------------------------------------------
class FELogElemDeformationGradientYZ : public FELogElemData
{
public:
	FELogElemDeformationGradientYZ(FEModel* pfem) : FELogElemData(pfem){}
	double value(FEElement& el);
};

//-----------------------------------------------------------------------------
class FELogElemDeformationGradientZX : public FELogElemData
{
public:
	FELogElemDeformationGradientZX(FEModel* pfem) : FELogElemData(pfem){}
	double value(FEElement& el);
};

//-----------------------------------------------------------------------------
class FELogElemDeformationGradientZY : public FELogElemData
{
public:
	FELogElemDeformationGradientZY(FEModel* pfem) : FELogElemData(pfem){}
	double value(FEElement& el);
};

//-----------------------------------------------------------------------------
class FELogElemDeformationGradientZZ : public FELogElemData
{
public:
	FELogElemDeformationGradientZZ(FEModel* pfem) : FELogElemData(pfem){}
	double value(FEElement& el);
};

//=============================================================================
// R I G I D   B O D Y    D A T A
//=============================================================================

//-----------------------------------------------------------------------------
class FELogRigidBodyPosX : public FELogObjectData
{
public:
	FELogRigidBodyPosX(FEModel* pfem) : FELogObjectData(pfem){}
	double value(FEObject& rb);
};

//-----------------------------------------------------------------------------
class FELogRigidBodyPosY : public FELogObjectData
{
public:
	FELogRigidBodyPosY(FEModel* pfem) : FELogObjectData(pfem){}
	double value(FEObject& rb);
};

//-----------------------------------------------------------------------------
class FELogRigidBodyPosZ : public FELogObjectData
{
public:
	FELogRigidBodyPosZ(FEModel* pfem) : FELogObjectData(pfem){}
	double value(FEObject& rb);
};

//-----------------------------------------------------------------------------
class FELogRigidBodyQuatX : public FELogObjectData
{
public:
	FELogRigidBodyQuatX(FEModel* pfem) : FELogObjectData(pfem){}
	double value(FEObject& rb);
};

//-----------------------------------------------------------------------------
class FELogRigidBodyQuatY : public FELogObjectData
{
public:
	FELogRigidBodyQuatY(FEModel* pfem) : FELogObjectData(pfem){}
	double value(FEObject& rb);
};

//-----------------------------------------------------------------------------
class FELogRigidBodyQuatZ : public FELogObjectData
{
public:
	FELogRigidBodyQuatZ(FEModel* pfem) : FELogObjectData(pfem){}
	double value(FEObject& rb);
};

//-----------------------------------------------------------------------------
class FELogRigidBodyQuatW : public FELogObjectData
{
public:
	FELogRigidBodyQuatW(FEModel* pfem) : FELogObjectData(pfem){}
	double value(FEObject& rb);
};

//-----------------------------------------------------------------------------
class FELogRigidBodyR11 : public FELogObjectData
{
public:
	FELogRigidBodyR11(FEModel* pfem) : FELogObjectData(pfem){}
	double value(FEObject& rb);
};

//-----------------------------------------------------------------------------
class FELogRigidBodyR12 : public FELogObjectData
{
public:
	FELogRigidBodyR12(FEModel* pfem) : FELogObjectData(pfem){}
	double value(FEObject& rb);
};

//-----------------------------------------------------------------------------
class FELogRigidBodyR13 : public FELogObjectData
{
public:
	FELogRigidBodyR13(FEModel* pfem) : FELogObjectData(pfem){}
	double value(FEObject& rb);
};

//-----------------------------------------------------------------------------
class FELogRigidBodyR21 : public FELogObjectData
{
public:
	FELogRigidBodyR21(FEModel* pfem) : FELogObjectData(pfem){}
	double value(FEObject& rb);
};

//-----------------------------------------------------------------------------
class FELogRigidBodyR22 : public FELogObjectData
{
public:
	FELogRigidBodyR22(FEModel* pfem) : FELogObjectData(pfem){}
	double value(FEObject& rb);
};

//-----------------------------------------------------------------------------
class FELogRigidBodyR23 : public FELogObjectData
{
public:
	FELogRigidBodyR23(FEModel* pfem) : FELogObjectData(pfem){}
	double value(FEObject& rb);
};

//-----------------------------------------------------------------------------
class FELogRigidBodyR31 : public FELogObjectData
{
public:
	FELogRigidBodyR31(FEModel* pfem) : FELogObjectData(pfem){}
	double value(FEObject& rb);
};

//-----------------------------------------------------------------------------
class FELogRigidBodyR32 : public FELogObjectData
{
public:
	FELogRigidBodyR32(FEModel* pfem) : FELogObjectData(pfem){}
	double value(FEObject& rb);
};

//-----------------------------------------------------------------------------
class FELogRigidBodyR33 : public FELogObjectData
{
public:
	FELogRigidBodyR33(FEModel* pfem) : FELogObjectData(pfem){}
	double value(FEObject& rb);
};

//-----------------------------------------------------------------------------
class FELogRigidBodyForceX : public FELogObjectData
{
public:
	FELogRigidBodyForceX(FEModel* pfem) : FELogObjectData(pfem){}
	double value(FEObject& rb);
};

//-----------------------------------------------------------------------------
class FELogRigidBodyForceY : public FELogObjectData
{
public:
	FELogRigidBodyForceY(FEModel* pfem) : FELogObjectData(pfem){}
	double value(FEObject& rb);
};

//-----------------------------------------------------------------------------
class FELogRigidBodyForceZ : public FELogObjectData
{
public:
	FELogRigidBodyForceZ(FEModel* pfem) : FELogObjectData(pfem){}
	double value(FEObject& rb);
};

//-----------------------------------------------------------------------------
class FELogRigidBodyTorqueX : public FELogObjectData
{
public:
	FELogRigidBodyTorqueX(FEModel* pfem) : FELogObjectData(pfem){}
	double value(FEObject& rb);
};

//-----------------------------------------------------------------------------
class FELogRigidBodyTorqueY : public FELogObjectData
{
public:
	FELogRigidBodyTorqueY(FEModel* pfem) : FELogObjectData(pfem){}
	double value(FEObject& rb);
};

//-----------------------------------------------------------------------------
class FELogRigidBodyTorqueZ : public FELogObjectData
{
public:
	FELogRigidBodyTorqueZ(FEModel* pfem) : FELogObjectData(pfem){}
	double value(FEObject& rb);
};

