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
		//	CDesignType overrides

		static CCharacterAttributeType *AsType (CDesignType *pType) { return ((pType && pType->GetType() == designCharacterAttributeType) ? (CCharacterAttributeType *)pType : NULL); }
		virtual CCommunicationsHandler *GetCommsHandler (void) override { return m_Comms.GetCommsHandler(GetInheritFrom()); }
		virtual CString GetNamePattern (DWORD dwNounFormFlags = 0, DWORD *retdwFlags = NULL) const;
		virtual DesignTypes GetType (void) const override { return designGenericType; }

	protected:
		//	CDesignType overrides

		virtual void OnAccumulateXMLMergeFlags (TSortMap<DWORD, DWORD> &MergeFlags) const override;
		virtual ALERROR OnBindDesign (SDesignLoadCtx &Ctx) override;
		virtual ALERROR OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc) override;
		virtual void OnUnbindDesign (void) override;

	private:
		CCommunicationsStack m_Comms;

	};
