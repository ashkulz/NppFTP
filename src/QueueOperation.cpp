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
#include "QueueOperation.h"

const int QueueConditionAcked = 0;
const int QueueConditionCount = 1;

QueueOperation::QueueOperation(QueueType type, HWND hNotify, int notifyCode, void * notifyData) :
	m_type(type),
	m_client(NULL),
	m_hNotify(hNotify),
	m_notifyCode(notifyCode),
	m_notifyData(notifyData),
	m_doConnect(true),
	m_doDisconnect(false),
	m_result(-1),	//error by default
	m_data(NULL),
	m_progress(0.0f),
	m_notifSent(0),
	m_running(false),
	m_ackMonitor(QueueConditionCount),
	m_terminating(false)
{
	m_winThread = GetWindowThreadProcessId(m_hNotify, NULL);
}

QueueOperation::~QueueOperation() {
	Terminate();
}

int QueueOperation::Terminate() {
	m_ackMonitor.Enter();
		m_terminating = true;
		m_ackMonitor.Signal(QueueConditionAcked);
	m_ackMonitor.Exit();

	return 0;
}

int QueueOperation::GetResult() const {
	return m_result;
}

void* QueueOperation::GetData() const {
	return m_data;
}

void* QueueOperation::GetNotifyData() const {
	return m_notifyData;
}

QueueOperation::QueueType QueueOperation::GetType() const {
	return m_type;
}

int QueueOperation::SetRunning(bool running) {
	m_running = running;
	return 0;
}

bool QueueOperation::GetRunning() const {
	return m_running;
}

int QueueOperation::SendNotification(QueueEvent event) {
	UINT msg = 0;
	if (event != QueueEventProgress && (m_notifSent & event) != 0)
		return 0;		//do not send duplicate notifications, except for progress

	m_notifSent |= event;

	switch(event) {
		case QueueEventStart:
			msg = NotifyMessageStart;
			break;
		case QueueEventEnd:
			msg = NotifyMessageEnd;
			break;
		case QueueEventAdd:
			msg = NotifyMessageAdd;
			break;
		case QueueEventRemove:
			msg = NotifyMessageRemove;
			break;
		case QueueEventProgress:
			msg = NotifyMessageProgress;
			break;
		default:
			return -1;
			break;
	}

	DWORD curThread = GetCurrentThreadId();
	if (m_winThread == curThread) {
		::SendMessage(m_hNotify, msg, m_notifyCode, (LPARAM)this);
		return 0;
	}

	m_ackMonitor.Enter();
		if (m_terminating) {
			m_ackMonitor.Exit();
			return 0;
		}

		::PostMessage(m_hNotify, msg, m_notifyCode, (LPARAM)this);
		m_ackMonitor.Wait(QueueConditionAcked);
	m_ackMonitor.Exit();

	return 0;
}

int QueueOperation::AckNotification() {
	m_ackMonitor.Enter();
		m_ackMonitor.Signal(QueueConditionAcked);
	m_ackMonitor.Exit();
	return 0;
}

int QueueOperation::ClearPendingNotifications() {
	if (!m_hNotify)
		return -1;

	MSG msg;
	BOOL res = ::PeekMessage(&msg, m_hNotify, NotifyMessageMIN, NotifyMessageMAX, PM_REMOVE);
	while(res == TRUE) {
		res = ::PeekMessage(&msg, m_hNotify, NotifyMessageMIN, NotifyMessageMAX, PM_REMOVE);
	}

	return 0;
}

int QueueOperation::SetProgress(float progress) {
	m_progress = progress;
	return 0;
}

float QueueOperation::GetProgress() const {
	return m_progress;
}

bool QueueOperation::Equals(const QueueOperation & other) {
	if (other.GetType() != m_type)
		return false;
	if (other.m_client != m_client || other.m_data != m_data || other.m_hNotify != m_hNotify || other.m_notifyData != m_notifyData)
		return false;

	return true;	//ignore everything else
}

