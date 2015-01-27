//=================================================================
//  class: CUT_Str
//  File:  UT_StrOp.cpp
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

#include "stdafx.h"
#include "ut_strop.h"

#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <assert.h>

//////////////////////////////////////////////////////////////////////
// UTCPIP specific fns to enable copy/concatenation of char and whchar. 
// Dest type determines conversion direction for source. (cvtxxx)
/////////////////////////////////////////////////////////////////////
#if defined _UNICODE
// In calling cvtcpy, allocate _tcslen + 1 of source for dest, and pass _tcslen as size.
void CUT_Str::cvtcpy(wchar_t * dest, size_t size, const char * source, UINT codePage)
{
	// now using MultiByteToWideChar to enable code page specification
	*dest = _T('\0');
	int count = MultiByteToWideChar(codePage, 0, source, -1, dest, (int)size);
	if (count == 0) {
		*(dest+size) = _T('\0');	// v4.2 helps for ATL CString sources
	}
}
void CUT_Str::cvtcpy(char * dest, size_t size, const wchar_t * source)
{
	*dest = '\0';
	// v4.2 now using this - CUT_Str::wcstombs wonky for big strings
	WideCharToMultiByte(CP_ACP,0, source, (int)size, dest, (int)size, NULL, NULL);
	*(dest+size) = '\0';	// v 4.2 helps for ATL CString sources
	// cchWideChar - [in] Specifies the number of wide characters in the string pointed 
	// to by the lpWideCharStr parameter. ** If this value is -1 ** , the string is assumed to
	// be null-terminated and the length is calculated automatically. The length will 
	// include the null-terminator.
}
void CUT_Str::cvtncpy(wchar_t * dest, size_t size, const char * source, size_t count)
{
	*dest = _T('\0');
	size_t len = strlen(source);
	if(len > 0) 
	{
		++len;
		_TCHAR *szConverted = new _TCHAR[len+1];
		mbstowcs(szConverted, len, source, len);
#if _MSC_VER >= 1400
		_tcsncpy_s(dest, size, szConverted, count);
#else
		UNREFERENCED_PARAMETER(size);
		::_tcsncpy(dest, szConverted, count);
#endif		
		delete [] szConverted;
	}
}
void CUT_Str::cvtncpy(char * dest, size_t size, const wchar_t * source, size_t count)
{
	*dest = '\0';
	size_t len = _tcslen(source);
	if(len > 0) 
	{
		++len;
		char *szConverted = new char[len+1];
		size_t chars;
		wcstombs(&chars, szConverted, len, source, len);
#if _MSC_VER >= 1400
		strncpy_s(dest, size, szConverted, count);
#else
		UNREFERENCED_PARAMETER(size);
		::strncpy(dest, szConverted, count);
#endif		
		delete [] szConverted;
	}
}
void CUT_Str::cvtcat(wchar_t * dest, size_t size, const char * source)
{
	*dest = _T('\0');
	size_t len = strlen(source);
	if(len > 0) 
	{
		++len;
		_TCHAR *szConverted = new _TCHAR[len+1];
		mbstowcs(szConverted, len, source, len);
#if _MSC_VER >= 1400
		_tcscat_s(dest, size, szConverted);
#else
		UNREFERENCED_PARAMETER(size);
		::_tcscat(dest, szConverted);
#endif		
		delete [] szConverted;
	}
}
void CUT_Str::cvtcat(char * dest, size_t size, const wchar_t * source)
{
	*dest = '\0';
	size_t len = _tcslen(source);
	if(len > 0) 
	{
		++len;
		char *szConverted = new char[len+1];
		size_t chars;
		wcstombs(&chars, szConverted, len, source, len);
#if _MSC_VER >= 1400
		strcat_s(dest, size, szConverted);
#else
		UNREFERENCED_PARAMETER(size);
		::strcat(dest, szConverted);
#endif		
		delete [] szConverted;
	}
}
void CUT_Str::cvtncat(wchar_t * dest, size_t size, const char * source, size_t count)
{
	*dest = _T('\0');
	size_t len = strlen(source);
	if(len > 0) 
	{
		++len;
		_TCHAR *szConverted = new _TCHAR[len+1];
		mbstowcs(szConverted, len, source, len);
#if _MSC_VER >= 1400
		_tcsncat_s(dest, size, szConverted, count);
#else
		UNREFERENCED_PARAMETER(size);
		::_tcsncat(dest, szConverted, count);
#endif		
		delete [] szConverted;
	}
}
void CUT_Str::cvtncat(char * dest, size_t size, const wchar_t * source, size_t count)
{
	*dest = '\0';
	size_t len = _tcslen(source);
	if(len > 0) 
	{
		++len;
		char *szConverted = new char[len+1];
		size_t chars;
		wcstombs(&chars, szConverted, len, source, len);
#if _MSC_VER >= 1400
		strncat_s(dest, size, szConverted, count);
#else
		UNREFERENCED_PARAMETER(size);
		::strncat(dest, szConverted, count);
#endif		
		delete [] szConverted;
	}
}
#endif		// _UNICODE

