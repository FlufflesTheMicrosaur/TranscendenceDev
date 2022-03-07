//	CCPointer.cpp
//
//	Implements CCPointer class

#include "PreComp.h"

CCPointer::CCPointer(void) :
	m_pValue(nullptr)

	//	CCPointer constructor

{
}

ICCItem* CCPointer::Clone(CCodeChain* pCC)

//	Clone
//
//	Returns a new item with a single ref-count

{
	ICCItem* pResult;
	CCPointer* pClone;

	pResult = pCC->CreatePointer(m_pValue);
	if (pResult->IsError())
		return pResult;

	pClone = (CCPointer*)pResult;
	pClone->CloneItem(this);

	return pClone;
}

void CCPointer::DestroyItem(void)

//	DestroyItem
//
//	Destroys the item

{
	CCodeChain::DestroyInteger(this);
}

CString CCPointer::Print(DWORD dwFlags) const

//	Print
//
//	Returns a text representation of this item

{
	//	If this is an error code, translate it

	if (IsError())
	{
		switch ((size_t)m_pValue)
		{
		case CCRESULT_NOTFOUND:
			return strPatternSubst(LITERAL("[%d] Item not found."), m_pValue);

		case CCRESULT_CANCEL:
			return strPatternSubst(LITERAL("[%d] Operation canceled."), m_pValue);

		case CCRESULT_DISKERROR:
			return strPatternSubst(LITERAL("[%d] Disk error."), m_pValue);

		default:
			return strPatternSubst(LITERAL("[%d] Unknown error."), m_pValue);
		}
	}

	//	Otherwise, just print the integer value

	else
		return strFromPtr(m_pValue);
}

void CCPointer::Reset(void)

//	Reset
//
//	Reset to initial conditions

{
	ASSERT(m_dwRefCount == 0);
	m_pValue = 0;
}

