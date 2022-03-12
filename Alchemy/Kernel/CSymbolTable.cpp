//	CSymboTable.cpp
//
//	Implementation of a symbol table
//	Copyright (c) 2019 Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

static DATADESCSTRUCT g_DataDesc[] =
	{	{ DATADESC_OPCODE_EMBED_OBJ,	1,	0 },		//	CPtrDictionary

		{ DATADESC_OPCODE_INT,			2,	0 },		//	m_bOwned, m_bNoReference
		{ DATADESC_OPCODE_STOP,	0,	0 } };
static CObjectClass<CSymbolTable>g_ClassData(OBJID_CSYMBOLTABLE, g_DataDesc);

#define VERSION2HACK				0xffffffff

CSymbolTable::CSymbolTable (void) : CPtrDictionary(&g_ClassData),
		m_bOwned(FALSE),
		m_bNoReference(TRUE)

//	CSymbolTable constructor
//	HACK: We need to default to these settings so that the CAtomTable
//	object can allocate an array of CSymbolTables

	{
	}

CSymbolTable::CSymbolTable (BOOL bOwned, BOOL bNoReference) : CPtrDictionary(&g_ClassData),
		m_bOwned(bOwned),
		m_bNoReference(bNoReference)

//	CSymbolTable constructor

	{
	}

CSymbolTable::~CSymbolTable (void)

//	CSymbolTable destructor

	{
	int i;

	for (i = 0; i < CPtrDictionary::GetCountInt(); i++)
		{
		void* vpKey;
		void* vpValue;
		CString *pKey;

		CPtrDictionary::GetEntry(i, &vpKey, &vpValue);
		pKey = (CString *)(size_t)vpKey;

		delete pKey;
		if (m_bOwned)
			{
			CObject *pValue = (CObject *)vpValue;
			delete pValue;
			}
		}
	}

CSymbolTable &CSymbolTable::operator= (const CSymbolTable &Obj)

//	CSymbolTable operator=

	{
	m_bOwned = Obj.m_bOwned;
	m_bNoReference = Obj.m_bNoReference;
	m_Array = Obj.m_Array;

	if (m_bOwned)
		CopyHandler((CSymbolTable *)&Obj);

	return *this;
	}

ALERROR CSymbolTable::AddEntry (const CString &sKey, CObject *pValue)

//	AddEntry
//
//	Add an entry to the symbol table

	{
	ALERROR error;
	CString *psKey;

	psKey = new CString(sKey);
	if (psKey == NULL)
		return ERR_MEMORY;

	//	Add key and value

	if (error = CPtrDictionary::AddEntry(psKey, (void*)pValue))
		{
		delete psKey;
		return error;
		}

	return NOERROR;
	}

int CSymbolTable::Compare (void* vpKey1, void* vpKey2) const

//	Compare
//
//	Compares two keys

	{
	CString *pKey1 = (CString *)vpKey1;
	CString *pKey2 = (CString *)vpKey2;

	return strCompareAbsolute(*pKey1, *pKey2);
	}

void CSymbolTable::CopyHandler (CObject *pOriginal)

//	CopyHandler
//
//	If we own the objects in the array, we need to make copies
//	of the objects also

	{
	int i;

	for (i = 0; i < GetCountInt(); i++)
		{
		//	Get the key and value

		void* vpKey;
		void* vpValue;
		GetEntry(i, &vpKey, &vpValue);

		//	Convert to the appropriate thing

		CString *pKey = (CString *)vpKey;
		CObject *pValue = (CObject *)vpValue;

		//	Bump the ref-count on the string

		CString *pNewKey = new CString(*pKey);

		//	If we own the object then make a copy too

		CObject *pNewValue;
		if (m_bOwned)
			pNewValue = pValue->Copy();
		else
			pNewValue = pValue;

		//	Stuff the new values (we don't need to free the previous
		//	values since they are kept by the original).

		SetEntry(i, pNewKey, pNewValue);
		}
	}

CString CSymbolTable::GetKey (size_t iEntry) const

//	GetKey
//
//	Returns the key of the nth entry

	{
	void* vpKey;
	void* vpValue;
	CString *pKey;

	GetEntry(iEntry, &vpKey, &vpValue);
	pKey = (CString *)vpKey;

	return *pKey;
	}

