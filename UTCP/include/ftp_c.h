// =================================================================
//  class: CUT_FTPClient
//  File:  ftp_c.h
//
//  Purpose:
//
//  FTP client class declaration.
//
//  The primary function  of FTP is to transfer files efficiently
//   and reliably among Hosts and to allow the convenient use of
//   remote file storage capabilities.
//
//    The objectives of FTP are
//      1) to promote sharing of files (computerDeleteFile
//         programs and/or data),
//      2) to encourage indirect or implicit (via
//         programs) use of remote computers,
//      3) to shield a user from
//         variations in file storage systems among Hosts, and
//      4) to transfer
//         data reliably and efficiently.
//
// INFORMATION: see RFC 959 and it's references
// =================================================================
// Ultimate TCP/IP v4.2
// This software along with its related components, documentation and files ("The Libraries")
// is © 1994-2007 The Code Project (1612916 Ontario Limited) and use of The Libraries is
// governed by a software license agreement ("Agreement").  Copies of the Agreement are
// available at The Code Project (www.codeproject.com), as part of the package you downloaded
// to obtain this file, or directly from our office.  For a copy of the license governing
// this software, you may contact us at legalaffairs@codeproject.com, or by calling 416-849-8900.
// =================================================================

/*
NppFTP modifications:
Add class modifier to friends
CUT_WSDataClient
	Removed existing secure functionality:
	Added SSL secure functionality
CUT_FTPClient
	Removed existing secure functionality:
	Added SSL secure functionality
	Make certain helper functions virtual
	GetDirInfo accepts path parameter
	Add PeekResponseCode
*/

#ifndef  __CUT_FTP_CLIENT
#define  __CUT_FTP_CLIENT
#include "ut_clnt.h"


class CUT_FTPClient;


//=================================================================
// Data transfer socket class
//  class: CUT_WSDataClient
class CUT_WSDataClient : public CUT_WSClient
{
	friend class CUT_FTPClient;

private:
	CUT_FTPClient	*ptrFTPClient;		// pointer to the FTP client class

public:
	CUT_WSDataClient() {}
	virtual ~CUT_WSDataClient() {}

protected:
	// Monitor progress and/or cancel the receive
	virtual BOOL	ReceiveFileStatus(long bytesReceived);

	// Monitor progress and/or cancel the send
	virtual BOOL	SendFileStatus(long bytesSent);

	virtual int		OnLoadCertificates(SSL_CTX * ctx);

	virtual int		OnSSLCertificate(const SSL * ssl, const X509* certificate, int verifyResult);

public:
	virtual int SocketOnConnected(SOCKET s, const char * lpszName)
	{
		UNREFERENCED_PARAMETER(s);
		UNREFERENCED_PARAMETER(lpszName);

	    //If SSL is enabled, perform the handshake etc.
		if (m_isSSL) {
			int res = ConnectSSL();
			if (res == UTE_ERROR) {
				CloseConnection();
				return OnError(UTE_ERROR);
			}
		}

		return UTE_SUCCESS;
	}
};

// directory infomation linked list - ascii fileName for internal use
typedef struct CUT_DIRINFOATag{
	char fileName[MAX_PATH+1];	// file or directory name
	long fileSize;				// size of directory or file in bytes
	int  day;					// the day digit of the file date
	int  month;					// the month digit of the file date
	int  year;					// the year digit of the file date
	int  hour;					// the hour digit of the file date
	int  minute;				// the minute digit of the file date
	int  isDir;					// flag if the entry is directory or a file
	CUT_DIRINFOATag * next;		// next available entry
}CUT_DIRINFOA;

// _TCHAR for UI
typedef struct CUT_DIRINFOTag{
	_TCHAR fileName[MAX_PATH+1];	// file or directory name
	long fileSize;				// size of directory or file in bytes
	int  day;					// the day digit of the file date
	int  month;					// the month digit of the file date
	int  year;					// the year digit of the file date
	int  hour;					// the hour digit of the file date
	int  minute;				// the minute digit of the file date
	int  isDir;					// flag if the entry is directory or a file
	CUT_DIRINFOTag * next;		// next available entry
}CUT_DIRINFO;


//=================================================================
//FTP client class
//  class: CUT_FTPClient
class CUT_FTPClient : public CUT_WSClient {


	friend class CUT_WSDataClient;

public:
	enum FTPSMode { FTP, FTPS, FTPES };

	CUT_FTPClient();								// constructor
	virtual ~CUT_FTPClient();						// destructor

protected: // changed to protected to allow for inheritance

	CUT_WSDataClient	m_wsData;					//data transfer socket
	char				m_szResponse[MAX_PATH+1];	// last response from the server
	int					m_nConnected;				//is connected flag
	int					m_nTransferType;			//current transfer type
	int					m_nTransferMode;			//current transfer mode
	int					m_nTransferStructure;		//current transfer structure
	int					m_nDataPort;				//current data port
	int					m_nControlPort;				//current control port
	int					m_nConnectTimeout;			// the wait for connect time out

