
#include "PreComp.h"

#define IMAGE_TAG								CONSTLIT("Image")
#define VARIANT_TAG								CONSTLIT("Variant")
#define VARIANTS_TAG							CONSTLIT("Variants")

#define ADJECTIVE_ATTRIB						CONSTLIT("adjective")
#define ID_ATTRIB								CONSTLIT("id")
#define MENU_NAME_ATTRIB						CONSTLIT("menuName")
#define NAME_ATTRIB								CONSTLIT("name")
#define PLURAL_ATTRIB							CONSTLIT("plural")
#define SHORT_NAME_ATTRIB						CONSTLIT("shortName")


CString CGenomeType::GetDataField(const CString& sField) const

// GetDataField
//
// Get a data field if it is known, otherwise try the base DesignType FindDataField

	{
	if (sField == ADJECTIVE_ATTRIB)
		return m_sGenomeAdjective;
	else if (sField == ID_ATTRIB)
		return m_sSID;
	else if (sField == MENU_NAME_ATTRIB)
		return m_sGenomeMenuName;
	else if (sField == NAME_ATTRIB)
		return m_sGenomeName;
	else if (sField == PLURAL_ATTRIB)
		return m_bPluralForm ? CString("true") : CString("false");
	else if (sField == SHORT_NAME_ATTRIB)
		return m_sGenomeShortName;
	else
		{
		CString sValue;
		FindDataField(sField, &sValue);
		return sValue;
		}
	}

CString CGenomeType::GetGenomeNamePattern(DWORD dwNounFormFlags, DWORD* retdwFlags) const

//	GetNamePattern
//
//	Returns the name pattern

	{
		//	All flags are encoded in the name

		if (retdwFlags)
			*retdwFlags = 0;

		if (dwNounFormFlags & nounShort)
			return m_sGenomeShortName;
		else if (dwNounFormFlags & nounAdjective)
			return m_sGenomeAdjective;
		else
			return m_sGenomeName;
	}

ALERROR CGenomeType::OnCreateFromXML(SDesignLoadCtx& Ctx, CXMLElement* pDesc)

//	OnCreateFromXML
//
//	Load from XML

	{
		m_sSID = pDesc->GetAttribute(ID_ATTRIB);
		if (m_sSID.IsBlank())
			return ComposeLoadError(Ctx, CONSTLIT("Invalid ID"));

		//	Parse the genome name

		m_sGenomeName = pDesc->GetAttribute(NAME_ATTRIB);
		if (m_sGenomeName.IsBlank())
			return ComposeLoadError(Ctx, CONSTLIT("Invalid genome name"));

		m_sGenomeNameSingular = CLanguage::ParseNounForm(m_sGenomeName, NULL_STR, 0, false, true);
		if (m_sGenomeNameSingular.IsBlank())
			return ComposeLoadError(Ctx, CONSTLIT("Invalid singular form of genome name"));

		m_sGenomeNamePlural = CLanguage::ParseNounForm(m_sGenomeName, NULL_STR, 0, true, true);
		if (m_sGenomeNamePlural.IsBlank())
			return ComposeLoadError(Ctx, CONSTLIT("Invalid plural form of genome name"));

		m_sGenomeAdjective = pDesc->GetAttribute(ADJECTIVE_ATTRIB);
		if (m_sGenomeAdjective.IsBlank())
			m_sGenomeShortName = m_sGenomeNameSingular;

		m_sGenomeShortName = pDesc->GetAttribute(SHORT_NAME_ATTRIB);
		if (m_sGenomeShortName.IsBlank())
			m_sGenomeShortName = m_sGenomeName;

		m_sGenomeMenuName = pDesc->GetAttribute(MENU_NAME_ATTRIB);
		if (m_sGenomeMenuName.IsBlank())
			m_sGenomeMenuName = m_sGenomeNameSingular;

		m_bPluralForm = pDesc->GetAttributeBool(PLURAL_ATTRIB);

		//	Handle subelements

		//	Process image
		CXMLElement *pDescImg = pDesc->GetContentElementByTag(IMAGE_TAG);
		ALERROR error;
		if (error = m_Image.InitFromXML(Ctx, *pDescImg))
			return ComposeLoadError(Ctx, CONSTLIT("Unable to load image"));

		//	Images

		if (error = m_Image.OnDesignLoadComplete(Ctx))
			return error;

		return NOERROR;
	}

ALERROR CGenomeType::OnBindDesign(SDesignLoadCtx& Ctx)

//	OnBindDesign
//
//	Bind design

{
	return NOERROR;
}
