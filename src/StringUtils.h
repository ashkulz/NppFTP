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

	static TCHAR*			DupString(const TCHAR * string);
	static char*			strdup(const char * string);

	static wchar_t*			CharToWChar(const char * string);
	static char*			WCharToChar(const wchar_t * wstring);

	static int				FreeWChar(wchar_t * string);
	static int				FreeTChar(TCHAR * string);
	static int				FreeChar(char * string);
	static void				free(char * data);

	static bool				InString(const tstring & source, const tstring & find);
	static tstring			ReplaceString(const tstring & source, const tstring & find, const tstring & replace);
	static TCHAR*			TSprintfNB(const TCHAR * format, ...);	//NB: no buffer
	static int				TSprintf(TCHAR * buffer, size_t bufferSize, const TCHAR * format, ...);
	static int				TSprintfV(TCHAR * buffer, size_t bufferSize, const TCHAR * format, va_list vaList);

	static char*			DataToHex(const char * data, int len);	//result is always zero terminated
	static char*			HexToData(const char * hex, int len, bool addZero = true);	//result is zero terminated if addZero == true
private:
};

#endif //STRINGUTILS_H
