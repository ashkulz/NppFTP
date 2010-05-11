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

#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#ifdef UNICODE
typedef std::wstring tstring;
#else
typedef std::string tstring;
#endif

class SU {
public:
	static TCHAR*			Utf8ToTChar(const char * utf8string);
	static char*			TCharToUtf8(const TCHAR * string);
	static char*			TCharToCP(const TCHAR * string, int cp);

	static TCHAR*			DupString(const TCHAR* string);
	static char*			strdup(const char* string);

	static wchar_t*			CharToWChar(const char * string);
	static char*			WCharToChar(const wchar_t * wstring);

	static int				FreeTChar(const TCHAR * string);
	static int				FreeUtf8(const char * string);

	static tstring			ReplaceString(const tstring & source, const tstring & find, const tstring & replace);
	static TCHAR*			TSprintfNB(const TCHAR * format, ...);	//NB: no buffer
	static int				TSprintf(TCHAR * buffer, size_t bufferSize, const TCHAR * format, ...);
	static int				TSprintfV(TCHAR * buffer, size_t bufferSize, const TCHAR * format, va_list vaList);
private:
};

#endif //STRINGUTILS_H
