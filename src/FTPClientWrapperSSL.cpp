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
#include "FTPClientWrapper.h"

#include "SSLCertificates.h"
#include "MessageDialog.h"
#include <algorithm>

FTPClientWrapperSSL::FTPClientWrapperSSL(const char * host, int port, const char * user, const char * password) :
	FTPClientWrapper(Client_SSL, host, port, user, password),
	m_mode(CUT_FTPClient::FTP),
	m_ftpListParams(NULL)
{
	m_client.setsMode(m_mode);
}

FTPClientWrapperSSL::~FTPClientWrapperSSL()
{
	if (m_ftpListParams) {
		SU::free(m_ftpListParams);
		m_ftpListParams = NULL;
	}
}

FTPClientWrapper* FTPClientWrapperSSL::Clone() {
	FTPClientWrapperSSL* wrapper = new FTPClientWrapperSSL(m_hostname, m_port, m_username, m_password);
	wrapper->SetMode(m_mode);
	if (m_ftpListParams)
		wrapper->m_ftpListParams = SU::strdup(m_ftpListParams);
	wrapper->SetTimeout(m_timeout);
	wrapper->SetProgressMonitor(m_progmon);
	wrapper->SetCertificates(m_certificates);

	wrapper->m_client.SetFireWallMode(m_client.GetFireWallMode());
	wrapper->m_client.SetTransferType(m_client.GetTransferType());

	return wrapper;
}

int FTPClientWrapperSSL::SetProgressMonitor(ProgressMonitor * progmon) {
	int ret = FTPClientWrapper::SetProgressMonitor(progmon);
	ret = m_client.SetProgressMonitor(progmon);
	return ret;
}

int FTPClientWrapperSSL::SetCertificates(vX509 * x509Vect) {
	int ret = FTPClientWrapper::SetCertificates(x509Vect);
	ret = m_client.SetCertificates(x509Vect);
	return ret;
}

int FTPClientWrapperSSL::SetTimeout(int timeout) {
	int ret = FTPClientWrapper::SetTimeout(timeout);
	m_client.SetConnectTimeout(timeout);
	m_client.SetReceiveTimeOut(timeout*1000);
	m_client.SetSendTimeOut(timeout*1000);

	return ret;
}

int FTPClientWrapperSSL::Connect() {
	if (m_connected)
		return OnReturn(0);

	m_client.SetControlPort(m_port);
	int retcode = m_client.FTPConnect(m_hostname, m_username, m_password, "");
	if (retcode == UTE_SUCCESS)
		m_connected = true;

	return OnReturn((retcode == UTE_SUCCESS)?0:-1);
}

int FTPClientWrapperSSL::Disconnect() {
	if (!m_connected)
		return OnReturn(0);

	int retcode = m_client.Close();

	m_connected = false;	//just set to disconnected state, ignore errors

	return OnReturn((retcode == UTE_SUCCESS)?0:-1);
}

