//	CArmorMassDefintions.cpp
//
//	CArmorMassDefintions class
//	Copyright (c) 2018 Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

#define MASS_DESC_ARMOR_TAG						CONSTLIT("ArmorMassDesc")
#define MASS_DESC_AMMO_TAG						CONSTLIT("AmmoMassDesc")
#define MASS_DESC_WEAPON_TAG					CONSTLIT("WeaponMassDesc")
#define MASS_DESC_SHIELD_TAG					CONSTLIT("ShieldMassDesc")
#define MASS_DESC_REACTOR_TAG					CONSTLIT("ReactorMassDesc")
#define MASS_DESC_DEVICE_TAG					CONSTLIT("DeviceMassDesc")
#define MASS_DESC_ITEM_TAG						CONSTLIT("ItemMassDesc")

#define MASS_TAG								CONSTLIT("MassClass")
#define MASS_TAG_LEGACY							CONSTLIT("ArmorMass")	//	Used for backwards compatibility

#define CRITERIA_ATTRIB							CONSTLIT("criteria")
#define ID_ATTRIB								CONSTLIT("id")
#define LABEL_ATTRIB							CONSTLIT("label")
#define MASS_ATTRIB								CONSTLIT("mass")

const CItemMassDefinitions CItemMassDefinitions::Null;

void CItemMassDefinitions::Append (const CItemMassDefinitions &Src)

//	Append
//
//	Appends the source definitions.

	{
	//	Copy all definitions from source to us.

	m_Definitions.Merge(Src.m_Definitions);
	InvalidateIDIndex();
	}

CItemMassDefinitions::SItemMassEntry *CItemMassDefinitions::FindMassEntryActual (const CItem &Item)

//	FindMassEntryActual
//
//	Returns the entry for the given item.

	{
	int iMass = Item.GetMassKg();

	//	First look for any definitions with an explicit criteria.

	for (int i = 0; i < m_Definitions.GetCount(); i++)
		{
		if (m_Definitions.GetKey(i).IsBlank())
			continue;

		SItemMassDefinition &Def = m_Definitions[i];
		if (!Item.MatchesCriteria(Def.Criteria))
			continue;

		for (int j = 0; j < Def.Classes.GetCount(); j++)
			if (iMass <= Def.Classes[j].iMaxMass)
				return &Def.Classes[j];
		}

	//	If not found, look for the default definition

	SItemMassDefinition *pDef = m_Definitions.GetAt(NULL_STR);
	if (pDef == NULL)
		return NULL;

	for (int j = 0; j < pDef->Classes.GetCount(); j++)
		if (iMass <= pDef->Classes[j].iMaxMass)
			return &pDef->Classes[j];

	return NULL;
	}

bool CItemMassDefinitions::FindPreviousMassClass (const CString &sID, CString *retsPrevID, int *retiPrevMass) const

//	FindPreviousMassClass
//
//	Finds the largest mass definition that is smaller than the given mass class.

	{
	//	Find the armor class by ID

	SItemMassEntry *pMax;
	if (!m_ByID.Find(sID, &pMax))
		return false;

	//	Now find the definition where this came from

	const SItemMassDefinition *pDef = m_Definitions.GetAt(pMax->sDefinition);
	if (pDef == NULL)
		return false;

	//	Look through the definition for the previous class.

	int iBest = -1;
	int iBestMass = 0;
	for (int i = 0; i < pDef->Classes.GetCount(); i++)
		{
		const SItemMassEntry &Entry = pDef->Classes[i];

		if (Entry.iMaxMass < pMax->iMaxMass
				&& (iBest == -1 || Entry.iMaxMass > iBestMass))
			{
			iBest = i;
			iBestMass = Entry.iMaxMass;
			}
		}

	//	Done

	if (iBest == -1)
		return false;

	if (retsPrevID)
		*retsPrevID = pDef->Classes[iBest].sID;

	if (retiPrevMass)
		*retiPrevMass = pDef->Classes[iBest].iMaxMass;

	return true;
	}

Metric CItemMassDefinitions::GetFrequencyMax (const CString &sID) const

//	GetFrequencyMax
//
//	Returns the percent of all armor types that are at or below the given mass
//	class. This is used to determine what percent of armor types are usable by
//	a ship class with a given armor mass limit.
//
//	E.g., if sID == "heavy" then we return the percent of all armor types that
//	are either ultraLight, light, medium, or heavy.
//
//	NOTE: This call is only valid after all armor types have been bound. You may
//	call this after BindDesign

	{
	//	Find the armor class by ID

	SItemMassEntry *pMax;
	if (!m_ByID.Find(sID, &pMax))
		return 0.0;

	int iMaxMass = pMax->iMaxMass;

	//	Now find the definition where this came from

	const SItemMassDefinition *pDef = m_Definitions.GetAt(pMax->sDefinition);
	if (pDef == NULL)
		return 0.0;

	//	Add up counts.

	int iCount = 0;
	int iTotal = 0;
	for (int i = 0; i < pDef->Classes.GetCount(); i++)
		{
		const SItemMassEntry &Entry = pDef->Classes[i];
		iTotal += Entry.iCount;

		if (Entry.iMaxMass <= iMaxMass)
			iCount += Entry.iCount;
		}

	//	Compute the frequency.

	if (iTotal == 0)
		return 0.0;

	return (Metric)iCount / (Metric)iTotal;
	}

