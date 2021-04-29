// =================================================================
//  class: CUT_FTPClient
//  File:  ftp_c.cpp
//
//  Purpose:
//  The primary function  of FTP is to transfer files efficiently and
//  reliably among Hosts and to allow the convenient use of
//  remote file storage capabilities.
//    The objectives of FTP are
//      1) to promote sharing of files (computer
//         programs and/or data),
//      2) to encourage indirect or implicit (via
//         programs) use of remote computers,
//      3) to shield a user from
//         variations in file storage systems among Hosts, and
//      4) to transfer
//         data reliably and efficiently.
// FOR MORE INFORMATION see RFC 959 and it's refrences
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
NppFTP:
Modification made April 2010:
-Replaced existing secure functionality with OpenSSL functionality
-Add SetDataSecure
-Rewritten FTP secure handshake
-GetMultiLineResponse was made virtual
-Clear response list when calling GetResponseCode

Modification made May 2012:
-Clear recieve buffer if failed to initiate a data connection for STOR, RETR or LIST.
*/

#ifdef _WINSOCK_2_0_
    #define _WINSOCKAPI_    /* Prevent inclusion of winsock.h in windows.h   */
                            /* Remove this line if you are using WINSOCK 1.1 */
#endif


#include "stdafx.h"
#include <time.h>


#include "ftp_c.h"

#include "ut_strop.h"


/***************************************************

    CUT_WSDataClient class implementation

***************************************************/

/***************************************************
ReceiveFileStatus
    This virtual function is called during a
    ReceiveToFile function.
Params
    bytesReceived - number of bytes received so far
Return
    TRUE - allow the receive to continue
    FALSE - abort the receive
****************************************************/
BOOL CUT_WSDataClient::ReceiveFileStatus(long bytesReceived){
    return ptrFTPClient->ReceiveFileStatus(bytesReceived);
}
/***************************************************
SendFileStatus
    This virtual function is called during a
    SendFile function.
Params
    bytesSent - number of bytes sent so far
Return
    TRUE - allow the send to continue
    FALSE - abort the send
****************************************************/
BOOL CUT_WSDataClient::SendFileStatus(long bytesSent){
    return ptrFTPClient->SendFileStatus(bytesSent);
}

int CUT_WSDataClient::OnLoadCertificates(SSL_CTX * ctx) {
    return ptrFTPClient->OnLoadCertificates(ctx);
}

int CUT_WSDataClient::OnSSLCertificate(const SSL * ssl, const X509* certificate, int verifyResult) {
    return ptrFTPClient->OnSSLCertificate(ssl, certificate, verifyResult);
}

/***************************************************

    CUT_FTPClient class implementation

***************************************************/

/***************************************
Constructor
****************************************/
CUT_FTPClient::CUT_FTPClient() :
    m_nConnected(FALSE),                // Connection is not established yet
    m_nTransferType(1),                 // Set default transfer type -     1 - :image (binary)
    m_nTransferMode(0),                 // Set default transfer mode - stream
    m_nTransferStructure(0),            // Set default transfer structure - file
    m_nControlPort(21),                 // Set default control port to 21
    m_nConnectTimeout(5),               // Set default connect time out to 5 sec.
    m_nFirewallMode(FALSE),             // No firewall mode by default
    m_DirInfo(NULL),                    // Initialize DirInfo pointer
    m_nDirInfoCount(0),                 // Number of DirInfo items - 0
    m_lastResponseCode(0),
    m_cachedResponse(false),

    m_sMode(FTP),
    m_dataSecLevel(0),                  //Default is clear data
    m_nDataPortMin(10000),
    m_nDataPortMax(32000)
{

    // initialize pointer
    m_wsData.ptrFTPClient = this;

    //set up the defaults
    m_szResponse[0]         = '\0';     // Last response from the server
    m_nDataPort              =   10000 + GetTickCount()%20000;
    if(m_nDataPort > 32000 || m_nDataPort < 0)
        m_nDataPort = 10000;
}
/***************************************
Destructor
****************************************/
CUT_FTPClient::~CUT_FTPClient(){

    // clear response list
    m_listResponse.ClearList();

    //destory any allocated memory
    ClearDirInfo();
    Close();
}
/***************************************
FTPConnect
    Connect to the given FTP site, the default
    user name is 'anonymous' and password is
    'anonymous@anonymous.com'
    DEFAULT ACCOUNT IS ""
Params
    hostname - the name of the FTP site to
        connect to
    userName - string containing the user name
    password - string containing the password
    acct - string containing the account
Return
    UTE_SUCCESS         - success
    UTE_BAD_HOSTNAME    - bad hostname format
    UTE_CONNECT_FAILED  - connection failed
    UTE_NO_RESPONSE     - no response
    UTE_INVALID_RESPONSE- negataive response
    UTE_USER_NA         - USER command not accepted
    UTE_PASS_NA         - PASS command not accepted
    UTE_ACCT_NA         - ACCT command not accepted
****************************************/
#if defined _UNICODE
int CUT_FTPClient::FTPConnect(LPCWSTR hostname,LPCWSTR userName,LPCWSTR password,LPCWSTR account){
    return FTPConnect(AC(hostname), AC(userName), AC(password), AC(account));}
#endif
int CUT_FTPClient::FTPConnect(LPCSTR hostname,LPCSTR userName,LPCSTR password,LPCSTR account){

    int  rt, error;

    //close any existing connection
    Close();

    // clear response list
    m_listResponse.ClearList();
    m_szResponse [0]= '\0';
    m_lastResponseCode = 0;
    m_cachedResponse = false;

    if (m_sMode != FTP) {
        if (m_sMode == FTPS) {  //in case of implicit SSL, negotiate security version with v23
            SetSecurityMode(CUT_FTPClient::TLS);
            m_wsData.SetSecurityMode(CUT_FTPClient::TLS);
        } else {
            //Try TLS first, SSL later
            SetSecurityMode(CUT_FTPClient::TLS);
            m_wsData.SetSecurityMode(CUT_FTPClient::TLS);
        }
        EnableSSL(true);
        m_wsData.EnableSSL(true);
    } else {
        EnableSSL(false);
        m_wsData.EnableSSL(false);
    }

    //connect
    if((error = Connect(m_nControlPort, hostname, m_nConnectTimeout)) != UTE_SUCCESS)
        return OnError(error);



    //send the user name
    _snprintf(m_szBuf,sizeof(m_szBuf)-1,"USER %s\r\n",userName);
    Send(m_szBuf);

    //check for a return of 2?? or 3??
    rt = GetResponseCode(this);
    if(rt == 0)
        return OnError(UTE_NO_RESPONSE);            //no response
    if(rt < 200 || rt > 399)
        return OnError(UTE_USER_NA);                //negative response

    //if 3?? then enter in PASS
    if(rt >=300 && rt <= 399){

        //send the password
        _snprintf(m_szBuf,sizeof(m_szBuf)-1,"PASS %s\r\n",password);
        Send(m_szBuf);

        //check for a return of 2?? or 3??
        rt = GetResponseCode(this);
        if(rt == 0)
            return OnError(UTE_NO_RESPONSE);        //no response
        if(rt < 200 || rt > 399)
            return OnError(UTE_PASS_NA);            //negative response

        //if 3?? then enter in ACCT
        if(rt >=300 && rt <= 399){

            //send the account
            _snprintf(m_szBuf,sizeof(m_szBuf)-1,"ACCT %s\r\n",account);
            Send(m_szBuf);

            //check for a return of 2?? or 3??
            rt = GetResponseCode(this);
            if(rt == 0)
                return OnError(UTE_NO_RESPONSE);    //no response
            if(rt < 200 || rt > 399)
                return OnError(UTE_ACCT_NA);        //negative response
        }
    }

    //set datachannel security
    if (m_sMode != FTP) {
        rt = SetDataSecure(m_dataSecLevel);
        if (rt != UTE_SUCCESS) {
            Close();
            return OnError(rt);
        }
    }

    //set transfer type and mode
    SetTransferType(0);
    SetTransferMode(0);
    SetTransferStructure(0);

    return OnError(UTE_SUCCESS);
}
/***************************************
Close
    Closes an open connection
Params
    none
Return
    UTE_SUCCESS - success
****************************************/
int CUT_FTPClient::Close() {

    Send("Quit\r\n");

    CloseConnection();
    m_wsData.CloseConnection();

    m_nConnected = FALSE;

    return OnError(UTE_SUCCESS);
}
/***************************************
ReceiveFile
    Retrieves the specified file from the
    currently connected FTP site. The file
    is then saved to the specified destination
    file.
Params
    sourceFile  - name of the file to receive
    destFile    - name of the file to save
Return
    UTE_SUCCESS                     - success
    UTE_SVR_DATA_CONNECT_FAILED     - data port could not be opened
    UTE_PORT_FAILED                 - PORT command failed
    UTE_RETR_FAILED                 - RETR command failed
    UTE_CONNECT_TERMINATED          - Connection terminated before completion
****************************************/
#if defined _UNICODE
int CUT_FTPClient::ReceiveFile(LPCWSTR sourceFile, LPCTSTR destFile){
    return ReceiveFile(AC(sourceFile), destFile);}
#endif
int CUT_FTPClient::ReceiveFile(LPCSTR sourceFile, LPCTSTR destFile){
    CUT_FileDataSource ds(destFile);
    return ReceiveFile(ds, sourceFile);
}

/***************************************
ReceiveFile
    Retrieves the specified file from the
    currently connected FTP site. The file
    is then saved to the specified destination
    file.
Params
    sourceFile  - name of the file to receive
    destFile    - name of the file to save
Return
    UTE_SUCCESS                     - success
    UTE_SVR_DATA_CONNECT_FAILED     - data port could not be opened
    UTE_PORT_FAILED                 - PORT command failed
    UTE_RETR_FAILED                 - RETR command failed
    UTE_CONNECT_TERMINATED          - Connection terminated before completion
****************************************/
int CUT_FTPClient::ReceiveFile(CUT_DataSource & dest, LPCSTR sourceFile)
{
    int rt,loop,len;
    char addr[32];



    if ( m_nFirewallMode )
        return ReceiveFilePASV(dest, sourceFile);

    m_wsData.SSLSetReuseSession(SSLGetCurrentSession());

    //open up a data port, if the one requested is busy then
    //increment to the next port, try 128 times then fail
    for(loop=0; loop < 128; loop++) {
        if( m_wsData.WaitForConnect((unsigned short)m_nDataPort) == UTE_SUCCESS )
            break;
        m_nDataPort++;
        }

    if(loop == 128)
        return OnError(UTE_SVR_DATA_CONNECT_FAILED);

    //get the host address
    GetHostAddress(addr,sizeof(addr));

    //create the port set up string
    len = (int)strlen(addr);
    for(loop=0;loop<len;loop++){
        if(addr[loop]=='.')
            addr[loop] = ',';
        }

    _snprintf(m_szBuf,sizeof(m_szBuf)-1,"PORT %s,%d,%d\r\n",addr,HIBYTE(m_nDataPort),LOBYTE(m_nDataPort));
    //send the PORT command
    Send(m_szBuf);

    //setup the next port number
    m_nDataPort++;
    //if(m_nDataPort > 32000 || m_nDataPort < 0)
    //    m_nDataPort = 10000;
    if(m_nDataPort > m_nDataPortMax || m_nDataPort < m_nDataPortMin)
        m_nDataPort = m_nDataPortMin;

    //check for a return of 2??
    rt = GetResponseCode(this);
    if(rt < 200 || rt >=300){
        //close the connection down
        m_wsData.CloseConnection();
        return OnError(UTE_PORT_FAILED);
        }

    //send the RETR command
    _snprintf(m_szBuf,sizeof(m_szBuf)-1,"RETR %s\r\n",sourceFile);
    Send(m_szBuf);

    //check for a return of 100 or 200 code
    rt = GetResponseCode(this);
    if(rt < 100 || rt >=300){
        //close the connection down
        m_wsData.CloseConnection();
        return OnError(UTE_RETR_FAILED);
        }

    //wait for a connection on the data port
    if(m_wsData.WaitForAccept(5)!= UTE_SUCCESS){
        //close the connection down
        m_wsData.CloseConnection();
        ClearReceiveBuffer();
        return OnError(UTE_SVR_DATA_CONNECT_FAILED);
        }


    //accept a connect on the data port
    m_wsData.AcceptConnection();

    //retrieve the file
    rt = m_wsData.Receive(dest, UTM_OM_WRITING,5);

    //close the connection down
    m_wsData.CloseConnection();

    if(rt != UTE_SUCCESS) {
        GetResponseCode(this);
        return rt;
    }

    //check for a return of 2??
    rt = GetResponseCode(this);
    if(rt < 200 || rt >=300)
        return OnError(UTE_CONNECT_TERMINATED);
    else
        return OnError(UTE_SUCCESS);
}
/***************************************
ResumeReceiveFile
    Retrieves the specified file from the
    currently connected FTP site. The file
    is then saved to the specified destination
    file.
Params
    sourceFile  - name of the file to receive
    destFile    - name of the file to save
Return
    UTE_SUCCESS                     - success
    UTE_SVR_DATA_CONNECT_FAILED     - data port could not be opened
    UTE_PORT_FAILED                 - PORT command failed
    UTE_RETR_FAILED                 - RETR command failed
    UTE_CONNECT_TERMINATED          - Connection terminated before completion
****************************************/
#if defined _UNICODE
int CUT_FTPClient::ResumeReceiveFile(LPCWSTR sourceFile, LPCTSTR destFile){
    return ResumeReceiveFile(AC(sourceFile), destFile);}
