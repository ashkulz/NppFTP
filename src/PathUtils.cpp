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
#include "PathUtils.h"

#include <shlobj.h>
#include <commdlg.h>

int PU::LocalToExternalPath(const TCHAR * local, char * external, int externalsize) {
	if (!local || !external || externalsize == 0)
		return -1;

	int i = 0;
	int j = 0;

	for(i = 0; i < externalsize; i++, j++) {
		if (local[i] == 0) {
			external[j] = 0;
			break;
		}

		if (local[i] == TEXT('\\')) {						//replace path separators
			external[j] = '/';
		} else {
#ifndef UNICODE
			external[j] = (char)(external[i]);
#else
			int res = WideCharToMultiByte(CP_ACP, 0, local+i, 1, external+j, externalsize-j, NULL, NULL);
			if (res == 0)
				return -1;
#endif
		}
	}

	if (i == externalsize)
		return -1;

	return 0;
}

int PU::ExternalToLocalPath(const char * external, TCHAR * local, int localsize) {
	if (!external || !local || localsize == 0)
		return -1;

	int i = 0;
	int j = 0;
	bool converted = false;

	for(i = 0; i < localsize; i++, j++) {
		if (external[i] == 0) {
			local[j] = 0;
			break;
		}

		if (external[i] == '/') {						//replace path separators
			local[j] = TEXT('\\');
		} else if (!IsValidLocalChar(external[i])) {	//replace invalid chars
			local[j] = TEXT('_');
			converted = true;
		} else {
			local[j] = (TCHAR)(external[i]);
		}
	}

	if (i == localsize)
		return -1;

	return converted?1:0;
}

const char* PU::FindExternalFilename(const char * externalpath) {
	if (!externalpath)
		return NULL;

	const char * name = strrchr(externalpath, '/');
	if (name)
		name++;	//skip '/'
	else
		return NULL;

	if (name == externalpath || name[0] == '/' || name[0] == 0)
		return NULL;

	return name;
}

const TCHAR* PU::FindLocalFilename(const TCHAR * localpath) {
	if (!localpath)
		return NULL;

	TCHAR* name = PathFindFileName(localpath);
	if (localpath == name)
		return NULL;

	return name;
}

int PU::ConcatLocal(const TCHAR * path, const TCHAR * rest, TCHAR * buffer, int bufsize) {
	if (bufsize < MAX_PATH)
		return -1;

	if (rest[0] == TEXT('\\'))
		rest++;

	LPTSTR res = PathCombine(buffer, path, rest);
	if (res == NULL)
		return -1;

	return 0;
}

int PU::ConcatExternal(const char * path, const char * rest, char * buffer, int bufsize) {
	int len1 = strlen(path);
	int len2 = strlen(rest);

	if (path[len1-1]=='/' && rest[0] == '/') {
		rest++;
		len2--;
	}

	bool hassep = path[len1-1]=='/' || rest[0] == '/';

	int total = len1+len2+1+(hassep?0:1);

	if (total > bufsize)
		return -1;

	strcpy(buffer, path);
	if (!hassep)
		strcat(buffer, "/");
	strcat(buffer, rest);

	return 0;
}

int PU::ConcatLocalToExternal(const char * external, const TCHAR * _local, char * extbuf, int extsize) {
	char local[MAX_PATH];
	int res = PU::LocalToExternalPath(_local, local, MAX_PATH);
	if (res == -1)
		return -1;

	res = PU::ConcatExternal(external, local, extbuf, extsize);
	if (res == -1)
		return -1;

	return 0;
}

int PU::ConcatExternalToLocal(const TCHAR * local, const char * _external, TCHAR * localbuf, int localsize) {
	TCHAR external[MAX_PATH];
	int res = PU::ExternalToLocalPath(_external, external, MAX_PATH);
	if (res == -1)
		return -1;

	res = PU::ConcatLocal(local, external, localbuf, localsize);
	if (res == -1)
		return -1;

	return 0;
}

int PU::QuoteLocalPath(const TCHAR * path, TCHAR * buffer, int buffersize) {
	int len = lstrlen(path);
	if (buffersize < len+3)
		return -1;

	lstrcpy(buffer, TEXT("\""));
	lstrcat(buffer, path);
	lstrcat(buffer, TEXT("\""));
	return 0;
}

int PU::QuoteExternalPath(const char * path, char * buffer, int buffersize) {
	int len = strlen(path);
	if (buffersize < len+3)
		return -1;

	strcpy(buffer, "\"");
	strcat(buffer, path);
	strcat(buffer, "\"");
	return 0;
}

int PU::CreateLocalDir(const TCHAR * local) {
	int res = SHCreateDirectoryEx(NULL, local, NULL);
	if (
		res != ERROR_SUCCESS &&
		res != ERROR_ALREADY_EXISTS &&
		res != ERROR_FILE_EXISTS
		) {
		OutErr("Failed to create directory %T", local);
		return -1;
		}

	//OutMsg("Created dir %T", local);
	return 0;
}