int FTPClientWrapperSSL::GetDir(const char * path, FTPFile** files) {
	int retcode = 0;
	CUT_DIRINFO di;
	FTPFile * ftpfiles;

	//store original directory
	//commented out: Cwd is not used in NppFTP at the moment
	//char curpath[MAX_PATH];
	//retcode = m_client.GetCurDir(curpath, MAX_PATH);
	//if (retcode != UTE_SUCCESS)
	//	return OnReturn(-1);

	// change the current working directory to the one specified
	retcode = m_client.ChDir(path);
	if (retcode != UTE_SUCCESS)
		return OnReturn(-1);

	if (strlen(m_ftpListParams) > 0)
		retcode = m_client.GetDirInfo(m_ftpListParams);//path);
	else
		retcode = m_client.GetDirInfo();//path);

	//return to original directory
	//commented out: Cwd is not used in NppFTP at the moment
	//m_client.ChDir(curpath);

	if (retcode != UTE_SUCCESS) {
		return OnReturn(-1);
	}

	bool endslash = path[strlen(path)-1] == '/';

	if (retcode != UTE_SUCCESS)
	{
		return OnReturn(-1);
	}

	int count = m_client.GetDirInfoCount();

	std::vector<FTPFile> vfiles;

	for(int i = 0; i < count; i++) {
		m_client.GetDirEntry(i,&di);

		if (!lstrcmp(TEXT("."), di.fileName) || !lstrcmp(TEXT(".."), di.fileName))
			continue;

		FTPFile ftpfile{};

		ftpfile.filePath[0] = 0;

		char * utf8name = SU::TCharToUtf8(di.fileName);
		char nameCpy[MAX_PATH+1];	//buffer used to handle symlinks

		strncpy(nameCpy, utf8name, MAX_PATH);
		nameCpy[MAX_PATH] = '\0';
		SU::FreeChar(utf8name);

		char * linkLocation = strstr(nameCpy, " -> ");
		if (linkLocation != NULL) {
			*linkLocation = 0;
		}

		strncpy(ftpfile.filePath, path, MAX_PATH);
		ftpfile.filePath[MAX_PATH] = '\0';
		if (!endslash) {
			strncat(ftpfile.filePath, "/", (MAX_PATH + 1) - strlen(ftpfile.filePath) - 1);
		}
		strncat(ftpfile.filePath, nameCpy, (MAX_PATH + 1) - strlen(ftpfile.filePath) - 1);
/*
		char * fullName = nameCpy;
		if (linkLocation != NULL) {
			fullName = linkLocation+4;	//find path link refers to, by skipping " -> " part
			char buffer[MAX_PATH];
			PU::SimplifyExternalPath(fullName, path, buffer, MAX_PATH);
			OutMsg("Link path: %s \\ %s to %s", path, fullName, buffer);
			strcpy(ftpfile.filePath, buffer);

		} else {
			strcpy(ftpfile.filePath, path);
			if (!endslash) {
				strcat(ftpfile.filePath, "/");
			}
			strcat(ftpfile.filePath, nameCpy);
		}
*/


		ftpfile.fileSize = (long)di.fileSize;

		FILETIME time = ConvertFiletime(di.day, di.month, di.year, di.hour, di.minute);
		ftpfile.atime = time;
		ftpfile.mtime = time;
		ftpfile.ctime = time;

		switch(di.isDir) {
			case TRUE:
				ftpfile.fileType = FTPTypeDir;
				break;
			case FALSE:
				ftpfile.fileType = FTPTypeFile;
				break;
			case 2:
				ftpfile.fileType = FTPTypeLink;
				break;
		}

		vfiles.push_back(ftpfile);
	}

	if (!vfiles.size()) {
		return OnReturn(-1);
	}

	ftpfiles = new FTPFile[vfiles.size()];
	memcpy(ftpfiles, &vfiles[0], sizeof(FTPFile)*vfiles.size());
	*files = ftpfiles;

	return OnReturn(vfiles.size());
}

int FTPClientWrapperSSL::Cwd(const char * path) {
	int retcode = m_client.ChDir(path);

	return OnReturn((retcode == UTE_SUCCESS)?0:-1);
}

int FTPClientWrapperSSL::Pwd(char* buf, size_t size) {
	int retcode = m_client.GetCurDir(buf, size);

	size_t len = strlen(buf);
	if (len > 1 && buf[len-1] == '/')
		buf[len-1] = 0;

	return OnReturn((retcode == UTE_SUCCESS)?0:-1);
}

int FTPClientWrapperSSL::Rename(const char * from, const char * to) {
	int retcode = m_client.RenameFile(from, to);

	return OnReturn((retcode == UTE_SUCCESS)?0:-1);
}

int FTPClientWrapperSSL::ChmodFile(const char * path, const char * mode) {
	int retcode = m_client.ChmodFile(path, mode);

	return OnReturn((retcode == UTE_SUCCESS)?0:-1);
}

int FTPClientWrapperSSL::MkDir(const char * path) {
	int retcode = m_client.MkDir(path);

	return OnReturn((retcode == UTE_SUCCESS)?0:-1);
}

int FTPClientWrapperSSL::RmDir(const char * path) {
	int retcode = m_client.RmDir(path);

	return OnReturn((retcode == UTE_SUCCESS)?0:-1);
}

