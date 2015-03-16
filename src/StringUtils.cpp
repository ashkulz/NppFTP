/*
    NppFTP: FTP/SFTP functionality for Notepad++
    Copyright (C) 2010  Harry (harrybharry@users.sourceforge.net)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "StdInc.h"
//#include "StringUtils.h"

#include <wchar.h>

#ifdef UNICODE
#define EXECUNICODE(s) {s}
#define EXECMBYTE(s)
#else
#define EXECUNICODE(s)
#define EXECMBYTE(s) {s}
#endif //UNICODE

TCHAR* SU::Utf8ToTChar(const char * utf8string) {
	if (utf8string == NULL)
		return NULL;

	if (utf8string[0] == 0) {
		TCHAR * empty = new TCHAR[1];
		empty[0] = 0;
		return empty;
	}

	int size = ::MultiByteToWideChar(CP_UTF8, 0, utf8string, -1, NULL, 0);
	if (size == 0)
		return NULL;

	wchar_t * wstring = new wchar_t[size];
	int ret = ::MultiByteToWideChar(CP_UTF8, 0, utf8string, -1, wstring, size);
	if (ret == 0) {
		delete [] wstring;
		return NULL;
	}

#ifdef UNICODE
	return wstring;
#else
	size = ::WideCharToMultiByte(CP_ACP, 0, wstring, -1, NULL, 0, NULL, NULL);
	if (size == 0) {
		delete [] wstring;
		return NULL;
	}
	char * tstring = new char[size];
	ret = ::WideCharToMultiByte(CP_ACP, 0, wstring, -1, tstring, size, NULL, NULL);
	delete [] wstring;

	if (ret == 0) {
		delete [] tstring;
		return NULL;
	}

	return tstring;
#endif //UNICODE
}

char* SU::TCharToUtf8(const TCHAR * string) {
	return TCharToCP(string, CP_UTF8);
}

char* SU::TCharToCP(const TCHAR * string, int cp) {
	int size = 0;
	int ret = 0;

	if (string == NULL)
		return NULL;

	if (string[0] == 0) {
		char * empty = new char[1];
		empty[0] = 0;
		return empty;
	}

#ifndef UNICODE
	size = ::MultiByteToWideChar(CP_ACP, 0, string, -1, NULL, 0);
	if (size == 0)
		return NULL;

	wchar_t * wstring = new wchar_t[size];
	ret = ::MultiByteToWideChar(CP_ACP, 0, string, -1, wstring, size);
	if (ret == 0) {
		delete [] wstring;
		return NULL;
	}
#else
	const wchar_t * wstring = string;
#endif //UNICODE

	size = ::WideCharToMultiByte(cp, 0, wstring, -1, NULL, 0, NULL, NULL);
	if (size == 0) {
		EXECMBYTE(delete [] wstring;)
		return NULL;
	}
	char * utf8string = new char[size];
	ret = ::WideCharToMultiByte(cp, 0, wstring, -1, utf8string, size, NULL, NULL);
	EXECMBYTE(delete [] wstring;)

	if (ret == 0) {
		delete [] utf8string;
		return NULL;
	}

	return utf8string;
}

TCHAR* SU::DupString(const TCHAR* string) {
	if (!string)
		return NULL;

	int size = lstrlen(string) + 1;
	TCHAR * newstring = new TCHAR[size];
	lstrcpy(newstring, string);
	return newstring;
}

char * SU::strdup(const char* string) {
	if (!string)
		return NULL;
	char * dupstr = new char[strlen(string)+1];//(char*)malloc((strlen(string)+1)*sizeof(char));
	strcpy(dupstr, string);
	return dupstr;
}

wchar_t* SU::CharToWChar(const char * string) {
	if (!string)
		return NULL;

	int size = 0;
	int ret = 0;

	if (string == NULL)
		return NULL;

	if (string[0] == 0) {
		wchar_t * empty = new wchar_t[1];
		empty[0] = 0;
		return empty;
	}

	size = ::MultiByteToWideChar(CP_ACP, 0, string, -1, NULL, 0);
	if (size == 0)
		return NULL;
	wchar_t * wstring = new wchar_t[size];
	ret = ::MultiByteToWideChar(CP_ACP, 0, string, -1, wstring, size);
	if (ret == 0) {
		delete [] wstring;
		return NULL;
	}
	return wstring;

}

char* SU::WCharToChar(const wchar_t * wstring) {
	int size = 0;
	int ret = 0;

	if (wstring == NULL)
		return NULL;

	if (wstring[0] == 0) {
		char * empty = new char[1];
		empty[0] = 0;
		return empty;
	}

	size = ::WideCharToMultiByte(CP_ACP, 0, wstring, -1, NULL, 0, NULL, NULL);
	if (size == 0)
		return NULL;

	char * string = new char[size];
	ret = ::WideCharToMultiByte(CP_ACP, 0, wstring, -1, string, size, NULL, NULL);
	if (ret == 0) {
		delete wstring;
		return NULL;
	}
	return string;
}

int SU::FreeWChar(wchar_t * string) {
	if (!string)
		return -1;

	delete [] string;
	return 0;
}

int SU::FreeTChar(TCHAR * string) {
	if (!string)
		return -1;

	delete [] string;
	return 0;
}

int SU::FreeChar(char * string) {
	if (!string)
		return -1;

	delete [] string;
	return 0;
}

void SU::free(char * data) {
	delete [] data;
	return;
}

bool SU::InString(const tstring & source, const tstring & find) {
	tstring result(source);

	tstring::size_type pos = result.find(find, pos);
	
	return pos != tstring::npos ? true : false;
}

tstring SU::ReplaceString(const tstring & source, const tstring & find, const tstring & replace) {
	tstring result(source);
	size_t findlen = find.length();
	size_t replacelen = replace.length();

	tstring::size_type pos = 0;
	while( (pos = result.find(find, pos)) != tstring::npos ) {
		result.erase(pos, findlen);
		result.insert(pos, replace);
		pos += replacelen;
	}

	return result;
}

#ifdef UNICODE
#define _vsntprintf _vsnwprintf
#define _vsctprintf _vscwprintf
#else
#define _vsntprintf vsnprintf
#define _vsctprintf _vscprintf
#endif

//update MinGW to get this in CRT
//_CRTIMP int __cdecl __MINGW_NOTHROW	_vscwprintf (const wchar_t*, __VALIST);

TCHAR* SU::TSprintfNB(const TCHAR * format, ...) {
	va_list vaList;
	va_start(vaList, format);
	int size = TSprintfV(NULL, 0, format, vaList);
	if (size < 1) {
		va_end(vaList);
		return NULL;
	}
	TCHAR * buffer = new TCHAR[size+1];
	/*int ret = */TSprintfV(buffer, size+1, format, vaList);

	va_end(vaList);
	return buffer;
}

