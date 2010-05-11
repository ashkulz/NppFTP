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
#include "FTPSession.h"

#include "FTPWindow.h"

FTPSession::FTPSession() :
	m_currentProfile(NULL),
	m_ftpGlobalCache(NULL),

	m_mainWrapper(NULL),
	m_transferWrapper(NULL),

	m_mainQueue(NULL),
	m_transferQueue(NULL),

	m_running(false),

	m_hNotify(NULL),
	m_ftpWindow(NULL),
	m_isInit(false),

	m_rootObject(NULL),

	m_certificates(NULL)
{
}

FTPSession::~FTPSession() {
	if (m_running)
		TerminateSession();

	Clear();
}

int FTPSession::Init(FTPWindow * ftpWindow, FTPCache * cache) {
	if (m_isInit)
		return -1;

	m_ftpWindow = ftpWindow;
	m_ftpGlobalCache = cache;
	m_hNotify = m_ftpWindow->GetHWND();
	m_isInit = true;

	return 0;
}

int FTPSession::Deinit() {
	if (!m_isInit)
		return 0;

	m_ftpWindow = NULL;
	m_hNotify = NULL;

	m_isInit = false;

	return 0;
}

int FTPSession::SetCertificates(vX509 * x509Vect) {
	m_certificates = x509Vect;
	return 0;
}

int FTPSession::StartSession(FTPProfile * sessionProfile) {
	if (m_running)
		return -1;

	m_currentProfile = sessionProfile;
	m_currentProfile->AddRef();

	m_ftpGlobalCache->SetEnvironment(m_currentProfile->GetHostname(), m_currentProfile->GetUsername());

	m_mainWrapper = m_currentProfile->CreateWrapper();
	m_mainWrapper->SetCertificates(m_certificates);
	m_transferWrapper = m_mainWrapper->Clone();

	m_mainQueue = new FTPQueue(m_mainWrapper);
	m_transferQueue = new FTPQueue(m_transferWrapper);

	m_mainQueue->Initialize();
	m_transferQueue->Initialize();

	m_rootObject = new FileObject("/", true, false);
	m_rootObject->SetParent(m_rootObject);

	m_running = true;

	return 0;
}

int FTPSession::TerminateSession() {
	if (!m_running)
		return 0;

	if (m_transferQueue->GetQueueSize() > 0) {
		int ret = ::MessageBox(_MainOutputWindow, TEXT("There are still transfers running, do you want to close the connection?"), TEXT("Closing connection"), MB_YESNO);
		if (ret != IDYES)
			return -1;
	}

	m_running = false;

	Clear();
	if (m_currentProfile)
		m_currentProfile->Release();
	m_currentProfile = NULL;

	delete m_rootObject;
	m_rootObject = NULL;

	return 0;
}

bool FTPSession::IsConnected() {
	return m_running;
}

int FTPSession::Connect() {
	if (!m_running)
		return -1;

	QueueConnect * connop = new QueueConnect(m_hNotify, 0);
	m_mainQueue->AddQueueOp(connop);

	return 0;
}

int FTPSession::GetDirectory(const char * dir) {
	if (!m_running)
		return -1;

	QueueGetDir * dirop = new QueueGetDir(m_hNotify, dir);

	m_mainQueue->AddQueueOp(dirop);

	return 0;
}

int FTPSession::DownloadFileCache(const char * sourcefile) {
	if (!m_running)
		return -1;

	if (sourcefile == NULL)
		return -1;

	TCHAR target[MAX_PATH];
	target[0] = 0;

	int res = m_currentProfile->GetCacheLocal(sourcefile, target, MAX_PATH);
	if (res != 0)
		return res;

	return DownloadFile(sourcefile, target, false, 0);
}

int FTPSession::DownloadFile(const char * sourcefile, const TCHAR * target, bool targetIsDir, int code) {
	if (!m_running)
		return -1;

	if (sourcefile == NULL || target == NULL)
		return -1;

	TCHAR * sourcenamelocal = SU::Utf8ToTChar(PU::FindExternalFilename(sourcefile));

	TCHAR * targetfile;
	if (targetIsDir) {
		targetfile = new TCHAR[MAX_PATH];
		PU::ConcatLocal(target, sourcenamelocal, targetfile, MAX_PATH);
	} else {
		targetfile = (TCHAR*)target;
	}

	Transfer_Mode tMode = m_currentProfile->GetFileTransferMode(sourcenamelocal);
	SU::FreeTChar(sourcenamelocal);

	QueueDownload * dldop = new QueueDownload(m_hNotify, sourcefile, targetfile, tMode, code);
	m_transferQueue->AddQueueOp(dldop);

	if (targetIsDir) {
		delete [] targetfile;
	}

	return 0;
}

int FTPSession::UploadFileCache(const TCHAR * sourcefile) {
	if (!m_running)
		return -1;

	if (sourcefile == NULL)
		return -1;

	char target[MAX_PATH];
	target[0] = 0;

	int res = m_currentProfile->GetCacheExternal(sourcefile, target, MAX_PATH);
	if (res != 0)
		return res;

	return UploadFile(sourcefile, target, false, 0);
}