int FTPClientWrapperSSL::MkFile(const char * path) {
	MemoryDataSource mdata(NULL, 0, false);

	int retcode = m_client.SendFile(mdata, path);

	return OnReturn((retcode == UTE_SUCCESS)?0:-1);
}

int FTPClientWrapperSSL::SendFile(const TCHAR * localfile, const char * ftpfile) {

	m_client.SetCurrentTotal(-1);
	HANDLE hFile = ::CreateFile(localfile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile != INVALID_HANDLE_VALUE) {
		DWORD lowsize = ::GetFileSize(hFile, NULL);

		m_client.SetCurrentTotal((long)lowsize);
		CloseHandle(hFile);
	}

	int retcode = m_client.SendFile(localfile, ftpfile);

	return OnReturn((retcode == UTE_SUCCESS)?0:-1);
}

int FTPClientWrapperSSL::ReceiveFile(const TCHAR * localfile, const char * ftpfile) {
	int res = PU::CreateLocalDirFile(localfile);
	if (res == -1)
		return -1;

	long size = 0;
	m_client.SetCurrentTotal(-1);
	int sizeres = m_client.GetSize(ftpfile, &size);
	if (sizeres == UTE_SUCCESS)
		m_client.SetCurrentTotal(size);

	int retcode = m_client.ReceiveFile(ftpfile, localfile);

	return OnReturn((retcode == UTE_SUCCESS)?0:-1);
}

int FTPClientWrapperSSL::SendFile(HANDLE hFile, const char * ftpfile) {

	m_client.SetCurrentTotal(-1);
	DWORD lowsize = ::GetFileSize(hFile, NULL);
	m_client.SetCurrentTotal((long)lowsize);

	HandleDataSource hds(hFile, true, false);
	int retcode = m_client.SendFile(hds, ftpfile);

	return OnReturn((retcode == UTE_SUCCESS)?0:-1);
}

int FTPClientWrapperSSL::ReceiveFile(HANDLE hFile, const char * ftpfile) {
	long size = 0;
	m_client.SetCurrentTotal(-1);
	int sizeres = m_client.GetSize(ftpfile, &size);
	if (sizeres == UTE_SUCCESS)
		m_client.SetCurrentTotal(size);

	HandleDataSource hds(hFile, false, true);
	int retcode = m_client.ReceiveFile(hds, ftpfile);

	return OnReturn((retcode == UTE_SUCCESS)?0:-1);
}

int	 FTPClientWrapperSSL::DeleteFile(const char * path) {
	int retcode = m_client.DeleteFile(path);

	return OnReturn((retcode == UTE_SUCCESS)?0:-1);
}

bool FTPClientWrapperSSL::IsConnected() {
	if (!m_connected)
		return false;

	bool clientconnected = m_client.IsConnected() == TRUE;
	if (!clientconnected) {
		Disconnect();	//thought to be connected, but not anymore, disconnect
		return false;
	}

	return true;
}

int FTPClientWrapperSSL::Abort() {
	int ret = FTPClientWrapper::Abort();
	if (ret == -1)
		return -1;

	m_client.SetAborted(TRUE);
	return 0;
}

int FTPClientWrapperSSL::OnReturn(int ret) {
	ret = FTPClientWrapper::OnReturn(ret);
	m_client.SetAborted(FALSE);

	//m_client.ClearResponseList();

	return ret;
}


int FTPClientWrapperSSL::SetMode(CUT_FTPClient::FTPSMode mode) {
	m_mode = mode;
	m_client.setsMode(m_mode);

	return 0;
}

int FTPClientWrapperSSL::SetConnectionMode(Connection_Mode cMode) {
	switch(cMode) {
		case Mode_Active: {
			m_client.SetFireWallMode(FALSE);
			break; }
		case Mode_Passive: {
			m_client.SetFireWallMode(TRUE);
			break; }
		case Mode_ConnectionMax:
		default:
			return -1;
	}

	return 0;
}