#endif
int CUT_FTPClient::ResumeReceiveFile(LPCSTR sourceFile, LPCTSTR destFile){
    CUT_FileDataSource ds(destFile);
    return ResumeReceiveFile(ds, sourceFile);
}
/***************************************
ResumeReceiveFile
    Resumes a file receive call from the point where we have received it
    The file we have already received will be searched first if it exists then the
    file pointer is advanced to the end of the file and the current available length
    is sent to the server to inform it at what point to start tranfsering.
    If the file does not exist it is created then.
Params
    dest    - name of the file to save
    sourceFile  - name of the file to receive
Return
    UTE_SUCCESS                     - success
    UTE_SVR_DATA_CONNECT_FAILED     - data port could not be opened
    UTE_PORT_FAILED                 - PORT command failed
    UTE_RETR_FAILED                 - RETR command failed
    UTE_CONNECT_TERMINATED          - Connection terminated before completion
****************************************/
int CUT_FTPClient::ResumeReceiveFile(CUT_DataSource & dest, LPCSTR sourceFile)
{
    int rt,loop,len;
    char addr[32];
    OpenMsgType fileType = UTM_OM_WRITING;  // by default we will write a file unless it exist
                                            // then we will append to it

    if ( m_nFirewallMode )
        return ResumeReceiveFilePASV(dest, sourceFile);

    m_wsData.SSLSetReuseSession(SSLGetCurrentSession());

    //open up a data port, if the one requested is busy then
    //increment to the next port, try 128 times then fail
    for(loop=0; loop < 128; loop++) {
        if( m_wsData.WaitForConnect((unsigned short)m_nDataPort) == UTE_SUCCESS )
            break;
        m_nDataPort++;
        }

    if(loop == 128)
        return OnError(UTE_SVR_DATA_CONNECT_FAILED);

    //get the host address
    GetHostAddress(addr,sizeof(addr));

    //create the port set up string
    len = (int)strlen(addr);
    for(loop=0;loop<len;loop++){
        if(addr[loop]=='.')
            addr[loop] = ',';
        }

    _snprintf(m_szBuf,sizeof(m_szBuf)-1,"PORT %s,%d,%d\r\n",addr,HIBYTE(m_nDataPort),LOBYTE(m_nDataPort));
    //send the PORT command
    Send(m_szBuf);

    //setup the next port number
    m_nDataPort++;
    if(m_nDataPort > 32000 || m_nDataPort < 0)
        m_nDataPort = 10000;

    //check for a return of 2??
    rt = GetResponseCode(this);
    if(rt < 200 || rt >=300){
        //close the connection down
        m_wsData.CloseConnection();
        return OnError(UTE_PORT_FAILED);
        }

    rt = dest.Open (UTM_OM_READING);

    if (rt == UTE_SUCCESS)
    {

    // if the file exist then we will send the REST command  with the size of the file we have
    // otherwise we just call retrieve as we do normally
    _snprintf(m_szBuf,sizeof(m_szBuf)-1,"REST %ld\r\n",dest.Seek (0,SEEK_END));
    dest.Close ();
    Send(m_szBuf);
    //check for a return of 100 or 200 code
    rt = GetResponseCode(this);
    if( rt != 350){
        //close the connection down
        m_wsData.CloseConnection();
        return OnError(UTE_REST_COMMAND_NOT_SUPPORTED);
        }
    fileType = UTM_OM_APPEND ; // appending
    }

    //send the RETR command
    _snprintf(m_szBuf,sizeof(m_szBuf)-1,"RETR %s\r\n",sourceFile);
    Send(m_szBuf);

    //check for a return of 100 or 200 code
    rt = GetResponseCode(this);
    if(rt < 100 || rt >=300){
        //close the connection down
        m_wsData.CloseConnection();
        return OnError(UTE_RETR_FAILED);
        }

    //wait for a connection on the data port
    if(m_wsData.WaitForAccept(5)!= UTE_SUCCESS){
        //close the connection down
        m_wsData.CloseConnection();
        ClearReceiveBuffer();
        return OnError(UTE_SVR_DATA_CONNECT_FAILED);
        }


    //accept a connect on the data port
    m_wsData.AcceptConnection();

    //retrieve the file based on the fileType  UTM_OM_APPEND if it does exist UTM_OM_WRITING if we need to creat it
    rt = m_wsData.Receive(dest, fileType);

    //close the connection down
    m_wsData.CloseConnection();

    if(rt != UTE_SUCCESS)
        return rt;

    //check for a return of 2??
    rt = GetResponseCode(this);
    if(rt < 200 || rt >=300)
        return OnError(UTE_CONNECT_TERMINATED);
    else
        return OnError(UTE_SUCCESS);
}

/***************************************
    FIXME (This function Will never be called ) ??

ReceiveFilePASV
    Retrieves the specified file from the
    currently connected FTP site. The file
    is then saved to the specified destination
    file.
    This routine duplicates the ReceiveFile()
    member, but in this routine the client
    originates data connections.  Client side
    origination of data connections is
    preferred for use in firewall controlled
    environments.
Params
    sourceFile - name of the file to receive
    destFile - name of the file to save
Return
    UTE_SUCCESS                     - success
    UTE_SVR_DATA_CONNECT_FAILED     - data port could not be opened
    UTE_PORT_FAILED                 - PORT command failed
    UTE_RETR_FAILED                 - RETR command failed
    UTE_CONNECT_TERMINATED          - Connection terminated before completion
    UTE_CONNECT_TIMEOUT             - connection timeout
****************************************/
int CUT_FTPClient::ReceiveFilePASV( LPCSTR sourceFile,LPCTSTR destFile ){
    CUT_FileDataSource ds(destFile);
    return ReceiveFilePASV(ds, sourceFile);
}
/***************************************
ResumeReceiveFilePASV
    Resumes retrieving the specified file from the
    currently connected FTP site. The file
    is then saved to the specified destination
    file.
    This routine duplicates the ReceiveFile()
    member, but in this routine the client
    originates data connections.  Client side
    origination of data connections is
    preferred for use in firewall controlled
    environments.
Params
    dest        - data source to receive to
    sourceFile  - file to get
Return
    UTE_SUCCESS                     - success
    UTE_SVR_DATA_CONNECT_FAILED     - data port could not be opened
    UTE_PORT_FAILED                 - PORT command failed
    UTE_RETR_FAILED                 - RETR command failed
    UTE_CONNECT_TERMINATED          - Connection terminated before completion
    UTE_CONNECT_TIMEOUT             - connection timeout
****************************************/
int CUT_FTPClient::ResumeReceiveFilePASV(CUT_DataSource & dest, LPCSTR sourceFile) {

    UINT    loop;
    int     rt, error;
    char    responseBuf[100];
    int     port;
    char    ipAddress[20];
    char    *token;
    OpenMsgType fileType = UTM_OM_WRITING;  // by default we will write a file unless it exist
                                            // then we will append to it



    //send the port command
    Send("PASV\r\n");

    m_wsData.SSLSetReuseSession(SSLGetCurrentSession());

    // we need to get the full IP address and port number from the
    // PASV return line, so that we can originate the data connection.

    //check for a return of 2??, which indicates success
    rt = GetResponseCode( this, responseBuf, sizeof(responseBuf) );
    if(rt < 200 || rt >299)
        return OnError(UTE_PORT_FAILED);


    // find the first '(' in the response and then parse out
    // the address supplied by the server
    loop = 0;
    while ( loop < strlen(responseBuf) ){
        if ( responseBuf[loop] == '(' )
            break;
        loop++;
        }

    if ( loop == strlen(responseBuf) ) // we hit the end of the buffer
        return OnError(UTE_RETR_FAILED);

    // token out the ipaddress and port string
    token = strtok( &responseBuf[loop], "()" );

    // get the four portions of the ip address
    strcpy(ipAddress, strtok( token, ",)\r\n" ));
    strcat(ipAddress, ".");
    strcat(ipAddress, strtok( NULL, ",)\r\n" ));
    strcat(ipAddress, ".");
    strcat(ipAddress, strtok( NULL, ",)\r\n" ));
    strcat(ipAddress, ".");
    strcat(ipAddress, strtok( NULL, ",)\r\n" ));

    // get the two portions that make up the port number
    port = atoi( strtok(NULL, ",)\r\n") ) * 256;
    port += atoi( strtok(NULL, ",)\r\n") );

    // test the results to ensure everything is OK.
    if ( !IsIPAddress(ipAddress) )
        return OnError(UTE_SVR_DATA_CONNECT_FAILED);

    if ( port<=0 || port > 65535 )
        return OnError(UTE_CONNECT_TERMINATED);

    // Check for abortion flag
    if(IsAborted()) {
        m_wsData.CloseConnection();
        return OnError(UTE_ABORTED);
        }

    rt = dest.Open (UTM_OM_READING);

    if (rt == UTE_SUCCESS)
    {

    // if the file exist then we will send the REST command  with the size of the file we have
    // otherwise we just call retrieve as we do normally
    _snprintf(m_szBuf,sizeof(m_szBuf)-1,"REST %ld\r\n",dest.Seek (0,SEEK_END));
    dest.Close ();
    Send(m_szBuf);
    //check for a return of 100 or 200 code
    rt = GetResponseCode(this);
    if( rt != 350){
        //close the connection down
        m_wsData.CloseConnection();
        return OnError(UTE_REST_COMMAND_NOT_SUPPORTED);
        }
    fileType = UTM_OM_APPEND ; // appending
    }

    //send the RETR command
    _snprintf(m_szBuf,sizeof(m_szBuf)-1,"RETR %s\r\n",sourceFile);
    Send(m_szBuf);

    // connect to the server supplied port to establish the
    // data connection.
    // connect using a timeout
    if((error = m_wsData.Connect(port, ipAddress, m_nConnectTimeout)) != UTE_SUCCESS) {
        m_wsData.CloseConnection();
        ClearReceiveBuffer();
        return OnError(error);
        }

    //check for a return of 100 or 200 code
    rt = GetResponseCode(this);
    if(rt < 100 || rt >=300){
        m_wsData.CloseConnection();
        return OnError(UTE_RETR_FAILED);
        }

     //retrieve the file based on the fileType  UTM_OM_APPEND if it does exist UTM_OM_WRITING if we need to creat it
    rt = m_wsData.Receive(dest, fileType);

    //close the connection down
    m_wsData.CloseConnection();

    if(rt != UTE_SUCCESS) {
        GetResponseCode(this);
        return rt;
    }

    //check for a return of 2??
    rt = GetResponseCode(this);
    if(rt < 200 || rt >=300)
        return OnError(UTE_CONNECT_TERMINATED);
    else
        return OnError(UTE_SUCCESS);
}
/***************************************
ReceiveFilePASV
    Retrieves the specified file from the
    currently connected FTP site. The file
    is then saved to the specified destination
    file.
    This routine duplicates the ReceiveFile()
    member, but in this routine the client
    originates data connections.  Client side
    origination of data connections is
    preferred for use in firewall controlled
    environments.
Params
    dest        - data source to receive to
    sourceFile  - file to get
Return
    UTE_SUCCESS                     - success
    UTE_SVR_DATA_CONNECT_FAILED     - data port could not be opened
    UTE_PORT_FAILED                 - PORT command failed
    UTE_RETR_FAILED                 - RETR command failed
    UTE_CONNECT_TERMINATED          - Connection terminated before completion
    UTE_CONNECT_TIMEOUT             - connection timeout
****************************************/
int CUT_FTPClient::ReceiveFilePASV(CUT_DataSource & dest, LPCSTR sourceFile) {

    UINT    loop;
    int     rt, error;
    char    responseBuf[100];
    int     port;
    char    ipAddress[20];
    char    *token;


    //send the port command
    Send("PASV\r\n");

    m_wsData.SSLSetReuseSession(SSLGetCurrentSession());

    // we need to get the full IP address and port number from the
    // PASV return line, so that we can originate the data connection.

    //check for a return of 2??, which indicates success
    rt = GetResponseCode( this, responseBuf, sizeof(responseBuf) );
    if(rt < 200 || rt >299)
        return OnError(UTE_PORT_FAILED);


    // find the first '(' in the response and then parse out
    // the address supplied by the server
    loop = 0;
    while ( loop < strlen(responseBuf) ){
        if ( responseBuf[loop] == '(' )
            break;
        loop++;
        }

    if ( loop == strlen(responseBuf) ) // we hit the end of the buffer
        return OnError(UTE_RETR_FAILED);

    // token out the ipaddress and port string
    token = strtok( &responseBuf[loop], "()" );

    // get the four portions of the ip address
    strcpy(ipAddress, strtok( token, ",)\r\n" ));
    strcat(ipAddress, ".");
    strcat(ipAddress, strtok( NULL, ",)\r\n" ));
    strcat(ipAddress, ".");
    strcat(ipAddress, strtok( NULL, ",)\r\n" ));
    strcat(ipAddress, ".");
    strcat(ipAddress, strtok( NULL, ",)\r\n" ));

    // get the two portions that make up the port number
    port = atoi( strtok(NULL, ",)\r\n") ) * 256;
    port += atoi( strtok(NULL, ",)\r\n") );

    // test the results to ensure everything is OK.
    if ( !IsIPAddress(ipAddress) )
        return OnError(UTE_SVR_DATA_CONNECT_FAILED);

    if ( port<=0 || port > 65535 )
        return OnError(UTE_CONNECT_TERMINATED);

    // Check for abortion flag
    if(IsAborted()) {
        m_wsData.CloseConnection();
        return OnError(UTE_ABORTED);
        }

    //send the RETR command
    _snprintf(m_szBuf,sizeof(m_szBuf)-1,"RETR %s\r\n",sourceFile);
    Send(m_szBuf);

    // connect to the server supplied port to establish the
    // data connection.
    // connect using a timeout
    if((error = m_wsData.Connect(port, ipAddress, m_nConnectTimeout)) != UTE_SUCCESS) {
        m_wsData.CloseConnection();
        ClearReceiveBuffer();
        return OnError(error);
        }

    //check for a return of 100 or 200 code
    rt = GetResponseCode(this);
    if(rt < 100 || rt >=300){
        m_wsData.CloseConnection();
        return OnError(UTE_RETR_FAILED);
        }

    //retrieve the file
    rt = m_wsData.Receive(dest, UTM_OM_WRITING);

    //close the connection down
    m_wsData.CloseConnection();

    if(rt != UTE_SUCCESS)
        return rt;

    //check for a return of 2??
    rt = GetResponseCode(this);
    if(rt < 200 || rt >=300)
        return OnError(UTE_CONNECT_TERMINATED);
    else
        return OnError(UTE_SUCCESS);
}

