// FEElementLibrary.h: interface for the FEElementLibrary class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FEELEMENTLIBRARY_H__3DB47576_A8D2_48BC_A48A_FD247DD84B43__INCLUDED_)
#define AFX_FEELEMENTLIBRARY_H__3DB47576_A8D2_48BC_A48A_FD247DD84B43__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <vector>

class FEElement;
class FEElementTraits;

//-----------------------------------------------------------------------------
//! This class stores the different element traits classes

//! The purpose of this class is to store all the different element traits classes.
//! It are these traits classes that define the different element types. All different
//! traits must be registered before they can be assigned to elements.

class FEElementLibrary  
{
public:
	//! destructor
	~FEElementLibrary();

	//! return the element library
	static FEElementLibrary* GetInstance();

	//! Assign a traits class to an element
	static void SetElementTraits(FEElement& el, int id);

	//! return element traits data
	static FEElementTraits* GetElementTraits(int ntype);

private:
	//! constructor
	FEElementLibrary(){}
	FEElementLibrary(const FEElementLibrary&){}

	//! Function to register an element traits class
	int RegisterTraits(FEElementTraits* ptrait);

private:
	std::vector<FEElementTraits*>	m_Traits;	//!< pointer to registered element traits
	static FEElementLibrary*	m_pThis;
};

#endif // !defined(AFX_FEELEMENTLIBRARY_H__3DB47576_A8D2_48BC_A48A_FD247DD84B43__INCLUDED_)