void CUT_Str::tcscpy(TCHAR * dest, SIZE_T length, const TCHAR* src)
{
#if _MSC_VER >= 1400
	::_tcscpy_s(dest, length, src);
#else
	UNREFERENCED_PARAMETER(length);
	::_tcscpy(dest,src);
# endif
}

char* CUT_Str::fcvt(double val, int count, int * dec, int * sign)
{
#if _MSC_VER >= 1400
	UNREFERENCED_PARAMETER(sign);
	char * dest = new char[count * 2 + 10];
	::_fcvt_s(dest, count * 2 + 9, val, count, dec, sign);
	return dest;
#else
	return ::_fcvt(val, count, dec, sign);
# endif		
}

void CUT_Str::strncpy(char * dest, size_t size, const char * src, size_t count)
{
	if (size == 0) size = 1;
#if _MSC_VER >= 1400
	::strncpy_s(dest, size, src, count);
#else
	UNREFERENCED_PARAMETER(size);
	::strncpy(dest, src, count);
# endif
}

void CUT_Str::stprintf(TCHAR * dest, size_t size, const TCHAR * src, ...)
{
	va_list vl;
	va_start( vl, src );

#if _MSC_VER >= 1400
	_vstprintf_s_l(dest, size, src, NULL, vl);
#else
	UNREFERENCED_PARAMETER(size);
	_vstprintf(dest, src, vl);
# endif

	va_end( vl );
}

void CUT_Str::sntprintf(TCHAR * dest, size_t size, size_t count, const TCHAR * src, ...)
{
	va_list vl;
	va_start( vl, src );

#if _MSC_VER >= 1400
	_vsntprintf_s_l(dest, size, count, src, NULL, vl);
#else
	UNREFERENCED_PARAMETER(count);
	_sntprintf(dest, size, src, NULL, vl);
# endif

	va_end( vl );
}

void CUT_Str::sprintf(char * dest, size_t size, const char * src, ...)
{
	va_list vl;
	va_start( vl, src );

#if _MSC_VER >= 1400
    _vsprintf_s_l(dest, size, src, NULL, vl);
#else
	UNREFERENCED_PARAMETER(size);
	vsprintf(dest, src, vl);
# endif

	va_end( vl );
}


void CUT_Str::tcscat(TCHAR * dest, SIZE_T length, const TCHAR* src)
{
#if _MSC_VER >= 1400
	_tcscat_s(dest, length, src);
#else
	UNREFERENCED_PARAMETER(length);
	_tcscat(dest, src);
# endif
}

void CUT_Str::tcsncat(TCHAR * dest, size_t size, const TCHAR * src, size_t count)
{
#if _MSC_VER >= 1400
	_tcsncat_s(dest, size, src, count);
#else
	UNREFERENCED_PARAMETER(size);
	_tcsncat(dest, src, count);
# endif
}



void CUT_Str::itot(int value, TCHAR * dest, size_t size, int radix)
{
#if _MSC_VER >= 1400
	_itot_s(value, dest, size, radix);
#else
	UNREFERENCED_PARAMETER(size);
	_itot(value, dest, radix);
#endif
}

void CUT_Str::itoa(int value, char * dest, size_t size, int radix)
{
#if _MSC_VER >= 1400
	_itoa_s(value, dest, size, radix);
#else
	UNREFERENCED_PARAMETER(size);
	_itoa(value, dest, radix);
#endif
}

void CUT_Str::ltot(long value, TCHAR * dest, size_t size, int radix)
{
#if _MSC_VER >= 1400
	_ltot_s(value, dest, size, radix);
#else
	UNREFERENCED_PARAMETER(size);
	_ltot(value, dest, radix);
#endif
}

TCHAR* CUT_Str::tcstok(TCHAR* strToken, const TCHAR* strDelimit, TCHAR ** context)
{
#if _MSC_VER >= 1400
	return _tcstok_s(strToken, strDelimit, context);
#else
	UNREFERENCED_PARAMETER(context);
	return _tcstok(strToken, strDelimit);
#endif
}