/***************************************
SendFile
    Sends the specified local file to
    the given destination on the currently
    connected FTP site.
Params
    sourceFile - name of file to send
    destFile - name of FTP site destination
Return
    UTE_SUCCESS                     - success
    UTE_SVR_DATA_CONNECT_FAILED     - data port could not be opened
    UTE_PORT_FAILED                 - PORT command failed
    UTE_RETR_FAILED                 - RETR command failed
    UTE_CONNECT_TERMINATED          - Connection terminated before completion
    UTE_STOR_FAILED                 - STOR command failed
****************************************/
#if defined _UNICODE
int CUT_FTPClient::SendFile(LPCTSTR sourceFile,LPCWSTR destFile){
    return SendFile(sourceFile, AC(destFile));}
#endif
int CUT_FTPClient::SendFile(LPCTSTR sourceFile,LPCSTR destFile){
    CUT_FileDataSource ds(sourceFile);
    return SendFile(ds, destFile);
}

/***************************************
SendFile
    Sends the specified local file to
    the given destination on the currently
    connected FTP site.
Params
    sourceFile  - name of the data source to send
    destFile    - name of FTP site destination
Return
    UTE_SUCCESS                     - success
    UTE_SVR_DATA_CONNECT_FAILED     - data port could not be opened
    UTE_PORT_FAILED                 - PORT command failed
    UTE_RETR_FAILED                 - RETR command failed
    UTE_CONNECT_TERMINATED          - Connection terminated before completion
    UTE_STOR_FAILED                 - STOR command failed
****************************************/
int CUT_FTPClient::SendFile(CUT_DataSource & source, LPCSTR destFile)
{
    int     rt, loop, len;
    char    addr[32];

    if (m_nFirewallMode)
        return SendFilePASV(source, destFile);

    m_wsData.SSLSetReuseSession(SSLGetCurrentSession());

    //open up a data port, if the one requested is busy then
    //increment to the next port, try 128 times then fail
    for(loop=0;loop<128;loop++){
        if(m_wsData.WaitForConnect((unsigned short)m_nDataPort)==UTE_SUCCESS)
            break;
        m_nDataPort++;
        }

    if(loop==128)
        return OnError(UTE_SVR_DATA_CONNECT_FAILED);

    //get the host address
    GetHostAddress(addr,sizeof(addr));

    //create the port set up string
    len = (int)strlen(addr);
    for(loop=0;loop<len;loop++){
        if(addr[loop]=='.')
            addr[loop] = ',';
        }

    _snprintf(m_szBuf,sizeof(m_szBuf)-1,"PORT %s,%d,%d\r\n",addr,HIBYTE(m_nDataPort),LOBYTE(m_nDataPort));
    //send the port command
    Send(m_szBuf);

    //setup the next port number
    m_nDataPort++;
    //if(m_nDataPort > 32000 || m_nDataPort < 0)
    //    m_nDataPort = 10000;
    if(m_nDataPort > m_nDataPortMax || m_nDataPort < m_nDataPortMin)
        m_nDataPort = m_nDataPortMin;

    //check for a return of 2??
    rt = GetResponseCode(this);
    if(rt < 200 || rt >=300){
        //close the connection down
        m_wsData.CloseConnection();
        return OnError(UTE_PORT_FAILED);
        }

    // Check for abortion flag
    if(IsAborted()) {
        m_wsData.CloseConnection();
        return OnError(UTE_ABORTED);
        }

    //send the store command
    _snprintf(m_szBuf,sizeof(m_szBuf)-1,"STOR %s\r\n",destFile);
    Send(m_szBuf);

    //check for a return of 100 or 200 code
    rt = GetResponseCode(this);
    if(rt < 100 || rt >=300){
        //close the connection down
        m_wsData.CloseConnection();
        return OnError(UTE_STOR_FAILED);
        }

    //wait for a connection on the data port
    if(m_wsData.WaitForAccept(5)!= UTE_SUCCESS){
        //close the connection down
        m_wsData.CloseConnection();
        ClearReceiveBuffer();
        return OnError(UTE_SVR_DATA_CONNECT_FAILED);
        }

    //accept a connect on the data port
    m_wsData.AcceptConnection();

    //retrieve the file
    rt = m_wsData.Send(source);

    //close the connection down
    m_wsData.CloseConnection();

    if(rt != UTE_SUCCESS) {
        GetResponseCode(this);
        return rt;
    }

    //check for a return of 2??
    rt = GetResponseCode(this);
    if(rt < 200 || rt >=300)
        return OnError(UTE_CONNECT_TERMINATED);
    else
        return OnError(UTE_SUCCESS);
}

/***************************************
SendFilePASV
    Sends the specified local file to
    the given destination on the currently
    connected FTP site.
    This routine duplicates the SendFilePASV()
    member, but in this routine the client
    originates data connections.  Client side
    origination of data connections is
    preferred for use in firewall controlled
    environments.
Params
    sourceFile  - name of file to send
    destFile    - name of FTP site destination
Return
    UTE_SUCCESS                     - success
    UTE_SVR_DATA_CONNECT_FAILED     - data port could not be opened
    UTE_PORT_FAILED                 - PORT command failed
    UTE_RETR_FAILED                 - RETR command failed
    UTE_CONNECT_TERMINATED          - Connection terminated before completion
    UTE_STOR_FAILED                 - STOR command failed
    UTE_CONNECT_TIMEOUT             - connection time out
****************************************/
int CUT_FTPClient::SendFilePASV(LPCTSTR sourceFile,LPCSTR destFile){
    CUT_FileDataSource ds(sourceFile);
    return SendFilePASV(ds, destFile);
}

/***************************************
SendFilePASV
    Sends the specified local file to
    the given destination on the currently
    connected FTP site.
    This routine duplicates the SendFilePASV()
    member, but in this routine the client
    originates data connections.  Client side
    origination of data connections is
    preferred for use in firewall controlled
    environments.
Params
    source      - data source to send
    destFile    - name of FTP site destination
Return
    UTE_SUCCESS                     - success
    UTE_SVR_DATA_CONNECT_FAILED     - data port could not be opened
    UTE_PORT_FAILED                 - PORT command failed
    UTE_RETR_FAILED                 - RETR command failed
    UTE_CONNECT_TERMINATED          - Connection terminated before completion
    UTE_STOR_FAILED                 - STOR command failed
    UTE_CONNECT_TIMEOUT             - connection time out
****************************************/
int CUT_FTPClient::SendFilePASV(CUT_DataSource & source, LPCSTR destFile) {

    UINT    loop;
    int     error, rt, port;
    char    *token;
    char    ipAddress[20];
    char    responseBuf[100];


    //send the port command
    Send("PASV\r\n");

    m_wsData.SSLSetReuseSession(SSLGetCurrentSession());

    // we need to get the full IP address and port number from the
    // PASV return line, so that we can originate the data connection.

    //check for a return of 2??, which indicates success
    rt = GetResponseCode( this, responseBuf, sizeof(responseBuf) );
    if(rt < 200 || rt >299)
        return OnError(UTE_PORT_FAILED);

    // find the first '(' in the response and then parse out
    // the address supplied by the server
    loop = 0;
    while ( loop < strlen(responseBuf) ){
        if ( responseBuf[loop] == '(' )
            break;
        loop++;
        }

    if ( loop == strlen(responseBuf) ) // we hit the end of the buffer
        return OnError(UTE_STOR_FAILED);

    // token out the ipaddress and port string
    token = strtok( &responseBuf[loop], "()" );

    // get the four portions of the ip address
    strcpy(ipAddress, strtok( token, ",)\r\n" ));
    strcat(ipAddress, ".");
    strcat(ipAddress, strtok( NULL, ",)\r\n" ));
    strcat(ipAddress, ".");
    strcat(ipAddress, strtok( NULL, ",)\r\n" ));
    strcat(ipAddress, ".");
    strcat(ipAddress, strtok( NULL, ",)\r\n" ));

    // get the two portions that make up the port number
    port = atoi( strtok(NULL, ",)\r\n") ) * 256;
    port += atoi( strtok(NULL, ",)\r\n") );

    // test the results to ensure everything is OK.
    if ( !IsIPAddress(ipAddress) )
        return OnError(UTE_SVR_DATA_CONNECT_FAILED);

    if ( port<=0 || port > 65535 )
        return OnError(UTE_CONNECT_TERMINATED);

    // Check for abortion flag
    if(IsAborted()) {
        return OnError(UTE_ABORTED);
        }

    //send the store command
    _snprintf(m_szBuf,sizeof(m_szBuf)-1,"STOR %s\r\n",destFile);
    Send(m_szBuf);

    // connect to the server supplied port to establish the
    // data connection.
    // connect using a timeout
    if((error = m_wsData.Connect(port, ipAddress, m_nConnectTimeout)) != UTE_SUCCESS) {
        m_wsData.CloseConnection();
        ClearReceiveBuffer();
        return OnError(error);
        }

    //check for a return of 100 or 200 code
    rt = GetResponseCode(this);
    if(rt < 100 || rt >=300){
        m_wsData.CloseConnection();
        return OnError(UTE_STOR_FAILED);
        }

    //retrieve the file
    rt = m_wsData.Send(source);

    //close the connection down
    m_wsData.CloseConnection();

    // If there is an error sending data
    if(rt != UTE_SUCCESS) {
        GetResponseCode(this);
        return rt;
    }

    //check for a return of 2??
    rt = GetResponseCode(this);
    if(rt < 200 || rt >=300)
        return OnError(UTE_CONNECT_TERMINATED);
    else
        return OnError(UTE_SUCCESS);
}


/***************************************
DeleteFile
    Deletes  the specified file off of the
    currently connect FTP server.
Params
    file - name of file to delete
Return
    UTE_SUCCESS             - success
    UTE_NO_RESPONSE         - no response
    UTE_SVR_REQUEST_DENIED  - request denied by server
****************************************/
#if defined _UNICODE
int CUT_FTPClient::DeleteFile(LPCWSTR file){
    return DeleteFile(AC(file));}
#endif
int CUT_FTPClient::DeleteFile(LPCSTR file){

    int     rt;

    _snprintf(m_szBuf,sizeof(m_szBuf)-1,"DELE %s\r\n",file);
    Send(m_szBuf);

    //check for a return of 2??
    rt = GetResponseCode(this);
    if(rt == 0)
        return OnError(UTE_NO_RESPONSE);   //no response
    else if(rt >=200 && rt <=299)
        return OnError(UTE_SUCCESS);

    return OnError(UTE_SVR_REQUEST_DENIED);
}
/***************************************
RenameFile
    Renames the given file on the currently
    connected FTP site.
Params
    sourceFile - name of file on the FTP server
    destFile - new name to give the file
Return
    UTE_SUCCESS             - success
    UTE_NO_RESPONSE         - no response
    UTE_RNFR_NA             - RNFR command not accepted
    UTE_RNTO_NA             - RNTO command not accepted
****************************************/
#if defined _UNICODE
int CUT_FTPClient::RenameFile(LPCWSTR sourceFile,LPCWSTR destFile){
    return RenameFile(AC(sourceFile), AC(destFile));}
