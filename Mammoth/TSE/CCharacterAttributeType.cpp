
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
#define ATTRIBUTE_TYPE_ATTRIB					CONSTLIT("attributeType")


CString CCharacterAttributeType::GetDataField(const CString& sField) const

// GetDataField
//
// Get a data field if it is known, otherwise try the base DesignType FindDataField

	{
	if (sField == ADJECTIVE_ATTRIB)
		return m_sAdjective;
	else if (sField == ID_ATTRIB)
		return m_sSID;
	else if (sField == MENU_NAME_ATTRIB)
		return m_sMenuName;
	else if (sField == NAME_ATTRIB)
		return m_sName;
	else if (sField == PLURAL_ATTRIB)
		return m_bPluralForm ? CString("true") : CString("false");
	else if (sField == SHORT_NAME_ATTRIB)
		return m_sShortName;
	else if (sField == ATTRIBUTE_TYPE_ATTRIB)
		return m_sAttributeType;
	else
		{
		CString sValue;
		FindDataField(sField, &sValue);
		return sValue;
		}
	}

CString CCharacterAttributeType::GetNamePattern(DWORD dwNounFormFlags, DWORD* retdwFlags) const

//	GetNamePattern
//
//	Returns the name pattern

	{
		//	All flags are encoded in the name

		if (retdwFlags)
			*retdwFlags = 0;

		if (dwNounFormFlags & nounShort)
			return m_sShortName;
		else if (dwNounFormFlags & nounAdjective)
			return m_sAdjective;
		else
			return m_sName;
	}

ALERROR CCharacterAttributeType::OnCreateFromXML(SDesignLoadCtx& Ctx, CXMLElement* pDesc)

//	OnCreateFromXML
//
//	Load from XML

	{
		m_sSID = pDesc->GetAttribute(ID_ATTRIB);
		if (m_sSID.IsBlank())
			return ComposeLoadError(Ctx, CONSTLIT("Invalid ID"));

		//	Parse the genome name

		m_sName = pDesc->GetAttribute(NAME_ATTRIB);
		if (m_sName.IsBlank())
			return ComposeLoadError(Ctx, CONSTLIT("Invalid genome name"));

		m_sNameSingular = CLanguage::ParseNounForm(m_sName, NULL_STR, 0, false, true);
		if (m_sNameSingular.IsBlank())
			return ComposeLoadError(Ctx, CONSTLIT("Invalid singular form of character attribute name"));

		m_sNamePlural = CLanguage::ParseNounForm(m_sName, NULL_STR, 0, true, true);
		if (m_sNamePlural.IsBlank())
			return ComposeLoadError(Ctx, CONSTLIT("Invalid plural form of character attribute name"));

		m_sAdjective = pDesc->GetAttribute(ADJECTIVE_ATTRIB);
		if (m_sAdjective.IsBlank())
			m_sShortName = m_sNameSingular;

		m_sShortName = pDesc->GetAttribute(SHORT_NAME_ATTRIB);
		if (m_sShortName.IsBlank())
			m_sShortName = m_sName;

		m_sMenuName = pDesc->GetAttribute(MENU_NAME_ATTRIB);
		if (m_sMenuName.IsBlank())
			m_sMenuName = m_sNameSingular;

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

ALERROR CCharacterAttributeType::OnBindDesign(SDesignLoadCtx& Ctx)

//	OnBindDesign
//
//	Bind design

{
	return NOERROR;
}
