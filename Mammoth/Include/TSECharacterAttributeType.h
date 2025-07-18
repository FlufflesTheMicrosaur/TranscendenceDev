#pragma once
#include <TSELanguage.h>

class CCharacterAttributeType : public CDesignType
	{

	public:
		//	CCharacterAttributeType

		enum ECharacterAttributeTypes {
			attributeTypeUnknown,
			attributeTypeCharacterClass,
			attributeTypeGenome,
			attributeTypeGender,
			attributeTypeCustom,

			numCharacterAttributeType
			};

		CCharacterAttributeType(void) { }

		inline const CString& GetNamePlural(void) const { return m_sNamePlural; }
		inline const CString& GetNameSingular(void) const { return m_sNameSingular; }
		inline const CString& GetShortNamePlural(void) const { return m_sShortNamePlural; }
		inline const CString& GetShortNameSingular(void) const { return m_sShortNameSingular; }
		inline const CString& GetAdjective(void) const { return m_sAdjective; }
		inline const CString& GetMenuName(void) const { return m_sMenuName; }
		inline const CString& GetSID(void) const { return m_sSID; }
		inline const CString& GetAttributeType(void) const { return m_sAttributeType; }
		const CObjectImageArray& GetImage(bool bActual = false) const { return m_Image; };
		virtual const CCompositeImageDesc& GetTypeImage(void) const override { return m_ImageDesc; }

		//	CDesignType overrides
		static CCharacterAttributeType* AsType(CDesignType* pType) { return ((pType && pType->GetType() == designCharacterAttributeType) ? (CCharacterAttributeType*)pType : NULL); }
		static const CCharacterAttributeType* AsType(const CDesignType* pType) { return ((pType && pType->GetType() == designCharacterAttributeType) ? (const CCharacterAttributeType*)pType : NULL); }
		CString GetDataField(const CString& sField) const;
		virtual CString GetNamePattern(DWORD dwNounFormFlags = 0, DWORD* retdwFlags = NULL) const;
		virtual DesignTypes GetType(void) const override { return designCharacterAttributeType; }

	protected:
		//	CDesignType overrides
		virtual ALERROR OnBindDesign(SDesignLoadCtx& Ctx) override;
		virtual ALERROR OnCreateFromXML(SDesignLoadCtx& Ctx, CXMLElement* pDesc) override;

	private:
		CString m_sSID;									//	String ID (e.g., "microsaur")
		CString m_sName;								//	Annotated name: "Microsaur(s)"
		CString m_sNameSingular;						//	Singular form: "1 Microsaur"
		CString m_sNamePlural;							//	Plural form: "10 Microsaurs"; "You don't have enough microsaurs"
		CString m_sShortName;							//	":the Velociraptor(s)" (as opposed to ":the Velociraptor Microsaur(s)")
		CString m_sShortNameSingular;					//	"1 Velociraptor"
		CString m_sShortNamePlural;						//	"10 Velociraptors"
		CString m_sAdjective;							//	"Saurian"
		CString m_sMenuName;							//	Name for certain UIs where you want to show specific text for variants: "Microsaur (Velociraptor)"
		CString m_sAttributeType;						//	"genome" (used by a character's CharacterAttributeStack. Must match the appropriate stack layer attribute type.)
		bool m_bPluralForm = false;						//	"Those Dwarg are together"; "That Dwarg is alone"

		//	Image

		CCompositeImageDesc m_ImageDesc;				//	Image desc of icon
		CObjectImageArray m_Image;						//	Icon
	};