#endif
int CUT_FTPClient::RenameFile(LPCSTR sourceFile,LPCSTR destFile){

    int     rt;

    // send rename from command
    _snprintf(m_szBuf,sizeof(m_szBuf)-1,"RNFR %s\r\n",sourceFile);
    Send(m_szBuf);

    //check for a return of 3??
    rt = GetResponseCode(this);
    if(rt == 0)
        return OnError(UTE_NO_RESPONSE);   //no response

    else if(rt >= 300 && rt <= 399) {
        //send rename to command
        _snprintf(m_szBuf,sizeof(m_szBuf)-1,"RNTO %s\r\n",destFile);
        Send(m_szBuf);

        //check for a return of 2??
        rt = GetResponseCode(this);
        if(rt == 0)
            return OnError(UTE_NO_RESPONSE);   //no response
        else if(rt >=200 && rt <=299)
            return OnError(UTE_SUCCESS);
        else
            return OnError(UTE_RNTO_NA);
    }
    else
        return OnError(UTE_RNFR_NA);
}


/***************************************
ChmodFile
    Description: change file permissions
Params
    sourceFile - name of file on the FTP server
    permissions - new file premissions
Return
    UTE_SUCCESS             - success
    UTE_NO_RESPONSE         - no response
    UTE_SVR_REQUEST_DENIED  - request denied by server
****************************************/
#if defined _UNICODE
int CUT_FTPClient::ChmodFile(LPCWSTR sourceFile,LPCWSTR permissions){
    return ChmodFile(AC(sourceFile), AC(permissions));}
#endif
int CUT_FTPClient::ChmodFile(LPCSTR sourceFile,LPCSTR permissions){

    int     rt;

    // SITE CHMOD 644 transfer.png

    _snprintf(m_szBuf,sizeof(m_szBuf)-1,"SITE CHMOD %s %s\r\n",permissions,sourceFile);
    Send(m_szBuf);

    //check for a return of 2??
    rt = GetResponseCode(this);
    if(rt == 0)
        return OnError(UTE_NO_RESPONSE);   //no response
    else if(rt >=200 && rt <=299)
        return OnError(UTE_SUCCESS);
    return OnError(UTE_SVR_REQUEST_DENIED);
}


/***************************************
GetCurDir
    Returns the name of the current
    directory on the currently connected
    FTP server.
Params
    directory - buffer to hold the dir. name
    maxlen - length of the buffer
Return
    UTE_SUCCESS             - success
    UTE_NO_RESPONSE         - no response
    UTE_SVR_REQUEST_DENIED  - request denied by server
****************************************/
#if defined _UNICODE
int CUT_FTPClient::GetCurDir(LPWSTR directory,int maxlen){
    char * directoryA = (char*) alloca(maxlen);
    *directoryA = '\0';
    int result = GetCurDir(directoryA, maxlen);
    if(result == UTE_SUCCESS) {
        CUT_Str::cvtcpy(directory, maxlen, directoryA);
    }
    return result;}
#endif
int CUT_FTPClient::GetCurDir(LPSTR directory,int maxlen){

    int rt;

    Send("PWD\r\n");

    //check for a return of 2??
    rt = GetResponseCode(this,directory,maxlen);
    if(rt == 0)
        return OnError(UTE_NO_RESPONSE);            //no response
    if(rt < 200 || rt > 299)
        return OnError(UTE_SVR_REQUEST_DENIED);     //negative response

    // now lets parse the directory from the actual response
    char buf1[MAX_PATH+1];
    char buf2[MAX_PATH+1];
    strcpy(buf1,directory);
    CUT_StrMethods::ParseString(buf1,"\"",0,buf2,sizeof(buf2));
    CUT_StrMethods::ParseString(buf1,"\"",0,buf2,sizeof(buf2));
    strcpy(directory,buf2);//,&buf1[strlen(buf2)]);

    return OnError(UTE_SUCCESS);
}
/***************************************
ChDir
    Changes the current directory of the
    currently connected FTP server
Params
    directory - name of directory to move to
Return
    UTE_SUCCESS             - success
    UTE_NO_RESPONSE         - no response
    UTE_SVR_REQUEST_DENIED  - request denied by server
****************************************/
#if defined _UNICODE
int CUT_FTPClient::ChDir(LPCWSTR directory){
    return ChDir(AC(directory));}
#endif
int CUT_FTPClient::ChDir(LPCSTR directory){

    int     rt;

    _snprintf(m_szBuf,sizeof(m_szBuf)-1,"CWD %s\r\n",directory); // FEB 1999 added /
    Send(m_szBuf);

    //check for a return of 2??
    rt = GetResponseCode(this);
    if(rt == 0)
        return OnError(UTE_NO_RESPONSE);   //no response
    else if(rt >=200 && rt <=299)
        return OnError(UTE_SUCCESS);

    return OnError(UTE_SVR_REQUEST_DENIED);
}
/***************************************
CdUP
    Moves up one directory level on the
    currently connected FTP server
Params
    none
Return
    UTE_SUCCESS             - success
    UTE_NO_RESPONSE         - no response
    UTE_SVR_REQUEST_DENIED  - request denied by server
****************************************/
int CUT_FTPClient::CdUp(){

    int     rt;

    Send("CDUP\r\n");

    //check for a return of 2??
    rt = GetResponseCode(this);
    if(rt == 0)
        return OnError(UTE_NO_RESPONSE);   //no response
    else if(rt >=500 && rt <=599)
        return OnError(UTE_SVR_REQUEST_DENIED);
    else if(rt >=400 && rt <=499)
        return OnError(UTE_SVR_REQUEST_DENIED);
    else if(rt >=200 && rt <=299)
        return OnError(UTE_SUCCESS);

    return OnError(UTE_SVR_REQUEST_DENIED);
}
/***************************************
MkDir
    Creates a new directory on the
    currently connected FTP server
Params
    directory - name of directory to create
Return
    UTE_SUCCESS             - success
    UTE_NO_RESPONSE         - no response
    UTE_SVR_REQUEST_DENIED  - request denied by server
****************************************/
#if defined _UNICODE
int CUT_FTPClient::MkDir(LPCWSTR directory){
    return MkDir(AC(directory));}
#endif
int CUT_FTPClient::MkDir(LPCSTR directory){

    int     rt;

    _snprintf(m_szBuf,sizeof(m_szBuf)-1,"MKD %s\r\n",directory);
    Send(m_szBuf);

    //check for a return of 2??
    rt = GetResponseCode(this);
    if(rt == 0)
        return OnError(UTE_NO_RESPONSE);   //no response
    else if(rt >=200 && rt <=299)
        return OnError(UTE_SUCCESS);
    return OnError(UTE_SVR_REQUEST_DENIED);
}
/***************************************
RmDir
    Removes a directory on the
    currently connected FTP server
Params
    directory - name of directory to remove
Return
    UTE_SUCCESS             - success
    UTE_NO_RESPONSE         - no response
    UTE_SVR_REQUEST_DENIED  - request denied by server
****************************************/
#if defined _UNICODE
int CUT_FTPClient::RmDir(LPCWSTR directory){
    return RmDir(AC(directory));}
#endif
int CUT_FTPClient::RmDir(LPCSTR directory){

    int     rt;

    _snprintf(m_szBuf,sizeof(m_szBuf)-1,"RMD %s\r\n",directory);
    Send(m_szBuf);

    //check for a return of 2??
    rt = GetResponseCode(this);
    if(rt == 0)
        return OnError(UTE_NO_RESPONSE);   //no response
    else if(rt >=200 && rt <=299)
        return OnError(UTE_SUCCESS);
    return OnError(UTE_SVR_REQUEST_DENIED);
}

int CUT_FTPClient::GetSize(LPCSTR path, long * size) {
    int     rt;

    _snprintf(m_szBuf,sizeof(m_szBuf)-1,"SIZE %s\r\n",path);
    Send(m_szBuf);
    //check for a return of 213
    rt = GetResponseCode(this);
    if(rt == 0)
        return OnError(UTE_NO_RESPONSE);   //no response
    else if(rt == 213) {
        //Response is a single line with "213 SIZE", so can safely substring
        LPCSTR response = GetMultiLineResponse(0);
        response += 4;  //skip "213 "
        *size = atol(response);

        return OnError(UTE_SUCCESS);
    }
    return OnError(UTE_SVR_NOT_SUPPORTED);
}

#if defined _UNICODE
int CUT_FTPClient::GetSize(LPCWSTR path, long * size) {
    return GetSize(AC(path), size);}
