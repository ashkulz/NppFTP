//=================================================================
//  class: CUT_Str
//  File:  UT_StrOp.h
//
//  Purpose:
//
//  Various ANSI to widechar conversion helpers.
//
//=================================================================
// Ultimate TCP/IP v4.2
// This software along with its related components, documentation and files ("The Libraries")
// is © 1994-2007 The Code Project (1612916 Ontario Limited) and use of The Libraries is
// governed by a software license agreement ("Agreement").  Copies of the Agreement are
// available at The Code Project (www.codeproject.com), as part of the package you downloaded
// to obtain this file, or directly from our office.  For a copy of the license governing
// this software, you may contact us at legalaffairs@codeproject.com, or by calling 416-849-8900.
//=================================================================

#ifndef UT_Str59345439583
#define UT_Str59345439583

#include <malloc.h>

class CUT_Str
{
public:

	static void wcscpy(wchar_t * dest, size_t size, const wchar_t * source);

	static void tcscpy(TCHAR * dest, SIZE_T length, const TCHAR* src);

	static char* fcvt(double val, int count, int * dec, int * sign);

	static void strncpy(char * dest, size_t size, const char * src, size_t count);

	static void stprintf(TCHAR * dest, size_t size, const TCHAR * src, ...);

	static void sntprintf(TCHAR * dest, size_t size, size_t count, const TCHAR * src, ...);

	static void sprintf(char * dest, size_t size, const char * src, ...);

	static void strcpy(char * dest, size_t size, const char * src);

	static void tcscat(TCHAR * dest, SIZE_T length, const TCHAR* src);

	static void tcsncat(TCHAR * dest, size_t size, const TCHAR * src, size_t count);

	static void itot(int value, TCHAR * dest, size_t size, int radix);

	static void itoa(int value, char * dest, size_t size, int radix);

	static void ltot(long value, TCHAR * dest, size_t size, int radix);

	static TCHAR* tcstok(TCHAR* strToken, const TCHAR* strDelimit, TCHAR ** context);

	static void tsplitpath(const TCHAR * path, TCHAR * drive, size_t driveSizeInTCHARacters, TCHAR * dir,
		size_t dirSizeInTCHARacters, TCHAR * fname, size_t nameSizeInTCHARacters, TCHAR * ext, size_t extSizeInBytes);

	static void tcsncpy(TCHAR * dest, size_t dstSize, const TCHAR * src, size_t maxCount);

	static TCHAR * tgetenv(const TCHAR *varname);

	static size_t mbstowcs(wchar_t *wcstr, size_t sizeInWords, const char *mbstr, size_t count );

	static void wcstombs (size_t *pConvertedChars, char *mbstr, size_t sizeInBytes, const wchar_t *wcstr, size_t count); 

	// UTCPIP specific fns to enable copy/concatenation of char and whchar 
	// dest type determines conversion.
#if defined _UNICODE
	static void cvtcpy(wchar_t * dest, size_t size, const char * source, UINT codePage = CP_ACP);
	static void cvtcpy(char * dest, size_t size, const wchar_t * source);
	static void cvtncpy(wchar_t * dest, size_t size, const char * src, size_t count);
	static void cvtncpy(char * dest, size_t size, const wchar_t * src, size_t count);
	static void cvtcat(wchar_t * dest, size_t size, const char * source);
	static void cvtcat(char * dest, size_t size, const wchar_t * source);
	static void cvtncat(wchar_t * dest, size_t size, const char * src, size_t count);
	static void cvtncat(char * dest, size_t size, const wchar_t * src, size_t count);
#else
#define cvtcpy   tcscpy
#define cvtncpy  tcsncpy
#define cvtcat   tcscat
#define cvtncat  tcsncat
#endif

#if defined _UNICODE
	static LPTSTR _WC(LPTSTR dest, LPCSTR source);
	static LPSTR _AC(LPSTR dest, LPCTSTR source);
#endif

#if defined _UNICODE
#define WC(x)\
	(x == NULL) ? NULL : CUT_Str::_WC((_TCHAR*) alloca((strlen(x)+1)*sizeof(_TCHAR)), x)

#define AC(x)\
	(x == NULL) ? NULL : CUT_Str::_AC((char*) alloca((_tcslen(x)+1)*sizeof(char)), x)

#else
#define WC(x) x
#define AC(x) x
#endif
};

#endif