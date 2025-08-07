//	CPtrDictionaryArray.cpp
//
//	Implementation of a sorted dictionary
//	Copyright (c) 2022 Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

static DATADESCSTRUCT g_DataDesc[] =
{ { DATADESC_OPCODE_EMBED_OBJ,	1,	0 },		//	m_Array
	{ DATADESC_OPCODE_STOP,	0,	0 } };
static CObjectClass<CPtrDictionary>g_ClassData(OBJID_CPTRDICTIONARY, g_DataDesc);

CPtrDictionary::CPtrDictionary(void) : CObject(&g_ClassData)

//	CPtrDictionary constructor

{
}

CPtrDictionary::CPtrDictionary(IObjectClass* pClass) : CObject(pClass)

//	CPtrDictionary constructor

{
}

CPtrDictionary::~CPtrDictionary(void)

//	CPtrDictionary destructor

{
}

ALERROR CPtrDictionary::AddEntry(void* pKey, void* pValue)

//	AddEntry
//
//	Adds an entry associating iKey with iValue. No duplication is allowed.

{
	ALERROR error;
	size_t iPos;

	//	Look for the key in the array. If we find the key, return an error

	if (FindSlot(pKey, &iPos))
		return ERR_FAIL;

	//	Add an element to both arrays

	if (error = m_Array.ExpandArray(iPos, 1))
		return error;
	if (error = m_Keys.ExpandArray(iPos, 1))
	{
		m_Array.RemoveElement(iPos);
		return error;
	}

	//	Set the elements

	m_Keys.ReplaceElement(iPos, pKey);
	m_Array.ReplaceElement(iPos, pValue);

	return NOERROR;
}

int CPtrDictionary::Compare(void* pKey1, void* pKey2) const

//	Compare
//
//	Compares the two keys and returns which is greater. If 0, both
//	keys are equal. If 1, Key1 is greater. If -1, Key2 is greater

{
	if (pKey1 == pKey2)
		return 0;
	else if (pKey1 > pKey2)
		return 1;
	else
		return -1;
}

ALERROR CPtrDictionary::Find(void* pKey, void** retpValue) const

//	Find
//
//	Looks up the given key in the dictionary and returns the value. If the
//	value is not found, return ERR_NOTFOUND.

{
	size_t iPos;

	if (!FindSlot(pKey, &iPos))
		return ERR_NOTFOUND;

	*retpValue = m_Array.GetElement(iPos);
	return NOERROR;
}

ALERROR CPtrDictionary::FindEx(void* pKey, size_t* retiEntry) const

//	FindEx
//
//	Finds the key and returns the entry

{
	size_t iPos;

	if (!FindSlot(pKey, &iPos))
		return ERR_NOTFOUND;

	*retiEntry = iPos;
	return NOERROR;
}

ALERROR CPtrDictionary::FindOrAdd(void* pKey, void* pValue, bool* retbFound, void** retpValue)

//	FindOrAdd
//
//	Looks for the given key. If the key is found, retbFound is TRUE and retiValue
//	is the value of the key. If the key is not found, retbFound is FALSE and the
//	key is added using the given value.

{
	ALERROR error;
	size_t iPos;

	if (FindSlot(pKey, &iPos))
	{
		*retbFound = TRUE;
		*retpValue = m_Array.GetElement(iPos);
	}
	else
	{
		*retbFound = FALSE;
		*retpValue = pValue;

		//	Add an element to each array

		if (error = m_Array.ExpandArray(iPos, 1))
			return error;
		if (error = m_Keys.ExpandArray(iPos, 1))
		{
			m_Array.RemoveElement(iPos);
			return error;
		}

		//	Set the elements

		m_Keys.ReplaceElement(iPos, pKey);
		m_Array.ReplaceElement(iPos, pValue);
	}

	return NOERROR;
}

bool CPtrDictionary::FindSlot(void* pKey, size_t* retiPos) const

//	FindSlot
//
//	Returns the position in the array for the given key. If
//	the key was not found, retiPos is the position at which the
//	key should be inserted. Return TRUE if the key is found,
//	FALSE otherwise.