const CString &CItemMassDefinitions::GetMassClassID (const CItem &Item) const

//	GetMassClassID
//
//	Finds the item and returns the mass class ID (or NULL_STR if not found).

	{
	const SItemMassEntry *pEntry = FindMassEntry(Item);
	if (pEntry == NULL)
		return NULL_STR;

	return pEntry->sID;
	}

const CString &CItemMassDefinitions::GetMassClassLabel (const CString &sID) const

//	GetMassClassLabel
//
//	Returns the text.

	{
	SItemMassEntry *pEntry;
	if (!m_ByID.Find(sID, &pEntry))
		return NULL_STR;

	return pEntry->sText;
	}

int CItemMassDefinitions::GetMassClassMass (const CString &sID) const

//	GetMassClassMass
//
//	Returns the mass for the given mass class.

	{
	SItemMassEntry *pEntry;
	if (!m_ByID.Find(sID, &pEntry))
		return 0;

	return pEntry->iMaxMass;
	}

ALERROR CItemMassDefinitions::InitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	InitFromXML
//
//	Initialize from XML

	{
	//	Read the tag

	CString sTag = pDesc->GetTag();
	if (sTag == MASS_DESC_ARMOR_TAG)
		m_iCategoryMask = ItemCategories::itemcatArmor;
	else if (sTag == MASS_DESC_AMMO_TAG)
		m_iCategoryMask = ItemCategories::itemcatMissile;
	else if (sTag == MASS_DESC_WEAPON_TAG)
		m_iCategoryMask = ItemCategories::itemcatWeapon;
	else if (sTag == MASS_DESC_SHIELD_TAG)
		m_iCategoryMask = ItemCategories::itemcatShields;
	else if (sTag == MASS_DESC_REACTOR_TAG)
		m_iCategoryMask = ItemCategories::itemcatReactor;
	else if (sTag == MASS_DESC_DEVICE_TAG)
		m_iCategoryMask = ItemCategories::itemcatDeviceMask;
	else if (sTag == MASS_DESC_ITEM_TAG)
		m_iCategoryMask = ItemCategories::itemcatNone;

	//	Read the criteria

	CString sCriteria = pDesc->GetAttribute(CRITERIA_ATTRIB);

	//	Add a new definitions, for this criteria

	SItemMassDefinition *pDef = m_Definitions.SetAt(sCriteria);

	//	Parse the criteria

	pDef->Criteria.Init(sCriteria);

	//	Now read all the mass definitions

	for (int i = 0; i < pDesc->GetContentElementCount(); i++)
		{
		CXMLElement *pEntry = pDesc->GetContentElement(i);
		sTag = pEntry->GetTag();

		if (sTag == MASS_TAG || sTag == MASS_TAG_LEGACY)
			{
			int iMass = pEntry->GetAttributeIntegerBounded(MASS_ATTRIB, 1, -1, 0);
			if (iMass == 0)
				{
				Ctx.sError = CONSTLIT("Expected mass attributes.");
				m_Definitions.DeleteAll();
				return ERR_FAIL;
				}

			//	Insert

			SItemMassEntry *pMass = pDef->Classes.SetAt(iMass);
			pMass->sDefinition = sCriteria;
			pMass->sID = pEntry->GetAttribute(ID_ATTRIB);
			pMass->sText = pEntry->GetAttribute(LABEL_ATTRIB);
			pMass->iMaxMass = iMass;
			}
		else
			{
			Ctx.sError = CONSTLIT("Expected <MassClass>.");
			m_Definitions.DeleteAll();
			return ERR_FAIL;
			}
		}

	InvalidateIDIndex();

	return NOERROR;
	}

void CItemMassDefinitions::OnBindDesign (SDesignLoadCtx &Ctx, const CItem &Item, CString *retsMassClass)

//	OnBindDesign
//
//	This is called when we bind an armor type so that we can keep track of how
//	many armor types for each class.

	{
	SItemMassEntry *pEntry = FindMassEntryActual(Item);
	if (pEntry == NULL)
		{
		if (retsMassClass) *retsMassClass = NULL_STR;
		return;
		}

	//	Update the counts

	pEntry->iCount++;

	//	Return the mass class

	if (retsMassClass)
		*retsMassClass = pEntry->sID;
	}

void CItemMassDefinitions::OnInitDone (void)

//	OnInitDone
//
//	This is called just before we start calling Bind on an armor type.

	{
	//	Initialize the index.

	m_ByID.DeleteAll();

	for (int i = 0; i < m_Definitions.GetCount(); i++)
		{
		SItemMassDefinition &Def = m_Definitions[i];
		for (int j = 0; j < Def.Classes.GetCount(); j++)
			{
			SItemMassEntry &Entry = Def.Classes[j];

			//	While we're here, we initialize the counts, since we will soon
			//	get called at OnBindDesign.

			Entry.iCount = 0;

			//	Add to the index.

			m_ByID.SetAt(Entry.sID, &Entry);
			}
		}
	}
