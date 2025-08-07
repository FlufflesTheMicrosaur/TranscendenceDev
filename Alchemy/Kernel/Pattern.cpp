//	Pattern.cpp
//
//	String pattern package
//	Copyright (c) 2019 Kronosaur Productions, LLC. All Rights Reserved.
//
//	Pattern format:
//
//	%type
//
//	where type is one of the following:
//
//	%	Evaluates to a single percent sign
//
//	d	Argument is a signed 32-bit integer. The number is substituted
//
//	p	If the last integer argument not 1, then 's' is substituted.
//		This is used to pluralize words in the English language.
//
//	s	Argument is a CString. The string is substituted
//
//	x	Argument is an unsigned 32-bit integer. The hex value is substituted
//
//	&	Followed by an XML entity and semicolon

#include "PreComp.h"
#include "Internets.h"

CString Kernel::strTranslateStdEntity(Kernel::CString sEntity)
{
	return CHTML::TranslateStdEntity(sEntity);
}

#ifdef CSTRING_FORMAT_VARIADICS_IN_CPP

//Special cases for conversions that the standard inner loop cant handle:

template <class... Args>
Kernel::CString formatVariadicArg(INT64 i, std::string sFmt, LPWSTR current, Args ... args)
	{
	if (!i)
		return CString(const_cast<char*>(std::format(sFmt, strFromLPWSTR(current)).c_str()));
	return formatVariadicArg(--i, sFmt, args...);
	}

template <class... Args>
Kernel::CString formatVariadicArg(INT64 i, std::string sFmt, UINT16 current, Args ... args)
	{
	int iPromoted = (int)current;
	if (!i)
		return CString(const_cast<char*>(std::format(sFmt, iPromoted).c_str()));
	return formatVariadicArg(--i, sFmt, args...);
	}

//Generic case for unknown arg that we should return a placeholder for

template <class T, class... Args>
Kernel::CString formatVariadicArg(INT64 i, std::string sFmt, T current, Args ... args)
	{
	if (!i)
		return CString(const_cast<char*>(std::format("<|UNKNOWN strPatternSubst Arg Type encountered!: {} - please add handling for this type in KernelString.h of Kernel|>", typeid(T).name()).c_str()));
	return formatVariadicArg(--i, sFmt, args...);
	}

//helper interior function for the standard conversion overloads
template <class T, class... Args>
Kernel::CString strInnerVariadicArgFormat(INT64 i, std::string sFmt, T current, Args ... args)
	{
	if (!i)
		return CString(const_cast<char*>(std::format(sFmt, current).c_str()));
	return formatVariadicArg(--i, sFmt, args...);
	}

template <typename ... Args>
Kernel::CString strPatternSubst(Kernel::CString sLine, Args ... args)

