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

#ifndef FTPCLIENTWRAPPER_H
#define FTPCLIENTWRAPPER_H

#include "ftp_c.h"
#include "ProgressMonitor.h"
#include <libssh/libssh.h>
#include <libssh/sftp.h>
#include "FTPFile.h"
#include "SSLCertificates.h"

enum Client_Type { Client_SSL, Client_SSH };

enum Security_Mode {Mode_FTP = 0, Mode_FTPES = 1, Mode_FTPS = 2, Mode_SFTP = 3, Mode_SecurityMax = 4};
enum Connection_Mode {Mode_Passive = 0, Mode_Active = 1, Mode_ConnectionMax = 2};
enum Transfer_Mode {Mode_Binary = 0, Mode_ASCII = 1, Mode_TransferMax = 2};
enum AuthenticationMethods {Method_Password=0x01, Method_Key=0x02, Method_Interactive=0x04, Method_All=0x07};

class FtpSSLWrapper : public CUT_FTPClient {
public:
							FtpSSLWrapper();
	virtual					~FtpSSLWrapper();

	virtual int				Send(LPCSTR data, int len);

	virtual	int				SetProgressMonitor(ProgressMonitor * progmon);
	virtual int				SetAborted(BOOL aborted);
	virtual int				SetCurrentTotal(long total);

	virtual int				ClearResponseList();

	virtual int				SetCertificates(vX509 * x509Vect);
protected:
	virtual int				GetResponseCode(CUT_WSClient *ws,LPSTR string = NULL,int maxlen = 0);

	virtual BOOL			ReceiveFileStatus(long bytesReceived);

	// Monitor progress and/or cancel the send
	virtual BOOL			SendFileStatus(long bytesSent);

	virtual BOOL			IsAborted();

	virtual int				OnLoadCertificates(SSL_CTX * ctx);
	virtual int				OnSSLCertificate(const SSL * ssl, const X509* certificate, int verifyResult);

	BOOL					m_isAborted;
	ProgressMonitor*		m_progmon;
	long					m_currentTotal;	//kinda hacky
	vX509*					m_certificates;
};

class FTPClientWrapper {
public:
							FTPClientWrapper(Client_Type type, const char * host, int port, const char * user, const char * password);
	virtual					~FTPClientWrapper();

	virtual FTPClientWrapper*	Clone() = 0;	//Copy settings, but not connection status or anything

	virtual Client_Type		GetType();

	virtual int				SetProgressMonitor(ProgressMonitor * progmon);
	virtual int				SetTimeout(int timeout);
	virtual int				SetCertificates(vX509 * x509Vect);

	virtual int				Connect() = 0;
	virtual int				Disconnect() = 0;

	//Don't forget to call releasedir
	virtual int				GetDir(const char * path, FTPFile** files) = 0;
	static int				ReleaseDir(FTPFile* files, int size);

	virtual int				Cwd(const char * path) = 0;
	virtual int				Pwd(char* buf, size_t size) = 0;	//Guarantee no trailing slash (unless root)

	//Modifying operations
	virtual int				Rename(const char * from, const char * to) = 0;

	virtual int				MkDir(const char * path) = 0;
	virtual int				RmDir(const char * path) = 0;

	virtual int				MkFile(const char * path) = 0;
	virtual int				SendFile(const TCHAR * localfile, const char * ftpfile) = 0;
	virtual int				ReceiveFile(const TCHAR * localfile, const char * ftpfile) = 0;
	virtual int				SendFile(HANDLE hFile, const char * ftpfile) = 0;
	virtual int				ReceiveFile(HANDLE hFile, const char * ftpfile) = 0;
	virtual int				DeleteFile(const char * path) = 0;

	virtual bool			IsConnected();
	virtual int				Abort();
protected:
	virtual int				OnReturn(int res);	//for use with time consuming operations

	Client_Type				m_type;

	bool					m_connected;

	char *					m_hostname;
	int						m_port;
	char *					m_username;
	char *					m_password;

	bool					m_aborting;	//since assignment to bools is pretty much atomic, no synchronization will be used.
	bool					m_busy;

	int						m_timeout;
	ProgressMonitor*		m_progmon;
	vX509*					m_certificates;
};

class FTPClientWrapperSSH : public FTPClientWrapper {
public:
							FTPClientWrapperSSH(const char * host, int port, const char * user, const char * password);
	virtual					~FTPClientWrapperSSH();

	virtual FTPClientWrapper*	Clone();

	virtual int				Connect();
	virtual int				Disconnect();

	virtual int				GetDir(const char * path, FTPFile** files);

	virtual int				Cwd(const char * path);
	virtual int				Pwd(char* buf, size_t size);

	//Modifying operations
	virtual int				Rename(const char * from, const char * to);

	virtual int				MkDir(const char * path);
	virtual int				RmDir(const char * path);

	virtual int				MkFile(const char * path);
	virtual int				SendFile(const TCHAR * localfile, const char * ftpfile);
	virtual int				ReceiveFile(const TCHAR * localfile, const char * ftpfile);
	virtual int				SendFile(HANDLE hFile, const char * ftpfile);
	virtual int				ReceiveFile(HANDLE hFile, const char * ftpfile);
	virtual int				DeleteFile(const char * path);

