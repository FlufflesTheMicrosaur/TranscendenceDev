//	CPtrArray.cpp
//
//	Implementation of dynamic array of pointers
//	Copyright (c) 2022 Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

#define ALLOC_INCREMENT					128

static DATADESCSTRUCT g_DataDesc[] =
{ { DATADESC_OPCODE_ALLOC_SIZE32,	1,	0 },		//	m_iAllocSize
	{ DATADESC_OPCODE_ALLOC_MEMORY,	1,	0 },		//	m_pData
	{ DATADESC_OPCODE_INT,			1,	0 },		//	m_iLength
	{ DATADESC_OPCODE_STOP,	0,	0 } };
static CObjectClass<CPtrArray>g_ClassData(OBJID_CPTRARRAY, g_DataDesc);

CPtrArray::CPtrArray(void) :
	CObject(&g_ClassData),
	m_iAllocSize(0),
	m_pData(NULL),
	m_iLength(0)

	//	CPtrArray constructor

{
}

CPtrArray::~CPtrArray(void)

//	CPtrArray destructor

{
	if (m_pData)
	{
		MemFree(m_pData);
		m_pData = NULL;
	}
}

CPtrArray& CPtrArray::operator= (const CPtrArray& Obj)

//	CPtrArray operator=

{
	if (m_pData)
		MemFree(m_pData);

	if (Obj.m_pData)
	{
		m_iAllocSize = Obj.m_iAllocSize;
		m_iLength = Obj.m_iLength;
		m_pData = (void**)::MemAlloc(sizeof(void*) * (Obj.m_iAllocSize));

		utlMemCopy((char*)Obj.m_pData, (char*)m_pData, sizeof(int) * m_iLength);
	}
	else
	{
		m_iAllocSize = 0;
		m_iLength = 0;
		m_pData = NULL;
	}

	return *this;
}

ALERROR CPtrArray::AppendElement(void* iElement, size_t* retiIndex)

//	AppendElement
//
//	Appends element to the array

{
	return InsertElement(iElement, m_iLength, retiIndex);
}

ALERROR CPtrArray::ExpandArray(size_t iPos, size_t iCount)

//	ExpandArray
//
//	Make room in the array at the given position for the
//	given number of elements. If iPos is -1, the array is
//	expanded at the end.

{
	size_t i;

	if (iPos < 0 || iPos > m_iLength)
		iPos = m_iLength;

	//	Reallocate if necessary

	if (m_iLength + iCount > m_iAllocSize)
	{
		void** pNewData;
		size_t iInc;

		iInc = AlignUp64(iCount, ALLOC_INCREMENT);

		//	Allocate a bigger buffer

		pNewData = (void**)MemAlloc(sizeof(void*) * (m_iAllocSize + iInc));
		if (pNewData == NULL)
			return ERR_MEMORY;

		//	Copy the data

		if (m_pData)
		{
			for (i = 0; i < m_iLength; i++)
				pNewData[i] = m_pData[i];
			MemFree(m_pData);
		}

		m_pData = pNewData;
		m_iAllocSize += iInc;
	}

	//	Move the array up

	for (i = m_iLength - 1; i >= iPos; i--)
		m_pData[i + iCount] = m_pData[i];

	m_iLength += iCount;

	return NOERROR;
}

size_t CPtrArray::FindElement(void* iElement) const

//	FindElement
//
//	Looks for the given element in the array. If it is found it returns
//	the position of the element; otherwise it returns -1.

{
	size_t i;

	for (i = 0; i < m_iLength; i++)
		if (m_pData[i] == iElement)
			return i;

	return -1;
}

size_t CPtrArray::GetCount(void) const

//	GetCount
//
//	Returns the number of elements in the array

{
	return m_iLength;
}

void* CPtrArray::GetElement(size_t iIndex) const

//	GetElement
//
//	Returns the nth element in the array

{
	ASSERT(iIndex >= 0 && iIndex < m_iLength);
	return m_pData[iIndex];
}

ALERROR CPtrArray::InsertElement(void* iElement, size_t iPos, size_t* retiIndex)

//	InsertElement
//
//	Inserts data into the list. iPos is the position in the list to insert at.
//	If -1, the data is inserted at the end of the list.

{
	ALERROR error;

	if (iPos < 0 || iPos > m_iLength)
		iPos = m_iLength;

	if (error = ExpandArray(iPos, 1))
		return error;

	m_pData[iPos] = iElement;
	if (retiIndex)
		*retiIndex = iPos;

	return NOERROR;
}