void CUT_Str::tsplitpath(const TCHAR * path, TCHAR * drive, size_t driveSizeInTCHARacters, TCHAR * dir,
   size_t dirSizeInTCHARacters, TCHAR * fname, size_t nameSizeInTCHARacters, TCHAR * ext, size_t extSizeInBytes)
{
#if _MSC_VER >= 1400
	_tsplitpath_s(path, drive, driveSizeInTCHARacters, dir, dirSizeInTCHARacters, fname, nameSizeInTCHARacters, ext, extSizeInBytes);
#else
	UNREFERENCED_PARAMETER(driveSizeInTCHARacters);
	UNREFERENCED_PARAMETER(dirSizeInTCHARacters);
	UNREFERENCED_PARAMETER(nameSizeInTCHARacters);
	UNREFERENCED_PARAMETER(extSizeInBytes);
	_tsplitpath(path, drive, dir, fname, ext);
#endif
}
void CUT_Str::tcsncpy(TCHAR * dest, size_t dstSize, const TCHAR * src, size_t maxCount)
{
#if _MSC_VER >= 1400
	_tcsncpy_s(dest, dstSize, src, maxCount);
#else
	UNREFERENCED_PARAMETER(dstSize);
#ifdef _UNICODE
	::wcsncpy(dest,src, maxCount);
#else
	::strncpy(dest,src, maxCount);
#endif
#endif
}

TCHAR * CUT_Str::tgetenv(const TCHAR *varname) 
{
#if _MSC_VER >= 1400
	TCHAR* retval;
	size_t requiredSize;
	// Get required length
	_tgetenv_s( &requiredSize, NULL, 0, varname);
	retval = new TCHAR[requiredSize];
	if (!_tgetenv_s( &requiredSize, retval, requiredSize, varname ))
		return retval;
	else
	{
		delete [] retval;
		return NULL;
	}
	
#else
	return _tgetenv(varname);
#endif
}

// v4.2 these internal mbstowcs and wcstombs are called by the AC and WC macros below.
// For translation to wide char with code page selection (i.e CP_UTF8) see cvtcpy above.
size_t CUT_Str::mbstowcs(wchar_t *wcstr, size_t sizeInWords, const char *mbstr, size_t count )
{
#if _MSC_VER >= 1400
	size_t retval;

	if (!mbstowcs_s(&retval, wcstr, sizeInWords, mbstr, count))
		return retval;

	return 0;
#else
	UNREFERENCED_PARAMETER(sizeInWords);
	return ::mbstowcs(wcstr, mbstr, count);
#endif
}

void CUT_Str::wcstombs(size_t *pConvertedChars, char *mbstr, size_t sizeInBytes, const wchar_t *wcstr, size_t count)
{
#if _MSC_VER >= 1400
	UNREFERENCED_PARAMETER(count);
	wcstombs_s(pConvertedChars, mbstr, sizeInBytes, wcstr, _TRUNCATE);
#else
	UNREFERENCED_PARAMETER(sizeInBytes);
	*pConvertedChars = ::wcstombs(mbstr, wcstr, count);
#endif
}



// return wide char version of char string 
// Use WC() macro defined in ut_strop.h
#if defined _UNICODE
LPTSTR CUT_Str::_WC(LPTSTR dest, LPCSTR str) {
	// the WC macro should have checked str is non-null.
	// the WC macro will have allocated strlen(str)+1 wchars for dest
	dest[0] = _T('\0');
	size_t len = strlen(str)+1;
	/*size_t res =*/ CUT_Str::mbstowcs(dest, len, str, len);
	//int size = MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,source,strlen(source),dest,strlen(source));
	return dest;
}
// return ascii char version of wchar string 
// Use AC() macro defined in ut_strop.h
LPSTR CUT_Str::_AC(LPSTR dest, LPCTSTR wstr) {
	// the AC macro should have checked for NULL wstr.
	// the AC macro should have allocated strlen(dest)+1 based on len of wstr.
	dest[0] = '\0';
	size_t chars;
	size_t len = _tcslen(wstr)+1;
	CUT_Str::wcstombs(&chars, dest, len, wstr, len);
//	assert(chars);	// not to worry - AC called with zero length string in most cases - this may catch some redundancies.
	return dest;
}
#endif

void CUT_Str::wcscpy(wchar_t * dest, size_t size, const wchar_t * source)
{
#if _MSC_VER >= 1400

	wcscpy_s(dest, size, source);
#else
	UNREFERENCED_PARAMETER(size);
	::wcscpy(dest, source);
#endif
}

void CUT_Str::strcpy(char * dest, size_t size, const char * src)
{
#if _MSC_VER >= 1400
	strcpy_s(dest, size, src);
#else
	UNREFERENCED_PARAMETER(size);
	::strcpy(dest, src);
#endif
}
