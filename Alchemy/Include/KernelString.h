//	KernelString.h
//
//	Kernel definitions.
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#define LITERAL(str)		((CString)(str))
#define CONSTLIT(str)		(CString(str, sizeof(str)-1, TRUE))

#define CONSTDEF(str)		((int)(sizeof(str) - 1)), str
#define CONSTDEFS(str)		{ ((int)(sizeof(str) - 1)), str }
#define CONSTUSE(constEntry)	CString((constEntry).pszString, (constEntry).iLen, true)

struct SConstString
	{
	size_t iLen;
	const char *pszString;
	};

class CString
	{
	public:
		enum CapitalizeOptions
			{
			capFirstLetter,
			};

		enum CharacterSets
			{
			csUnknown,

			csSystem,
			csUTF8,
			};

		CString (void);
		CString (const char *pString);
		CString (CharacterSets iCharSet, const char *pString);
		CString (const char* pString, size_t iLength);
		CString (const char* pString, size_t iLength, BOOL bExternal);
		CString (const Kernel::SConstString &String);
		~CString (void);

		CString (const CString &pString);
		CString &operator= (const CString &pString);
		operator LPSTR () const { return GetASCIIZPointer(); }
		explicit operator bool () const { return !IsBlank(); }
		bool operator== (const CString &sValue) const;
		bool operator!= (const CString &sValue) const;

		static constexpr DWORD FLAG_ALLOC_EXTRA = 0x00000001;
		void Append (LPCSTR pString, int iLength = -1, DWORD dwFlags = 0);
		void Append (const CString &sString, DWORD dwFlags = 0) { Append(sString.GetPointer(), sString.GetLength(), dwFlags); }

		void Capitalize (CapitalizeOptions iOption);
		char *GetASCIIZPointer (void) const;
		int GetLength (void) const;
		int GetMemoryUsage (void) const;
		char *GetPointer (void) const;
		char *GetWritePointer (size_t iLength);
		void GrowToFit (size_t iLength);
		bool IsBlank (void) const { return (GetLength() == 0); }
		void ReadFromStream (IReadStream *pStream);
		void Transcribe (const char *pString, size_t iLen);
		void Truncate (size_t iLength);
		void WriteToStream (IWriteStream *pStream) const;

		//	These are used internally only

		static void INTStringCleanUp (void);
		static ALERROR INTStringInit (void);

		//	These are used for custom string arrays

		static void *INTCopyStorage (void *pvStore);
		static void *INTGetStorage (const CString &sString);
		static void INTFreeStorage (void *pStore);
		static CString INTMakeString (void *pvStore);
		static void INTSetStorage (CString &sString, void *pStore);
		void INTTakeStorage (void *pStore);
		static void InitLowerCaseAbsoluteTable (void);

		//	Debugging APIs

#ifdef DEBUG_STRING_LEAKS
		static int DebugGetStringCount (void);
		static void DebugMark (void);
		static void DebugOutputLeakedStrings (void);
#endif

		//	Only used by CArchiver and CUnarchiver.

		ALERROR LoadHandler (CUnarchiver *pUnarchiver);
		ALERROR SaveHandler (CArchiver *pArchiver);

	private:
		struct STORESTRUCT
			{
			int iRefCount;
			int iAllocSize;				//	If negative, this is a read-only external allocation
			int iLength;
			char *pString;
#ifdef DEBUG_STRING_LEAKS
			int iMark;
#endif
			};
		typedef struct STORESTRUCT *PSTORESTRUCT;

		CString (void *pStore, bool bDummy);

		static void AddToFreeList (PSTORESTRUCT pStore, int iSize);
		PSTORESTRUCT AllocStore (int iSize, BOOL bAllocString);
#ifdef INLINE_DECREF
		void DecRefCount (void)
			{
			if (m_pStore && (--m_pStore->iRefCount) == 0)
				FreeStore(m_pStore);
			}
#else
		void DecRefCount (void);
#endif

		static void FreeStore (PSTORESTRUCT pStore);
		void IncRefCount (void) { if (m_pStore) m_pStore->iRefCount++; }
		BOOL IsExternalStorage (void) { return (m_pStore->iAllocSize < 0 ? TRUE : FALSE); }

		static constexpr DWORD FLAG_PRESERVE_CONTENTS =		0x00000001;
		static constexpr DWORD FLAG_GEOMETRIC_GROWTH =		0x00000002;
		void Size (int iLength, DWORD dwFlags = 0);

		PSTORESTRUCT m_pStore;

		static PSTORESTRUCT g_pStore;
		static int g_iStoreSize;
		static PSTORESTRUCT g_pFreeStore;
	};