ALERROR CPtrArray::InsertRange(CPtrArray* pList, size_t iStart, size_t iEnd, size_t iPos)

//	InsertRange
//
//	Inserts a range of integers from another array

{
	ALERROR error;
	size_t i, iCount;

	if (iPos < 0 || iPos > m_iLength)
		iPos = m_iLength;

	iCount = iEnd - iStart + 1;

	if (error = ExpandArray(iPos, iCount))
		return error;

	//	Copy the data

	for (i = 0; i < iCount; i++)
		m_pData[iPos + i] = pList->GetElement(iStart + i);

	return NOERROR;
}

ALERROR CPtrArray::MoveRange(size_t iStart, size_t iEnd, size_t iPos)

//	MoveRange
//
//	Move the set of elements to the given position

{
	size_t iDestSize;
	size_t iDest;
	size_t i;

	ASSERT(iStart >= 0 && iStart < m_iLength);
	ASSERT(iEnd >= 0 && iStart < m_iLength);
	ASSERT(iEnd >= iStart);

	if (iPos < 0 || iPos > m_iLength)
		iPos = m_iLength;

	//	If we're moving to the same position, do nothing

	if (iPos >= iStart && iPos <= iEnd + 1)
		return NOERROR;

	//	The algorithm below is a one-pass in-place move
	//	that slides elements around.

	if (iPos > iEnd + 1)
	{
		iDestSize = iEnd - iStart + 1;
		iDest = 0;

		for (i = iPos - 1; i > iStart; i--)
		{
			void* pTemp;

			pTemp = m_pData[iEnd - iDest];
			m_pData[iEnd - iDest] = m_pData[i];
			m_pData[i] = pTemp;

			iDest = (iDest + 1) % iDestSize;
		}
	}
	else if (iPos < iStart)
	{
		iDestSize = iEnd - iStart + 1;
		iDest = 0;

		for (i = iPos; i < iEnd; i++)
		{
			void* pTemp;

			pTemp = m_pData[iStart + iDest];
			m_pData[iStart + iDest] = m_pData[i];
			m_pData[i] = pTemp;

			iDest = (iDest + 1) % iDestSize;
		}
	}

	return NOERROR;
}

ALERROR CPtrArray::Set(size_t iCount, void** pData)

//	Set
//
//	Sets the contents of the array

{
	if (m_pData)
		MemFree(m_pData);

	//	Allocate the data

	m_iAllocSize = (1 + (iCount / ALLOC_INCREMENT)) * ALLOC_INCREMENT;
	m_pData = (void**)MemAlloc(sizeof(void*) * m_iAllocSize);
	if (m_pData == NULL)
		return ERR_MEMORY;

	//	Copy any initial data

	if (pData)
	{
		size_t i;

		m_iLength = iCount;
		for (i = 0; i < iCount; i++)
			m_pData[i] = pData[i];
	}
	else
		m_iLength = 0;

	return NOERROR;
}

ALERROR CPtrArray::RemoveAll(void)

//	RemoveAll
//
//	Remove all elements of the array

{
	m_iLength = 0;
	return NOERROR;
}

ALERROR CPtrArray::RemoveRange(size_t iStart, size_t iEnd)

//	RemoveRange
//
//	Removes elements from iStart to iEnd

{
	size_t i, iCount;

	ASSERT(iStart >= 0 && iStart < m_iLength);
	ASSERT(iEnd >= 0 && iEnd < m_iLength);
	ASSERT(iStart <= iEnd);

	iCount = (iEnd - iStart) + 1;

	for (i = iEnd + 1; i < m_iLength; i++)
		m_pData[i - iCount] = m_pData[i];

	m_iLength -= iCount;

	return NOERROR;
}

void CPtrArray::ReplaceElement(size_t iPos, void* pElement)

//	ReplaceElement
//
//	Replace the given element

{
	ASSERT(iPos >= 0 && iPos < m_iLength);
	m_pData[iPos] = pElement;
}

void CPtrArray::Shuffle(void)

//	Shuffle
//
//	Shuffles the values of the array
//	Uses Fisher-Yates algorithm as implemented by Durstenfeld.

{
	if (m_iLength < 2)
		return;

	size_t i = m_iLength - 1;
	while (i > 0)
	{
		size_t x = mathRandomPtr(0, i);

		void* pValue = m_pData[x];
		m_pData[x] = m_pData[i];
		m_pData[i] = pValue;

		i--;
	}
}
