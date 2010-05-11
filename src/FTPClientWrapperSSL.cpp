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

FTPClientWrapperSSL::FTPClientWrapperSSL(const char * host, int port, const char * user, const char * password) :
	FTPClientWrapper(Client_SSL, host, port, user, password),
	m_mode(CUT_FTPClient::FTP)
{
	m_client.setsMode(m_mode);
}

FTPClientWrapperSSL::~FTPClientWrapperSSL()
{
}

FTPClientWrapper* FTPClientWrapperSSL::Clone() {
	FTPClientWrapperSSL* wrapper = new FTPClientWrapperSSL(m_hostname, m_port, m_username, m_password);
	wrapper->SetMode(m_mode);
	wrapper->SetTimeout(m_timeout);
	wrapper->SetProgressMonitor(m_progmon);
	wrapper->SetCertificates(m_certificates);

	wrapper->m_client.SetFireWallMode(wrapper->m_client.GetFireWallMode());
	wrapper->m_client.SetTransferType(wrapper->m_client.GetTransferType());

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
/*
	char curpath[MAX_PATH];
	retcode = m_client.GetCurDir(curpath, MAX_PATH);
	if (retcode != UTE_SUCCESS)
		return OnReturn(-1);

	// change the current working directory to the one specified
	retcode = m_client.ChDir(path);
	if (retcode != UTE_SUCCESS)
		return OnReturn(-1);
*/
	retcode = m_client.GetDirInfo(path);
/*
	m_client.ChDir(curpath);
*/
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

		FTPFile ftpfile;

		ftpfile.filePath[0] = 0;
		ftpfile.fileName[0] = 0;

		strcpy(ftpfile.filePath, path);
		if (!endslash) {
			strcat(ftpfile.filePath, "/");
		}

		char * utf8name = SU::TCharToUtf8(di.fileName);
		strcat(ftpfile.filePath, utf8name);
		strcpy(ftpfile.fileName, utf8name);
		SU::FreeUtf8(utf8name);
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

	m_client.SetCurrentTotal(-1);
	FTPFile * files = NULL;
	int count = GetDir(ftpfile, &files);
	if (count > 0) {
		long size = files[0].fileSize;
		m_client.SetCurrentTotal(size);
	}
	ReleaseDir(files, count);

	int retcode = m_client.ReceiveFile(ftpfile, localfile);

	return OnReturn((retcode == UTE_SUCCESS)?0:-1);
}

int	 FTPClientWrapperSSL::DeleteFile(const char * path) {
	int retcode = m_client.DeleteFile(path);

	return OnReturn((retcode == UTE_SUCCESS)?0:-1);
}

bool FTPClientWrapperSSL::IsConnected() {
	if (!m_connected)
		return false;

	bool clientconnected = m_client.IsConnected();
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

	m_client.ClearResponseList();

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


int FTPClientWrapperSSL::Quote(const char * quote) {
	int retcode = m_client.Quote(quote);

	return (retcode == UTE_SUCCESS)?0:-1;
}

FILETIME FTPClientWrapperSSL::ConvertFiletime(int day, int month, int year, int hour, int minute) {
	FILETIME ft;
	SYSTEMTIME st;
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
	for(int i = 0; i < datalen; i++) {
		if (datacpy[i] == '\r' || datacpy[i] == '\n')
			datacpy[i] = ' ';
	}
	OutClnt("-> %s", datacpy);
	delete datacpy;

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

int FtpSSLWrapper::ClearResponseList() {
	m_listResponse.ClearList();
	return 0;
}

int FtpSSLWrapper::SetCertificates(vX509 * x509Vect) {
	m_certificates = x509Vect;
	return 0;
}

int FtpSSLWrapper::GetResponseCode(CUT_WSClient *ws,LPSTR string,int maxlen) {
	int res = CUT_FTPClient::GetResponseCode(ws, string, maxlen);

	int index = 0;
	for(;;){
		const char * pbuf = GetMultiLineResponse(index);
		index++;
		if(pbuf != NULL)
			OutClnt("%s", pbuf);
		else
			break;
	}

	ClearResponseList();

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
		static const TCHAR * msgString = TEXT(
				"The certificate is invalid because of the following reason:\r\n"
				"%s (code %d)\r\n"
				"Do you accept it anyway? "
				"Please note that it may have other errors as well." );

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
		return UTE_ERROR;
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
	size_t size = std::min(m_length-m_pointer, count);
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