	int					m_nFirewallMode;			// client originates data connections

	CUT_StringList		m_listResponse;				//multi-line response string list
	CUT_DIRINFOA		*m_DirInfo;					//directory information list
	int					m_nDirInfoCount;			//number of directory items in the list
	int					m_lastResponseCode;			//last response code received
	bool				m_cachedResponse;			//If true, the last response was cached and no receive has to be performed

	char				m_szBuf[MAX_PATH+1];			//general purpose buffer - reduce,reuse,recycle

	FTPSMode			m_sMode;
	int					m_dataSecLevel;
	int					m_nDataPortMin;
	int					m_nDataPortMax;

	/////////////////////
	// helper functions
	/////////////////////

	// Get  the response code received from the server for the last comnmand issued along with the server response octet string
	virtual int		GetResponseCode(CUT_WSClient *ws,LPSTR string = NULL,int maxlen = 0);
	// Same as GetResponseCode, but the next call to GetResponseCode will return the same as PeekResponseCode
	// Not the same as winsock peek!
	virtual int		PeekResponseCode(CUT_WSClient *ws,LPSTR string = NULL,int maxlen = 0);

	// Clear the current list of directory information
	virtual int		ClearDirInfo();

	// firewall friendly versions of SendFile, ReceiveFile and GetDirInfo
	// automatically selected if m_firewallMode flag set.  See SetFireWallMode()
	// v4.2 protected access, but essentially treated as internal, so no WSTR overloads")
	virtual int		SendFilePASV(CUT_DataSource & source, LPCSTR destFile);
	virtual int		SendFilePASV(LPCTSTR sourceFile, LPCSTR destFile);

	// firewall friendly versions of SendFile, ReceiveFile and GetDirInfo
	// automatically selected if m_firewallMode flag set.  See SetFireWallMode()
	// v4.2 protected access, but essentially treated as internal, so no WSTR overloads")
	virtual int		ReceiveFilePASV(CUT_DataSource & dest, LPCSTR destFile);
	virtual int		ReceiveFilePASV(LPCSTR sourceFile, LPCTSTR destFile);
	virtual int		ResumeReceiveFilePASV(CUT_DataSource & dest, LPCSTR sourceFile);


	// Get the directory in a passive mode
	virtual int		GetDirInfoPASV(LPCSTR path = NULL);

	// Get the directory information in a unix format
	virtual void	GetInfoInUNIXFormat( CUT_DIRINFOA * di);

	// Get the directory information in a DOS format
	virtual void	GetInfoInDOSFormat( CUT_DIRINFOA * di);

public:
	virtual void setsMode(FTPSMode mode) {m_sMode = mode;};

	// we are going to override the SocketOnConnect to preform our handshake
	virtual int SocketOnConnected(SOCKET s, const char *lpszName);


	// connect to FTP server
	virtual int		FTPConnect(LPCSTR hostname,LPCSTR userName = "anonymous",LPCSTR password = "anonymous@anonymous.com",LPCSTR account = "");
#if defined _UNICODE
	virtual int		FTPConnect(LPCWSTR hostname,LPCWSTR userName = _T("anonymous"),LPCWSTR password = _T("anonymous@anonymous.com"),LPCWSTR account = _T(""));
#endif
	// close the current connection
	virtual int		Close();

	// Retrieve file from the server to disk
	virtual int		ReceiveFile(CUT_DataSource & dest, LPCSTR sourceFile);
	virtual int		ReceiveFile(LPCSTR sourceFile, LPCTSTR destFile);
#if defined _UNICODE
	virtual int		ReceiveFile(LPCWSTR sourceFile, LPCTSTR destFile);
#endif

	// Resume File receive
	virtual int		ResumeReceiveFile(CUT_DataSource & dest, LPCSTR sourceFile);
	virtual int		ResumeReceiveFile(LPCSTR sourceFile, LPCTSTR destFile);
#if defined _UNICODE
	virtual int		ResumeReceiveFile(LPCWSTR sourceFile, LPCTSTR destFile);
#endif


	// Send file to server
	virtual int		SendFile(CUT_DataSource & source, LPCSTR destFile);
	virtual int		SendFile(LPCTSTR sourceFile,LPCSTR destFile);
#if defined _UNICODE
	virtual int		SendFile(LPCTSTR sourceFile,LPCWSTR destFile);
#endif

	//  Ask the server to rename the file to diffrent name
	virtual int		RenameFile(LPCSTR sourceFile,LPCSTR destFile);
#if defined _UNICODE
	virtual int		RenameFile(LPCWSTR sourceFile,LPCWSTR destFile);
#endif
	// ask server to delete the file from it's directory
	virtual int		DeleteFile(LPCSTR file);
#if defined _UNICODE
	virtual int		DeleteFile(LPCWSTR file);
#endif

