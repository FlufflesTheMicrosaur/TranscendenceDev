#pragma once
#include <TSELanguage.h>

class CGenomeType : public CDesignType
	{

	public:
		//	CGenomeType

		CGenomeType(void) { }

		inline const CString& GetGenomeNamePlural(void) const { return m_sGenomePlural; }
		inline const CString& GetGenomeNameSingular(void) const { return m_sGenomeSingular; }
		inline const CString& GetSID(void) const { return m_sSID; }
		const CObjectImageArray& GetImage(bool bActual = false) const { return m_Image; };
		virtual const CCompositeImageDesc& GetTypeImage(void) const override { return m_ImageDesc; }

		//	CDesignType overrides
		static CGenomeType* AsType(CDesignType* pType) { return ((pType && pType->GetType() == designGenomeType) ? (CGenomeType*)pType : NULL); }
		static const CGenomeType* AsType(const CDesignType* pType) { return ((pType && pType->GetType() == designGenomeType) ? (const CGenomeType*)pType : NULL); }
		virtual bool FindDataField(const CString& sField, CString* retsValue) const override;
		virtual CString GetNamePattern(DWORD dwNounFormFlags = 0, DWORD* retdwFlags = NULL) const;
		virtual DesignTypes GetType(void) const override { return designGenomeType; }

	protected:
		//	CDesignType overrides
		virtual ALERROR OnBindDesign(SDesignLoadCtx& Ctx) override;
		virtual ALERROR OnCreateFromXML(SDesignLoadCtx& Ctx, CXMLElement* pDesc) override;

	private:
		CString m_sSID;									//	String ID (e.g., "microsaur")
		CString m_sGenomeName;							//	Annotated name: "Microsaur(s)"
		CString m_sGenomeSingular;						//	Singular form: "1 Microsaur"
		CString m_sGenomePlural;						//	Plural form: "10 Microsaurs"; "You don't have enough microsaurs"
		CString m_sGenomeShortName;						//	":the Velociraptor" (as opposed to ":the Velociraptor Microsaur")
		CString m_sGenomeAdjective;						//	"Saurian"
		bool m_bPluralForm = false;						//	"Those Dwarg are together"; "That Dwarg is alone"

		//	Image

		CCompositeImageDesc m_ImageDesc;					//	Image dec of genome icon
		CObjectImageArray m_Image;							//	Genome icon
	};
