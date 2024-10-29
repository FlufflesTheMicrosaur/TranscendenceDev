
#include "PreComp.h"

#define IMAGE_TAG								CONSTLIT("Image")
#define VARIANT_TAG								CONSTLIT("Variant")
#define VARIANTS_TAG							CONSTLIT("Variants")

#define ADJECTIVE_ATTRIB						CONSTLIT("adjective")
#define ID_ATTRIB								CONSTLIT("id")
#define NAME_ATTRIB								CONSTLIT("name")
#define PLURAL_ATTRIB							CONSTLIT("plural")
#define SHORT_NAME_ATTRIB						CONSTLIT("shortName")


CString CGenomeType::GetNamePattern(DWORD dwNounFormFlags, DWORD* retdwFlags) const

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

bool CGenomeType::FindDataField(const CString& sField, CString* retsValue) const

//	FindDataField
//
//	Get a data field

	{
	if (strEquals(sField, NAME_ATTRIB))
		*retsValue = m_sGenomeSingular;
	else
		return CDesignType::FindDataField(sField, retsValue);

	return true;
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

		m_sGenomeSingular = CLanguage::ParseNounForm(m_sGenomeName, NULL_STR, 0, false, true);
		if (m_sGenomeSingular.IsBlank())
			return ComposeLoadError(Ctx, CONSTLIT("Invalid singular form of genome name"));

		m_sGenomePlural = CLanguage::ParseNounForm(m_sGenomeName, NULL_STR, 0, true, true);
		if (m_sGenomePlural.IsBlank())
			return ComposeLoadError(Ctx, CONSTLIT("Invalid plural form of genome name"));

		m_sGenomeAdjective = pDesc->GetAttribute(ADJECTIVE_ATTRIB);
		if (m_sGenomeAdjective.IsBlank())
			m_sGenomeShortName = m_sGenomeSingular;

		m_sGenomeShortName = pDesc->GetAttribute(SHORT_NAME_ATTRIB);
		if (m_sGenomeShortName.IsBlank())
			m_sGenomeShortName = m_sGenomeName;

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
