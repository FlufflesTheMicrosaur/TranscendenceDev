//	CSymboTable.cpp
//
//	Implementation of a symbol table
//	Copyright (c) 2019 Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

static DATADESCSTRUCT g_DataDesc[] =
	{	{ DATADESC_OPCODE_EMBED_OBJ,	1,	0 },		//	CPtrDictionary

		{ DATADESC_OPCODE_INT,			2,	0 },		//	m_bOwned, m_bNoReference
		{ DATADESC_OPCODE_STOP,	0,	0 } };
static CObjectClass<CIDTable>g_ClassData(OBJID_CIDTABLE, g_DataDesc);

CIDTable::CIDTable (void) : CPtrDictionary(&g_ClassData),
		m_bOwned(FALSE),
		m_bNoReference(TRUE)

//	CIDTable constructor

	{
	}

CIDTable::CIDTable (BOOL bOwned, BOOL bNoReference) : CPtrDictionary(&g_ClassData),
		m_bOwned(bOwned),
		m_bNoReference(bNoReference)

//	CIDTable constructor

	{
	}

CIDTable::~CIDTable (void)

//	CIDTable destructor

	{
	if (m_bOwned)
		{
		size_t i;

		for (i = 0; i < CPtrDictionary::GetCountInt(); i++)
			{
			void* pKey;
			CObject *pValue;

			CPtrDictionary::GetEntry(i, &pKey, (void **)&pValue);
			delete pValue;
			}
		}
	}

int CIDTable::Compare (size_t iKey1, size_t iKey2) const

//	Compare
//
//	Compares the two keys and returns which is greater. If 0, both
//	keys are equal. If 1, Key1 is greater. If -1, Key2 is greater

	{
	if (iKey1 == iKey2)
		return 0;
	else if ((DWORD)iKey1 > (DWORD)iKey2)
		return 1;
	else
		return -1;
	}

void CIDTable::CopyHandler (CObject *pOriginal)

//	CopyHandler
//
//	If we own the objects in the array, we need to make copies
//	of the objects also

	{
#ifndef LATER
	//	Not yet implemented
	ASSERT(FALSE);
#endif
	}

size_t CIDTable::GetKey (size_t iEntry) const

//	GetKey
//
//	Returns the key of the nth entry

	{
	void* pKey;
	void* pValue;

	GetEntry(iEntry, &pKey, &pValue);

	return (size_t)pKey;
	}

CObject *CIDTable::GetValue (size_t iEntry) const

//	GetValue
//
//	Returns the value of the nth entry

	{
	void* pKey;
	void* pValue;

	GetEntry(iEntry, &pKey, &pValue);
	return (CObject *)pValue;
	}

ALERROR CIDTable::LoadHandler (CUnarchiver *pUnarchiver)

//	LoadHandler
//
//	Loads object

	{
	ALERROR error;
	DWORD dwCount;
	int i;

	//	Read in whether or not we own the objects

	if (error = pUnarchiver->ReadData((char *)&m_bOwned, sizeof(BOOL)))
		return error;

	//	Read in whether or not the value is a reference

	if (error = pUnarchiver->ReadData((char *)&m_bNoReference, sizeof(BOOL)))
		return error;

	//	Read in a count of objects

	if (error = pUnarchiver->ReadData((char *)&dwCount, sizeof(DWORD)))
		return error;

	//	Make sure that there's room for all the objects

	if (error = CPtrDictionary::ExpandArray(0, dwCount))
		return error;

	//	Read in the objects themselves

	for (i = 0; i < (int)dwCount; i++)
		{
		CObject *pValue;
		void* pKey;

		//	Read in the key

		if (error = pUnarchiver->ReadData((char *)&pKey, sizeof(size_t)))
			return error;

		//	If we own the object, read in the object

		if (m_bOwned)
			{
			if (error = pUnarchiver->LoadObject(&pValue))
				return error;
			}
		else if (m_bNoReference)
			{
			if (error = pUnarchiver->ReadData((char *)&pValue, sizeof(size_t)))
				return error;
			}
#ifndef LATER
		//	We need to handle references here.
		else
			throw CException(ERR_FAIL);
#endif

		CPtrDictionary::SetEntry(i, pKey, (void*)pValue);
		}

	return NOERROR;
	}

ALERROR CIDTable::Lookup (size_t iKey, CObject **retpValue) const

//	Lookup
//
//	Do a look up. If not found, returns ERR_NOTFOUND

	{
	ALERROR error;
	void* pValue;

	if (error = CPtrDictionary::Find((void*)iKey, &pValue))
		return error;

	*retpValue = (CObject *)pValue;
	return NOERROR;
	}