int SU::TSprintf(TCHAR * buffer, size_t bufferSize, const TCHAR * format, ...) {
	va_list vaList;
	va_start(vaList, format);

	int ret = TSprintfV(buffer, bufferSize, format, vaList);

	va_end(vaList);
	return ret;
}

int SU::TSprintfV(TCHAR * buffer, size_t bufferSize, const TCHAR * format, va_list vaList) {
	TCHAR * msgTchar = new TCHAR[lstrlen(format)+1];
	lstrcpy(msgTchar, format);
	TCHAR * current = msgTchar;
	while(*current != 0) {
		if (*current == TEXT('%')) {
			switch(*(current+1)) {
				case TEXT('T'):
				#ifdef UNICODE
					*(current+1) = 's';
				#else
					*(current+1) = 'S';
				#endif
					break;
				case TEXT('s'):
				#ifdef UNICODE
					*(current+1) = 'S';
				#else
					*(current+1) = 's';
				#endif
					break;
				case TEXT('S'):
				#ifdef UNICODE
					*(current+1) = 's';
				#else
					*(current+1) = 'S';
				#endif
					break;
				default:
					break;
			}
		}
		current++;
	}

	int ret = 0;
	if (!buffer || bufferSize < 1) {
		ret = 1024;//_vsctprintf(msgTchar, vaList);	//TODO: update mingw and get this damned function
	} else {
		ret = _vsntprintf(buffer, bufferSize, msgTchar, vaList);
	}

	delete [] msgTchar;
	return ret;
}

char* SU::DataToHex(const char * data_, int len) {
	static const char * table = "0123456789ABCDEF";

	const unsigned char * data = (unsigned char*)data_;

	if (len == -1)
		len = strlen(data_);

	char * hexString = new char[len*2+1];
	for(int i = 0; i < len; i++) {
		unsigned int curVal = (unsigned int)data[i];
		hexString[i*2] = table[curVal/16];
		hexString[i*2+1] = table[curVal%16];
	}
	hexString[len*2] = 0;

	return hexString;
}

char* SU::HexToData(const char * hex, int len, bool addZero) {
	if (len == -1)
		len = strlen(hex);

	if (len%2 != 0)
		return NULL;

	len = len/2;
	unsigned char * data = new unsigned char[len + (addZero?1:0)];

	for(int i = 0; i < len; i++) {
		data[i] = 0;

		if (hex[i*2] <= '9')
			data[i] += (hex[i*2] - '0') * 16;
		else
			data[i] += ((hex[i*2] - 'A') + 10) * 16;

		if (hex[i*2+1] <= '9')
			data[i] += (hex[i*2+1] - '0');
		else
			data[i] += (hex[i*2+1] - 'A') + 10;
	}

	if (addZero) {
		data[len] = 0;
	}

	return (char*)data;
}
