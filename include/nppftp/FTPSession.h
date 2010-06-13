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

#ifndef FTPSESSION_H
#define FTPSESSION_H

#include "FTPProfile.h"
#include "FTPCache.h"
#include "FTPQueue.h"
#include "SSLCertificates.h"

class FTPWindow;

class FTPSession {
public:
							FTPSession();
							~FTPSession();

	int						Init(FTPWindow * ftpWindow, FTPCache * cache);
	int						Deinit();

	int						SetCertificates(vX509 * x509Vect);

	int						StartSession(FTPProfile * sessionProfile);
	int						TerminateSession();

	bool					IsConnected();
	const FTPProfile*		GetCurrentProfile();

	int						Connect();
	int						GetDirectory(const char * dir);

	int						DownloadFileCache(const char * sourcefile);	//return 0 on download, -1 on error, 1 when no cache match was found
	int						DownloadFile(const char * sourcefile, const TCHAR * target, bool targetIsDir, int code = 1);
	int						DownloadFileHandle(const char * sourcefile, HANDLE target);

	int						UploadFileCache(const TCHAR * sourcefile);	//return 0 on upload, -1 on error, 1 when no cache match was found
	int						UploadFile(const TCHAR * sourcefile, const char * target, bool targetIsDir, int code = 1);

	int						MkDir(const char * path);
	int						RmDir(const char * path);
	int						MkFile(const char * path);
	int						DeleteFile(const char * path);
	int						Rename(const char * oldpath, const char * newpath);

	FileObject*				GetRootObject();
	FileObject*				FindPathObject(const char * filepath);

	int						AbortOperation();
	int						AbortTransfer();
	int						CancelOperation(QueueOperation * cancelOp);
private:
	int						Clear();

	FTPProfile*				m_currentProfile;
	FTPCache*				m_ftpGlobalCache;

	FTPClientWrapper*		m_mainWrapper;
	FTPClientWrapper*		m_transferWrapper;

	FTPQueue*				m_mainQueue;		//file/directory operations
	FTPQueue*				m_transferQueue;	//file transfers

	bool					m_running;

	HWND					m_hNotify;
	FTPWindow*				m_ftpWindow;

	bool					m_isInit;

	FileObject*				m_rootObject;

	vX509*					m_certificates;
};

#endif //FTPSESSION_H