#endif
/***************************************
NoOp
    Performs a No-op operation. This is
    usually used to check and see if the
    connection is still up.
Params
    none
Return
    UTE_SUCCESS             - success
    UTE_NO_RESPONSE         - no response
    UTE_SVR_REQUEST_DENIED  - request denied by server
****************************************/
int CUT_FTPClient::NoOp(){

    int     rt;

    Send("NOOP\r\n");

    //check for a return of 2??
    rt = GetResponseCode(this);
    if(rt == 0)
        return OnError(UTE_NO_RESPONSE);   //no response
    else if(rt >=200 && rt <=299)
        return OnError(UTE_SUCCESS);
    return OnError(UTE_SVR_REQUEST_DENIED);
}
/***************************************
SetTransferType
    Sets the data transfer type. The data
    representation type used for data transfer and
    storage.  Type implies certain transformations
    between the time of data storage and data transfer.
    The representation types defined in FTP are described
    in the Section on Establishing Data Connections. of RFC 959
    NOTE that we only supporting asscii and image types.
PARAM
    type
    0 - :ascii
        "
        This is the default type and must be accepted by all FTP
         implementations.  It is intended primarily for the transfer of
         text files, except when both Hosts would find the EBCDIC type
         more convenient.
         The sender converts the data from his internal character
         representation to the standard 8-bit NVT-ASCII representation
         (see the TELNET specification).  The receiver will convert the
         data from the standard form to his own internal form.
         In accordance with the NVT standard, the <CRLF> sequence should
         be used, where necessary, to denote the end of a line of text.
         (See the discussion of file structure at the end of the Section
         on Data Representation and Storage).
         Using the standard NVT-ASCII representation means that data
         must be interpreted as 8-bit bytes." RFC 959

    1 - :image
        " The data are sent as contiguous bits which, for transfer, are
         packed into transfer bytes of the size specified in the BYTE
         command.  The receiving site must store the data as contiguous
         bits.  The structure of the storage system might necessitate
         the padding of the file (or of each record, for a
         record-structured file) to some convenient boundary (byte, word
         or block).  This padding, which must be all zeroes, may occur
         only at the end of the file (or at the end of each record) and
         there must be a way of identifying the padding bits so that
         they may be stripped off if the file is retrieved.  The padding
         transformation should be well publicized to enable a user to
         process a file at the storage site.
         Image type is intended for the efficient storage and retrieval
         of files and for the transfer of binary data.  It is
         recommended that this type be accepted by all FTP
         implementations. " RFC 959
Return
    UTE_SUCCESS                 - success
    UTE_NO_RESPONSE             - no response
    UTE_SVR_REQUEST_DENIED      - request denied by server
    UTE_PARAMETER_INVALID_VALUE - invalid type
****************************************/
int CUT_FTPClient::SetTransferType(int type){

    int     rt;

    //check for a valid range
    if(type <0 || type >1)
        return OnError(UTE_PARAMETER_INVALID_VALUE);

    //send the type info
    if(type ==0)        //ascii
        Send("TYPE A\r\n");
    else if(type ==1)       //image
        Send("TYPE I\r\n");

    //check for a return of 2??
    rt = GetResponseCode(this);

    if(rt == 0)
        return OnError(UTE_NO_RESPONSE);   //no response

    else if(rt >=200 && rt <=299) {
        m_nTransferType = type;
        return OnError(UTE_SUCCESS);
        }
    return OnError(UTE_SVR_REQUEST_DENIED);
}
/***************************************
GetTransferType
    Gets the data transfer type. The data
    representation type used for data transfer and
    storage.  Type implies certain transformations
    between the time of data storage and data transfer.
    The representation types defined in FTP are described
    in the Section on Establishing Data Connections. of RFC 959
    NOTE that we only supporting asscii and image types.
PARAM
    none
Return
    type
    0 - :ascii
        "
        This is the default type and must be accepted by all FTP
         implementations.  It is intended primarily for the transfer of
         text files, except when both Hosts would find the EBCDIC type
         more convenient.
         The sender converts the data from his internal character
         representation to the standard 8-bit NVT-ASCII representation
         (see the TELNET specification).  The receiver will convert the
         data from the standard form to his own internal form.
         In accordance with the NVT standard, the <CRLF> sequence should
         be used, where necessary, to denote the end of a line of text.
         (See the discussion of file structure at the end of the Section
         on Data Representation and Storage).
         Using the standard NVT-ASCII representation means that data
         must be interpreted as 8-bit bytes." RFC 959

    1 - :image
        " The data are sent as contiguous bits which, for transfer, are
         packed into transfer bytes of the size specified in the BYTE
         command.  The receiving site must store the data as contiguous
         bits.  The structure of the storage system might necessitate
         the padding of the file (or of each record, for a
         record-structured file) to some convenient boundary (byte, word
         or block).  This padding, which must be all zeroes, may occur
         only at the end of the file (or at the end of each record) and
         there must be a way of identifying the padding bits so that
         they may be stripped off if the file is retrieved.  The padding
         transformation should be well publicized to enable a user to
         process a file at the storage site.
         Image type is intended for the efficient storage and retrieval
         of files and for the transfer of binary data.  It is
         recommended that this type be accepted by all FTP
         implementations. " RFC 959
****************************************/
int CUT_FTPClient::GetTransferType() const
{
    return m_nTransferType;
}
/***************************************
SetTransferMode
    Sets the data transfer mode in which data is
    to be transferred via the data  connection.
    The mode defines the data format during transfer
    including EOR and EOF.
    The transfer modes defined in FTP are as described
    below in the param section
Params
    type
     0   -:stream
        "The data is transmitted as a stream of bytes.  There is no
         restriction on the representation type used; record structures
         are allowed, in which case the transfer byte size must be at
         least 3 bits!
         In a record structured file EOR and EOF will each be indicated
         by a two-byte control code of whatever byte size is used for
         the transfer.  The first byte of the control code will be all
         ones, the escape character.  The second byte will have the low
         order bit on and zeroes elsewhere for EOR and the second low
         order bit on for EOF; that is, the byte will have value 1 for
         EOR and value 2 for EOF.  EOR and EOF may be indicated together
         on the last byte transmitted by turning both low order bits on,
         i.e., the value 3.  If a byte of all ones was intended to be
         sent as data, it should be repeated in the second byte of the
         control code.
         If the file does not have record structure, the EOF is
         indicated by the sending Host closing the data connection and
         all bytes are data bytes."
    1  -:block
        " The file is transmitted as a series of data blocks preceded by
         one or more header bytes.  The header bytes contain a count
         field, and descriptor code.  The count field indicates the
         total length of the data block in bytes, thus marking the
         beginning of the next data block (there are no filler bits).
         The descriptor code defines:  last block in the file (EOF) last
         block in the record (EOR), restart marker (see the Section on
         Error Recovery and Restart) or suspect data (i.e., the data being
         transferred is suspected of errors and is not reliable).
         This last code is NOT intended for error control within FTP.
         It is motivated by the desire of sites exchanging certain types
         of data (e.g., seismic or weather data) to send and receive all
         the data despite local errors (such as "magnetic tape read
         errors"), but to indicate in the transmission that certain
         portions are suspect).  Record structures are allowed in this
         mode, and any representation type may be used.  There is no
         restriction on the transfer byte size. "
    2  -:compressed
        "The file is transmitted as series of bytes of the size
         specified by the BYTE command.  There are three kinds of
         information to be sent:  regular data, sent in a byte string;
         compressed data, consisting of replications or filler; and
         control information, sent in a two-byte escape sequence.  If
         the byte size is B bits and n>0 bytes of regular data are sent,
         these n bytes are preceded by a byte with the left-most bit set
         to 0 and the right-most B-1 bits containing the number n."
         FOR MORE INFORMATION SEE RFC 959
Return
    UTE_SUCCESS                 - success
    UTE_NO_RESPONSE             - no response
    UTE_SVR_REQUEST_DENIED      - request denied by server
    UTE_PARAMETER_INVALID_VALUE - invalid type
****************************************/
int CUT_FTPClient::SetTransferMode(int mode){

    int     rt;

    //check for a valid range
    if(mode <0 || mode >3)
        return OnError(UTE_PARAMETER_INVALID_VALUE);

    //send the mode info
    if(mode ==0)        //stream
        Send("MODE S\r\n");
    else if(mode ==1)       //block
        Send("MODE B\r\n");
    else if(mode ==2)       //compressed
        Send("MODE C\r\n");

    //check for a return of 2??
    rt = GetResponseCode(this);
    if(rt == 0)
        return OnError(UTE_NO_RESPONSE);   //no response

    else if(rt >=200 && rt <=299) {
        m_nTransferMode  = mode;
        return OnError(UTE_SUCCESS);
        }

    return OnError(UTE_SVR_REQUEST_DENIED);
}
/***************************************
GetTransferMode
    Gets the data transfer mode in which data is
    to be transferred via the data  connection.
    The mode defines the data format during transfer
    including EOR and EOF.
    The transfer modes defined in FTP are as described
    below in the param section
Params
    none
Return
    type
     0   -:stream
        "The data is transmitted as a stream of bytes.  There is no
         restriction on the representation type used; record structures
         are allowed, in which case the transfer byte size must be at
         least 3 bits!
         In a record structured file EOR and EOF will each be indicated
         by a two-byte control code of whatever byte size is used for
         the transfer.  The first byte of the control code will be all
         ones, the escape character.  The second byte will have the low
         order bit on and zeroes elsewhere for EOR and the second low
         order bit on for EOF; that is, the byte will have value 1 for
         EOR and value 2 for EOF.  EOR and EOF may be indicated together
         on the last byte transmitted by turning both low order bits on,
         i.e., the value 3.  If a byte of all ones was intended to be
         sent as data, it should be repeated in the second byte of the
         control code.
         If the file does not have record structure, the EOF is
         indicated by the sending Host closing the data connection and
         all bytes are data bytes."
    1  -:block
        " The file is transmitted as a series of data blocks preceded by
         one or more header bytes.  The header bytes contain a count
         field, and descriptor code.  The count field indicates the
         total length of the data block in bytes, thus marking the
         beginning of the next data block (there are no filler bits).
         The descriptor code defines:  last block in the file (EOF) last
         block in the record (EOR), restart marker (see the Section on
         Error Recovery and Restart) or suspect data (i.e., the data being
         transferred is suspected of errors and is not reliable).
         This last code is NOT intended for error control within FTP.
         It is motivated by the desire of sites exchanging certain types
         of data (e.g., seismic or weather data) to send and receive all
         the data despite local errors (such as "magnetic tape read
         errors"), but to indicate in the transmission that certain
         portions are suspect).  Record structures are allowed in this
         mode, and any representation type may be used.  There is no
         restriction on the transfer byte size. "
    2  -:compressed
        "The file is transmitted as series of bytes of the size
         specified by the BYTE command.  There are three kinds of
         information to be sent:  regular data, sent in a byte string;
         compressed data, consisting of replications or filler; and
         control information, sent in a two-byte escape sequence.  If
         the byte size is B bits and n>0 bytes of regular data are sent,
         these n bytes are preceded by a byte with the left-most bit set
         to 0 and the right-most B-1 bits containing the number n."
         FOR MORE INFORMATION SEE RFC 959
****************************************/
int CUT_FTPClient::GetTransferMode() const
{
    return m_nTransferMode;
}
/***************************************
SetTransferStructure
    Sets the data transfer structure
Params
    type - 0:file 1:record 2:page
Return
    UTE_SUCCESS                 - success
    UTE_NO_RESPONSE             - no response
    UTE_SVR_REQUEST_DENIED      - request denied by server
    UTE_PARAMETER_INVALID_VALUE - invalid type
****************************************/
int CUT_FTPClient::SetTransferStructure(int structure){
    int     rt;

    //check for a valid range
    if(structure < 0 || structure > 3)
        return OnError(UTE_PARAMETER_INVALID_VALUE);

    //send the mode info
    if(structure ==0)   //file
        Send("STRU F\r\n");
    else if(structure ==1)    //record
        Send("STRU R\r\n");
    else if(structure ==2)    //page
        Send("STRU P\r\n");

    //check for a return of 2??
    rt = GetResponseCode(this);

    if(rt == 0)
        return OnError(UTE_NO_RESPONSE);   //no response

    else if(rt >=200 && rt <=299) {
        m_nTransferStructure = structure;
        return OnError(UTE_SUCCESS);
        }

    return OnError(UTE_SVR_REQUEST_DENIED);
}
/***************************************
GetTransferStructure
    Gets the data transfer structure
Params
    none
Return
    transfer structure type -
                    0 : file
                    1 : record
                    2 : page
****************************************/
int CUT_FTPClient::GetTransferStructure() const
{
    return m_nTransferStructure;
}

/***************************************
SetControlPort
    Sets the default Control port
Params
    port - default 21
Return
    UTE_SUCCESS - success
    UTE_ERROR   - error
****************************************/
int CUT_FTPClient::SetControlPort(int port){
    m_nControlPort = port;
    return UTE_SUCCESS;
}
/***************************************
GetControlPort
    Gets the Control port
Params
    none
Return
    port
****************************************/
int CUT_FTPClient::GetControlPort() const
{
    return m_nControlPort;
}
/***************************************
SetDataPort
    Sets the default data port
Params
    port (0-32000) over 1000 recommended
Return
    UTE_SUCCESS - success
    UTE_ERROR   - error
****************************************/
int CUT_FTPClient::SetDataPort(int port){
    m_nDataPort = port;
    return UTE_SUCCESS;
}
/***************************************
GetDataPort
    Gets the data port
Params
    none
Return
    port (0-32000) over 1000 recommended
****************************************/
int CUT_FTPClient::GetDataPort() const
{
    return m_nDataPort;
}

/**
* level 0: PROT C
* level 1: PROT P
*/
int CUT_FTPClient::SetDataSecure(int level) {
    int rt = 0;

    if (true) { //always send PBSZ
        Send("PBSZ 0\r\n");

        rt = GetResponseCode(this);

        if(rt == 0)
            return OnError(UTE_NO_RESPONSE);        //no response
        if(rt != 200) {
            return OnError(UTE_CONNECT_FAIL_NO_SSL_SUPPORT);        //negative response
        }
    }

    if (level == 1) {
        Send("PROT P\r\n");
    } else if (level == 0) {
        Send("PROT C\r\n");
    } else {
        return OnError(UTE_ERROR);  //unknown level
    }

    m_dataSecLevel = level;

    rt = GetResponseCode(this);

    if(rt == 0)
            return OnError(UTE_NO_RESPONSE);        //no response
    if(rt != 200) {
        return OnError(UTE_CONNECT_FAIL_NO_SSL_SUPPORT);        //negative response
    }

    m_wsData.EnableSSL((level == 1));

    return OnError(UTE_SUCCESS);

}

int CUT_FTPClient::GetDataSecure() {
    return m_dataSecLevel;
}

int CUT_FTPClient::SetDataPortRange(int min, int max) {
    m_nDataPortMin = min;
    m_nDataPortMax = max;

    if (m_nDataPortMin < 1000)
        m_nDataPortMin = 1000;
    if (m_nDataPortMin > 65000)
        m_nDataPortMin = 65000;

    if (m_nDataPortMax < m_nDataPortMin)
        m_nDataPortMax = m_nDataPortMin+1;

    if (m_nDataPortMax > 65001)
        m_nDataPortMax = 65001;

    //avoid division by zero error
    if(m_nDataPortMax == m_nDataPortMin)
        m_nDataPortMax = m_nDataPortMin + 1;

    m_nDataPort = m_nDataPortMin + GetTickCount()%(m_nDataPortMax-m_nDataPortMin);

    return UTE_SUCCESS;
}

int CUT_FTPClient::GetDataPortRange(int * min, int * max) {
    if (!min || !max)
        return UTE_ERROR;

    *min = m_nDataPortMin;
    *max = m_nDataPortMax;

    return UTE_SUCCESS;
}