int QueueOperation::SetClient(FTPClientWrapper* wrapper) {
	m_client = wrapper;
	return 0;
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

QueueConnect::QueueConnect(HWND hNotify, int notifyCode, void * notifyData) :
	QueueOperation(QueueTypeConnect, hNotify, notifyCode, notifyData)
{
}

QueueConnect::~QueueConnect() {
}

int QueueConnect::Perform() {
	m_result = m_client->Connect();

	return m_result;
}

bool QueueConnect::Equals(const QueueOperation & other) {
	if (!QueueOperation::Equals(other))
		return false;
	//const QueueConnect & otherConnect = (QueueConnect&) other;

	return true;
}

//////////////////////////////////////

QueueDisconnect::QueueDisconnect(HWND hNotify, int notifyCode, void * notifyData) :
	QueueOperation(QueueTypeDisconnect, hNotify, notifyCode, notifyData)
{
}

QueueDisconnect::~QueueDisconnect() {
}

int QueueDisconnect::Perform() {
	m_result = m_client->Disconnect();

	return m_result;
}

bool QueueDisconnect::Equals(const QueueOperation & other) {
	if (!QueueOperation::Equals(other))
		return false;
	//const QueueDisconnect & otherDisconnect = (QueueDisconnect&) other;

	return true;
}

//////////////////////////////////////

QueueDownload::QueueDownload(HWND hNotify, const char * externalFile, const TCHAR * localFile, Transfer_Mode tMode, int notifyCode, void * notifyData) :
	QueueOperation(QueueTypeDownload, hNotify, notifyCode, notifyData),
	m_tMode(tMode)
{
	m_localFile = SU::DupString(localFile);
	m_externalFile = SU::strdup(externalFile);
}

QueueDownload::~QueueDownload() {
	SU::FreeTChar(m_localFile);
	free(m_externalFile);
}

int QueueDownload::Perform() {
	if (m_doConnect && !m_client->IsConnected()) {
		m_result = m_client->Connect();
		if (m_result == -1)
			return m_result;
		m_result = -1;
	}

	if (m_client->GetType() == Client_SSL) {
		((FTPClientWrapperSSL*)m_client)->SetTransferMode(m_tMode);
	}

	m_result = m_client->ReceiveFile(m_localFile, m_externalFile);
	return m_result;
}

bool QueueDownload::Equals(const QueueOperation & other) {
	if (!QueueOperation::Equals(other))
		return false;
	const QueueDownload & otherDld = (QueueDownload&) other;

	return (!lstrcmp(otherDld.m_localFile, m_localFile) && !strcmp(otherDld.m_externalFile, m_externalFile) && !m_running && !otherDld.m_running);
}

const TCHAR* QueueDownload::GetLocalPath() {
	return m_localFile;
}

const char* QueueDownload::GetExternalPath() {
	return m_externalFile;
}

//////////////////////////////////////

QueueUpload::QueueUpload(HWND hNotify, const char * externalFile, const TCHAR * localFile, Transfer_Mode tMode, int notifyCode, void * notifyData) :
	QueueOperation(QueueTypeUpload, hNotify, notifyCode, notifyData),
	m_tMode(tMode)
{
	m_localFile = SU::DupString(localFile);
	m_externalFile = SU::strdup(externalFile);
}

QueueUpload::~QueueUpload() {
	SU::FreeTChar(m_localFile);
	free(m_externalFile);
}

int QueueUpload::Perform() {
	if (m_doConnect && !m_client->IsConnected()) {
		m_result = m_client->Connect();
		if (m_result == -1)
			return m_result;
		m_result = -1;
	}

	if (m_client->GetType() == Client_SSL) {
		((FTPClientWrapperSSL*)m_client)->SetTransferMode(m_tMode);
	}

	m_result = m_client->SendFile(m_localFile, m_externalFile);
	return m_result;
}

bool QueueUpload::Equals(const QueueOperation & other) {
	if (!QueueOperation::Equals(other))
		return false;
	const QueueUpload & otherUld = (QueueUpload&) other;

	return (!lstrcmp(otherUld.m_localFile, m_localFile) && !strcmp(otherUld.m_externalFile, m_externalFile) && !m_running && !otherUld.m_running);
}

const TCHAR* QueueUpload::GetLocalPath() {
	return m_localFile;
}

const char* QueueUpload::GetExternalPath() {
	return m_externalFile;
}

//////////////////////////////////////

QueueGetDir::QueueGetDir(HWND hNotify, const char * dirPath, int notifyCode, void * notifyData) :
	QueueOperation(QueueTypeDirectoryGet, hNotify, notifyCode, notifyData),
	m_fileCount(0)
{
	m_dirPath = SU::strdup(dirPath);
}

QueueGetDir::~QueueGetDir() {
	if (m_data) {
		FTPFile* files = (FTPFile*)m_data;
		m_client->ReleaseDir(files, m_fileCount);
		m_data = NULL;
	}

	free(m_dirPath);
}

int QueueGetDir::Perform() {
	if (m_doConnect && !m_client->IsConnected()) {
		m_result = m_client->Connect();
		if (m_result == -1)
			return m_result;
		m_result = -1;
	}

	FTPFile* files;
	m_result = m_client->GetDir(m_dirPath, &files);

	if (m_result == -1)
		return m_result;

	m_data = files;
	m_fileCount = m_result;

	m_result = 0;	//filecount is returned

	return m_result;
}

bool QueueGetDir::Equals(const QueueOperation & other) {
	if (!QueueOperation::Equals(other))
		return false;
	const QueueGetDir & otherGet = (QueueGetDir&) other;

	return (!strcmp(otherGet.m_dirPath, m_dirPath));
}

char * QueueGetDir::GetDirPath() {
	return m_dirPath;
}

int QueueGetDir::GetFileCount() {
	return m_fileCount;
}

//////////////////////////////////////

QueueCreateDir::QueueCreateDir(HWND hNotify, const char * dirPath, int notifyCode, void * notifyData) :
	QueueOperation(QueueTypeDirectoryCreate, hNotify, notifyCode, notifyData)

{
	m_dirPath = SU::strdup(dirPath);
}

QueueCreateDir::~QueueCreateDir() {
	free(m_dirPath);
}

int QueueCreateDir::Perform() {
	if (m_doConnect && !m_client->IsConnected()) {
		m_result = m_client->Connect();
		if (m_result == -1)
			return m_result;
		m_result = -1;
	}

	m_result = m_client->MkDir(m_dirPath);
	return m_result;
}

bool QueueCreateDir::Equals(const QueueOperation & other) {
	if (!QueueOperation::Equals(other))
		return false;
	const QueueCreateDir & otherMkdir = (QueueCreateDir&) other;

	return (!strcmp(otherMkdir.m_dirPath, m_dirPath));
}

char * QueueCreateDir::GetDirPath() {
	return m_dirPath;
}

//////////////////////////////////////

QueueRemoveDir::QueueRemoveDir(HWND hNotify, const char * dirPath, int notifyCode, void * notifyData) :
	QueueOperation(QueueTypeDirectoryRemove, hNotify, notifyCode, notifyData)
{
	m_dirPath = SU::strdup(dirPath);
}

QueueRemoveDir::~QueueRemoveDir() {
	free(m_dirPath);
}

int QueueRemoveDir::Perform() {
	if (m_doConnect && !m_client->IsConnected()) {
		m_result = m_client->Connect();
		if (m_result == -1)
			return m_result;
		m_result = -1;
	}

	m_result = m_client->RmDir(m_dirPath);
	return m_result;
}

bool QueueRemoveDir::Equals(const QueueOperation & other) {
	if (!QueueOperation::Equals(other))
		return false;
	const QueueRemoveDir & otherRmdir = (QueueRemoveDir&) other;

	return (!strcmp(otherRmdir.m_dirPath, m_dirPath));
}

char * QueueRemoveDir::GetDirPath() {
	return m_dirPath;
}

//////////////////////////////////////

QueueCreateFile::QueueCreateFile(HWND hNotify, const char * filePath, int notifyCode, void * notifyData) :
	QueueOperation(QueueTypeFileCreate, hNotify, notifyCode, notifyData)
{
	m_filePath = SU::strdup(filePath);
}

QueueCreateFile::~QueueCreateFile() {
	free(m_filePath);
}

int QueueCreateFile::Perform() {
	if (m_doConnect && !m_client->IsConnected()) {
		m_result = m_client->Connect();
		if (m_result == -1)
			return m_result;
		m_result = -1;
	}

	m_result = m_client->MkFile(m_filePath);
	return m_result;
}

bool QueueCreateFile::Equals(const QueueOperation & other) {
	if (!QueueOperation::Equals(other))
		return false;
	const QueueCreateFile & otherMkfile = (QueueCreateFile&) other;

	return (!strcmp(otherMkfile.m_filePath, m_filePath));
}


char * QueueCreateFile::GetFilePath() {
	return m_filePath;
}

//////////////////////////////////////

QueueDeleteFile::QueueDeleteFile(HWND hNotify, const char * filePath, int notifyCode, void * notifyData) :
	QueueOperation(QueueTypeFileDelete, hNotify, notifyCode, notifyData)
{
	m_filePath = SU::strdup(filePath);
}

QueueDeleteFile::~QueueDeleteFile() {
	free(m_filePath);
}

int QueueDeleteFile::Perform() {
	if (m_doConnect && !m_client->IsConnected()) {
		m_result = m_client->Connect();
		if (m_result == -1)
			return m_result;
		m_result = -1;
	}

	m_result = m_client->DeleteFile(m_filePath);
	return m_result;
}

bool QueueDeleteFile::Equals(const QueueOperation & other) {
	if (!QueueOperation::Equals(other))
		return false;
	const QueueDeleteFile & otherRmfile = (QueueDeleteFile&) other;

	return (!strcmp(otherRmfile.m_filePath, m_filePath));
}

char * QueueDeleteFile::GetFilePath() {
	return m_filePath;
}

//////////////////////////////////////

QueueRenameFile::QueueRenameFile(HWND hNotify, const char * filePath, const char * newpath, int notifyCode, void * notifyData) :
	QueueOperation(QueueTypeFileRename, hNotify, notifyCode, notifyData)
{
	m_filePath = SU::strdup(filePath);
	m_newPath = SU::strdup(newpath);
}

QueueRenameFile::~QueueRenameFile() {
	free(m_filePath);
	free(m_newPath);
}

int QueueRenameFile::Perform() {
	if (m_doConnect && !m_client->IsConnected()) {
		m_result = m_client->Connect();
		if (m_result == -1)
			return m_result;
		m_result = -1;
	}

	m_result = m_client->Rename(m_filePath, m_newPath);

	return m_result;
}

bool QueueRenameFile::Equals(const QueueOperation & other) {
	if (!QueueOperation::Equals(other))
		return false;
	const QueueRenameFile & otherRename = (QueueRenameFile&) other;

	return (!strcmp(otherRename.m_filePath, m_filePath) && !strcmp(otherRename.m_newPath, m_newPath));
}

char * QueueRenameFile::GetFilePath() {
	return m_filePath;
}

char * QueueRenameFile::GetNewPath() {
	return m_newPath;
}

//////////////////////////////////////

//Requires SSL client wrapper
QueueQuote::QueueQuote(HWND hNotify, const char * quote, int notifyCode, void * notifyData) :
	QueueOperation(QueueTypeQuote, hNotify, notifyCode, notifyData)
{
	m_quote = SU::strdup(quote);
}

QueueQuote::~QueueQuote() {
	free(m_quote);
}

int QueueQuote::Perform() {
	if (m_doConnect && !m_client->IsConnected()) {
		m_result = m_client->Connect();
		if (m_result == -1)
			return m_result;
		m_result = -1;
	}

	FTPClientWrapperSSL* client = (FTPClientWrapperSSL*)m_client;
	m_result = client->Quote(m_quote);
	return m_result;
}

bool QueueQuote::Equals(const QueueOperation & other) {
	if (!QueueOperation::Equals(other))
		return false;
	const QueueQuote & otherQuote = (QueueQuote&) other;

	return (!strcmp(otherQuote.m_quote, m_quote));
}

char * QueueQuote::GetQuote() {
	return m_quote;
}
