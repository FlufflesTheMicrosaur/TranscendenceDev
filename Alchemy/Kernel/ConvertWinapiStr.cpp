//	ConvertWinapiStr.cpp
//
//	Wrapper for ATLSTR's ATL::CW2A to convert WINAPI wide strings
//	Copyright (c) 2022 Kronosaur Productions, LLC. All Rights Reserved.

#include "precomp.h"
#include <atlstr.h>

//This function has to be in its own file to limit the blast radius
//of atlstr.h's ATL::CString, which causes all of george's implicit
//CString decl to become ambiguous between his Kernel::CString and
//atlstr's ATL::CString

std::string Kernel::strFromLPWSTR(LPWSTR s)
{
	return std::string(ATL::CW2A(s));
}