ALERROR CIDTable::LookupEx (size_t iKey, size_t*retiEntry) const

//	LookupEx
//
//	Do a look up and return the entry number

	{
	ALERROR error;

	if (error = CPtrDictionary::FindEx((void*)iKey, retiEntry))
		return error;

	return NOERROR;
	}

ALERROR CIDTable::RemoveAll (void)

//	RemoveAll
//
//	Removes all entries

	{
	//	Delete all objects

	if (m_bOwned)
		{
		for (int i = 0; i < GetCountInt(); i++)
			{
			CObject *pObj = GetValue(i);
			if (pObj)
				delete pObj;
			}
		}

	//	Done

	return CPtrDictionary::RemoveAll();
	}

ALERROR CIDTable::RemoveEntry (size_t iKey, CObject **retpOldValue)

//	RemoveEntry
//
//	Removes the given entry

	{
	ALERROR error;
	void* pOldValue;

	//	Let the dictionary do the removing

	if (error = CPtrDictionary::RemoveEntry((void*)iKey, &pOldValue))
		return error;

	CObject *pOldObj = (CObject *)pOldValue;

	//	If the caller wants us to return the old value, do it; otherwise,
	//	we delete it, if necessary

	if (retpOldValue)
		*retpOldValue = pOldObj;
	else if (m_bOwned && pOldObj)
		delete pOldObj;

	return NOERROR;
	}

ALERROR CIDTable::ReplaceEntry (size_t iKey, CObject *pValue, bool bAdd, CObject **retpOldValue)

//	ReplaceEntry
//
//	If the key is found in the symbol table, it is replaced
//	with the given value. Otherwise, it returns GAERR_NOTFOUND

	{
	ALERROR error;
	void* pOldValue;
	CObject *pOldObj;
	bool  bAdded;

	//	Let the dictionary code do the actual adding

	if (error = CPtrDictionary::ReplaceEntry((void*)iKey, (void*)pValue, bAdd, &bAdded, &pOldValue))
		return error;

	//	If we added a new object, then there is no old value

	if (bAdded)
		pOldObj = NULL;
	else
		pOldObj = (CObject *)pOldValue;

	//	If the caller wants us to return the old value, do it; otherwise,
	//	we delete it, if necessary

	if (retpOldValue)
		*retpOldValue = pOldObj;
	else if (m_bOwned && pOldObj)
		delete pOldObj;

	return NOERROR;
	}

ALERROR CIDTable::SaveHandler (CArchiver *pArchiver)

//	SaveHandler
//
//	Save object

	{
	ALERROR error;
	DWORD dwCount;
	int i;

	//	Write out whether we are owned or not

	if (error = pArchiver->WriteData((char *)&m_bOwned, sizeof(BOOL)))
		return error;

	//	Write out whether we are referencing or not

	if (error = pArchiver->WriteData((char *)&m_bNoReference, sizeof(BOOL)))
		return error;

	//	Write out the number of entries that we've got

	dwCount = (DWORD)CPtrDictionary::GetCountInt();
	if (error = pArchiver->WriteData((char *)&dwCount, sizeof(DWORD)))
		return error;

	//	Write out each object

	for (i = 0; i < CPtrDictionary::GetCountInt(); i++)
		{
		void* pKey;
		void* vpValue;

		CPtrDictionary::GetEntry(i, &pKey, &vpValue);

		//	Write out the key

		if (error = pArchiver->WriteData((char *)&pKey, sizeof(void*)))
			return error;

		//	If we're owned, write out the value. If we're not a reference
		//	just write out the value; otherwise, write out the reference

		if (m_bOwned)
			{
			CObject *pValue = (CObject *)vpValue;

			if (error = pArchiver->SaveObject(pValue))
				return error;
			}
		else if (m_bNoReference)
			{
			if (error = pArchiver->WriteData((char *)&vpValue, sizeof(void*)))
				return error;
			}
		else
			{
			size_t iID;
			CObject *pValue = (CObject *)vpValue;

			if (error = pArchiver->Reference2ID(pValue, &iID))
				return error;

			if (error = pArchiver->WriteData((char *)&iID, sizeof(void*)))
				return error;
			}
		}

	return NOERROR;
	}

void CIDTable::SetValue (size_t iEntry, CObject *pValue, CObject **retpOldValue)

//	SetValue
//
//	Sets the value

	{
	void* pKey;

	GetEntry(iEntry, &pKey, (void**)&pValue);

	if (retpOldValue)
		*retpOldValue = (CObject *)pValue;

	CPtrDictionary::SetEntry(iEntry, pKey, (void*)pValue);
	}
