//	CArrayBase.cpp
//
//	Implements CArrayBase object
//	Copyright (c) 2019 Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

#ifdef DEBUG_ARRAY_STATS
DWORD g_dwArraysCreated = 0;
DWORD g_dwTotalBytesAllocated = 0;
DWORD g_dwTotalBytesMoved = 0;
DWORD g_dwArraysResized = 0;
#endif

::placement_new_class placement_new;

Kernel::CArrayBase::CArrayBase (HANDLE hHeap, INT64 iGranularity) : m_pBlock(NULL)

//	CArrayBase constructor

	{
	//	If we have anything except the default options then we need
	//	to allocate the block

	if (hHeap || (iGranularity != DEFAULT_ARRAY_GRANULARITY))
		{
		if (hHeap == NULL)
			hHeap = ::GetProcessHeap();

		AllocBlock(hHeap, iGranularity);
		}
	}

CArrayBase::~CArrayBase (void)

//	CArrayBase destructor

	{
	CleanUpBlock();
	}

void CArrayBase::AllocBlock (HANDLE hHeap, INT64 iGranularity)

//	AllocBlock
//
//	Allocate the main block

	{
	if (m_pBlock)
		throw CException(ERR_FAIL);

#ifdef DEBUG_ARRAY_STATS
	g_dwArraysCreated++;
#endif

	m_pBlock = (SHeader *)::HeapAlloc(hHeap, 0, sizeof(SHeader));
	if (!m_pBlock)
		throw CException(ERR_MEMORY);

	m_pBlock->m_hHeap = hHeap;
	m_pBlock->m_iAllocSize = sizeof(SHeader);
	m_pBlock->m_iGranularity = iGranularity;
	m_pBlock->m_iSize = 0;
	}

void CArrayBase::CleanUpBlock (void)

//	CleanUpBlock
//
//	Frees the block

	{
	if (m_pBlock)
		{
#ifdef DEBUG_ARRAY_STATS
		g_dwArraysCreated--;
		g_dwTotalBytesAllocated -= (m_pBlock->m_iAllocSize - sizeof(SHeader));
#endif
		::HeapFree(m_pBlock->m_hHeap, 0, m_pBlock);
		m_pBlock = NULL;
		}
	}

void CArrayBase::CopyOptions (const CArrayBase &Src)

//	CopyOptions
//
//	Copies heap an granularity information from source

	{
	//	If we're changing heaps then we need to reallocate

	if (GetHeap() != Src.GetHeap() 
			|| (m_pBlock == NULL && GetGranularity() != Src.GetGranularity()))
		{
		ASSERT(GetSize() == 0);

#ifdef DEBUG_ARRAY_STATS
		if (m_pBlock == NULL)
			g_dwArraysCreated++;
#endif

		if (m_pBlock)
			::HeapFree(m_pBlock->m_hHeap, 0, m_pBlock);

		m_pBlock = (SHeader *)::HeapAlloc(Src.GetHeap(), 0, sizeof(SHeader));
		if (!m_pBlock)
			throw CException(ERR_MEMORY);

		m_pBlock->m_hHeap = Src.GetHeap();
		m_pBlock->m_iAllocSize = sizeof(SHeader);
		m_pBlock->m_iGranularity = Src.GetGranularity();
		m_pBlock->m_iSize = 0;
		}

	//	Otherwise we just change the granularity

	else if (GetGranularity() != Src.GetGranularity())
		{
		if (!m_pBlock)
			throw CException(ERR_FAIL);

		m_pBlock->m_iGranularity = Src.GetGranularity();
		}
	}

CString CArrayBase::DebugGetStats (void)

//	DebugGetStats
//
//	Get performance stats

	{
#ifdef DEBUG_ARRAY_STATS
	return strPatternSubst(CONSTLIT("Arrays Created: %,d\nArrays Resized: %,d\nTotal Bytes: %,d\nBytes Moved: %,d\n"),
			g_dwArraysCreated,
			g_dwArraysResized,
			g_dwTotalBytesAllocated,
			g_dwTotalBytesMoved);
#else
	return NULL_STR;
#endif
	}

void CArrayBase::DeleteBytes (INT64 iOffset, INT64 iLength)

