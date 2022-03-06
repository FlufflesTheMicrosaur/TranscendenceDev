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

ALERROR CPtrDictionary::AddEntry(int iKey, void* pValue)

//	AddEntry
//
//	Adds an entry associating iKey with iValue. No duplication is allowed.

{
	ALERROR error;
	int iPos;

	//	Look for the key in the array. If we find the key, return an error

	if (FindSlot(iKey, &iPos))
		return ERR_FAIL;

	//	Add two elements to the array. The first is the key, the second
	//	is the value.

	if (error = m_Array.ExpandArray(iPos, 2))
		return error;

	//	Set the elements

	m_Array.ReplaceElement(iPos, (void*)iKey);
	m_Array.ReplaceElement(iPos + 1, pValue);

	return NOERROR;
}

int CPtrDictionary::Compare(int iKey1, int iKey2) const

//	Compare
//
//	Compares the two keys and returns which is greater. If 0, both
//	keys are equal. If 1, Key1 is greater. If -1, Key2 is greater

{
	if (iKey1 == iKey2)
		return 0;
	else if (iKey1 > iKey2)
		return 1;
	else
		return -1;
}

ALERROR CPtrDictionary::Find(int iKey, void** retpValue) const

//	Find
//
//	Looks up the given key in the dictionary and returns the value. If the
//	value is not found, return ERR_NOTFOUND.

{
	int iPos;

	if (!FindSlot(iKey, &iPos))
		return ERR_NOTFOUND;

	*retpValue = m_Array.GetElement(iPos + 1);
	return NOERROR;
}

ALERROR CPtrDictionary::FindEx(int iKey, int* retiEntry) const

//	FindEx
//
//	Finds the key and returns the entry

{
	int iPos;

	if (!FindSlot(iKey, &iPos))
		return ERR_NOTFOUND;

	*retiEntry = iPos / 2;
	return NOERROR;
}

ALERROR CPtrDictionary::FindOrAdd(int iKey, void* pValue, bool* retbFound, void** retpValue)

//	FindOrAdd
//
//	Looks for the given key. If the key is found, retbFound is TRUE and retiValue
//	is the value of the key. If the key is not found, retbFound is FALSE and the
//	key is added using the given value.

{
	ALERROR error;
	int iPos;

	if (FindSlot(iKey, &iPos))
	{
		*retbFound = TRUE;
		*retpValue = m_Array.GetElement(iPos + 1);
	}
	else
	{
		*retbFound = FALSE;
		*retpValue = pValue;

		//	Add two elements to the array. The first is the key, the second
		//	is the value.

		if (error = m_Array.ExpandArray(iPos, 2))
			return error;

		//	Set the elements

		m_Array.ReplaceElement(iPos, (void*)iKey);
		m_Array.ReplaceElement(iPos + 1, pValue);
	}

	return NOERROR;
}

bool CPtrDictionary::FindSlot(int iKey, int* retiPos) const

//	FindSlot
//
//	Returns the position in the array for the given key. If
//	the key was not found, retiPos is the position at which the
//	key should be inserted. Return TRUE if the key is found,
//	FALSE otherwise.

{
	int iLeft, iRight, iCount;
	int iEntryKey, iCompare;

	//	If there are no entries, then we always fail

	iCount = GetCount();
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
		int iTry;

		//	Pick a point in between our two extremes

		iTry = iLeft + (iRight - iLeft) / 2;

		//	Get the key at that point. Remember that
		//	we store the key and the value in a single array, so
		//	we need to multiply by two.

		iEntryKey = (int)m_Array.GetElement(iTry * 2);

		//	Figure out if we've matched the key

		iCompare = Compare(iEntryKey, iKey);

		if (iCompare == 0)
		{
			*retiPos = iTry * 2;
			return TRUE;
		}
		else if (iCompare == 1)
			iRight = iTry - 1;
		else
			iLeft = iTry + 1;
	}

	//	If we could not find the key, compute the insertion
	//	position

	iEntryKey = (int)m_Array.GetElement(iLeft * 2);
	iCompare = Compare(iEntryKey, iKey);
	if (iCompare == 0)
	{
		*retiPos = iLeft * 2;
		return TRUE;
	}
	else if (iCompare == 1)
		*retiPos = iLeft * 2;
	else
		*retiPos = (iLeft + 1) * 2;

	return FALSE;
}

void CPtrDictionary::GetEntry(int iEntry, int* retiKey, void** retpValue) const

//	GetEntry
//
//	Returns the given entry

{
	if (retiKey)
		*retiKey = (int)m_Array.GetElement(iEntry * 2);
	if (retpValue)
		*retpValue = m_Array.GetElement(iEntry * 2 + 1);
}

ALERROR CPtrDictionary::RemoveEntry(int iKey, void** retpOldValue)

//	RemoveEntry
//
//	Removes the given entry

{
	ALERROR error;
	int iPos;

	if (FindSlot(iKey, &iPos))
	{
		void* pOldValue;

		//	Get the old value

		pOldValue = m_Array.GetElement(iPos + 1);

		//	Remove the entry

		if (error = m_Array.RemoveRange(iPos, iPos + 1))
			return error;

		//	Done

		if (retpOldValue)
			*retpOldValue = pOldValue;
	}
	else
		return ERR_NOTFOUND;

	return NOERROR;
}

ALERROR CPtrDictionary::RemoveEntryByOrdinal(int iEntry, void** retpOldValue)

//	RemoveEntryByOrdinal
//
//	Removes the given entry

{
	ALERROR error;
	void* pOldValue;

	//	Get the old value

	pOldValue = m_Array.GetElement(iEntry + 1);

	//	Remove the entry

	if (error = m_Array.RemoveRange(iEntry, iEntry + 1))
		return error;

	//	Done

	if (retpOldValue)
		*retpOldValue = pOldValue;

	return NOERROR;
}

ALERROR CPtrDictionary::ReplaceEntry(int iKey, void* pValue, bool bAdd, bool* retbAdded, void** retpOldValue)

//	ReplaceEntry
//
//	Looks for the given key and replaces it with iValue. If the key
//	is not found and bAdd is TRUE, then the new value is added. Otherwise,
//	if the key is not found, it returns GAERR_NOTFOUND

{
	ALERROR error;
	int iPos;
	void* pOldValue;

	//	Look for the key in the array. If we don't find the key, return an error

	if (!FindSlot(iKey, &iPos))
	{
		if (bAdd)
		{
			//	Add two elements to the array. The first is the key, the second
			//	is the value.

			if (error = m_Array.ExpandArray(iPos, 2))
				return error;

			//	Set the elements

			m_Array.ReplaceElement(iPos, (void*)iKey);
			m_Array.ReplaceElement(iPos + 1, pValue);

			//	Done

			if (retbAdded)
				*retbAdded = TRUE;

			return NOERROR;
		}
		else
			return ERR_NOTFOUND;
	}

	//	Remember the old value

	pOldValue = m_Array.GetElement(iPos + 1);

	//	Set the value

	m_Array.ReplaceElement(iPos + 1, pValue);

	//	Return it

	if (retbAdded)
		*retbAdded = FALSE;

	if (retpOldValue)
		*retpOldValue = pOldValue;

	return NOERROR;
}

void CPtrDictionary::SetEntry(int iEntry, int iKey, void* pValue)

//	SetEntry
//
//	Sets the given entry

{
	m_Array.ReplaceElement(iEntry * 2, (void*)iKey);
	m_Array.ReplaceElement(iEntry * 2 + 1, pValue);
}