	// Chmod
	virtual int	ChmodFile(LPCSTR sourceFile,LPCSTR permissions);
#if defined _UNICODE
	virtual int	ChmodFile(LPCWSTR sourceFile,LPCWSTR permissions);
#endif

	// Get the Current working directory
	virtual int		GetCurDir(LPSTR directory,int maxlen);
#if defined _UNICODE
	virtual int		GetCurDir(LPWSTR directory,int maxlen);
#endif

	// change the current working directory to the one specified
	virtual int		ChDir(LPCSTR directory);
#if defined _UNICODE
	virtual int		ChDir(LPCWSTR directory);
#endif

	// Change directory one level up
	virtual int		CdUp();

	// create a directory on the server
	virtual int		MkDir(LPCSTR directory);
#if defined _UNICODE
	virtual int		MkDir(LPCWSTR directory);
#endif
	// remove a directory from server
	virtual int		RmDir(LPCSTR directory);
#if defined _UNICODE
	virtual int		RmDir(LPCWSTR directory);
#endif

	// get size of a file
	virtual int		GetSize(LPCSTR path, long * size);
#if defined _UNICODE
	virtual int		GetSize(LPCWSTR path, long * size);
#endif

	// Send a No Operation command
	virtual int		NoOp();

	// Set/Get the transfer type to  0:ASCII  1:IMAGE
	int		SetTransferType(int type);
	int		GetTransferType() const;

	// Set/Get the transfer mode to  0:STREAM  1:BLOCK  2:COMPRESSED
	int		SetTransferMode(int mode);
	int		GetTransferMode() const;

	// Set/Get connection timeout
	int		SetConnectTimeout(int secs);
	int		GetConnectTimeout() const;

	// Set/Get the structure to  0:FILE  1:RECORD  2:PAGE
	int		SetTransferStructure(int structure);
	int		GetTransferStructure() const;

	// Set/Get the fire wall mode To TRUE or FALSE
	void	SetFireWallMode(BOOL mode);
	int		GetFireWallMode() const;

	// Set/Get the data port to be  as specified
	int		SetDataPort(int port);
	int		GetDataPort() const;

	// Set/Get the control port to be  as specified
	int		SetControlPort(int port);
	int		GetControlPort() const;

	//Set/Get data security level
	int		SetDataSecure(int level);
	int		GetDataSecure();

	int		SetDataPortRange(int min, int max);
	int		GetDataPortRange(int * min, int * max);

	// Get the current Directory information
	virtual int		GetDirInfo();
	virtual int		GetDirInfo(LPCSTR path);
#if defined _UNICODE
	virtual int		GetDirInfo(LPCWSTR path);
#endif

	//  Get the number of entries in the Directory information
	//  (ie how many files and directories)
	int		GetDirInfoCount() const;

	// Select a directory entry as string
	int		GetDirEntry(int index,LPSTR entry,int maxlen);
#if defined _UNICODE
	int		GetDirEntry(int index,LPWSTR entry,int maxlen);
#endif

	// Populate the directory Info structure passed in
	// with the one at index
	// v4.2 CUT_DIRINFO has _TCHAR filename for UI. CUT_DIRINFOA used internally
	int		GetDirEntry(int index,CUT_DIRINFO *dirInfo);

	// Get help for the specified command
	virtual int		GetHelp(LPCSTR param);
#if defined _UNICODE
	virtual int		GetHelp(LPCWSTR param);
#endif

	// send a custom to be excuted on the server
	virtual int		Quote(LPCSTR command);
#if defined _UNICODE
	virtual int		Quote(LPCWSTR command);
#endif
	// Returns a number of lines in the multiline response list.
	virtual LONG	GetMultiLineResponseLineCount() const;

	// Returns string from multiline response
	virtual LPCSTR	GetMultiLineResponse(int index) const;
	// v4.2 refactored for wide char
	virtual int		GetMultiLineResponse(LPSTR response, size_t maxSize, int index, size_t *size);
#if defined _UNICODE
	virtual int		GetMultiLineResponse(LPWSTR response, size_t maxSize, int index, size_t *size);
#endif

	// get the last response of the server
	virtual LPCSTR	GetLastResponse() const;
	// v4.2 refactored for wide char
	virtual int		GetLastResponse(LPSTR response, size_t maxSize, size_t *size);
#if defined _UNICODE
	virtual int		GetLastResponse(LPWSTR response, size_t maxSize, size_t *size);
#endif

protected:

	// Monitor progress and/or cancel the receive
	virtual BOOL	ReceiveFileStatus(long bytesReceived);

	// Monitor progress and/or cancel the send
	virtual BOOL	SendFileStatus(long bytesSent);
};


#endif