/***************************************
GetDirInfo
    Retrieves the current directory infomation
    on the currently connected FTP server.
Params
    path                            - If NULL current directory, otherwise of given path
Return
    UTE_SUCCESS                     - success
    UTE_DATAPORT_FAILED             - data port could not be opened
    UTE_PORT_FAILED                 - PORT command failed
    UTE_SVR_DATA_CONNECT_FAILED     - Server failed to connect on data port
    UTE_LIST_FAILED                 - LIST command failed
    UTE_ABORTED                     - aborted
****************************************/
int CUT_FTPClient::GetDirInfo(){
    return GetDirInfo((LPCSTR)NULL);
}
#if defined _UNICODE
int CUT_FTPClient::GetDirInfo(LPCWSTR path){
    if (path == NULL)
        return GetDirInfo((LPCSTR)NULL);
    else
        return GetDirInfo(AC(path));
}
#endif
int CUT_FTPClient::GetDirInfo(LPCSTR path){

    int     rt,loop,len;
    char    addr[32];

    if (m_nFirewallMode)
        return GetDirInfoPASV(path);

    m_wsData.SSLSetReuseSession(SSLGetCurrentSession());

    //open up a data port, if the one requested is busy then
    //increment to the next port, try 50 times then fail
    for(loop=0;loop<128;loop++) {
        if(m_wsData.WaitForConnect((unsigned short)m_nDataPort)==UTE_SUCCESS)
            break;
        m_nDataPort++;
        }

    if(loop==128)
        return OnError(UTE_DATAPORT_FAILED);

    //get the host address
    GetHostAddress( addr,sizeof(addr) );

    //create the port set up string
    len = (int)strlen(addr);
    for(loop=0;loop<len;loop++){
        if(addr[loop]=='.')
            addr[loop] = ',';
    }
    _snprintf(m_szBuf,sizeof(m_szBuf)-1,"PORT %s,%d,%d\r\n",addr,HIBYTE(m_nDataPort),LOBYTE(m_nDataPort));

    //setup the next port number
    m_nDataPort++;
    //if(m_nDataPort > 32000 || m_nDataPort < 0)
    //    m_nDataPort = 10000;
        if(m_nDataPort > m_nDataPortMax || m_nDataPort < m_nDataPortMin)
        m_nDataPort = m_nDataPortMin;

    //send the port command
    Send(m_szBuf);

    //check for a return of 2??
    rt = GetResponseCode(this);
    if(rt < 200 || rt >299){
        //close the connection down
        m_wsData.CloseConnection();
        return OnError(UTE_PORT_FAILED);
        }

    //send the list command
    if (path != NULL)
        _snprintf(m_szBuf,sizeof(m_szBuf)-1,"LIST %s\r\n",path);
    else
        _snprintf(m_szBuf,sizeof(m_szBuf)-1,"LIST\r\n");
    Send(m_szBuf);

    //wait for a connection on the data port
    if(m_wsData.WaitForAccept(15)!= UTE_SUCCESS){  // GW: July 18 the wait Time Out is increased to 15 sec
        //close the connection down
        m_wsData.CloseConnection();
        ClearReceiveBuffer();
        return OnError(UTE_SVR_DATA_CONNECT_FAILED);
        }


      //check for a return of 100 or 200 code
    rt = GetResponseCode(this);
    if(rt < 100 || rt >=300) {
        m_wsData.CloseConnection();
        return OnError(UTE_SVR_DATA_CONNECT_FAILED);
        }

    m_wsData.AcceptConnection();

    //clear the DirInfo linked list
    ClearDirInfo();
    CUT_DIRINFOA * di = NULL;
    BOOL once = TRUE;

    // v4.2 change to eliminate C4127: conditional expression is constant
    for(;;) {
        // Check for abortion flag
        if(IsAborted()) {
            m_wsData.CloseConnection();
            return OnError(UTE_ABORTED);
            }

        //retrive a dir line
        if (m_wsData.ReceiveLine(m_szBuf,sizeof(m_szBuf)) <= 0)
            break;

        CUT_StrMethods::RemoveCRLF(m_szBuf);


        // With out this step the client will assume the field TOTAL (which is provided by some unix servers), It will be assumed
        // as a directory. Misleading the user to think that GetDirInfo returns an extra directory entry
        if ((_strnicmp("total",m_szBuf,5) == 0) && once == TRUE) {
            once = FALSE;
            continue;
            }

        //create the linked list item
        if(di==NULL) {
            di = new CUT_DIRINFOA;
            m_DirInfo = di;
            di->next = NULL;
            }
        else {
            di->next = new CUT_DIRINFOA;
            di = di->next;
            di->next = NULL;
            }

        //parse and store the directory information

        if ( isdigit(m_szBuf[0]))
            GetInfoInDOSFormat( di);
        //  call the dos function
        // end of Dos Format file names
        else   /// Unix  Style
            // Call the unix Format
            GetInfoInUNIXFormat(di);

        //increment the dirinfo count
        m_nDirInfoCount ++;
        }

    //close the connection down
    m_wsData.CloseConnection();

    //check for a return of 2??
    rt = GetResponseCode(this);
    if (rt < 200 || rt >= 300) {
        if (rt <200) {
            rt = GetResponseCode(this);
            if (rt < 200 || rt >= 300)
                return OnError(UTE_LIST_FAILED);
            else
                return OnError(UTE_SUCCESS);
            }
        return OnError(UTE_LIST_FAILED);
        }
    else
        return OnError(UTE_SUCCESS);
}

/***************************************
GetDirInfoPASV
    Retrieves the current directory infomation
    on the currently connected FTP server.

    This routine duplicates the GetDirInfo()
    member, but in this routine the client
    originates data connections.  Client side
    origination of data connections is
    preferred for use in firewall controlled
    environments.
Params
    none
Return
    UTE_SUCCESS                     - success
    UTE_PORT_FAILED                 - PORT command failed
    UTE_SVR_DATA_CONNECT_FAILED     - Server failed to connect on data port
    UTE_LIST_FAILED                 - LIST command failed
    UTE_PORT_FAILED                 - data port could not be opened
    UTE_CONNECT_TIMEOUT             - connection timeout
    UTE_ABORTED                     - aborted
****************************************/
int CUT_FTPClient::GetDirInfoPASV(LPCSTR path){
    int     error, rt;
    char    responseBuf[100];
    char    *token;
    char    ipAddress[20];
    int     port;
    UINT    loop;

    //send the port command
    Send("PASV\r\n");

    m_wsData.SSLSetReuseSession(SSLGetCurrentSession());


    // we need to get the full IP address and port number from the
    // PASV return line, so that we can originate the data connection.

    //check for a return of 2??
    rt = GetResponseCode(this, responseBuf, sizeof(responseBuf));
    if(rt < 200 || rt >299)
        return OnError(UTE_PORT_FAILED);

    // find the first '(' in the response and then parse out
    // the address supplied by the server
    loop = 0;
    while ( loop < strlen(responseBuf) ) {
        if ( responseBuf[loop] == '(' )
            break;
        loop++;
        }

    if ( loop == strlen(responseBuf) ) // we hit the end of the buffer
        return OnError(UTE_SVR_DATA_CONNECT_FAILED);

    // token out the ipaddress and port string
    token = strtok( &responseBuf[loop], "()" );

    // get the four portions of the ip address
    strcpy(ipAddress, strtok( token, ",)\r\n" ));
    strcat(ipAddress, ".");
    strcat(ipAddress, strtok( NULL, ",)\r\n" ));
    strcat(ipAddress, ".");
    strcat(ipAddress, strtok( NULL, ",)\r\n" ));
    strcat(ipAddress, ".");
    strcat(ipAddress, strtok( NULL, ",)\r\n" ));

    // get the two portions that make up the port number
    port = atoi( strtok(NULL, ",)\r\n") ) * 256;
    port += atoi( strtok(NULL, ",)\r\n") );

    // test the results to ensure everything is OK.
    if ( !IsIPAddress(ipAddress) )
        return OnError(UTE_LIST_FAILED);

    if ( port <= 0 || port > 65535 )
        return OnError(UTE_DATAPORT_FAILED);


    //send the list command, the server will then wait for us to
    // connect on the port it provided in the PASV statement.
    if (path != NULL)
        _snprintf(m_szBuf,sizeof(m_szBuf)-1,"LIST %s\r\n",path);
    else
        _snprintf(m_szBuf,sizeof(m_szBuf)-1,"LIST\r\n");
    Send(m_szBuf);

    // connect to the server supplied port to establish the
    // data connection.
    // connect using a timeout
    if((error = m_wsData.Connect(port, ipAddress, m_nConnectTimeout)) != UTE_SUCCESS) {
        m_wsData.CloseConnection();
        ClearReceiveBuffer();
        return OnError(error);
        }

    //check for a return of 100 or 200 code
    rt = GetResponseCode(this);
    if(rt < 100 || rt >=300) {
        m_wsData.CloseConnection();
        return OnError(UTE_SVR_DATA_CONNECT_FAILED);
        }

    BOOL once = TRUE;
    //clear the DirInfo linked list
    ClearDirInfo();
    CUT_DIRINFOA * di = NULL;

    // v4.2 change to eliminate C4127: conditional expression is constant
    for (;;) {
        // Check for abortion flag
        if(IsAborted()) {
            m_wsData.CloseConnection();
            return OnError(UTE_ABORTED);
            }

        //retrive a dir line
        if (m_wsData.ReceiveLine(m_szBuf,sizeof(m_szBuf),m_wsData.GetReceiveTimeOut ()/1000) <= 0)
            break;

        CUT_StrMethods::RemoveCRLF(m_szBuf);

        // GW:
        // With out this step the client will assume the field TOTAL (which is provided by some unix servers), It will be assumed
        // as a directory. Misleading the user to think that GetDirInfo returns an extra directory entry
        //Added on July 22nd 1998
        if ((_strnicmp("total",m_szBuf,5) == 0) && once == TRUE) {
            once = FALSE;
            continue;
            }

        //create the linked list item
        if(di==NULL) {
            di = new CUT_DIRINFOA;
            m_DirInfo = di;
            di->next = NULL;
            }
        else {
            di->next = new CUT_DIRINFOA;
            di = di->next;
            di->next = NULL;
            }

        //parse and store the directory information

        //filename
        if ( isdigit(m_szBuf[0]))  //  call the dos function
            GetInfoInDOSFormat(di);
            // end of Dos Format file names
        else   /// Unix  Style
        // Call the unix Format
            GetInfoInUNIXFormat( di);

        //increment the dirinfo count
        m_nDirInfoCount ++;
        }

    //close the connection down
    m_wsData.CloseConnection();

    //check for a return of 2??
    rt = GetResponseCode(this);
    if (rt < 200 || rt >= 300) {
        if (rt <200) {
            rt = GetResponseCode(this);
            if (rt < 200 || rt >= 300)
                return OnError(UTE_LIST_FAILED);
            else
                return OnError(UTE_SUCCESS);
            }
        return OnError(UTE_LIST_FAILED);
        }
    else
        return OnError(UTE_SUCCESS);
}
/***************************************
GetDirInfoCount
    Returns the number of directory entries
    retrieved during the last call to
    GetDirInfo
Params
    none
Return
    The number of directory entries
****************************************/
int CUT_FTPClient::GetDirInfoCount() const
{
    return m_nDirInfoCount;
}

/***************************************
GetDirEntry
    Returns a directory entry name using a
    zero based index value
Params
    index - 0 based index 0 to count-n
    entry - buffer where the name is returned
    maxlen - length of the buffer
Return
    UTE_SUCCESS     - success
    UTE_ERROR       - invalid index
****************************************/
#if defined _UNICODE
int CUT_FTPClient::GetDirEntry(int index, LPWSTR entry, int maxlen) {
    char * entryA = (char*) alloca(maxlen);
    *entryA = '\0';
    int result = GetDirEntry(index, entryA, maxlen);
    if(result == UTE_SUCCESS) {
        CUT_Str::cvtcpy(entry, maxlen, entryA);
    }
    return result;}
#endif
int CUT_FTPClient::GetDirEntry(int index, LPSTR entry, int maxlen) {

    CUT_DIRINFOA     *di   = m_DirInfo;
    int             count = 0;

    //check for a valid range
    if(index < 0 || index >= m_nDirInfoCount)
        return OnError(UTE_ERROR);

    //find the correct record
    while(count < index) {
        if(di->next == NULL)
            break;
        di = di->next;
        count++;
        }

    //copy the record
    maxlen--;
    strncpy(entry, di->fileName, maxlen);
    entry[maxlen] = 0;

    return OnError(UTE_SUCCESS);
}
/***************************************
GetDirEntry
    Returns a directory entry into a
    CUT_DIRINFO structure. The structure
    contains all of the information on a
    directory entry.
Params
    index - 0 based index 0 to count-n
    dirInfo - structure where the directory
     information is copied into
    maxlen - length of the buffer
Return
    UTE_SUCCESS             - success
    UTE_INDEX_OUTOFRANGE    - invalid index
****************************************/
int CUT_FTPClient::GetDirEntry(int index, CUT_DIRINFO *dirInfo) {

    CUT_DIRINFOA    *di   = m_DirInfo;
    int             count = 0;

    //check for a valid range
    if(index < 0 || index >= m_nDirInfoCount)
        return OnError(UTE_INDEX_OUTOFRANGE);

    //find the correct record
    while(count <index) {
        if(di->next == NULL)
            return OnError(UTE_INDEX_OUTOFRANGE);
        di = di->next;
        count++;
        }

    //copy the record, switching from char to _TCHAR for filename
    //use CP_UTF8 to transparently handle servers which send uft8 filenames
    //also not explicitly requested via OPTS UTF8 ON, see https://tools.ietf.org/html/draft-ietf-ftpext-utf-8-option-00
    //see also issue https://github.com/ashkulz/NppFTP/issues/55
    CUT_Str::cvtcpy(dirInfo->fileName,MAX_PATH, di->fileName, CP_UTF8);
    dirInfo->fileSize   = di->fileSize;
    dirInfo->day        = di->day;
    dirInfo->month      = di->month;
    dirInfo->year       = di->year;
    dirInfo->hour       = di->hour;
    dirInfo->minute     = di->minute;
    dirInfo->isDir      = di->isDir;

    return OnError(UTE_SUCCESS);
}
/***************************************
GetHelp
    Returns help information from the
    currently connected server. Once this
    command completes successfully then
    the information can be retrieved using
    the GetMultiLineResponse function.
Params
    param - the command to retrieve help on
        if "" is sent in then help on all
        commands is sent back
Return
    UTE_SUCCESS             - success
    UTE_NO_RESPONSE         - no response
    UTE_SVR_REQUEST_DENIED  - request denied by server
****************************************/
#if defined _UNICODE
int CUT_FTPClient::GetHelp(LPCWSTR param) {
    return GetHelp(AC(param));}
