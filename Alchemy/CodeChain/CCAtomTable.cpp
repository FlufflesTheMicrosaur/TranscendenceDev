//	CCAtomTable.cpp
//
//	Implements CCAtomTable class

#include "PreComp.h"

static CObjectClass<CCAtomTable>g_Class(OBJID_CCATOMTABLE, NULL);

CCAtomTable::CCAtomTable (void) : ICCAtom(&g_Class)

//	CCAtomTable constructor

	{
	}

bool CCAtomTable::AddEntry (ICCItem *pAtom, ICCItem *pEntry, bool bForceLocalAdd, bool bMustBeNew)

//	AddEntry
//
//	Adds an entry to the symbol table and returns
//	True for success.

	{
	ICCItem* pPrevEntry;
	bool bAdded;

	if (m_Table.ReplaceEntry(pAtom->GetIntegerValue(), pEntry->Reference(), true, &bAdded, (void**)&pPrevEntry) != NOERROR)
		throw CException(ERR_MEMORY);

	//	If we have a previous entry, decrement its refcount since we're
	//	throwing it away

	if (!bAdded && pPrevEntry)
		pPrevEntry->Discard();

	if (pPrevEntry && bMustBeNew)
		return false;

	return true;
	}

ICCItem *CCAtomTable::Clone (CCodeChain *pCC)

//	Clone
//
//	Clone this item

	{
	ASSERT(false);
	return pCC->CreateNil();
	}

void CCAtomTable::DestroyItem (void)

//	DestroyItem
//
//	Destroy this item

	{
	int i;

	//	Release all the entries

	for (i = 0; i < m_Table.GetCountInt(); i++)
		{
		void* pKey;
		ICCItem *pItem;

		m_Table.GetEntry(i, &pKey, (void**)&pItem);
		pItem->Discard();
		}

	//	Remove all symbols

	m_Table.RemoveAll();

	//	Destroy this item

	CCodeChain::DestroyAtomTable(this);
	}

ICCItem *CCAtomTable::ListSymbols (CCodeChain *pCC)

//	ListSymbols
//
//	Returns a list of all the atoms in the table

	{
	//	If there are no symbols, return Nil

	if (m_Table.GetCountInt() == 0)
		return pCC->CreateNil();

	//	Otherwise, make a list

	else
		{
		int i;
		ICCItem *pResult;
		CCLinkedList *pList;

		pResult = pCC->CreateLinkedList();
		if (pResult->IsError())
			return pResult;

		pList = (CCLinkedList *)pResult;

		for (i = 0; i < m_Table.GetCountInt(); i++)
			{
			ICCItem *pItem;
			void* pKey;

			m_Table.GetEntry(i, &pKey, NULL);

			//	Make an item for the symbol

			pItem = pCC->CreatePointer(pKey);

			//	Add the item to the list

			pList->Append(pItem);
			pItem->Discard();
			}

		return pList;
		}
	}

ICCItem *CCAtomTable::Lookup (CCodeChain *pCC, ICCItem *pAtom)

//	Lookup
//
//	Looks up the key and returns the association. If no
//	Association is found, returns Nil

	{
	return LookupEx(pCC, pAtom, NULL);
	}

ICCItem *CCAtomTable::LookupEx (CCodeChain *pCC, ICCItem *pAtom, bool *retbFound)

//	LookupEx
//
//	Looks up the key and returns the association. If no
//	Association is found, returns an error

	{
	ALERROR error;
	ICCItem *pBinding;

	if (error = m_Table.Find(pAtom->GetIntegerValue(), (void**)&pBinding))
		{
		if (error == ERR_NOTFOUND)
			{
			if (retbFound)
				*retbFound = false;

			return pCC->CreateErrorCode(CCRESULT_NOTFOUND);
			}
		else
			throw CException(ERR_MEMORY);
		}

	ASSERT(pBinding);

	if (retbFound)
		*retbFound = true;

	return pBinding->Reference();
	}

CString CCAtomTable::Print (DWORD dwFlags) const

//	Print
//
//	Render as text

	{
	return LITERAL("[atom table]");
	}

void CCAtomTable::Reset (void)

//	Reset
//
//	Reset the internal variables

	{
	m_Table.RemoveAll();
	}