//	Delete
//
//	Delete iLength bytes in the array at the given offset

	{
	if (iLength <= 0)
		return;

	if (!m_pBlock)
		throw CException(ERR_FAIL);

	//	Move stuff down

	char *pSource = GetBytes() + iOffset + iLength;
	char *pDest = GetBytes() + iOffset;
	for (int i = 0; i < GetSize() - (iOffset + iLength); i++)
		*pDest++ = *pSource++;

	//	Done

	m_pBlock->m_iSize -= iLength;
	}

void CArrayBase::InsertBytes (INT64 iOffset, void *pData, INT64 iLength, INT64 iAllocQuantum)

//	Insert
//
//	Insert the given data at the offset

	{
	INT64 i;

	if (iLength <= 0)
		return;

	if (iOffset == -1)
		iOffset = GetSize();

	ASSERT(iOffset >= 0 && iOffset <= GetSize());

    //	Reallocate if necessary

	Resize(GetSize() + iLength, true, iAllocQuantum);
    
	//	Move the array up
    
	char *pSource = GetBytes() + GetSize()-1;
	char *pDest = pSource + iLength;
    for (i = GetSize()-1; i >= iOffset; i--)
		*pDest-- = *pSource--;

	//	Copy the new values

	if (pData)
		{
		pSource = (char *)pData;
		pDest = GetBytes() + iOffset;
		for (i = 0; i < iLength; i++)
			*pDest++ = *pSource++;
		}

	//	Done
    
	m_pBlock->m_iSize += iLength;
	}

ALERROR CArrayBase::Resize (INT64 iNewSize, bool bPreserve, INT64 iAllocQuantum)

//	Resize
//
//	Resize the array so that it is at least the given new size

	{
	ASSERT(iAllocQuantum > 0);

	//	See if we need to reallocate the block

	if (m_pBlock == NULL || (m_pBlock->m_iAllocSize - (int)sizeof(SHeader) < iNewSize))
		{
		//	Allocate a new block
#ifdef _M_AMD64
		INT64 iNewAllocSize = sizeof(SHeader) + (INT64)Max((size_t)GetSize() * 2, AlignUp64(iNewSize, iAllocQuantum));
#else	//legacy 32bit mode
		INT64 iNewAllocSize = sizeof(SHeader) + (INT64)Max((size_t)GetSize() * 2, (size_t)AlignUp((int)iNewSize, (int)iAllocQuantum));
#endif
		SHeader *pNewBlock = (SHeader *)::HeapAlloc(GetHeap(), 0, (size_t)iNewAllocSize);
		if (pNewBlock == NULL)
			{
			::kernelDebugLogPattern("Out of memory allocating array of %d bytes.", iNewAllocSize);
			throw CException(ERR_MEMORY);
			}

		pNewBlock->m_hHeap = GetHeap();
		pNewBlock->m_iAllocSize = iNewAllocSize;
		pNewBlock->m_iGranularity = GetGranularity();
		pNewBlock->m_iSize = GetSize();

#ifdef DEBUG_ARRAY_STATS
		if (m_pBlock == NULL)
			{
			g_dwArraysCreated++;
			g_dwTotalBytesAllocated += pNewBlock->m_iAllocSize - sizeof(SHeader);
			}
		else
			{
			g_dwArraysResized++;
			g_dwTotalBytesAllocated += -(m_pBlock->m_iAllocSize - (size_t)sizeof(SHeader)) + (pNewBlock->m_iAllocSize - (size_t)sizeof(SHeader));
			g_dwTotalBytesMoved += (bPreserve ? GetSize() : 0);
			}
#endif

		//	Transfer the contents, if necessary

		if (m_pBlock && bPreserve)
			{
			INT64 i;
			char *pSource = GetBytes();
			char *pDest = (char *)(&pNewBlock[1]);

			for (i = 0; i < GetSize(); i++)
				*pDest++ = *pSource++;
			}

		//	Swap blocks

		if (m_pBlock)
			::HeapFree(m_pBlock->m_hHeap, 0, m_pBlock);

		m_pBlock = pNewBlock;
		}

	return NOERROR;
	}

void CArrayBase::TakeHandoffBase (CArrayBase &Src)

//	TakeHandoffBase
//
//	Takes the allocated array from Src

	{
	if (m_pBlock)
		{
		::HeapFree(m_pBlock->m_hHeap, 0, m_pBlock);

#ifdef DEBUG_ARRAY_STATS
		g_dwArraysCreated--;
#endif
		}

	m_pBlock = Src.m_pBlock;
	Src.m_pBlock = NULL;
	}