#endif
int CUT_FTPClient::GetHelp(LPCSTR param) {

    // clear response list
    m_listResponse.ClearList();

    //send the help command
    if(param == NULL)
        _snprintf(m_szBuf,sizeof(m_szBuf)-1,"HELP\r\n");
    else if(param[0] == 0)
        _snprintf(m_szBuf,sizeof(m_szBuf)-1,"HELP\r\n");
    else
        _snprintf(m_szBuf,sizeof(m_szBuf)-1,"HELP %s\r\n",param);

    Send(m_szBuf);

    //check for a return of 2??
    int rt = GetResponseCode(this);

    if(rt == 0)
        return OnError(UTE_NO_RESPONSE);   //no response

    else if(rt >=0 && rt <=299)
        return OnError(UTE_SUCCESS);

    return OnError(UTE_SVR_REQUEST_DENIED);
}
/***************************************
GetResponseCode(CUT_WSClient *ws,LPSTR string,int maxlen)
    A reply (ResponseCode) is an acknowledgment (positive or negative) sent from
    server to user via the TELNET connections in response to FTP
    commands.  The general form of a reply is a completion code
    (including error codes) followed by a text string.  The codes
    are for use by programs and the text is usually intended for
    human users
    "   The server sends FTP replies over the TELNET connection in response
   to user FTP commands.  The FTP replies constitute the acknowledgment
   or completion code (including errors).  The FTP-server replies are
   formatted for human or program interpretation.  Single line replies
   consist of a leading three-digit numeric code followed by a space,
   followed by a one-line text explanation of the code.  For replies
   that contain several lines of text, the first line will have a
   leading three-digit numeric code followed immediately by the
   character "-" (Hyphen, ASCII code 45), and possibly some text.  All
   succeeding continuation lines except the last are constrained NOT to
   begin with three digits; the last line must repeat the numeric code
   of the first line and be followed immediately by a space.  For   example:
          100-First Line
          Continuation Line
          Another Line
          100 Last Line
The assigned reply codes relating to FTP are:
    000  Announcing FTP.
    010  Message from system operator.
    020  Exected delay.
   030  Server availability information.
   050  FTP commentary or user information.
   100  System status reply.
   110  System busy doing...
   150  File status reply.
   151  Directory listing reply.
   200  Last command received correctly.
   201  An ABORT has terminated activity, as requested.
   202  Abort request ignored, no activity in progress.
   230  User is "logged in".  May proceed.
   231  User is "logged out".  Service terminated.
   232  Logout command noted, will complete when transfer done.
   233  User is "logged out".  Parameters reinitialized.
   250  FTP file transfer started correctly.
   251  FTP Restart-marker reply.
      Text is:  MARK yyyy = mmmm
         where 'yyyy' is user's data stream marker (yours)
         and mmmm is server's equivalent marker (mine)
      (Note the spaces between the markers and '=').
   252  FTP transfer completed correctly.
   253  Rename completed.
   254  Delete completed.
   257  Closing the data connection, transfer completed.
   300  Connection greeting message, awaiting input.
   301  Current command incomplete (no <CRLF> for long time).
   330  Enter password
   331  Enter account (if account required as part of login sequence).
   332  Login first, please.
   400  This service not implemented.
   401  This service not accepting users now, goodbye.
   402  Command not implemented for requested value or action.
   430  Log-on time or tries exceeded, goodbye.
   431  Log-on unsuccessful.  User and/or password invalid.
   432  User not valid for this service.
   433  Cannot transfer files without valid account.  Enter account and
        resend command.
   434  Log-out forced by operator action.  Phone site.
   435  Log-out forced by system problem.
   436  Service shutting down, goodbye.
   450  FTP:  File not found.
   451  FTP:  File access denied to you.
   452  FTP:  File transfer incomplete, data connection closed.
   453  FTP:  File transfer incomplete, insufficient storage space.
   454  FTP:  Cannot connect to your data socket.
   455  FTP:  File system error not covered by other reply codes.
   456  FTP:  Name duplication; rename failed.
   457  FTP:  Transfer parameters in error.
   500  Last command line completely unrecognized.
   501  Syntax of last command is incorrect.
   502  Last command incomplete, parameters missing.
   503  Last command invalid (ignored), illegal parameter combination.
   504  Last command invalid, action not possible at this time.
   505  Last command conflicts illegally with previous command(s).
   506  Last command not implemented by the server.
   507  Catchall error reply.
   550  Bad pathname specification (e.g., syntax error).
    " SEE ALSO RFC 542 & 959
PARAM
    CUT_WSClient *ws - the connection class be it the data connection or the Command connection
RETURN
    int - The Server reply code
****************************************/
int CUT_FTPClient::GetResponseCode(CUT_WSClient *ws, LPSTR string, int maxlen) {

    if (m_cachedResponse) {
        m_cachedResponse = false;

        //copy the rest of the data
        const char * pbuf = GetMultiLineResponse(0);
        if(string != NULL && pbuf != NULL) {
            maxlen--;
            strncpy(string, &pbuf[4], maxlen);
            string[maxlen - 1]  =0;
            }

        return m_lastResponseCode;
    }

    int code = PeekResponseCode(ws, string, maxlen);
    m_cachedResponse = false;

    return code;
}

int CUT_FTPClient::PeekResponseCode(CUT_WSClient *ws, LPSTR string, int maxlen) {
    char c;
    int  code;
    //int  once = TRUE;
    char mlCode[5];

    m_cachedResponse = true;

    m_listResponse.ClearList();

    if(ws->ReceiveLine(m_szBuf,sizeof(m_szBuf),m_wsData.GetReceiveTimeOut ()/1000) <= 0) {
        m_lastResponseCode = 0;
        return 0;  //no response
    }

    CUT_StrMethods::RemoveCRLF(m_szBuf);

    //get the code to return
    c = m_szBuf[3];
    m_szBuf[3] = 0;
    code = atoi(m_szBuf);
    m_szBuf[3] = c;
    mlCode[0] = m_szBuf[0];
    mlCode[1] = m_szBuf[1];
    mlCode[2] = m_szBuf[2];
    mlCode[3] = ' ';
    mlCode[4] = 0;

    //check for a multi-line response
    if(c =='-') {
        while(strstr(m_szBuf,mlCode) != m_szBuf) {

            //clear the multi-line response list the first time through
            /*if(once) {
                once = FALSE;

                // clear response list
                m_listResponse.ClearList();
                }*/

            m_listResponse.AddString(m_szBuf);

            //get the line
            if(ws->ReceiveLine(m_szBuf,sizeof(m_szBuf),m_wsData.GetReceiveTimeOut ()/1000) <=0)
                break;
            CUT_StrMethods::RemoveCRLF(m_szBuf);
            }

        m_listResponse.AddString(m_szBuf);
        }
    else
        m_listResponse.AddString(m_szBuf);

    strncpy(m_szResponse, m_szBuf, MAX_PATH);
    m_szResponse[MAX_PATH - 1] = '\0';

    //copy the rest of the data
    if(string != NULL) {
        maxlen--;
        strncpy(string, &m_szBuf[4], maxlen);
        string[maxlen - 1]  =0;
        }

    m_lastResponseCode = code;
    return code;
}

/***************************************
ClearDirInfo
    Clears the directory information that
    was stored during the last call to GetDirInfo
PARAM
    NONE
RETURN
    UTE_SUCCESS
****************************************/
int CUT_FTPClient::ClearDirInfo(){

    CUT_DIRINFOA * next;
    CUT_DIRINFOA * current;

    current = m_DirInfo;
    while(current != NULL){
        next = current->next;
        delete current;
        current = next;
    }
    m_DirInfo = NULL;
    m_nDirInfoCount = 0;

    return OnError(UTE_SUCCESS);
}

/***************************************
SetFireWallMode()
    This function, which sets the
    m_nFirewallMode member variable,
    changes the operation of the
    class so that all data connections
    are originated by the client.
    This behavior is required for use
    with some firewall software.
    Firewall software will often restrict
    incoming connections to a corporate
    computer, but will allow outgoing
    connections.
    By default, FTP clients contact the
    server to establish a "control" connection
    and the server calls the client back to
    establish the "data" connection.
    By calling this function the ftp client class
    will originate both connections.
PARAM
    BOOL mode - TRUE or FALSE
****************************************/
void CUT_FTPClient::SetFireWallMode(BOOL mode){
    m_nFirewallMode = mode;
}
/***************************************
GetFireWallMode()
    This function, which gets the
    m_nFirewallMode member variable,
    changes the operation of the
    class so that all data connections
    are originated by the client.
    This behavior is required for use
    with some firewall software.
    Firewall software will often restrict
    incoming connections to a corporate
    computer, but will allow outgoing
    connections.
    By default, FTP clients contact the
    server to establish a "control" connection
    and the server calls the client back to
    establish the "data" connection.
    By calling this function the ftp client class
    will originate both connections.
PARAM
    none
****************************************/
int CUT_FTPClient::GetFireWallMode() const
{
    return m_nFirewallMode;
}
/****************************************************
GetLastResponse()
    Use this function to debug your application and to be
    aware of each response a server sends for each command you
    prompt the server for.
PARAM
    NONE
RETURN
    LPCSTR - the last response string received from the server.
********************************************************/
LPCSTR CUT_FTPClient::GetLastResponse() const
{
    return m_szResponse;
}
/*************************************************
GetLastResponse()
Gets the last response returned from the server
PARAM
response - [out] pointer to buffer to receive response
maxSize  - length of buffer
index    - index response
size     - [out] length of response

  RETURN
  UTE_SUCCES            - ok -
  UTE_NULL_PARAM        - response and/or size is a null pointer
  UTE_INDEX_OUTOFRANGE  - response not found
  UTE_BUFFER_TOO_SHORT  - space in name buffer indicated by maxSize insufficient, realloc
  based on size returned.
  UTE_OUT_OF_MEMORY     - possible in wide char overload
**************************************************/
int CUT_FTPClient::GetLastResponse(LPSTR response, size_t maxSize, size_t *size) {

    int retval = UTE_SUCCESS;

    if(response == NULL || size == NULL) {
        retval = UTE_NULL_PARAM;
    }
    else {

        LPCSTR str = GetLastResponse();

        if(str == NULL) {
            retval = UTE_INDEX_OUTOFRANGE;
        }
        else {
            *size = strlen(str);
            if(*size >= maxSize) {
                ++(*size);
                retval = UTE_BUFFER_TOO_SHORT;
            }
            else {
                strcpy(response, str);
            }
        }
    }
    return retval;
}
#if defined _UNICODE
int CUT_FTPClient::GetLastResponse(LPWSTR response, size_t maxSize, size_t *size) {

    int retval;

    if(maxSize > 0) {
        char * responseA = new char [maxSize];

        if(responseA != NULL) {
            retval = GetLastResponse( responseA, maxSize, size);

            if(retval == UTE_SUCCESS) {
                CUT_Str::cvtcpy(response, maxSize, responseA);
            }
            delete [] responseA;
        }
        else {
            retval = UTE_OUT_OF_MEMORY;
        }
    }
    else {
        if(size == NULL) (retval = UTE_NULL_PARAM);
        else {
            LPCSTR lpStr = GetLastResponse();
            if(lpStr != NULL) {
                *size = strlen(lpStr)+1;
                retval = UTE_BUFFER_TOO_SHORT;
            }
            else {
                retval = UTE_INDEX_OUTOFRANGE;
            }
        }
    }
    return retval;

}
#endif