//  The original version of strPattern/strPatternSubst is now
//  under the '*legacy' function names.
//  This is an attempt to reimplement the logic using somewhat more
//  debuggable and modern code than the pre-C89 implementation
//  of variadic function that george was using previously.
//  Its not working quite right yet, due to issues retrieving the
//  args, but the legacy code was crashing too so...

	{
	CString sOutput;
	sOutput.GrowToFit(4000);
	char* pPatternPos = sLine.GetPointer();
	char* pPatternRunStart = pPatternPos;
	char* pPatternEnd = pPatternPos + sLine.GetLength();
	bool b1000Separator = false, bPadWithZeros = false, b64bit = false;
	CString sFormattedArg = CString("");
	INT64 iFieldWidth = 0, iLastNumber = 0, iVariadicIndex = 0;
	while (pPatternPos <= pPatternEnd)
		{
		if (*pPatternPos == '%')
			{
			pPatternPos++;
			//if string ended prematurely, just finish
			if (pPatternPos > pPatternEnd)
				break;
			//handle if just an escaped % sign
			if (*pPatternPos == '%')
				{
				pPatternPos++;
				continue;
				}
			//store what we have accumulated
			sOutput.Append(pPatternRunStart, (int)(INT64)(pPatternPos - pPatternRunStart - 1), CString::FLAG_ALLOC_EXTRA);
			//handle the format types
			// [opt] , = add a 1000s seperator
			// [opt] l = expects a 64bit number, support for legacy patterns
			// [opt] 0-9 = field width
			// Type:
			//  s = string
			//  d = digits
			//  x/X = hexidecimal lower/upper case (see: wsprintfA)
			//  p = plural ("s" if last numeric arg was != 1)
			//  & = entity name
			if (*pPatternPos == ',')
				{
				b1000Separator = true;
				pPatternPos++;
				}
			//if string ended prematurely, just finish
			if (pPatternPos > pPatternEnd)
				break;
			if (*pPatternPos == 'l')
				{
				b64bit = true;//might not need this
				pPatternPos++;
				}
			//if string ended prematurely, just finish
			if (pPatternPos > pPatternEnd)
				break;
			while (*pPatternPos >= '0' && *pPatternPos <= '9' && pPatternPos < pPatternEnd)
				{
				if (!iFieldWidth && *pPatternPos == '0')
					bPadWithZeros = true;
				iFieldWidth *= 10;
				iFieldWidth += (*pPatternPos - '0');
				pPatternPos++;
				}
			//if string ended prematurely, just finish
			if (pPatternPos > pPatternEnd)
				break;
			//Handle the actual options now
			if (*pPatternPos == 's')//expected arg is a string
				{
				sOutput.Append(formatVariadicArg(iVariadicIndex,"{}", args...), CString::FLAG_ALLOC_EXTRA);
				iVariadicIndex++;
				}
			else if (*pPatternPos == 'd')//expected arg is a number and to print in decimals
				{
				sFormattedArg = formatVariadicArg(iVariadicIndex, "{}", args...);
				iLastNumber = strtoll(sFormattedArg.GetPointer(), NULL, 10);
				DWORD dwFlags = (b1000Separator ? FORMAT_THOUSAND_SEPARATOR : 0)
					| (bPadWithZeros ? FORMAT_LEADING_ZERO : 0);
				sOutput.Append(strFormatInteger(iLastNumber, (int)iFieldWidth, dwFlags), CString::FLAG_ALLOC_EXTRA);
				iVariadicIndex++;
				}
			else if (*pPatternPos == 'p' && iLastNumber != 1)//no arg, just check for pluralization
				sOutput.Append("s", 1, CString::FLAG_ALLOC_EXTRA);
			else if (*pPatternPos == 'x' || *pPatternPos == 'X')//expected arg is a number and to print in hex
				{
				sFormattedArg = formatVariadicArg(iVariadicIndex, "{}", args...);
				iLastNumber = strtoll(sFormattedArg.GetPointer(), NULL, 10);
				char szBuffer[256];
				int iLen = autowsprintf(szBuffer, (*pPatternPos == 'x' ? "%x" : "%X"), iLastNumber);
				if (iFieldWidth > 0)
					WritePadding(sOutput, (bPadWithZeros ? '0' : ' '), (int)(iFieldWidth - iLen));
				sOutput.Append((LPCSTR)szBuffer, iLen, CString::FLAG_ALLOC_EXTRA);
				iVariadicIndex++;
				}
			else if (*pPatternPos == '&')//expected arg is an entity and to print the entity name
				{
				CString sResult = strTranslateStdEntity(formatVariadicArg(iVariadicIndex, "{}", args...));
				sOutput.Append(sResult);
				iVariadicIndex++;
				}
			else
				//invalid format, george's code seems to ignore this
				sOutput.Append(pPatternPos - 1, 2, CString::FLAG_ALLOC_EXTRA);
			pPatternPos++;
			//reset the run and other variables that might have been used
			pPatternRunStart = pPatternPos;
			iFieldWidth = 0;
			b1000Separator, b64bit = false;
			}
		else
			pPatternPos++;
		}
	//store what we have accumulated
	if (pPatternRunStart < pPatternEnd)
		sOutput.Append(pPatternRunStart, (int)(INT64)(pPatternEnd - pPatternRunStart), CString::FLAG_ALLOC_EXTRA);
	return sOutput;
	}
#endif

#ifdef CSTRING_VARIADICS_IN_HEADER
CString Kernel::strPatternLegacy (const CString &sPattern, LPVOID *pArgs)
#else
CString Kernel::strPattern (const CString &sPattern, LPVOID *pArgs)
#endif