CObject *CSymbolTable::GetValue (size_t iEntry) const

//	GetValue
//
//	Returns the value of the nth entry

	{
	void* vpKey;
	void* vpValue;

	GetEntry(iEntry, &vpKey, &vpValue);
	return (CObject *)vpValue;
	}

ALERROR CSymbolTable::LoadHandler (CUnarchiver *pUnarchiver)

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

	//	If count is -1, then this is a new version of the symbol table

	if (dwCount == VERSION2HACK)
		{
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
			CString *pKey;

			//	Read in the key

			if (error = pUnarchiver->LoadObject(&pKey))
				return error;

			//	If we own the object, read in the object

			if (m_bOwned)
				{
				if (error = pUnarchiver->LoadObject(&pValue))
					{
					delete pKey;
					return error;
					}
				}
			else if (m_bNoReference)
				{
				if (error = pUnarchiver->ReadData((char *)&pValue, sizeof(size_t)))
					{
					delete pKey;
					return error;
					}
				}
#ifndef LATER
			//	We need to handle references here.
			else
				{
				ASSERT(FALSE);
				pValue = NULL;
				}
#endif

			CPtrDictionary::SetEntry(i, pKey, pValue);
			}
		}

	//	Otherwise, continue loading the old version in compatibility mode

	else
		{
		//	Read in the objects themselves

		for (i = 0; i < (int)dwCount; i++)
			{
			CObject *pObject;
			CObject *pValue;
			CString *pKey;

			//	Read in the key

			if (error = pUnarchiver->LoadObject(&pObject))
				return error;

			pKey = dynamic_cast<CString *>(pObject);
			if (pKey == NULL)
				{
				delete pObject;
				return ERR_FAIL;
				}

			//	If we own the object, read in the object

			if (m_bOwned)
				{
				if (error = pUnarchiver->LoadObject(&pValue))
					{
					delete pKey;
					return error;
					}
				}
			else if (m_bNoReference)
				{
				if (error = pUnarchiver->ReadData((char *)&pValue, sizeof(size_t)))
					{
					delete pKey;
					return error;
					}
				}
#ifndef LATER
			//	We need to handle references here.
			else
				{
				ASSERT(FALSE);
				pValue = NULL;
				}
#endif

			//	For previous version we insert the objects in the table
			//	although this is less efficient, it is required because the
			//	sort order changed from version 1 to 2.

			if (error = CPtrDictionary::AddEntry(pKey, pValue))
				{
				delete pKey;
				delete pValue;
				return error;
				}
			}
		}

	return NOERROR;
	}

ALERROR CSymbolTable::Lookup (const CString &sKey, CObject **retpValue) const

//	Lookup
//
//	Do a look up. If not found, returns ERR_NOTFOUND

	{
	ALERROR error;
	void* vpValue;
	CString sKeyToFind(sKey);

	if (error = CPtrDictionary::Find(&sKeyToFind, &vpValue))
		return error;

	if (retpValue)
		*retpValue = (CObject *)vpValue;

	return NOERROR;
	}

ALERROR CSymbolTable::LookupEx (const CString &sKey, size_t *retiEntry) const

//	LookupEx
//
//	Do a look up and return the entry number

	{
	ALERROR error;
	CString sKeyToFind(sKey);

	if (error = CPtrDictionary::FindEx(&sKeyToFind, retiEntry))
		return error;

	return NOERROR;
	}

ALERROR CSymbolTable::RemoveAll (void)

//	RemoveAll
//
//	Remove all entries in symbol table

	{
	int i;

	for (i = 0; i < CPtrDictionary::GetCountInt(); i++)
		{
		void* vpKey;
		void* vpValue;
		CString *pKey;

		CPtrDictionary::GetEntry(i, &vpKey, &vpValue);
		pKey = (CString *)vpKey;

		delete pKey;
		if (m_bOwned)
			{
			CObject *pValue = (CObject *)vpValue;
			delete pValue;
			}
		}

	return CPtrDictionary::RemoveAll();
	}

ALERROR CSymbolTable::RemoveEntry (size_t iEntry, CObject **retpOldValue)