	virtual bool			IsConnected();

	//Class specific operations
	virtual int				SetKeyFile(const TCHAR * keyFile);
	virtual int				SetPassphrase(const char * passphrase);
	virtual int				SetUseAgent(bool useAgent);
	virtual int				SetAcceptedMethods(AuthenticationMethods acceptedMethods);
protected:
	ssh_session				m_sshsession;
	sftp_session			m_sftpsession;

	int						connect_ssh();
	int 					authenticate(ssh_session session);
	int 					authenticate_key(ssh_session session);
	int 					authenticate_password(ssh_session session);
	int 					authenticate_kbinteractive(ssh_session session);
	int						verify_knownhost(ssh_session session);
	int						disconnect();

	HANDLE					OpenFile(const TCHAR* file, bool write);
	FILETIME				ConvertFiletime(uint32_t nTime, uint32_t nNanosecs);

	TCHAR*					m_keyFile;
	char*					m_passphrase;
	bool					m_useAgent;
	unsigned int			m_acceptedMethods;
};

class FTPClientWrapperSSL : public FTPClientWrapper {
public:
							FTPClientWrapperSSL(const char * host, int port, const char * user, const char * password);
	virtual					~FTPClientWrapperSSL();

	virtual FTPClientWrapper*	Clone();

	virtual int				SetProgressMonitor(ProgressMonitor * progmon);
	virtual int				SetTimeout(int timeout);
	virtual int				SetCertificates(vX509 * x509Vect);

	virtual int				Connect();
	virtual int				Disconnect();

	virtual int				GetDir(const char * path, FTPFile** files);

	virtual int				Cwd(const char * path);
	virtual int				Pwd(char* buf, size_t size);

	//Modifying operations
	virtual int				Rename(const char * from, const char * to);

	virtual int				MkDir(const char * path);
	virtual int				RmDir(const char * path);

	virtual int				MkFile(const char * path);
	virtual int				SendFile(const TCHAR * localfile, const char * ftpfile);
	virtual int				ReceiveFile(const TCHAR * localfile, const char * ftpfile);
	virtual int				SendFile(HANDLE hFile, const char * ftpfile);
	virtual int				ReceiveFile(HANDLE hFile, const char * ftpfile);
	virtual int				DeleteFile(const char * path);

	virtual bool			IsConnected();
	virtual int				Abort();
	virtual int				OnReturn(int ret);

	//Class specific operations
	virtual int				SetMode(CUT_FTPClient::FTPSMode mode);
	virtual int				SetConnectionMode(Connection_Mode cMode);
	virtual int				SetTransferMode(Transfer_Mode tMode);

	virtual int				Quote(const char * quote);
protected:
	FtpSSLWrapper			m_client;
	CUT_FTPClient::FTPSMode	m_mode;

	FILETIME				ConvertFiletime(int day, int month, int year, int hour, int minute);
};

/////////////////////////////////////////////////////////
/////Class extending some classes from CUT
/////////////////////////////////////////////////////////

class MemoryDataSource : public CUT_DataSource {
protected:
	char *					m_data;
	size_t					m_length;
	bool					m_delete;
	size_t					m_pointer;
public:
							MemoryDataSource(char * data, int len, bool del);
	virtual					~MemoryDataSource();

	// Virtual clone constructor
	virtual CUT_DataSource *	clone();

	// Opens data file type == UTM_OM_READING, UTM_OM_WRITING, UTM_OM_APPEND
	virtual int				Open(OpenMsgType type);

	// Close message
	virtual int				Close();

	// Read one line
	virtual int				ReadLine(LPSTR buffer, size_t maxsize);

	// Write one line
	virtual int				WriteLine(LPCSTR buffer);

	// Read data
	virtual int				Read(LPSTR buffer, size_t count);

	// Write data
	virtual int				Write(LPCSTR buffer, size_t count);

	// Move a current pointer to the specified location.
	virtual long			Seek(long offset, int origin);
};

class HandleDataSource : public CUT_DataSource {
protected:
	HANDLE					m_handle;
	bool					m_allowRead;
	bool					m_allowWrite;
public:
							HandleDataSource(HANDLE handle, bool read, bool write);
	virtual					~HandleDataSource();

	// Virtual clone constructor
	virtual CUT_DataSource *	clone();

	// Opens data file type == UTM_OM_READING, UTM_OM_WRITING, UTM_OM_APPEND
	virtual int				Open(OpenMsgType type);

	// Close message
	virtual int				Close();

	// Read one line
	virtual int				ReadLine(LPSTR buffer, size_t maxsize);

	// Write one line
	virtual int				WriteLine(LPCSTR buffer);

	// Read data
	virtual int				Read(LPSTR buffer, size_t count);

	// Write data
	virtual int				Write(LPCSTR buffer, size_t count);

	// Move a current pointer to the specified location.
	virtual long			Seek(long offset, int origin);
};

#endif //FTPCLIENTWRAPPER_H