/***********************************************
GetInfoInDOSFormat()
    Although it is strongly discouraged to use DOS file format
    for the server directory structure. There are still the few
    number of FTP servers sites that provides the names in a DOS format.
    This function will parse the file information details based
    on the DOS format
PARAM:
    CUT_DIRINFO di - the directory information entry to be populated
RETURN
    VOID
**********************************************/
void CUT_FTPClient::GetInfoInDOSFormat( CUT_DIRINFOA * di){
    //parse and store the directory information
     char    buf[32];
    char    bufDos[MAX_PATH+1];
    char    dateBuf[20];
    long    value;

        //parse and store the directory information

        // Get the file name
        int nSpaces = 0, loop = 0;
        while(m_szBuf[loop] != 0) {
            if(m_szBuf[loop] == ' ') {
                ++ nSpaces;
                while(m_szBuf[loop] == ' ')
                    ++ loop;
                }
            else if(nSpaces == 3) {
                strncpy(di->fileName, &m_szBuf[loop], sizeof(di->fileName)-1);
                break;
                }
            else
                ++ loop;
        }

        CUT_StrMethods::ParseString(m_szBuf, " ", 2, bufDos, sizeof(bufDos));

        //directory  attrib
        if(bufDos[1]=='d' || bufDos[1] =='D')
            di->isDir = TRUE;
        else
            di->isDir = FALSE;

        //size portion of the file date
        di->fileSize = 0;
        CUT_StrMethods::ParseString(m_szBuf," ",2,&di->fileSize);


        //month portion of the file date
        di->month = 1;
        CUT_StrMethods::ParseString(m_szBuf," ",0,dateBuf,sizeof(dateBuf));
        CUT_StrMethods::ParseString(dateBuf,"-",0,buf,sizeof(buf));
        //find the month number from the string
        di->month = atoi(buf);
        //day portion of the file date
        di->day =1;
        CUT_StrMethods::ParseString(dateBuf,"-",1,&value);
        di->day = (int)value;
        //year and or hour portion of the file date
        di->year = 1900;
        di->hour = 12;
        di->minute = 0;
        CUT_StrMethods::ParseString(m_szBuf," ",0,dateBuf,sizeof(dateBuf));

        strncpy(dateBuf, &dateBuf[6],2);
        dateBuf[2] = '\0';

        int temp = atoi(dateBuf);
        if( 70 > temp)
            temp+= 100;
        di->year = di->year + temp;
        CUT_StrMethods::ParseString(m_szBuf," ",1,buf,sizeof(buf));

        //get the hour portion of the file date
        CUT_StrMethods::ParseString(buf,":",0,&value);
        di->hour = (int)value;
        //get the minute portion of the file date
        CUT_StrMethods::ParseString(buf,":",1,dateBuf,sizeof(dateBuf));
        if (dateBuf[2] =='P')   //AM or PM Digit
            di->hour +=12;

        //strncpy(dateBuf,&dateBuf[3],2);
        dateBuf[2] = '\0';
        di->minute = atoi(dateBuf);
}
/***********************************************
GetInfoInUNIXFormat()
        This function parses the directory entry information
        based on the UNIX format.
PARAM:
      CUT_DIRINFO di - the directory information entry to be populated
RET:
      VOID
**********************************************/
void CUT_FTPClient::GetInfoInUNIXFormat( CUT_DIRINFOA * di){


    const char *Month[]={"Jan","Feb","Mar","Apr","May","Jun","Jul",
        "Aug","Sep","Oct","Nov","Dec"};

    char        buf[32];
    int         loop;
    time_t      timer;
    struct tm   *tblock;
    long        value;
    int         linksIncluded = 0;
    di->fileName[0] = '\0';

    // Get the file name
    int nSpaces = 0;
    loop = 0;

    // check if the links or blocks attribute is included in the server answer
    if (CUT_StrMethods::GetParseStringPieces (m_szBuf," ") > 8)
        linksIncluded = 0 ;
    else
        linksIncluded = -1;

    while(m_szBuf[loop] != 0) {
        if(m_szBuf[loop] == ' ') {
            ++ nSpaces;
            int spaceCounter = 0;
            while(m_szBuf[loop] == ' ')
            {
                spaceCounter++;
                ++ loop;
            }

            }
        else if(nSpaces == 8 +linksIncluded) {
            strncpy(di->fileName, &m_szBuf[loop], sizeof(di->fileName)-1);
            break;
            }
        else
            ++ loop;
    }


    //directory  attrib
    if(m_szBuf[0]=='d' || m_szBuf[0] =='D')
        di->isDir = TRUE;
    else if (m_szBuf[0]=='l' || m_szBuf[0] =='L')
        di->isDir = 2;  //WARNING: HACK!
    else
        di->isDir = FALSE;

    //file size
    di->fileSize = 0;
    CUT_StrMethods::ParseString(m_szBuf," ",4+linksIncluded,&di->fileSize);

    //month portion of the file date
    di->month = 1;
    CUT_StrMethods::ParseString(m_szBuf," ",5+linksIncluded,buf,sizeof(buf));

    //find the month number from the string
    for(loop=0;loop<12;loop++) {
        if(_stricmp(buf,Month[loop])==0) {
            di->month = loop+1;
            break;
            }
        }

    //day portion of the file date
    di->day =1;
    CUT_StrMethods::ParseString(m_szBuf," ",6+linksIncluded,&value);
    di->day = (int)value;

    //year and or hour portion of the file date
    di->year = 1900;        // a unix type of ls -l will give a year or a time - not both
    di->hour = 00;          // we default to current year (below) and 12:00AM
    di->minute = 00;
    CUT_StrMethods::ParseString(m_szBuf," ",7+linksIncluded,buf,sizeof(buf));

    //check to see if it is a time
    if(strstr(buf,":") != NULL) {
        //get the current year
        timer = time(NULL);                 // default to current year...
        tblock = localtime(&timer);
        di->year = tblock->tm_year+1900;
        //get the hour
        CUT_StrMethods::ParseString(buf,":",0,&value);
        di->hour = (int)value;
        // So all of the time shown is
        //get the minute
        CUT_StrMethods::ParseString(buf,":",1,&value);
        di->minute = (int)value;
        // So all of the time shown is
        }
    else
        di->year = atoi(buf);

}

/***********************************************
GetMultiLineResponseLineCount
      Returns a number of lines in the multiline
      response list.
PARAM:
      none
RET:
      number of lines
**********************************************/
LONG CUT_FTPClient::GetMultiLineResponseLineCount() const
{
    return m_listResponse.GetCount();
}
/***********************************************
GetMultiLineResponse
      Returns a line from the multiline response
      list.
PARAM:
      index     - index of the line
RET:
      pointer to line or NULL
**********************************************/
LPCSTR CUT_FTPClient::GetMultiLineResponse(int index) const
{
    return m_listResponse.GetString(index);
}
/*************************************************
GetMultiLineResponse()
Gets the one line of a multiline response returned from the server (e.g. a response to Connect)
PARAM
response - [out] pointer to buffer to receive response
maxSize  - length of buffer
index    - index response
size     - [out] length of response

  RETURN
  UTE_SUCCES            - ok -
  UTE_NULL_PARAM        - response and/or size is a null pointer
  UTE_INDEX_OUTOFRANGE  - response not found
  UTE_BUFFER_TOO_SHORT  - space in name buffer indicated by maxSize insufficient, realloc
  based on size returned.
  UTE_OUT_OF_MEMORY     - possible in wide char overload
**************************************************/
int CUT_FTPClient::GetMultiLineResponse(LPSTR response, size_t maxSize, int index, size_t *size) {

    int retval = UTE_SUCCESS;

    if(response == NULL || size == NULL) {
        retval = UTE_NULL_PARAM;
    }
    else {

        LPCSTR str = GetMultiLineResponse(index);

        if(str == NULL) {
            retval = UTE_INDEX_OUTOFRANGE;
        }
        else {
            *size = strlen(str);
            if(*size >= maxSize) {
                ++(*size);
                retval = UTE_BUFFER_TOO_SHORT;
            }
            else {
                strcpy(response, str);
            }
        }
    }
    return retval;
}
#if defined _UNICODE
int CUT_FTPClient::GetMultiLineResponse(LPWSTR response, size_t maxSize, int index, size_t *size) {

    int retval;

    if(maxSize > 0) {
        char * responseA = new char [maxSize];

        if(responseA != NULL) {
            retval = GetMultiLineResponse( responseA, maxSize, index, size);

            if(retval == UTE_SUCCESS) {
                CUT_Str::cvtcpy(response, maxSize, responseA);
            }
            delete [] responseA;
        }
        else {
            retval = UTE_OUT_OF_MEMORY;
        }
    }
    else {
        if(size == NULL) (retval = UTE_NULL_PARAM);
        else {
            LPCSTR lpStr = GetMultiLineResponse(index);
            if(lpStr != NULL) {
                *size = strlen(lpStr)+1;
                retval = UTE_BUFFER_TOO_SHORT;
            }
            else {
                retval = UTE_INDEX_OUTOFRANGE;
            }
        }
    }
    return retval;
}
#endif

/***************************************************
ReceiveFileStatus
    This virtual function is called during a
    ReceiveToFile function.
Params
    bytesReceived - number of bytes received so far
Return
    TRUE - allow the receive to continue
    FALSE - abort the receive
****************************************************/
BOOL CUT_FTPClient::ReceiveFileStatus(long /* bytesReceived */){
    return !IsAborted();
}
/***************************************************
SendFileStatus
    This virtual function is called during a
    SendFile function.
Params
    bytesSent - number of bytes sent so far
Return
    TRUE - allow the send to continue
    FALSE - abort the send
****************************************************/
BOOL CUT_FTPClient::SendFileStatus(long /* bytesSent */){
    return !IsAborted();
}

/*********************************************
SetConnectTimeout
    Sets the time to wait for a connection
    in seconds
    5 seconds is the default time
Params
    secs - seconds to wait
  Return
    UTE_SUCCESS - success
    UTE_ERROR   - invalid input value
**********************************************/
int CUT_FTPClient::SetConnectTimeout(int secs){

    if(secs <= 0)
        return OnError(UTE_ERROR);

    m_nConnectTimeout = secs;

    return OnError(UTE_SUCCESS);
}
/*********************************************
GetConnectTimeout
    Gets the time to wait for a connection
    in seconds
Params
    none
Return
    current time out value in seconds
**********************************************/
int CUT_FTPClient::GetConnectTimeout() const
{
    return m_nConnectTimeout;
}
/***************************************
Quote(LPCSTR command)
    sends a custom command to the server
    custom command can be any valid FTP command or
    any server specific command.
    To see the server response call GetMultiResponseLine()
Params
    command - the command to Send to the server
Return
    UTE_SUCCESS             - success
    UTE_NO_RESPONSE         - no response
    sending error code
****************************************/
#if defined _UNICODE
int CUT_FTPClient::Quote(LPCWSTR command) {
    return Quote(AC(command));}
#endif
int CUT_FTPClient::Quote(LPCSTR command) {

    m_szResponse[0]         = '\0';
    if (!command || strlen(command) <1)
        return OnError(UTE_QUOTE_LINE_IS_EMPTY);
    // clear response list
    m_listResponse.ClearList();
    int rt = 0;
    SendAsLine(command, (int)strlen(command),256);
    // we will leave the server error messages to be handled by the developer
    rt = GetResponseCode(this);

    if(rt == 0)
        return OnError(UTE_NO_RESPONSE);   //no response
    return OnError(UTE_SUCCESS);
}


/**************************************************************
SocketOnConnected(SOCKET s, const char *lpszName)

        If the security is enabled then perform te SSL neogotiation
        otherwise just return a success and let the plain text FTP handles
        the comunication

        To let the server know that we are looking for SSL or TLS we need to
        send the following command
        FEAT
        The Feature negotiation mechanism for the File Transfer Protocol

        To ask the server for SSL or TLS negotiation
        we will send the AUTH command.

        A parameter for the AUTH command to indicate that TLS is
        required.  It is recommended that 'TLS', 'TLS-C', 'SSL' and 'TLS-P'
        are acceptable, and mean the following :-

         'TLS' or 'TLS-C' - the TLS protocol or the SSL protocol will be
            negotiated on the control connection.  The default protection
            setting for the Data connection is 'Clear'.

            'SSL' or 'TLS-P' - the TLS protocol or the SSL protocol will be
            negotiated on the control connection.  The default protection
            setting for the Data connection is 'Private'.  This is primarily
            for backward compatibility.

    Notice that we will first send a TLS P to select The highest implementation.
    the server might response with the response
    503 unknown security mechanism

    if this happened we will issue an Auth SSL command.

Param:
    SOCKET s        - The newly created socket
    lpszName        - apointer to the host name we are attempting to connect to


Return:
    UTE_NO_RESPONSE - Server did not response to our command
    UTE_CONNECT_FAIL_NO_SSL_SUPPORT - Server does not support SSL.
    UTE_CONNECT_FAILED  -    The connection have failed
    security  errors   - This function may fail with other error
            UTE_LOAD_SECURITY_LIBRARIES_FAILED
            UTE_OUT_OF_MEMORY
            UTE_FAILED_TO_GET_SECURITY_STREAM_SIZE
            UTE_OUT_OF_MEMORY
            UTE_FAILED_TO_QUERY_CERTIFICATE
            UTE_NULL_PARAM
            UTE_PARAMETER_INVALID_VALUE
            UTE_FAILED_TO_GET_CERTIFICATE_CHAIN
            UTE_FAILED_TO_VERIFY_CERTIFICATE_CHAIN
            UTE_FAILED_TO_VERIFY_CERTIFICATE_TRUST

**************************************************************/
int CUT_FTPClient::SocketOnConnected(SOCKET /*s*/, const char * /*lpszName*/){

    int rt = UTE_SUCCESS;

    bool performAuth = (m_sMode == FTPES);
    bool performProt = (m_sMode != FTP);

    m_dataSecLevel = performProt?1:0;   //do not call the function yet, let FTPConnect handle that after authentication

    if (m_sMode == FTPS) {  //just connect ssl
        rt = ConnectSSL();
        if (rt != UTE_SUCCESS)
            return OnError(rt);
    }

    {
        // if the security is enabled then
        // Attempt to send the auth command
        rt = GetResponseCode(this);

        if(rt == 0)
            return OnError(UTE_NO_RESPONSE);     //no response

        if(rt < 200 || rt > 399)
            return OnError(UTE_CONNECT_FAILED);      //negative response


        /*
        // Check for abortion flag
        if(IsAborted())
        {
            return OnError(UTE_ABORTED);
        }
        */
    }

    if (performAuth)
    {
        Send("AUTH TLS\r\n");                                   // Send the TLS negotiation command

        rt = GetResponseCode(this);

        if(rt == 0)
            return OnError(UTE_NO_RESPONSE);                    //no response

        // check the response
        if(rt < 200 || rt > 399)
        {
            Send("AUTH SSL\r\n");                               // Send the TLS negotiation command

            rt = GetResponseCode(this);

            if(rt == 0)
                return OnError(UTE_NO_RESPONSE);        //no response

            if(rt < 200 || rt > 399)
            {
                return OnError(UTE_CONNECT_FAIL_NO_SSL_SUPPORT);        //negative response
            }
            else                                                                // If the SSL succeded then set the protocol to SSL
            {
                SetSecurityMode(CUT_WSClient::TLS);
                m_wsData.SetSecurityMode(CUT_WSClient::TLS);
                rt = ConnectSSL();
            }
        }
        else
        {
            //TLS is the default starting with TLS 1.3 -> SSLV3
            SetSecurityMode(CUT_WSClient::TLS);
            m_wsData.SetSecurityMode(CUT_WSClient::TLS);
            rt = ConnectSSL();
        }

        if (rt != UTE_SUCCESS)
            return OnError(rt);
    }

    return OnError(UTE_SUCCESS);
}