class CWordParser
	{
	public:
		CWordParser (const char *pStart, const char *pEnd) :
				m_pStart(pStart),
				m_pEnd(pEnd),
				m_pPos(pStart)
			{ }

		Kernel::CString ParseNextWord ();
		void SetExcludePercentFields (bool bValue = true) { m_bExcludePercentFields = bValue; }
		void SetHandleAccelerators (bool bValue = true) { m_bHandleAccelerators = bValue; }

		static bool IsPunctuation (const char *pPos)
			{
			switch (*pPos)
				{
				case '!':
				case '&':
				case '(':
				case ')':
				case '[':
				case ']':
				case '{':
				case '}':
				case ',':
				case '.':
				case ':':
				case ';':
				case '?':
				case '\'':
				case '\"':
				case '\x97':
					return true;

				default:
					return false;
				}
			}

	private:
		const char *m_pStart = "";
		const char *m_pEnd = "";

		const char *m_pPos = "";

		bool m_bExcludePercentFields = false;
		bool m_bHandleAccelerators = false;

		bool m_bInDoubleQuote = false;
		bool m_bInSingleQuote = false;
	};

extern const Kernel::CString NULL_STR;
extern bool g_bLowerCaseAbsoluteTableInit;
extern char g_LowerCaseAbsoluteTable[256];

//	String functions (CString.cpp)

Kernel::CString strCat (const Kernel::CString &sString1, const Kernel::CString &sString2);
int strCompare (const Kernel::CString &sString1, const Kernel::CString &sString2);
int strCompareAbsolute (const Kernel::CString &sString1, const Kernel::CString &sString2);
int strCompareAbsolute (LPCSTR pS1, LPCSTR pS2);
Kernel::CString strConvert (const Kernel::CString &sText, DWORD dwFromCP, DWORD dwToCP);
inline Kernel::CString strANSIToUTF8 (const Kernel::CString &sText) { return Kernel::strConvert(sText, CP_ACP, CP_UTF8); }
inline Kernel::CString strUTF8ToANSI (const Kernel::CString &sText) { return Kernel::strConvert(sText, CP_UTF8, CP_ACP); }

int strDelimitCount (const Kernel::CString &sString, char cDelim, DWORD dwFlags);
Kernel::CString strDelimitGet (const Kernel::CString &sString, char cDelim, DWORD dwFlags, int iIndex);

Kernel::CString strCapitalize (const Kernel::CString &sString, int iOffset = 0);
Kernel::CString strCapitalizeWords (const Kernel::CString &sString);
Kernel::CString strCEscapeCodes (const Kernel::CString &sString);
Kernel::CString strConvertToToken (const Kernel::CString &sString, bool bLowercase = false);
Kernel::CString strEncodeUTF8Char (DWORD dwCodePoint);
Kernel::CString strEncodeW1252ToUTF8Char (char chChar);
bool strEndsWith (const Kernel::CString &sString, const Kernel::CString &sStringToFind);
bool strEquals (const Kernel::CString &sString1, const Kernel::CString &sString2);
bool strEqualsCase (const Kernel::CString &sString1, const Kernel::CString &sString2);
int strFind (const Kernel::CString &sString, const Kernel::CString &sStringToFind);

Kernel::CString strFormatBytes (DWORD dwBytes);

#define FORMAT_LEADING_ZERO						0x00000001
#define FORMAT_THOUSAND_SEPARATOR				0x00000002
#define FORMAT_UNSIGNED							0x00000004
Kernel::CString strFormatInteger (int iValue, int iMinFieldWidth = -1, DWORD dwFlags = 0);
Kernel::CString strFormatInteger (INT64 iValue, int iMinFieldWidth = -1, DWORD dwFlags = 0);

Kernel::CString strFormatMicroseconds (DWORD dwMicroseconds);
Kernel::CString strFormatMilliseconds (DWORD dwMilliseconds);
Kernel::CString strFromDouble (double rValue, int iDecimals = -1);
Kernel::CString strFromInt (int iInteger, bool bSigned = true);
Kernel::CString strFromPtr (void* pPtr);
int strGetHexDigit (const char *pPos);
char strGetHexDigit (int iDigit);
inline bool strIsAlpha (const char *pPos) { return (::IsCharAlpha(*pPos) == TRUE); }
inline bool strIsAlphaNumeric (const char *pPos) { return (::IsCharAlphaNumeric(*pPos) == TRUE); }
inline bool strIsASCIIAlpha (const char *pPos) { return (*pPos >= 'a' && *pPos <= 'z') || (*pPos >= 'A' && *pPos <= 'Z'); }
inline bool strIsASCIIControl (const char *pPos) { return ((BYTE)*pPos <= (BYTE)0x1f) || *pPos == 0x7f; }
bool strIsASCIISymbol (const char *pPos);
inline bool strIsDigit (const char *pPos) { return (*pPos >= '0' && *pPos <= '9'); }
bool strIsInt (const Kernel::CString &sValue, DWORD dwFlags = 0, int *retiValue = NULL);
bool strIsUpper (const char *pPos);
inline bool strIsWhitespace (const char *pPos) { return *pPos == ' ' || *pPos == '\t' || *pPos == '\n' || *pPos == '\r'; }
Kernel::CString strLoadFromRes (HINSTANCE hInst, int iResID);
inline char strLowerCaseAbsolute (char chChar) { if (!Kernel::g_bLowerCaseAbsoluteTableInit) Kernel::CString::InitLowerCaseAbsoluteTable(); return Kernel::g_LowerCaseAbsoluteTable[(BYTE)chChar]; }
bool strNeedsEscapeCodes (const Kernel::CString &sString);