{
	INT64 iLeft, iRight, iCount, iCompare;
	void* pEntryKey;

	//	If there are no entries, then we always fail

	iCount = GetCountInt();
	if (iCount == 0)
	{
		*retiPos = 0;
		return FALSE;
	}

	//	Do a binary search looking for the key

	iLeft = 0;
	iRight = iCount - 1;

	while (iRight > iLeft)
	{
		size_t iTry;

		//	Pick a point in between our two extremes

		iTry = (size_t)(iLeft + (iRight - iLeft) / 2);

		//	Get the key at that point. Remember that
		//	we store the key and the value in a single array, so
		//	we need to multiply by two.

		pEntryKey = m_Keys.GetElement(iTry);

		//	Figure out if we've matched the key

		iCompare = Compare(pEntryKey, pKey);

		if (iCompare == 0)
		{
			*retiPos = iTry;
			return TRUE;
		}
		else if (iCompare == 1)
			iRight = iTry - 1;
		else
			iLeft = iTry + 1;
	}

	//	If we could not find the key, compute the insertion
	//	position

	pEntryKey = m_Keys.GetElement(iLeft);
	iCompare = Compare(pEntryKey, pKey);
	if (iCompare == 0)
	{
		*retiPos = (size_t)iLeft;
		return TRUE;
	}
	else if (iCompare == 1)
		*retiPos = (size_t)iLeft;
	else
		*retiPos = (size_t)(iLeft + 1);

	return FALSE;
}

void CPtrDictionary::GetEntry(size_t iEntry, void** retpKey, void** retpValue) const

//	GetEntry
//
//	Returns the given entry

{
	if (retpKey)
		*retpKey = m_Keys.GetElement(iEntry);
	if (retpValue)
		*retpValue = m_Array.GetElement(iEntry);
}

ALERROR CPtrDictionary::RemoveEntry(void* pKey, void** retpOldValue)

//	RemoveEntry
//
//	Removes the given entry

{
	ALERROR error;
	size_t iPos;

	if (FindSlot(pKey, &iPos))
	{
		void* pOldValue;

		//	Get the old value

		pOldValue = m_Array.GetElement(iPos);

		//	Remove the entry

		if (error = m_Array.RemoveRange(iPos, iPos) | m_Keys.RemoveRange(iPos, iPos))
			return error;

		//	Done

		if (retpOldValue)
			*retpOldValue = pOldValue;
	}
	else
		return ERR_NOTFOUND;

	return NOERROR;
}

ALERROR CPtrDictionary::RemoveEntryByOrdinal(size_t iEntry, void** retpOldValue)

//	RemoveEntryByOrdinal
//
//	Removes the given entry

{
	ALERROR error;
	void* pOldValue;

	//	Get the old value

	pOldValue = m_Array.GetElement(iEntry);

	//	Remove the entry

	if (error = m_Array.RemoveRange(iEntry, iEntry) | m_Keys.RemoveRange(iEntry, iEntry))
		return error;

	//	Done

	if (retpOldValue)
		*retpOldValue = pOldValue;

	return NOERROR;
}

ALERROR CPtrDictionary::ReplaceEntry(void* pKey, void* pValue, bool bAdd, bool* retbAdded, void** retpOldValue)

//	ReplaceEntry
//
//	Looks for the given key and replaces it with iValue. If the key
//	is not found and bAdd is TRUE, then the new value is added. Otherwise,
//	if the key is not found, it returns GAERR_NOTFOUND

{
	ALERROR error;
	size_t iPos;
	void* pOldValue;

	//	Look for the key in the array. If we don't find the key, return an error

	if (!FindSlot(pKey, &iPos))
	{
		if (bAdd)
		{
			//	Add an element to each array

			if (error = m_Array.ExpandArray(iPos, 1))
				return error;
			if (error = m_Keys.ExpandArray(iPos, 1))
			{
				m_Array.RemoveElement(iPos);
				return error;
			}

			//	Set the elements

			m_Keys.ReplaceElement(iPos, pKey);
			m_Array.ReplaceElement(iPos, pValue);

			//	Done

			if (retbAdded)
				*retbAdded = TRUE;

			return NOERROR;
		}
		else
			return ERR_NOTFOUND;
	}

	//	Remember the old value

	pOldValue = m_Array.GetElement(iPos);

	//	Set the value

	m_Array.ReplaceElement(iPos, pValue);

	//	Return it

	if (retbAdded)
		*retbAdded = FALSE;

	if (retpOldValue)
		*retpOldValue = pOldValue;

	return NOERROR;
}

void CPtrDictionary::SetEntry(size_t iEntry, void* pKey, void* pValue)

//	SetEntry
//
//	Sets the given entry

{
	m_Keys.ReplaceElement(iEntry, pKey);
	m_Array.ReplaceElement(iEntry, pValue);
}