int FTPSession::UploadFile(const TCHAR * sourcefile, const char * target, bool targetIsDir, int code) {
	if (!m_running)
		return -1;

	if (sourcefile == NULL || target == NULL)
		return -1;

	const TCHAR * sourcenamelocal = PU::FindLocalFilename(sourcefile);

	char * targetfile;
	if (targetIsDir) {
		targetfile = new char[MAX_PATH];
		PU::ConcatLocalToExternal(target, sourcenamelocal, targetfile, MAX_PATH);
	} else {
		targetfile = (char*)target;
	}

	Transfer_Mode tMode = m_currentProfile->GetFileTransferMode(sourcenamelocal);
	QueueUpload * uldop = new QueueUpload(m_hNotify, targetfile, sourcefile, tMode, code);
	m_transferQueue->AddQueueOp(uldop);

	if (targetIsDir) {
		delete [] targetfile;
	}

	return 0;
}

int FTPSession::MkDir(const char * path) {
	if (!m_running)
		return -1;

	QueueCreateDir * dirop = new QueueCreateDir(m_hNotify, path);

	m_mainQueue->AddQueueOp(dirop);

	return 0;
}

int FTPSession::RmDir(const char * path) {
	if (!m_running)
		return -1;

	QueueRemoveDir * dirop = new QueueRemoveDir(m_hNotify, path);

	m_mainQueue->AddQueueOp(dirop);

	return 0;
}

int FTPSession::MkFile(const char * path) {
	if (!m_running)
		return -1;

	QueueCreateFile * fileop = new QueueCreateFile(m_hNotify, path);

	m_mainQueue->AddQueueOp(fileop);

	return 0;
}

int FTPSession::DeleteFile(const char * path) {
	if (!m_running)
		return -1;

	QueueDeleteFile * fileop = new QueueDeleteFile(m_hNotify, path);

	m_mainQueue->AddQueueOp(fileop);

	return 0;
}

int FTPSession::Rename(const char * oldpath, const char * newpath) {
	if (!m_running)
		return -1;

	QueueRenameFile * fileop = new QueueRenameFile(m_hNotify, oldpath, newpath);

	m_mainQueue->AddQueueOp(fileop);

	return 0;
}

FileObject* FTPSession::GetRootObject() {
	char dir[MAX_PATH];
	strcpy(dir, m_currentProfile->GetInitialDir());

	int res = 0;

	if (strlen(dir) < 1 || dir[0] != '/') {	//no initial dir or invalid
		res = -1;
	} else {
		res = m_mainWrapper->Cwd(dir);
	}

	if (res == -1) {
		res = m_mainWrapper->Pwd(&dir[0], MAX_PATH);
	}

	if (res == -1 || strlen(dir) <= 1)
		return m_rootObject;

	FileObject * child = NULL;
	child = new FileObject(dir, true, false);
	FileObject * prevDir = NULL;
	prevDir = child;
	char * curDir;
	curDir = strrchr(dir, '/');
	while(curDir != NULL) {
		if (curDir == dir) {
			child = m_rootObject;
			child->AddChild(prevDir);
			child->SetRefresh(false);
			break;
		}

		*curDir = 0;
		child = new FileObject(dir, true, false);
		child->AddChild(prevDir);
		child->SetRefresh(false);
		prevDir = child;

		curDir = strrchr(dir, '/');
	}

	return m_rootObject;
}

FileObject* FTPSession::FindPathObject(const char * filepath) {
	if (!filepath)
		return NULL;

	char tempstr[MAX_PATH];
	strcpy(tempstr, filepath);

	FileObject * current = m_rootObject;

	char * curname = strtok (tempstr, "/");

	while(curname) {
		if (current->GetChildCount() == 0)	//search did not finish, but cannot find child
			return NULL;

		int i = 0;
		int count = current->GetChildCount();
		for(i = 0; i < count; i++) {
			if ( !strcmp( current->GetChild(i)->GetName(), curname ) ) {
				current = current->GetChild(i);
				curname = strtok (NULL, "/");
				break;
			}
		}

		if (i == count)	//none of the children match
			return NULL;
	}

	return current;
}

int FTPSession::AbortOperation() {
	return m_mainWrapper->Abort();
}

int FTPSession::AbortTransfer() {
	return m_transferWrapper->Abort();
}

int FTPSession::CancelOperation(QueueOperation * cancelOp) {
	return m_transferQueue->CancelQueueOp(cancelOp);
}

int FTPSession::Clear() {
	if (m_mainQueue)
		m_mainQueue->ClearQueue();
	if (m_transferQueue)
		m_transferQueue->ClearQueue();

	if (m_transferWrapper) {
		m_transferWrapper->Abort();
	}
	if (m_mainWrapper) {
		m_mainWrapper->Abort();
	}

	if (m_transferQueue) {
		m_transferQueue->Deinitialize();
		delete m_transferQueue;
		m_transferQueue = NULL;
	}
	if (m_mainQueue) {
		m_mainQueue->Deinitialize();
		delete m_mainQueue;
		m_mainQueue = NULL;
	}

	QueueDisconnect * opdisc = new QueueDisconnect(m_hNotify);

	if (m_transferWrapper) {
		if (m_transferWrapper->IsConnected()) {
			opdisc->SetClient(m_transferWrapper);
			opdisc->SendNotification(QueueOperation::QueueEventStart);
			opdisc->Perform();
			opdisc->SendNotification(QueueOperation::QueueEventEnd);
		}
		delete m_transferWrapper;
		m_transferWrapper = NULL;
	}

	if (m_mainWrapper) {
		if (m_mainWrapper->IsConnected()) {
			opdisc->SetClient(m_mainWrapper);
			opdisc->SendNotification(QueueOperation::QueueEventStart);
			opdisc->Perform();
			opdisc->SendNotification(QueueOperation::QueueEventEnd);
		}
		delete m_mainWrapper;
		m_mainWrapper = NULL;
	}

	delete opdisc;

	return 0;
}