//	strPattern
//
//	Returns a string with a pattern substitution

	{
	CString sOutput;
	sOutput.GrowToFit(4000);
	const char *pPos = sPattern.GetPointer();
	INT64 iLength = sPattern.GetLength();
	const char *pRunStart;
	int iRunLength;
	int iLastInteger = 1;

	//	Start

	pRunStart = pPos;
	iRunLength = 0;

	//	Loop

	while (iLength > 0)
		{
		if (*pPos == '%')
			{
			//	Save out what we've got til now

			if (iRunLength > 0)
				{
				sOutput.Append(pRunStart, iRunLength, CString::FLAG_ALLOC_EXTRA);
				}

			//	Check the actual pattern code

			pPos++;
			iLength--;
			if (iLength > 0)
				{
				int iMinFieldWidth = 0;
				bool bPadWithZeros = false;
				bool b1000Separator = false;
				bool b64bit = false;

				//	A leading comma means add a thousands separator

				if (*pPos == ',')
					{
					b1000Separator = true;
					pPos++;
					iLength--;
					}

				//	See if this is a long long

				if (*pPos == 'l')
					{
					pPos++;
					iLength--;

					if (*pPos == 'l')
						{
						b64bit = true;
						pPos++;
						iLength--;
						}
					}

				//	See if we've got a field width value

				if (*pPos >= '0' && *pPos <= '9')
					{
					const char *pNewPos;
					bPadWithZeros = (*pPos == '0');
					iMinFieldWidth = strParseInt(pPos, 0, &pNewPos);

					if (iLength > (pNewPos - pPos))
						{
						iLength -= (pNewPos - pPos);
						pPos = pNewPos;
						}
					}

				//	Parse the type

				if (*pPos == 's')
					{
					CString *pParam = (CString *)pArgs;

					if (iMinFieldWidth > 0)
						WritePadding(sOutput, (bPadWithZeros ? '0' : ' '), iMinFieldWidth - pParam->GetLength());

					sOutput.Append(*pParam, CString::FLAG_ALLOC_EXTRA);

					pArgs += AlignUp(sizeof(CString), sizeof(LPVOID)) / sizeof(LPVOID);
					pPos++;
					iLength--;
					}
				else if (*pPos == 'd')
					{
					if (b64bit)
						{
						INT64 *pVar = (INT64 *)pArgs;

						DWORD dwFlags = (b1000Separator ? FORMAT_THOUSAND_SEPARATOR : 0)
								| (bPadWithZeros ? FORMAT_LEADING_ZERO : 0);

						CString sNew = strFormatInteger(*pVar, iMinFieldWidth, dwFlags);

						sOutput.Append(sNew, CString::FLAG_ALLOC_EXTRA);

						//	Remember the last integer (all we care about is whether it
						//	is 1 or not, for pluralization).

						iLastInteger = (*pVar == 1 ? 1 : 0);

						//	Next

						pArgs++;
						pArgs++;
						}
					else
						{
						int *pInt = (int *)pArgs;

						DWORD dwFlags = (b1000Separator ? FORMAT_THOUSAND_SEPARATOR : 0)
								| (bPadWithZeros ? FORMAT_LEADING_ZERO : 0);

						CString sNew = strFormatInteger(*pInt, iMinFieldWidth, dwFlags);

						sOutput.Append(sNew, CString::FLAG_ALLOC_EXTRA);

						//	Remember the last integer

						iLastInteger = *pInt;

						//	Next

						pArgs++;
						}

					pPos++;
					iLength--;
					}
				else if (*pPos == 'x' || *pPos == 'X')
					{
					int *pInt = (int *)pArgs;
					char szBuffer[256];
					int iLen = wsprintf(szBuffer, (*pPos == 'x' ? "%x" : "%X"), *pInt);

					if (iMinFieldWidth > 0)
						WritePadding(sOutput, (bPadWithZeros ? '0' : ' '), iMinFieldWidth - iLen);

					sOutput.Append(szBuffer, iLen, CString::FLAG_ALLOC_EXTRA);

					//	Remember the last integer

					iLastInteger = *pInt;

					//	Next

					pArgs++;
					pPos++;
					iLength--;
					}
				else if (*pPos == 'p')
					{
					if (iLastInteger != 1)
						{
						sOutput.Append("s", 1, CString::FLAG_ALLOC_EXTRA);
						}

					pPos++;
					iLength--;
					}
				else if (*pPos == '&')
					{
					//	Get the entity name

					pPos++;
					iLength--;
					const char *pStart = pPos;
					while (iLength > 0 && *pPos != ';')
						{
						pPos++;
						iLength--;
						}

					CString sEntity(pStart, (int)(pPos - pStart));
					CString sResult = CHTML::TranslateStdEntity(sEntity);

					sOutput.Append(sResult);

					if (iLength > 0 && *pPos == ';')
						{
						pPos++;
						iLength--;
						}
					}
				else if (*pPos == '%')
					{
					sOutput.Append("%", 1, CString::FLAG_ALLOC_EXTRA);

					pPos++;
					iLength--;
					}
				}

			pRunStart = pPos;
			iRunLength = 0;
			}
		else
			{
			iRunLength++;
			iLength--;
			pPos++;
			}
		}

	//	Save out the last run

	if (iRunLength > 0)
		{
		sOutput.Append(pRunStart, iRunLength, CString::FLAG_ALLOC_EXTRA);
		}

	//	Resize to the correct length

	return CString(sOutput.GetPointer(), sOutput.GetLength());
	}


#ifdef CSTRING_VARIADICS_IN_HEADER
CString Kernel::strPatternSubstLegacy (CString sLine, ...)
#else
CString Kernel::strPatternSubst (CString sLine, ...)
#endif

//	strPatternSubst
//
//	Substitutes patterns

	{
	char *pArgs;
	CString sParsedLine;

	pArgs = (char *) &sLine + sizeof(sLine);
#ifdef CSTRING_VARIADICS_IN_HEADER
	sParsedLine = strPatternLegacy(sLine, (void **)pArgs);
#else
	sParsedLine = strPattern(sLine, (void **)pArgs);
#endif
	return sParsedLine;
	}

void Kernel::WritePadding (Kernel::CString &sOutput, char chChar, int iLen)
	{
	if (iLen > 0)
		{
		char szBuffer[256];
		char *pBuffer;
		if (iLen <= sizeof(szBuffer))
			pBuffer = szBuffer;
		else
			pBuffer = new char [iLen];

		char *pPos = pBuffer;
		char *pEndPos = pPos + iLen;
		while (pPos < pEndPos)
			*pPos++ = chChar;

		sOutput.Append(pBuffer, iLen, Kernel::CString::FLAG_ALLOC_EXTRA);

		if (pBuffer != szBuffer)
			delete [] pBuffer;
		}
	}
