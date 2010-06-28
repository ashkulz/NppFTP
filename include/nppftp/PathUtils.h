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

#ifndef PATHUTILS_H
#define PATHUTILS_H

extern TCHAR * _ConfigPath;

class PU {
public:
	static int				LocalToExternalPath(const TCHAR * local, char * external, int externalsize);
	static int				ExternalToLocalPath(const char * external, TCHAR * local, int localsize);		//-1 error, 0 ok, 1 characters converted

	static const char*		FindExternalFilename(const char * externalpath);
	static const TCHAR*		FindLocalFilename(const TCHAR * localpath);

	static int				ConcatLocal(const TCHAR * path, const TCHAR * rest, TCHAR * buffer, int bufsize);
	static int				ConcatExternal(const char * path, const char * rest, char * buffer, int bufsize);

	static int				ConcatLocalToExternal(const char * external, const TCHAR * _local, char * extbuf, int extsize);
	static int				ConcatExternalToLocal(const TCHAR * local, const char * _external, TCHAR * localbuf, int localsize);

	static int				QuoteLocalPath(const TCHAR * path, TCHAR * buffer, int buffersize);
	static int				QuoteExternalPath(const char * path, char * buffer, int buffersize);

	static int				CreateLocalDir(const TCHAR * local);
	static int				CreateLocalDirFile(const TCHAR * file);

	static int				GetOpenFilename(TCHAR * buffer, int bufSize, HWND hOwner);	//if buffer[0] != 0, buffer contains initial name
	static int				GetSaveFilename(TCHAR * buffer, int bufSize, HWND hOwner);	//if buffer[0] != 0, buffer contains initial name
	static int				BrowseDirectory(TCHAR * buffer, int bufSize, HWND hOwner);

	static int				SimplifyExternalPath(const char * path, const char * currentDir, char * buffer, int bufSize);
private:
	static bool				IsValidLocalChar(const TCHAR localchar);
};


/*
Reserved chars:
    * < (less than)
    * > (greater than)
    * : (colon)
    * " (double quote)
    * / (forward slash)
    * \ (backslash)
    * | (vertical bar or pipe)
    * ? (question mark)
    * * (asterisk)
    * Others depending on filesystem
    8.3 also (?):
    + , ; = [ ]
*/

#endif //PATHUTILS_H
