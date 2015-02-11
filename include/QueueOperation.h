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

#ifndef QUEUEOPERATION_H
#define QUEUEOPERATION_H

#include "FTPClientWrapper.h"
#include "Monitor.h"

class FTPQueue;

const int NotifyMessageMIN               = WM_USER + 500;

const unsigned int NotifyMessageStart    = WM_USER + 500;
const unsigned int NotifyMessageEnd      = WM_USER + 501;
const unsigned int NotifyMessageAdd      = WM_USER + 502;
const unsigned int NotifyMessageRemove   = WM_USER + 503;
const unsigned int NotifyMessageProgress = WM_USER + 504;

const unsigned int NotifyMessageMAX      = WM_USER + 504;

/*
Queue will delete/free data gathered during operations, but not given at constructor time
e.g. getdir will free FTPFile array, but not FileObject
*/

class QueueOperation {
	friend class FTPQueue;
	friend class FTPSession;
public:
	enum QueueType { QueueTypeConnect, QueueTypeDisconnect, QueueTypeDownload, QueueTypeUpload,
	                 QueueTypeDirectoryGet, QueueTypeDirectoryCreate, QueueTypeDirectoryRemove,
	                 QueueTypeFileCreate, QueueTypeFileDelete, QueueTypeFileRename, QueueTypeQuote,
	                 QueueTypeDownloadHandle, QueueTypeFileChmod
	               };

	enum QueueEvent { QueueEventStart=0x01, QueueEventEnd=0x02, QueueEventAdd=0x04, QueueEventRemove=0x08, QueueEventProgress=0x10 };
public:
							QueueOperation(QueueType type, HWND hNotify, int notifyCode = 0, void * notifyData = NULL);
	virtual					~QueueOperation();

	virtual int				Perform() = 0;
	virtual int				Terminate();

	virtual int				GetResult() const;
	virtual void*			GetNotifyData() const;
	virtual void*			GetData() const;
	virtual QueueType		GetType() const;

	virtual bool			GetRunning() const;
	virtual int				SetRunning(bool running);

	virtual int				SendNotification(QueueEvent event);
	virtual int				AckNotification();
	virtual int				ClearPendingNotifications();

	virtual int				SetProgress(float progress);
	virtual float			GetProgress() const;

	virtual bool			Equals(const QueueOperation & other);
protected:
	virtual int				SetClient(FTPClientWrapper* wrapper);

	QueueType				m_type;

	FTPClientWrapper*		m_client;

	HWND					m_hNotify;
	int						m_notifyCode;	//WPARAM
	void*					m_notifyData;

	bool					m_doConnect;
	bool					m_doDisconnect;

	int						m_result;
	void*					m_data;
	float					m_progress;	//0.0-100.0
	unsigned int			m_notifSent;

	bool					m_running;

	Monitor					m_ackMonitor;
	bool					m_terminating;
	DWORD					m_winThread;

};

class QueueConnect : public QueueOperation {
public:
							QueueConnect(HWND hNotify, int notifyCode = 0, void * notifyData = NULL);
	virtual					~QueueConnect();

	virtual int				Perform();

	virtual bool			Equals(const QueueOperation & other);
};

class QueueDisconnect : public QueueOperation {
public:
							QueueDisconnect(HWND hNotify, int notifyCode = 0, void * notifyData = NULL);
	virtual					~QueueDisconnect();

	virtual int				Perform();

	virtual bool			Equals(const QueueOperation & other);
};

/*
notifyCode: 0: Automatic location. 1: User specified location
*/
class QueueDownload : public QueueOperation {
public:
							QueueDownload(HWND hNotify, const char * externalFile, const TCHAR * localFile, Transfer_Mode tMode, int notifyCode = 0, void * notifyData = NULL);
	virtual					~QueueDownload();

	virtual int				Perform();

	virtual bool			Equals(const QueueOperation & other);

	virtual const TCHAR*	GetLocalPath();
	virtual const char*		GetExternalPath();
protected:
	char*					m_externalFile;
	TCHAR*					m_localFile;
	Transfer_Mode			m_tMode;
};

class QueueDownloadHandle : public QueueOperation {
public:
							QueueDownloadHandle(HWND hNotify, const char * externalFile, HANDLE hFile, Transfer_Mode tMode, int notifyCode = 0, void * notifyData = NULL);
	virtual					~QueueDownloadHandle();

	virtual int				Perform();

	virtual bool			Equals(const QueueOperation & other);

