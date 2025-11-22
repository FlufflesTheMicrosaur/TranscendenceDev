//	TSECharacterAttributeType.h
//
//	Classes and functions for character attributes.
//	Copyright (c) 2025 Kronosaur Productions, LLC. All Rights Reserved.
#pragma once

//	For some reason I have to have TSE.h, theres probably some config somewhere to fix this
#include "TSE.h"

class CCharacterAttributeType : public CDesignType
	{
	public:
		enum ECharacterAttributeCategory {
			//	Well-defined engine categories

			eCharacterGenome =			0,	//Species of the character (e.g.: Augmented Human, Microsaur, AI, Erebian, etc)
			eCharacterGender =			1,	//Gender of the character (e.g.: Feminine, Masculine, Plural, Other, None, etc)
			eCharacterClass =			2,	//Character class of the character (e.g.: Pilgrim, Mercenary, Trader, Explorer, etc)

			//	Custom categories

			eCharacterCustomAttrib =	-1,	//An XML-defined category
			eCharacterStackableAttrib =	-2,	//An XML-defined category that can be stacked
			};

		//	CDesignType overrides

		static CCharacterAttributeType *AsType (CDesignType *pType) { return ((pType && pType->GetType() == designCharacterAttributeType) ? (CCharacterAttributeType *)pType : NULL); }
		virtual CString GetNamePattern (DWORD dwNounFormFlags = 0, DWORD *retdwFlags = NULL) const;
		virtual DesignTypes GetType (void) const override { return designCharacterAttributeType; }

	protected:
		//	CDesignType overrides

		virtual void OnAccumulateXMLMergeFlags (TSortMap<DWORD, DWORD> &MergeFlags) const override;
		virtual ALERROR OnBindDesign (SDesignLoadCtx &Ctx) override;
		virtual ALERROR OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc) override;
		virtual void OnUnbindDesign (void) override;

	private:

		//	Character Attribute Category

		ECharacterAttributeCategory m_iCategory = eCharacterCustomAttrib;
		CString m_sCustomCategory = CONSTLIT("");

		//	Language variants (ex, gendered variants)

		TMap<CString, CLanguageDataBlock> m_LanguageVariants;

		//	Art

		CObjectImage m_ImgHero;				//	Hero image (e.g. character class or genome art)
		CObjectImage m_ImgIcon;				//	Icon image (e.g. a small display icon for UIs) 

	};