#define PARSE_THOUSAND_SEPARATOR				0x00000001
double strParseDouble (const char *pStart, double rNullResult, const char **retpEnd, bool *retbNullValue);
int strParseInt (const char *pStart, int iNullResult, DWORD dwFlags, const char **retpEnd = NULL, bool *retbNullValue = NULL);
inline int strParseInt (const char *pStart, int iNullResult, const char **retpEnd = NULL, bool *retbNullValue = NULL) { return Kernel::strParseInt(pStart, iNullResult, 0, retpEnd, retbNullValue); }
int strParseIntOfBase (const char *pStart, int iBase, int iNullResult, const char **retpEnd = NULL, bool *retbNullValue = NULL);

void strParseWhitespace (const char *pPos, const char **retpPos);
Kernel::CString strPatternLegacy (const Kernel::CString &sPattern, LPVOID *pArgs);
Kernel::CString strPatternSubstLegacy(Kernel::CString sLine, ...);
void WritePadding(Kernel::CString& sOutput, char chChar, int iLen);
Kernel::CString strTranslateStdEntity(Kernel::CString sEntity);

//template <typename ... Args>
//Kernel::CString strPatternSubst (Kernel::CString sLine, Args ... args);

constexpr DWORD STRPROC_NO_DOUBLE_QUOTES =			0x00000001;
constexpr DWORD STRPROC_ESCAPE_DOUBLE_QUOTES =		0x00000002;
Kernel::CString strProcess (const Kernel::CString &sValue, DWORD dwFlags);

Kernel::CString strRepeat (const Kernel::CString &sString, int iCount);
Kernel::CString strRomanNumeral (int i);
bool strStartsWith (const Kernel::CString &sString, const Kernel::CString &sStringToFind);
Kernel::CString strSubString (const Kernel::CString &sString, int iOffset, int iLength = -1);
Kernel::CString strTitleCapitalize (const Kernel::CString &sString, const char **pExceptions = NULL, int iExceptionsCount = 0);
double strToDouble (const Kernel::CString &sString, double rFailResult, bool *retbFailed = NULL);
Kernel::CString strToFilename (const Kernel::CString &sString);
int strToInt (const Kernel::CString &sString, int iFailResult, bool *retbFailed = NULL);
Kernel::CString strToLower (const Kernel::CString &sString);
Kernel::CString strToUpper (const Kernel::CString &sString);
Kernel::CString strToXMLText (const Kernel::CString &sString, bool bInBody = false);
Kernel::CString strTrimWhitespace (const Kernel::CString &sString, bool bLeading = true, bool bTrailing = true);
Kernel::CString strWord (const Kernel::CString &sString, int iWordPos);

//  Variadic Template based string functions that have to be in this file otherwise linking fails

template <class... Args>
int autowsprintf(LPSTR retBuffer, LPCTSTR formatBuffer, Args ... args) { return wsprintfA(retBuffer, formatBuffer, args...); }

template <class... Args>
int autowsprintf(LPWSTR retBuffer, LPCWSTR formatBuffer, Args ... args) { return wsprintfW(retBuffer, formatBuffer, args...); }

inline Kernel::CString formatVariadicArg(INT64 i, std::string sFmt) { return Kernel::CString(""); }

template <class T, class... Args>
Kernel::CString formatVariadicArg(INT64 i, std::string sFmt, T current, Args ... args)
{
	if (!i)
		return CString(const_cast<char*>(std::format(sFmt, current).c_str()));
	return formatVariadicArg(i--, sFmt, args...);
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
			sOutput.Append(pPatternRunStart, (int)(INT64)(pPatternPos - pPatternRunStart), CString::FLAG_ALLOC_EXTRA);
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
		sOutput.Append(pPatternRunStart, (int)(INT64)(pPatternPos - pPatternRunStart), CString::FLAG_ALLOC_EXTRA);
	return sOutput;
}