	virtual const TCHAR*	GetLocalPath();
	virtual const char*		GetExternalPath();
protected:
	char*					m_externalFile;
	HANDLE					m_hFile;
	Transfer_Mode			m_tMode;
};

/*
notifyCode: 0: Automatic upload. 1: User specified location. 2: User specified file+location
*/
class QueueUpload : public QueueOperation {
public:
							QueueUpload(HWND hNotify, const char * externalFile, const TCHAR * localFile, Transfer_Mode tMode, int notifyCode = 0, void * notifyData = NULL);
	virtual					~QueueUpload();

	virtual int				Perform();

	virtual bool			Equals(const QueueOperation & other);

	virtual const TCHAR*	GetLocalPath();
	virtual const char*		GetExternalPath();
protected:
	char*					m_externalFile;
	TCHAR*					m_localFile;
	Transfer_Mode			m_tMode;
};

class QueueGetDir : public QueueOperation {
public:
							QueueGetDir(HWND hNotify, const char * dirPath, int notifyCode = 0, void * notifyData = NULL);
							QueueGetDir(HWND hNotify, const char * dirPath, std::vector<char*> inputParentDirs, int notifyCode = 0, void * notifyData = NULL);
	virtual					~QueueGetDir();

	virtual int				Perform();

	virtual bool			Equals(const QueueOperation & other);

	virtual char*			GetDirPath();
	virtual int				GetFileCount();
	std::vector<FTPDir*>    GetParentDirObjs();

protected:
	char*					m_dirPath;
	int						m_fileCount;
	std::vector<char*>      parentDirs;
	std::vector<FTPDir*>    parentDirObjs;
};

class QueueCreateDir : public QueueOperation {
public:
							QueueCreateDir(HWND hNotify, const char * dirPath, int notifyCode = 0, void * notifyData = NULL);
	virtual					~QueueCreateDir();

	virtual int				Perform();

	virtual bool			Equals(const QueueOperation & other);

	virtual char*			GetDirPath();
protected:
	char*					m_dirPath;
};

class QueueRemoveDir : public QueueOperation {
public:
							QueueRemoveDir(HWND hNotify, const char * dirPath, int notifyCode = 0, void * notifyData = NULL);
	virtual					~QueueRemoveDir();

	virtual int				Perform();

	virtual bool			Equals(const QueueOperation & other);

	virtual char*			GetDirPath();
protected:
	char*					m_dirPath;
};

class QueueCreateFile : public QueueOperation {
public:
							QueueCreateFile(HWND hNotify, const char * filePath, int notifyCode = 0, void * notifyData = NULL);
	virtual					~QueueCreateFile();

	virtual int				Perform();

	virtual bool			Equals(const QueueOperation & other);

	virtual char*			GetFilePath();
protected:
	char*					m_filePath;
};

class QueueDeleteFile : public QueueOperation {
public:
							QueueDeleteFile(HWND hNotify, const char * filePath, int notifyCode = 0, void * notifyData = NULL);
	virtual					~QueueDeleteFile();

	virtual int				Perform();

	virtual bool			Equals(const QueueOperation & other);

	virtual char*			GetFilePath();
protected:
	char*					m_filePath;
};

class QueueRenameFile : public QueueOperation {
public:
							QueueRenameFile(HWND hNotify, const char * filePath, const char * newPath, int notifyCode = 0, void * notifyData = NULL);
	virtual					~QueueRenameFile();

	virtual int				Perform();

	virtual bool			Equals(const QueueOperation & other);

	virtual char*			GetFilePath();
	virtual char*			GetNewPath();
protected:
	char*					m_filePath;
	char*					m_newPath;
};

class QueueChmodFile : public QueueOperation {
public:
							QueueChmodFile(HWND hNotify, const char * filePath, const char * newMode, int notifyCode = 0, void * notifyData = NULL);
	virtual					~QueueChmodFile();

	virtual int				Perform();

	virtual bool			Equals(const QueueOperation & other);

	virtual char*			GetFilePath();
	virtual char*			GetNewMode();
protected:
	char*					m_filePath;
	char*					m_newMode;
};

//Requires SSL client wrapper
class QueueQuote : public QueueOperation {
public:
							QueueQuote(HWND hNotify, const char * quote, int notifyCode = 0, void * notifyData = NULL);
	virtual					~QueueQuote();

	virtual int				Perform();

	virtual bool			Equals(const QueueOperation & other);

	virtual char*			GetQuote();
protected:
	char*					m_quote;
};

#endif //QUEUEOPERATION_H