//	RemoveEntry
//
//	Removes the given entry

	{
	ALERROR error;
	void* vpOldValue;

	//	Let the dictionary do the removing

	if (error = CPtrDictionary::RemoveEntryByOrdinal(iEntry, &vpOldValue))
		return error;

	CObject *pOldObj = (CObject *)vpOldValue;

	//	If the caller wants us to return the old value, do it; otherwise,
	//	we delete it, if necessary

	if (retpOldValue)
		*retpOldValue = pOldObj;
	else if (m_bOwned && pOldObj)
		delete pOldObj;

	return NOERROR;
	}

ALERROR CSymbolTable::RemoveEntry (const CString &sKey, CObject **retpOldValue)

//	RemoveEntry
//
//	Removes the given entry

	{
	ALERROR error;
	void* vpOldValue;

	//	Let the dictionary do the removing

	if (error = CPtrDictionary::RemoveEntry((void*) &sKey, &vpOldValue))
		return error;

	CObject *pOldObj = (CObject *)vpOldValue;

	//	If the caller wants us to return the old value, do it; otherwise,
	//	we delete it, if necessary

	if (retpOldValue)
		*retpOldValue = pOldObj;
	else if (m_bOwned && pOldObj)
		delete pOldObj;

	return NOERROR;
	}

ALERROR CSymbolTable::ReplaceEntry (const CString &sKey, CObject *pValue, bool bAdd, CObject **retpOldValue)

//	ReplaceEntry
//
//	If the key is found in the symbol table, it is replaced
//	with the given value. Otherwise, it returns ERR_NOTFOUND

	{
	ALERROR error;
	void* vpOldValue;
	CObject *pOldObj;
	CString *pKey;
	bool bAdded;

	//	We need to allocate a new string in case the key is added

	if (bAdd)
		{
		pKey = new CString(sKey);
		if (pKey == NULL)
			return ERR_MEMORY;
		}
	else
		pKey = const_cast<CString *>(&sKey);

	//	Let the dictionary code do the actual adding

	if (error = CPtrDictionary::ReplaceEntry(pKey, pValue, bAdd, &bAdded, &vpOldValue))
		return error;

	//	If we didn't actually add a key, then we free the string
	//	that we allocated

	if (bAdd && !bAdded)
		delete pKey;

	//	If we added a new object, then there is no old value

	if (bAdded)
		pOldObj = NULL;
	else
		pOldObj = (CObject *)vpOldValue;

	//	If the caller wants us to return the old value, do it; otherwise,
	//	we delete it, if necessary

	if (retpOldValue)
		*retpOldValue = pOldObj;
	else if (m_bOwned && pOldObj)
		delete pOldObj;

	return NOERROR;
	}

ALERROR CSymbolTable::SaveHandler (CArchiver *pArchiver)

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

	//	Write out -1 to indicate that this is the new version

	dwCount = VERSION2HACK;
	if (error = pArchiver->WriteData((char *)&dwCount, sizeof(DWORD)))
		return error;

	//	Write out the number of entries that we've got

	dwCount = (DWORD)CPtrDictionary::GetCountInt();
	if (error = pArchiver->WriteData((char *)&dwCount, sizeof(DWORD)))
		return error;

	//	Write out each object

	for (i = 0; i < CPtrDictionary::GetCountInt(); i++)
		{
		void* vpKey;
		void* vpValue;
		CString *pKey;

		CPtrDictionary::GetEntry(i, &vpKey, &vpValue);
		pKey = (CString *)vpKey;

		//	Write out the key

		if (error = pArchiver->SaveObject(pKey))
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
			if (error = pArchiver->WriteData((char *)&vpValue, sizeof(int)))
				return error;
			}
		else
			{
			size_t iID;
			CObject *pValue = (CObject *)vpValue;

			if (error = pArchiver->Reference2ID(pValue, &iID))
				return error;

			if (error = pArchiver->WriteData((char *)&iID, sizeof(size_t)))
				return error;
			}
		}

	return NOERROR;
	}

void CSymbolTable::SetValue (size_t iEntry, CObject *pValue, CObject **retpOldValue)

//	SetValue
//
//	Sets the value

	{
	void* vpKey;
	void* vpValue;

	GetEntry(iEntry, &vpKey, &vpValue);

	if (retpOldValue)
		*retpOldValue = (CObject *)vpValue;

	CPtrDictionary::SetEntry(iEntry, vpKey, (void*)pValue);
	}
