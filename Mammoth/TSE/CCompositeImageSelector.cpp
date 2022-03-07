//	CCompositeImageSelector.cpp
//
//	CCompositeImageSelector class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "PreComp.h"

static CObjectImageArray EMPTY_IMAGE;
CCompositeImageSelector CCompositeImageSelector::g_NullSelector;

bool CCompositeImageSelector::operator== (const CCompositeImageSelector &Val) const

//	Operator ==

	{
	int i;

	if (m_Sel.GetCount() != Val.m_Sel.GetCount())
		return false;

	for (i = 0; i < m_Sel.GetCount(); i++)
		{
		const SEntry *pThis = &m_Sel[i];
		const SEntry *pVal = &Val.m_Sel[i];

		if (pThis->dwID != pVal->dwID)
			return false;

		if (pThis->iVariant != pVal->iVariant)
			return false;

		if (pThis->pExtra != pVal->pExtra)
			return false;
		}

	return true;
	}

void CCompositeImageSelector::AddFlotsam (DWORD dwID, CItemType *pItemType)

//	AddFlotsam
//
//	Adds an item image representation

	{
	SEntry *pEntry = m_Sel.Insert();

	pEntry->dwID = dwID;
	pEntry->pExtra = pItemType;
	pEntry->iVariant = -1;
	}

void CCompositeImageSelector::AddShipwreck (DWORD dwID, CShipClass *pWreckClass, int iVariant)

//	AddShipwreck
//
//	Adds a shipwreck selection

	{
	SEntry *pEntry = m_Sel.Insert();

	pEntry->dwID = dwID;
	pEntry->pExtra = pWreckClass;
	pEntry->iVariant = 0;
	}

void CCompositeImageSelector::AddVariant (DWORD dwID, int iVariant)

//	AddVariant
//
//	Adds a variant selection

	{
	SEntry *pEntry = m_Sel.Insert();
	pEntry->dwID = dwID;
	pEntry->iVariant = iVariant;
	pEntry->pExtra = 0;
	}

const CCompositeImageSelector::SEntry *CCompositeImageSelector::FindEntry (DWORD dwID) const

//	FindEntry
//
//	Finds the entry with the given ID (or NULL)

	{
	int i;

	for (i = 0; i < m_Sel.GetCount(); i++)
		if (m_Sel[i].dwID == dwID)
			return &m_Sel[i];

	return NULL;
	}

CCompositeImageSelector::ETypes CCompositeImageSelector::GetEntryType (const SEntry &Entry) const

//	GetEntryType
//
//	Returns the type of entry

	{
	if (Entry.pExtra == 0)
		return typeVariant;
	else if (Entry.iVariant == -1)
		return typeItemType;
	else
		return typeShipClass;
	}

CObjectImageArray &CCompositeImageSelector::GetFlotsamImage (DWORD dwID) const

//	GetFlotsamImage
//
//	Returns the image

	{
	CItemType *pItemType = GetFlotsamType(dwID);
	if (pItemType == NULL)
		return EMPTY_IMAGE;

	return pItemType->GetFlotsamImage();
	}

CItemType *CCompositeImageSelector::GetFlotsamType (DWORD dwID) const

//	GetFlotsamType
//
//	Returns the flotsam type

	{
	const SEntry *pEntry = FindEntry(dwID);
	if (pEntry == NULL || GetEntryType(*pEntry) != typeItemType)
		return NULL;

	return (CItemType *)pEntry->pExtra;
	}

DWORD CCompositeImageSelector::GetHash (void) const

//  GetHash
//
//  Returns a hash for the structure

    {
    int i;

    if (m_Sel.GetCount() == 0)
        return 0;

    DWORD dwHash = GetHash(m_Sel[0]);
    for (i = 1; i < m_Sel.GetCount(); i++)
        dwHash = dwHash ^ (GetHash(m_Sel[i]) << i);

    return dwHash;
    }

DWORD CCompositeImageSelector::GetHash (const SEntry &Entry) const

//  GetHash
//
//  Hashes an entry

    {
    return ((Entry.dwID << 16) | ((DWORD)(Entry.iVariant) & 0xffff)) ^ (DWORD)(size_t)Entry.pExtra;
    }

CShipClass *CCompositeImageSelector::GetShipwreckClass (DWORD dwID) const

//	GetShipwreckClass
//
//	Returns the shipwreck class for the given selection

	{
	const SEntry *pEntry = FindEntry(dwID);
	if (pEntry == NULL || GetEntryType(*pEntry) != typeShipClass)
		return NULL;

	return (CShipClass *)pEntry->pExtra;
	}

CCompositeImageSelector::ETypes CCompositeImageSelector::GetType (DWORD dwID) const

//	GetType
//
//	Returns the type of selector

	{
	const SEntry *pEntry = FindEntry(dwID);
	if (pEntry == NULL)
		return typeNone;

	return GetEntryType(*pEntry);
	}

int CCompositeImageSelector::GetVariant (DWORD dwID) const

//	GetVariant
//
//	Returns the variant for the given selection

	{
	const SEntry *pEntry = FindEntry(dwID);
	if (pEntry == NULL)
		return 0;

	return pEntry->iVariant;
	}

void CCompositeImageSelector::ReadFromItem (const CDesignCollection &Design, ICCItemPtr pData)

