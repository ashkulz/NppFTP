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

#ifndef FTPFILE_H
#define FTPFILE_H

enum FTPFileType { FTPTypeFile, FTPTypeDir, FTPTypeLink };

struct FTPFile {
	char 					filePath[MAX_PATH+1];	// full path to file (name included)
	long					fileSize;				// size of directory or file in bytes
	FILETIME				ctime;
	FILETIME				mtime;
	FILETIME				atime;
	FTPFileType				fileType;				// flag if the entry is directory, file or a link
};

struct FTPDir {
    struct FTPFile*         files;
    char*                   dirPath;	// dir path
    int                     count;
};

#endif //FTPFILE_H