int FTPClientWrapperSSL::SetTransferMode(Transfer_Mode tMode) {
	if (!m_connected)	//requires active connection
		return -1;

	switch(tMode) {
		case Mode_ASCII: {
			m_client.SetTransferType(0);	//0=ASCII
			break; }
		case Mode_Binary: {
			m_client.SetTransferType(1);	//1=IMAGE
			break; }
		case Mode_TransferMax:
		default:
			return -1;
	}

	return 0;
}

int FTPClientWrapperSSL::SetPortRange(int min, int max) {
	m_client.SetDataPortRange(min, max);

	return 0;
}

int FTPClientWrapperSSL::SetListParams(const char * params) {
	if (m_ftpListParams)
		SU::free(m_ftpListParams);
	m_ftpListParams = SU::strdup(params);
	return 0;
}

int FTPClientWrapperSSL::Quote(const char * quote) {
	int retcode = m_client.Quote(quote);

	return (retcode == UTE_SUCCESS)?0:-1;
}

FILETIME FTPClientWrapperSSL::ConvertFiletime(int day, int month, int year, int hour, int minute) {
	FILETIME ft;
	SYSTEMTIME st{};
	st.wYear = year;
	st.wMonth = month;
	st.wDayOfWeek = 0;	//ignored
	st.wDay = day;
	st.wHour = hour;
	st.wMinute = minute;
	st.wSecond = 0;
	st.wMilliseconds = 0;

	SystemTimeToFileTime(&st, &ft);

	return ft;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

FtpSSLWrapper::FtpSSLWrapper() :
	CUT_FTPClient(),
	m_isAborted(FALSE),
	m_progmon(NULL),
	m_currentTotal(-1),
	m_certificates(NULL)
{
}

FtpSSLWrapper::~FtpSSLWrapper() {
}

int FtpSSLWrapper::Send(LPCSTR data, int len) {
	if (len == 0)
		return CUT_WSClient::Send(data, len);

	int datalen = len;
	if (datalen < 0)
		datalen = strlen(data);


	char * datacpy = new char[datalen+1];
	datacpy[datalen] = 0;
	memcpy(datacpy, data, datalen*sizeof(char));
	if (!strncmp(datacpy, "PASS", 4)) {
		delete [] datacpy;
		datacpy = new char[14];
		strcpy(datacpy, "PASS *HIDDEN*");
	}
	for(int i = 0; i < datalen; i++) {
		if (datacpy[i] == '\r' || datacpy[i] == '\n')
			datacpy[i] = ' ';
	}
	OutClnt("-> %T", SU::Utf8ToTChar(datacpy));
	delete [] datacpy;

	return CUT_WSClient::Send(data, len);
}

int FtpSSLWrapper::SetProgressMonitor(ProgressMonitor * progmon) {
	m_progmon = progmon;
	return 0;
}

int FtpSSLWrapper::SetAborted(BOOL aborted) {
	m_isAborted = aborted;
	return 0;
}

int FtpSSLWrapper::SetCurrentTotal(long total) {
	m_currentTotal = total;
	return 0;
}

int FtpSSLWrapper::SetCertificates(vX509 * x509Vect) {
	m_certificates = x509Vect;
	return 0;
}

int FtpSSLWrapper::GetResponseCode(CUT_WSClient *ws,LPSTR string,int maxlen) {
	int res = CUT_FTPClient::GetResponseCode(ws, string, maxlen);

	if (res == 0) {
		//OutErr("[FTP] No response from server");
		return res;
	}

	int index = 0;
	for(;; index++){
		const char * pbuf = GetMultiLineResponse(index);
		if(pbuf != NULL)
			OutClnt("%T", SU::Utf8ToTChar(pbuf));
		else
			break;
	}

	return res;
}

BOOL FtpSSLWrapper::ReceiveFileStatus(long bytesReceived) {
	BOOL res = CUT_FTPClient::ReceiveFileStatus(bytesReceived);
	if (res == FALSE)
		return res;

	if (m_progmon)
		m_progmon->OnDataReceived(bytesReceived, m_currentTotal);

	return res;
}

BOOL FtpSSLWrapper::SendFileStatus(long bytesSent) {
	BOOL res = CUT_FTPClient::SendFileStatus(bytesSent);
	if (res == FALSE)
		return res;

	if (m_progmon)
		m_progmon->OnDataSent(bytesSent, m_currentTotal);

	return res;
}

BOOL FtpSSLWrapper::IsAborted() {
	return m_isAborted;
}

int FtpSSLWrapper::OnLoadCertificates(SSL_CTX * ctx) {
	if (!m_certificates)
		return UTE_SUCCESS;

	 X509_STORE * x509Store = SSL_CTX_get_cert_store(ctx);
	 if (!x509Store)
		return UTE_SUCCESS;

	unsigned long err = 0;
	while ((err = ERR_get_error()) != 0) {}

	int size = (int)m_certificates->size();
	for(int i = 0; i < size; i++) {
		//int ret = SSL_CTX_use_certificate(ctx, (X509*)m_certificates->at(i));
		int ret = X509_STORE_add_cert(x509Store, (X509*)m_certificates->at(i));
		if (ret == 0) {
			err = ERR_get_error();
			OutErr("[FTPS] X509_STORE_add_cert failed (%d): %s.", err, ERR_reason_error_string(err));
			OutErr("[FTPS] Removing certificate from trusted list.", ret);
			size--;
			m_certificates->erase(m_certificates->begin()+i);
			i--;	//assuming i is signed
		}
	}
	return UTE_SUCCESS;
}

int FtpSSLWrapper::OnSSLCertificate(const SSL * ssl, const X509* certificate, int verifyResult) {
	if (certificate == NULL) {
		OutErr("[FTPS] No certificate presented by server, aborting connection.");
		return UTE_ERROR;
	}

	//since the CTX is setup before connections are made, it is possible a previously untrusted certificate becomes trusted but verify_cert
	//has no idea of that. This loop allows those certificates to 'pass' as well, as they would have anyway later on
	bool previouslyAccepted = false;
	int size = (int)m_certificates->size();
	for(int i = 0; i < size; i++) {
		if (!X509_cmp(certificate, (X509*)m_certificates->at(i))) {
			previouslyAccepted = true;
			break;
		}
	}

	if (verifyResult == X509_V_OK || previouslyAccepted) {
		OutMsg("[FTPS] Certificate valid.");
	} else {
		//X509_error_string(verifyResult);
		OutErr("[FTPS] Certificate invalid (%d): %s.", verifyResult, X509_verify_cert_error_string(verifyResult));

		//MessageDialog md;
		//int ret = md.Create(_MainOutputWindow, TEXT("FTP(E)S certificate verification"), TEXT("The certificate is unknown. Do you trust it?"));
		static const TCHAR * msgString =
				TEXT("The certificate is invalid because of the following reason:\r\n")
				TEXT("%s (code %d)\r\n")
				TEXT("Do you accept it anyway? ")
				TEXT("Please note that it may have other errors as well.");

		TCHAR * msgBuf = SU::TSprintfNB(msgString, X509_verify_cert_error_string(verifyResult), verifyResult);
		int ret = MessageBox(_MainOutputWindow, msgBuf, TEXT("FTP(E)S certificate verification"), MB_YESNO | MB_ICONWARNING);
		SU::FreeTChar(msgBuf);
		if (ret == IDYES) {
			OutMsg("[FTPS] Certificate accepted");

			if (m_certificates) {
				SSL_get_peer_certificate(ssl);	//increase reference counter
				m_certificates->push_back(certificate);
			}
		} else {
			OutMsg("[FTPS] Certificate rejected");
			return UTE_ERROR;
		}
	}
	return UTE_SUCCESS;
}

BOOL FtpSSLWrapper::IsConnected() {
	if (IsDataWaiting())
		PeekResponseCode(this);

	BOOL connected = CUT_WSClient::IsConnected();
	if (connected == FALSE)
		return FALSE;

	//also test if sending is possible

	fd_set writeSet{};
	struct timeval tv{};

	tv.tv_sec = 0;      // do not block - for polling
	tv.tv_usec = 0;

	FD_ZERO(&writeSet);  // always reinit

	FD_SET(m_socket,&writeSet);

	int rt1 = select(-1,NULL,&writeSet,NULL,&tv);

	if(rt1 == SOCKET_ERROR || rt1 == 0) {
		return FALSE;
	}

	return TRUE;
}


////////////////////////

MemoryDataSource::MemoryDataSource(char * data, int len, bool del) {
	m_data = data;
	m_length = len;
	m_delete = del;
	m_pointer = 0;
}

MemoryDataSource::~MemoryDataSource() {
	if (m_delete)
		delete [] m_data;
}

CUT_DataSource * MemoryDataSource::clone() {
	MemoryDataSource* copy = new MemoryDataSource(m_data, m_length, false);
	copy->m_pointer = 0;

	return copy;
}

int MemoryDataSource::Open(OpenMsgType type) {
	if (type != UTM_OM_READING)
		return -1;
	return UTE_SUCCESS;
}

int MemoryDataSource::Close() {
	return UTE_SUCCESS;
}

int MemoryDataSource::ReadLine(LPSTR /*buffer*/, size_t /*maxsize*/) {
	return UTE_ERROR;
}

int MemoryDataSource::WriteLine(LPCSTR /*buffer*/) {
	return UTE_ERROR;
}

int MemoryDataSource::Read(LPSTR buffer, size_t count) {
	size_t size = (std::min)(m_length-m_pointer, count);
	if (size == 0)
		return 0;

	memcpy(buffer+m_pointer, m_data, size);
	m_pointer += size;

	return size;
}

int MemoryDataSource::Write(LPCSTR /*buffer*/, size_t /*count*/) {
	return UTE_ERROR;
}

long MemoryDataSource::Seek(long offset, int origin) {
	size_t uoffset = (size_t)offset;
	switch(origin) {
		case SEEK_SET:
			if (offset < 0)
				return UTE_ERROR;
			if (uoffset < m_length) {
				m_pointer = uoffset;
				return UTE_SUCCESS;
			}
			break;
		case SEEK_CUR:
			if (offset < 0 || uoffset > m_length)
				return UTE_ERROR;
			m_pointer = uoffset;
			return UTE_SUCCESS;
			break;
		case SEEK_END:
			if (offset > 0 || (-offset) > (long)m_length)
				return UTE_ERROR;
			m_pointer = m_length-uoffset;
			return UTE_SUCCESS;
			break;
	}

	return UTE_ERROR;
}

//////////////////////////

HandleDataSource::HandleDataSource(HANDLE handle, bool read, bool write) :
	m_handle(handle),
	m_allowRead(read),
	m_allowWrite(write)
{
}

HandleDataSource::~HandleDataSource() {
}

// Virtual clone constructor
CUT_DataSource *HandleDataSource::clone() {
	return new HandleDataSource(m_handle, m_allowRead, m_allowWrite);
}

// Opens data file type == UTM_OM_READING, UTM_OM_WRITING, UTM_OM_APPEND
int HandleDataSource::Open(OpenMsgType type) {
	if (type == UTM_OM_READING && !m_allowRead)
		return -1;
	if (type == UTM_OM_WRITING && !m_allowWrite)
		return -1;
	if (type == UTM_OM_APPEND)
		return -1;

	return UTE_SUCCESS;
}

// Close message
int HandleDataSource::Close() {
	::CloseHandle(m_handle);
	return 0;
}

// Read one line
int HandleDataSource::ReadLine(LPSTR /*buffer*/, size_t /*maxsize*/) {
	return -1;
}

// Write one line
int HandleDataSource::WriteLine(LPCSTR /*buffer*/){
	return -1;
}

// Read data
int HandleDataSource::Read(LPSTR buffer, size_t count) {
	DWORD len = 0;
	BOOL res = ReadFile(m_handle, buffer, static_cast<DWORD>(count), &len, NULL);
	if (res == FALSE)
		return -1;

	return len;
}

// Write data
int HandleDataSource::Write(LPCSTR buffer, size_t count) {
	DWORD len = 0;
	BOOL res = WriteFile(m_handle, buffer, static_cast<DWORD>(count), &len, NULL);
	if (res == FALSE)
		return -1;

	return len;
}

// Move a current pointer to the specified location.
long HandleDataSource::Seek(long /*offset*/, int /*origin*/) {
	return -1;
}