//	ReadFromItem
//
//	Reads from a stored item.

	{
	int i;

	m_Sel.DeleteAll();
	if (pData->IsNil() || pData->GetCount() == 0 || !pData->IsList())
		return;

	m_Sel.InsertEmpty(pData->GetCount());
	for (i = 0; i < m_Sel.GetCount(); i++)
		{
		ICCItem *pEntry = pData->GetElement(i);

		m_Sel[i].dwID = (DWORD)pEntry->GetIntegerAt(CONSTLIT("id"));
		m_Sel[i].iVariant = (DWORD)pEntry->GetIntegerAt(CONSTLIT("variant"));
		m_Sel[i].pExtra = 0;

		DWORD dwUNID = (DWORD)pEntry->GetIntegerAt(CONSTLIT("itemType"));
		if (dwUNID)
			{
			const CItemType *pItemType = CItemType::AsType(Design.FindEntry(dwUNID));
			if (pItemType)
				m_Sel[i].pExtra = (void*)pItemType;
			else
				m_Sel[i].pExtra = 0;
			}
		else
			{
			dwUNID = (DWORD)pEntry->GetIntegerAt(CONSTLIT("shipClass"));
			if (dwUNID)
				{
				const CShipClass *pShipClass = CShipClass::AsType(Design.FindEntry(dwUNID));
				if (pShipClass)
					m_Sel[i].pExtra = (void*)pShipClass;
				else
					m_Sel[i].pExtra = 0;
				}
			}
		}
	}

void CCompositeImageSelector::ReadFromStream (SLoadCtx &Ctx)

//	ReadFromStream
//
//	Load from a stream
//
//	DWORD		No of entries
//	For each
//	DWORD		dwID
//	DWORD		iVariant
//	DWORD		Shipwreck UNID or ItemType UNID

	{
	int i;
	DWORD dwLoad;

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	if (dwLoad > 0)
		{
		ASSERT(m_Sel.GetCount() == 0);
		m_Sel.InsertEmpty(dwLoad);

		for (i = 0; i < m_Sel.GetCount(); i++)
			{
			Ctx.pStream->Read((char *)&m_Sel[i].dwID, sizeof(DWORD));
			Ctx.pStream->Read((char *)&m_Sel[i].iVariant, sizeof(DWORD));

			Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
			if (dwLoad == 0)
				m_Sel[i].pExtra = 0;
			else if (m_Sel[i].iVariant == -1)
				m_Sel[i].pExtra = Ctx.GetUniverse().FindItemType(dwLoad);
			else
				m_Sel[i].pExtra = Ctx.GetUniverse().FindShipClass(dwLoad);
			}
		}
	}

ICCItemPtr CCompositeImageSelector::WriteToItem (void) const

//	WriteToItem
//
//	Writes to an ICCItem

	{
	int i;

	if (m_Sel.GetCount() == 0)
		return ICCItemPtr(ICCItem::Nil);

	ICCItemPtr pSel(ICCItem::List);
	for (i = 0; i < m_Sel.GetCount(); i++)
		{
		ICCItemPtr pEntry(ICCItem::SymbolTable);
		pSel->Append(pEntry);

		pEntry->SetIntegerAt(CONSTLIT("id"), m_Sel[i].dwID);
		pEntry->SetIntegerAt(CONSTLIT("variant"), m_Sel[i].iVariant);

		ETypes iType = GetEntryType(m_Sel[i]);
		switch (iType)
			{
			case typeItemType:
				{
				CItemType *pItemType = (CItemType *)m_Sel[i].pExtra;
				pEntry->SetIntegerAt(CONSTLIT("itemType"), pItemType->GetUNID());
				break;
				}

			case typeShipClass:
				{
				CShipClass *pWreckClass = (CShipClass *)m_Sel[i].pExtra;
				pEntry->SetIntegerAt(CONSTLIT("shipClass"), pWreckClass->GetUNID());
				break;
				}
			}
		}

	return pSel;
	}

void CCompositeImageSelector::WriteToStream (IWriteStream *pStream) const

//	WriteToStream
//
//	Writes to a stream
//
//	DWORD		No of entries
//	For each
//	DWORD		dwID
//	DWORD		iVariant
//	DWORD		Shipwreck UNID or ItemType UNID

	{
	int i;
	DWORD dwSave;

	dwSave = m_Sel.GetCount();
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	
	//	Save each entry

	for (i = 0; i < m_Sel.GetCount(); i++)
		{
		pStream->Write((char *)&m_Sel[i].dwID, sizeof(DWORD));
		pStream->Write((char *)&m_Sel[i].iVariant, sizeof(DWORD));

		ETypes iType = GetEntryType(m_Sel[i]);
		switch (iType)
			{
			case typeItemType:
				{
				CItemType *pItemType = (CItemType *)m_Sel[i].pExtra;
				dwSave = pItemType->GetUNID();
				break;
				}

			case typeShipClass:
				{
				CShipClass *pWreckClass = (CShipClass *)m_Sel[i].pExtra;
				dwSave = pWreckClass->GetUNID();
				break;
				}

			default:
				dwSave = 0;
			}

		pStream->Write((char *)&dwSave, sizeof(DWORD));
		}
	}
