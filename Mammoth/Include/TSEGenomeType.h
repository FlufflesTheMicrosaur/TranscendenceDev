#pragma once
#include <TSELanguage.h>

class CGenomeType : public CDesignType
	{

	public:
		//	CGenomeType

		CGenomeType(void) { }

		inline const CString& GetGenomeNamePlural(void) const { return m_sGenomeNamePlural; }
		inline const CString& GetGenomeNameSingular(void) const { return m_sGenomeNameSingular; }
		inline const CString& GetGenomeShortNamePlural(void) const { return m_sGenomeShortNamePlural; }
		inline const CString& GetGenomeShortNameSingular(void) const { return m_sGenomeShortNameSingular; }
		inline const CString& GetGenomeAdjective(void) const { return m_sGenomeAdjective; }
		inline const CString& GetGenomeMenuName(void) const { return m_sGenomeMenuName; }
		inline const CString& GetSID(void) const { return m_sSID; }
		const CObjectImageArray& GetImage(bool bActual = false) const { return m_Image; };
		virtual const CCompositeImageDesc& GetTypeImage(void) const override { return m_ImageDesc; }

		//	CDesignType overrides
		static CGenomeType* AsType(CDesignType* pType) { return ((pType && pType->GetType() == designGenomeType) ? (CGenomeType*)pType : NULL); }
		static const CGenomeType* AsType(const CDesignType* pType) { return ((pType && pType->GetType() == designGenomeType) ? (const CGenomeType*)pType : NULL); }
		CString GetDataField(const CString& sField) const;
		virtual CString GetGenomeNamePattern(DWORD dwNounFormFlags = 0, DWORD* retdwFlags = NULL) const;
		virtual DesignTypes GetType(void) const override { return designGenomeType; }

	protected:
		//	CDesignType overrides
		virtual ALERROR OnBindDesign(SDesignLoadCtx& Ctx) override;
		virtual ALERROR OnCreateFromXML(SDesignLoadCtx& Ctx, CXMLElement* pDesc) override;

	private:
		CString m_sSID;									//	String ID (e.g., "microsaur")
		CString m_sGenomeName;							//	Annotated name: "Microsaur(s)"
		CString m_sGenomeNameSingular;					//	Singular form: "1 Microsaur"
		CString m_sGenomeNamePlural;					//	Plural form: "10 Microsaurs"; "You don't have enough microsaurs"
		CString m_sGenomeShortName;						//	":the Velociraptor(s)" (as opposed to ":the Velociraptor Microsaur(s)")
		CString m_sGenomeShortNameSingular;				//	"1 Velociraptor"
		CString m_sGenomeShortNamePlural;				//	"10 Velociraptors"
		CString m_sGenomeAdjective;						//	"Saurian"
		CString m_sGenomeMenuName;						//	Name for certain UIs where you want to show specific text for variants: "Microsaur (Velociraptor)"
		bool m_bPluralForm = false;						//	"Those Dwarg are together"; "That Dwarg is alone"

		//	Image

		CCompositeImageDesc m_ImageDesc;					//	Image dec of genome icon
		CObjectImageArray m_Image;							//	Genome icon
	};