int PU::CreateLocalDirFile(const TCHAR * file) {
	TCHAR path[MAX_PATH];
	lstrcpyn(path, file, MAX_PATH);
	LPTSTR name = ::PathFindFileName(path);
	if (!name)
		return -1;
	name[0] = 0;
	return CreateLocalDir(path);
}

int PU::GetOpenFilename(TCHAR * buffer, int bufSize, HWND hOwner) {
	if (!buffer || bufSize == 0)
		return -1;

	if (!hOwner)
		hOwner = _MainOutputWindow;

	OPENFILENAME ofn;
	ofn.lStructSize = sizeof(ofn);	//not NT4.0 compatible
	ofn.hwndOwner = hOwner;
	ofn.lpstrFilter = NULL;	//accept everything
	ofn.lpstrCustomFilter = NULL;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = buffer;
	ofn.nMaxFile = bufSize;
	ofn.lpstrFileTitle = NULL;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle = NULL;
	ofn.Flags = OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_LONGNAMES|OFN_PATHMUSTEXIST;
	ofn.lpstrDefExt = NULL;
	ofn.FlagsEx = 0;

	BOOL res = ::GetOpenFileName(&ofn);
	if (res == FALSE)
		return -1;
	return 0;
}

int PU::GetSaveFilename(TCHAR * buffer, int bufSize, HWND hOwner) {
	if (!buffer || bufSize == 0)
		return -1;

	if (!hOwner)
		hOwner = _MainOutputWindow;

	OPENFILENAME ofn;
	ofn.lStructSize = sizeof(ofn);	//not NT4.0 compatible
	ofn.hwndOwner = hOwner;
	ofn.lpstrFilter = NULL;	//accept everything
	ofn.lpstrCustomFilter = NULL;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = buffer;
	ofn.nMaxFile = bufSize;
	ofn.lpstrFileTitle = NULL;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle = NULL;
	ofn.Flags = OFN_HIDEREADONLY|OFN_LONGNAMES|OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt = NULL;
	ofn.FlagsEx = 0;

	BOOL res = ::GetSaveFileName(&ofn);
	if (res == FALSE)
		return -1;
	return 0;

	return 0;
}

int PU::BrowseDirectory(TCHAR * buffer, int bufSize, HWND hOwner) {
	if (bufSize < MAX_PATH)
		return -1;

	if (!hOwner)
		hOwner = _MainOutputWindow;
	//TODO: detect Vista+ and use IFileDialog
	BROWSEINFO bi;
	bi.hwndOwner = hOwner;
	bi.pidlRoot = NULL;	//desktop
	bi.pszDisplayName = buffer;
	bi.lpszTitle = TEXT("Please pick a location");
	bi.ulFlags = 0;
	bi.lpfn = NULL;
	bi.lParam = 0;
	bi.iImage = 0;

	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
	if (!pidl)
		return -1;

	BOOL bResult = SHGetPathFromIDList(pidl, buffer);
	CoTaskMemFree(pidl);

	if (bResult == FALSE)
		return -1;

	return 0;
}

int PU::SimplifyExternalPath(const char * path, const char * currentDir, char * buffer, int bufSize) {
	if (buffer == NULL || bufSize <= 1)
		return -1;

	if (currentDir == NULL && path[0] != '/')	//relative paths only supported if curDir is given
		return -1;

	int pathlen = strlen(path);
	int dirlen = strlen(currentDir);
	char * temp = new char[pathlen+dirlen+2];	//pathlen + '/' + dirlen + '\0'
	temp[0] = 0;
	if (path[0] != '/') {
		strcpy(temp, currentDir);
		strcat(temp, "/");
	}
	strcat(temp, path);

	std::vector<const char*> dirs;
	const char * name = strtok(temp,"/");
	while (name != NULL) {
		dirs.push_back(name);
		name = strtok(NULL, "/");
	}

	size_t size = dirs.size();
	size_t i = 0;
	while(i < size) {
		if (!strcmp(dirs[i], ".")) {
			dirs.erase(dirs.begin()+i);
			size--;
			continue;
		}
		if (!strcmp(dirs[i], "..")) {
			dirs.erase(dirs.begin()+i);
			size--;
			if (i > 0) {
				dirs.erase(dirs.begin()+i-1);
				size--;
				i--;
			}
			continue;
		}

		i++;
	}

	buffer[0] = '/';
	buffer[1] = 0;
	size_t totalLen = 1;
	i = 0;
	for(; i < dirs.size(); i++) {
		size_t len = strlen(dirs[i]);
		if (totalLen+len >= (size_t)bufSize) {
			delete [] temp;
			return -1;
		}
		strcat(buffer, dirs[i]);
		totalLen += len;
		if (i != dirs.size()-1) {
			strcat(buffer, "/");
			totalLen += 1;
		}
	}

	delete [] temp;

	return 0;
}

bool PU::IsValidLocalChar(const TCHAR localchar) {
	if (
		localchar == '\\' ||
		localchar == '/' ||	//path separators also considered invalid in this context
		localchar == '*' ||
		localchar == '?' ||
		localchar == '\"' ||
		localchar == '<' ||
		localchar == '>' ||
		localchar == '|' ||
		localchar == ':'
		)
		return false;

	return true;
}
