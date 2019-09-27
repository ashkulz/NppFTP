//=================================================================
//  class: CUT_WSClient
//  File:  Ut_clnt.cpp
//
//  Purpose:
//
//  Implementation of the main Client Winsock class
//  (the Main engine, and the building block) of the
//  Ultimate TCP/IP Library.
//
//  This class encapsulates the functionality of winsock.dll in order to give
//  the user a friendly, and object oriented framework for Winsock programming
//
// ===================================================================
// Ultimate TCP/IP v4.2
// This software along with its related components, documentation and files ("The Libraries")
// is © 1994-2007 The Code Project (1612916 Ontario Limited) and use of The Libraries is
// governed by a software license agreement ("Agreement").  Copies of the Agreement are
// available at The Code Project (www.codeproject.com), as part of the package you downloaded
// to obtain this file, or directly from our office.  For a copy of the license governing
// this software, you may contact us at legalaffairs@codeproject.com, or by calling 416-849-8900.
// ===================================================================

/*
NppFTP:
Modification made April 2010:
-Replaced existing secure functionality with OpenSSL functionality
*/

#ifdef _WINSOCK_2_0_
    #define _WINSOCKAPI_    /* Prevent inclusion of winsock.h in windows.h   */
                            /* Remove this line if you are using WINSOCK 1.1 */
#endif

#include "stdafx.h"

#include <stdio.h>

#include "ut_clnt.h"

#include "ut_strop.h"


#ifdef __BORLANDC__
    #include "mem.h"
#else
    #include "memory.h"
#endif


/***********************************************
Constructor
************************************************/
CUT_WSClient::CUT_WSClient() :

    m_nRemotePort(0),                   // Initialize the remote port
    m_nLocalPort(0),                    // Initialize the local port
    m_nAcceptPort(0),                   // Initialize the accept port
    m_nProtocol(0),                     // Initialize protocol
    m_nFamily(2),                       // Initialize socket family
    m_nSockType(1),                     // Initialize socket type
    m_socket(INVALID_SOCKET),           // Initialize socket
    m_serverSocket(INVALID_SOCKET),     // Initialize server socket
    m_nBlockingMode(CUT_BLOCKING),      // Default mode - BLOCKING
    m_nSuccessful(TRUE),                // Set Windows Socket DLL initialization flag to TRUE
    m_hAsyncWnd(NULL),                      // Initialize window handle with NULL
    m_lSendTimeOut(30000),              // Set default Send Time Out value
    m_lRecvTimeOut(30000),              // Set default Receive Time Out value

    m_isSSL(false),
    m_SSLconnected(false),
    m_sslMode(NONE),

    m_meth(NULL),
    m_ctx(NULL),
    m_ssl(NULL),
    m_reuseSession(NULL)

{
    static bool isSSLInit = false;
    if (!isSSLInit) {
        SSL_library_init();
        SSL_load_error_strings();
        //ERR_load_BIO_strings();
        OpenSSL_add_all_algorithms();
        isSSLInit = true;
    }

    // Initialize address array
    m_szAddress[0]    = 0;

    //initialize the Windows Socket DLL
    if( WSAStartup(WINSOCKVER, &m_data) != 0 )
        m_nSuccessful = FALSE;

    //set up the default send are receive time-outs
    SetReceiveTimeOut(m_lRecvTimeOut);
    SetSendTimeOut(m_lSendTimeOut);

}
/***********************************************
Destructor
************************************************/
CUT_WSClient::~CUT_WSClient(){

    CloseConnection();              // Close any open connection

    WSACleanup();                   // Shut down the winsock DLL

    if(m_hAsyncWnd != NULL)             // Destory the async window
        DestroyWindow(m_hAsyncWnd);

}
/***********************************************
Connect
    Connects to a specified port
Params
    port        - port to connect to
    address     - address to connect to (ex."204.64.75.73")
    [timeout]   - time to wait for connection
    [family]    - protocol family: AF_INET, AF_AF_IPX, etc. Default AF_INET
    [sockType]  - SOCK_STREAM (TCP) or SOCK_DGRAM (UDP) Default is SOCK_STREAM
Return
    UTE_SOCK_ALREADY_OPEN   - socket already open or in use
    UTE_SOCK_CREATE_FAILED  - socket creation failed
    UTE_SOCK_CONNECT_FAILED - socket connection failed
    UTE_INVALID_ADDRESS     - invalid address
    UTE_SUCCESS             - success
    UTE_CONNECT_TIMEOUT     - connect time out
************************************************/
#if defined _UNICODE
int CUT_WSClient::Connect(unsigned int port, LPCWSTR address, long timeout, int family, int sockType){
    return Connect(port, AC(address), timeout, family, sockType);}
#endif
int CUT_WSClient::Connect(unsigned int port, LPCSTR address, long timeout, int family, int sockType)
{
    int nError = UTE_SUCCESS;

    if(m_socket != INVALID_SOCKET)
        return OnError(UTE_SOCK_ALREADY_OPEN);

    //copy the params
    //check to see if the domain is a name or address
    if(IsIPAddress(address) != TRUE) {
        if(GetAddressFromName(address, m_szAddress, sizeof(m_szAddress)) != UTE_SUCCESS)
            return OnError(UTE_INVALID_ADDRESS);
        }
    else
        strncpy(m_szAddress, address, sizeof(m_szAddress));

    m_nFamily    = family;
    m_nSockType  = sockType;

    //Set up the SockAddr structure
    memset(&m_sockAddr, 0, sizeof(m_sockAddr));             //clear all
    m_sockAddr.sin_port         = (unsigned short) htons((u_short   )port);             //port
    m_sockAddr.sin_family       = (short) family;                   //family
    m_sockAddr.sin_addr.s_addr  = inet_addr(m_szAddress);   //address

    //create a socket
    if(CreateSocket(m_socket, family,sockType) == UTE_ERROR)
        return OnError(UTE_SOCK_CREATE_FAILED);         // ERROR: socket unsuccessful

    // switch to nonblocking mode to support timeout
    if(timeout >= 0)
        SetBlockingMode(CUT_NONBLOCKING);

    if( connect(m_socket,(LPSOCKADDR)&m_sockAddr,sizeof(m_sockAddr))==SOCKET_ERROR){
        if(WSAGetLastError() == WSAEWOULDBLOCK ) {
            if(timeout >= 0) {
                if(WaitForSend(timeout, 0) != CUT_SUCCESS) {
                    SocketClose(m_socket);
                    m_socket = INVALID_SOCKET;
                    return OnError(UTE_CONNECT_TIMEOUT);
                    }

                SetBlockingMode(CUT_BLOCKING);
                }

            //set up the default send are receive time-outs
            SetReceiveTimeOut(m_lRecvTimeOut);
            SetSendTimeOut(m_lSendTimeOut);

            // save the remote port
            m_nRemotePort = ntohs(m_sockAddr.sin_port);

            // save the local port
            SOCKADDR_IN sa;
            int len = sizeof(SOCKADDR_IN);
            getsockname(m_socket, (SOCKADDR*) &sa, &len);
            m_nLocalPort = ntohs(sa.sin_port);

            // Call socket connection notification
            if((nError = SocketOnConnected(m_socket, address)) != UTE_SUCCESS)
            {
                SocketClose(m_socket);
                m_socket = INVALID_SOCKET;
            }
            return OnError(nError);
        }
        else {
            // did not have these two lines prior to Feb 11 1999
            SocketClose(m_socket);
            m_socket = INVALID_SOCKET;
        }

        return OnError(UTE_SOCK_CONNECT_FAILED);        // ERROR: connect unsuccessful
    }

    // set up the default send are receive time-outs
    SetReceiveTimeOut(m_lRecvTimeOut);
    SetSendTimeOut(m_lSendTimeOut);

    // Call socket connection notification
    if((nError = SocketOnConnected(m_socket, address)) != UTE_SUCCESS)
    {
        SocketClose(m_socket);
        m_socket = INVALID_SOCKET;
    }

    return OnError(nError);
}

/***********************************************
ConnectBound
    Connects to a specified port from a specified port
Params
    localPort - local port to connect the socket to
    remotePort - remote port to connect the socket to
    localAddress - local address to connect to
    remoteAddress - remote address to connect to
    family - protocol family: AF_INET, AF_AF_IPX, etc.
    sockType - SOCK_STREAM (TCP) or SOCK_DGRAM (UDP)
Return
    UTE_SOCK_ALREADY_OPEN   - socket already open or in use
    UTE_SOCK_CREATE_FAILED  - socket creation failed
    UTE_SOCK_BIND_FAILED    - socket binding to local port failed
    UTE_SOCK_CONNECT_FAILED - socket connection failed
    UTE_SUCCESS             - success
************************************************/
#if defined _UNICODE
int CUT_WSClient::ConnectBound(unsigned int localPort, unsigned int remotePort,
            LPWSTR localAddress, LPWSTR remoteAddress, int family, int sockType) {
       return ConnectBound(localPort, remotePort, AC(localAddress),
           AC(remoteAddress), family, sockType);
}
#endif
int CUT_WSClient::ConnectBound(unsigned int localPort,unsigned int remotePort,
            LPSTR localAddress,LPSTR remoteAddress,int family,int sockType){

    if(m_socket != INVALID_SOCKET)
        return OnError(UTE_SOCK_ALREADY_OPEN);

    //copy the params
    strncpy(m_szAddress, remoteAddress, sizeof(m_szAddress));
    m_nFamily    = family;
    m_nSockType  = sockType;

    //create a socket
    if((CreateSocket(m_socket, family, sockType)) == UTE_ERROR){
        return OnError(UTE_SOCK_CREATE_FAILED);   // ERROR: socket unsuccessful
    }

    //associate the socket with the address
    SOCKADDR_IN addr;
    memset(&addr,0,sizeof(addr));                   //clear all
    addr.sin_port       = (unsigned short)htons((u_short   )localPort);         //port
    addr.sin_family     = (short)family;                   //family
    addr.sin_addr.s_addr= inet_addr(localAddress);  //address

    if(bind(m_socket,(LPSOCKADDR)&addr,sizeof(addr)) == SOCKET_ERROR){
        return  OnError(UTE_SOCK_BIND_FAILED);  // ERROR: bind unsuccessful
    }

    //Set up the SockAddr structure
    memset(&m_sockAddr, 0, sizeof(m_sockAddr));             //clear all
    m_sockAddr.sin_port         = (unsigned short)  htons((u_short   )remotePort);      //port
    m_sockAddr.sin_family       = (short) family;                   //family
    m_sockAddr.sin_addr.s_addr  = inet_addr(remoteAddress); //address

    if( connect(m_socket,(LPSOCKADDR)&m_sockAddr,sizeof(m_sockAddr))==SOCKET_ERROR){
        return OnError(UTE_SOCK_CONNECT_FAILED); // ERROR: connect unsuccessful
    }

    // save the remote port
    m_nRemotePort = ntohs(m_sockAddr.sin_port);

    // save the local port
    SOCKADDR_IN sa;
    int len = sizeof(SOCKADDR_IN);
    getsockname(m_socket, (SOCKADDR*) &sa, &len);
    m_nLocalPort = ntohs(sa.sin_port);

    //set up the default send are receive time-outs
    SetReceiveTimeOut(m_lRecvTimeOut);
    SetSendTimeOut(m_lSendTimeOut);

    return OnError(UTE_SUCCESS);
}


/***********************************************
CreateUDPSocket
    Creates a socket for UDP use.  The socket is
    setup and bound to the given ports, but
    no connection is made
Params
    localPort       - local port to bind socket to
    remotePort      - port to use when SendTo and
                      ReceiveFrom are used
    remoteAddress   - addresess to use when using
                      SendTo and ReceiveFrom
    [localAddress]  - local address to bind the socket to
Return
    UTE_SOCK_ALREADY_OPEN   - socket already open or in use
    UTE_SOCK_CREATE_FAILED  - socket creation failed
    UTE_SOCK_BIND_FAILED    - socket binding to local port failed
    UTE_SUCCESS             - success
************************************************/
#ifdef CUT_UDP_SOCKET
#if defined _UNICODE
int CUT_WSClient::CreateUDPSocket(unsigned int localPort, unsigned int remotePort,
        LPWSTR remoteAddress, LPWSTR localAddress) {
    return CreateUDPSocket(localPort, remotePort, AC(remoteAddress), AC(localAddress));
}
#endif
int CUT_WSClient::CreateUDPSocket(unsigned int localPort,unsigned int remotePort,
                                  LPSTR remoteAddress,LPSTR localAddress){


    if(m_socket != INVALID_SOCKET)
        return OnError(UTE_SOCK_ALREADY_OPEN);

    //copy the params
    strncpy(m_szAddress, remoteAddress, sizeof(m_szAddress));
    m_nFamily    = AF_INET;
    m_nSockType  = SOCK_DGRAM;

    //create a socket
    if(CreateSocket(m_socket, m_nFamily, m_nSockType) == UTE_ERROR){
        return OnError(UTE_SOCK_CREATE_FAILED);   // ERROR: socket unsuccessful
    }

    //associate the socket with the address
    SOCKADDR_IN addr;
    memset(&addr,0,sizeof(addr));                       //clear all
    addr.sin_port       = (unsigned short) htons((u_short   )localPort);                //port
    addr.sin_family     = (short) m_nFamily;                    //family

    if(localAddress == NULL)
        addr.sin_addr.s_addr= htonl(INADDR_ANY);        //address
    else
        addr.sin_addr.s_addr= inet_addr(localAddress);  //address

   if(bind(m_socket,(LPSOCKADDR)&addr,sizeof(addr)) == SOCKET_ERROR){
      SocketClose(m_socket);
        m_socket = INVALID_SOCKET;
        return  OnError(UTE_SOCK_BIND_FAILED);  // ERROR: bind unsuccessful
    }

    // save the local port number for GetLocalPort...
    SOCKADDR_IN sa;
    int len = sizeof(SOCKADDR_IN);
    getsockname(m_socket, (SOCKADDR*) &sa, &len);
    m_nLocalPort = ntohs(sa.sin_port);

    // and the remote port for sendto and GetRemotePort
    m_nRemotePort      = remotePort;

    //set up the default send are receive time-outs
    SetReceiveTimeOut(m_lRecvTimeOut);
    SetSendTimeOut(m_lSendTimeOut);

    return OnError(UTE_SUCCESS);
}
#endif //CUT_UDP_SOCKET

/***********************************************
WaitForConnect
    Creates a socket and binds it to the given port
    and address. Then puts the socket in a listening state
Params
    port        - port to listen on.  If 0, winsock
                    will assign chose a port between
                    1024 and 5000.  Call GetAcceptPort
                    to retrieve this port number.
    [queSize]   - que size for incomming connections
    [family]    - protocol family: AF_INET, AF_AF_IPX, etc.
    [address]   - address to listen on
Return
    UTE_SOCK_ALREADY_OPEN   - socket already open or in use
    UTE_SOCK_CREATE_FAILED  - socket creation failed
    UTE_SOCK_BIND_FAILED    - socket binding to local port failed
    UTE_SOCK_LISTEN_ERROR   - listen failed
    UTE_SUCCESS             - success
************************************************/
int CUT_WSClient::WaitForConnect(unsigned short port,int queSize,short family,
        unsigned long address) {

    if(m_serverSocket != INVALID_SOCKET)
        return OnError(UTE_SOCK_ALREADY_OPEN);

    //Set up the serverSockAddr structure
    memset(&m_serverSockAddr, 0, sizeof(m_serverSockAddr)); //clear all
    m_serverSockAddr.sin_port           = htons(port);      //port
    m_serverSockAddr.sin_family         = family;           //Internet family
    m_serverSockAddr.sin_addr.s_addr    = address;          //address (any)

    //create a socket
    if(CreateSocket(m_serverSocket, family, SOCK_STREAM, m_nProtocol) == UTE_ERROR){
        return OnError(UTE_SOCK_CREATE_FAILED);   // ERROR: socket unsuccessful
    }

    //associate the socket with the address and port
    if(bind(m_serverSocket,(LPSOCKADDR)&m_serverSockAddr,sizeof(m_serverSockAddr))
     == SOCKET_ERROR){
        return OnError(UTE_SOCK_BIND_FAILED);  // ERROR: bind unsuccessful
    }
    //allow the socket to take connections
    if( listen(m_serverSocket, queSize ) == SOCKET_ERROR){
        return OnError(UTE_SOCK_LISTEN_ERROR);   // ERROR: listen unsuccessful
    }

    //set up the default send are receive time-outs
    SetReceiveTimeOut(m_lRecvTimeOut);
    SetSendTimeOut(m_lSendTimeOut);

    // save the port number for GetAcceptPort
    SOCKADDR_IN sa;
    int len = sizeof(SOCKADDR_IN);
    getsockname(m_serverSocket, (SOCKADDR*) &sa, &len);
    m_nAcceptPort = ntohs(sa.sin_port);

    return OnError(UTE_SUCCESS);
}

/***********************************************
AcceptConnection
    This function is used to accept a connection
    once a socket is set up for listening see:
    WaitForConnect
    To see if a connection is waiting call:
    WaitForAccept
Params
    none
Return
    UTE_ERROR   - error
    UTE_SUCCESS - success
************************************************/
int CUT_WSClient::AcceptConnection(){

    int addrLen = sizeof(SOCKADDR_IN);

    // close the socket if open - may be blocking though
    if(m_socket != INVALID_SOCKET){
        if(m_nSockType == SOCK_STREAM){
            if(SocketShutDown(m_socket, 2) == SOCKET_ERROR){
                return OnError(UTE_ERROR);
            }
        }
        if(SocketClose(m_socket) == SOCKET_ERROR){
            return OnError(UTE_ERROR);
        }
    }


    // accept the connection request when one is received
    m_socket = accept(m_serverSocket,(LPSOCKADDR)&m_sockAddr,&addrLen);
    if(m_socket == INVALID_SOCKET)
        return OnError(UTE_ERROR);

    strcpy(m_szAddress, inet_ntoa(m_sockAddr.sin_addr));

    // save the remote port
    m_nRemotePort = ntohs(m_sockAddr.sin_port);

    // save the local port
    SOCKADDR_IN sa;
    int len = sizeof(SOCKADDR_IN);
    getsockname(m_socket, (SOCKADDR*) &sa, &len);
    m_nLocalPort = ntohs(sa.sin_port);

    return SocketOnConnected(m_socket, m_szAddress);

    // now close the server socket so we can reuse it - return no errors
//    SocketShutDown(m_serverSocket, 2);
//    SocketClose(m_serverSocket);
//    m_serverSocket = INVALID_SOCKET;
//    m_nAcceptPort = 0;

  //  return OnError(UTE_SUCCESS);
}

/***************************************************
WaitForAccept
    Waits up til the specified time to see if there
    is an incomming connection waiting with a read set. If this
    function returns UTE_SUCCESS then call
    AcceptConnection to connect.
Params
    secs - the max. number of seconds to wait
Return
    UTE_ERROR   - error
    UTE_SUCCESS - success
****************************************************/
int CUT_WSClient::WaitForAccept(long secs){

    fd_set readSet;
    struct timeval tv;

    tv.tv_sec = secs;
    tv.tv_usec = 0;

    FD_ZERO(&readSet);

    FD_SET(m_serverSocket,&readSet);

    //wait up to the specified time to see if data is avail
    if( select(-1,&readSet,NULL,NULL,&tv)!= 1){
        return OnError(UTE_ERROR);
    }

    return OnError(UTE_SUCCESS);
}

/***********************************************
CloseConnection
    Call this function to close any used and/or
    open port.
Params
    none
Return (bitwise)
    UTE_SUCCESS     - success
    UTE_ERROR       - failed

    1 - client socket shutdown failed
    2 - client socket close failed
    3 - server socket shutdown failed
    4 - server socket close failed
************************************************/
int CUT_WSClient::CloseConnection(){

    int rt = UTE_SUCCESS;
    //if the socket is not open then just return
    if(m_socket != INVALID_SOCKET){
        if (m_SSLconnected) {
            DisconnectSSL();
        }
        if(m_nSockType == SOCK_STREAM){
            if(SocketShutDown(m_socket, 2) == SOCKET_ERROR){
                rt = UTE_ERROR; //1
            }
        }
        if(SocketClose(m_socket) == SOCKET_ERROR){
            rt = UTE_ERROR;     //2
        }
    }
    if(m_serverSocket != INVALID_SOCKET){
        if(SocketShutDown(m_serverSocket, 2) == SOCKET_ERROR){
            rt = UTE_ERROR;     //3
        }
        if(SocketClose(m_serverSocket) == SOCKET_ERROR){
            rt = UTE_ERROR;     //4
        }
    }

    //clear the socket variables
    m_socket        = INVALID_SOCKET;
    m_serverSocket  = INVALID_SOCKET;

    m_nRemotePort = 0;
    m_nLocalPort = 0;
    m_nAcceptPort = 0;

    strcpy(m_szAddress, "");

    return OnError(rt);
}

/****
SSL related
****/
int CUT_WSClient::EnableSSL(bool enable) {
    if (enable == m_isSSL)
        return UTE_SUCCESS;

    if (!enable) {
        DisconnectSSL();
        m_isSSL = false;

        if (m_ssl)
            SSL_free(m_ssl);
        if (m_ctx)
            SSL_CTX_free(m_ctx);

        m_ssl = NULL;
        m_ctx = NULL;

        return UTE_SUCCESS;
    }

    if (enable) {
        if (!m_meth)
            return UTE_ERROR;

        while (ERR_get_error() != 0);

        m_ctx = SSL_CTX_new(m_meth);
        if (!m_ctx) {
            int errval = ERR_get_error();
            printf(ERR_error_string(errval, NULL));
        }
        SSL_CTX_set_mode(m_ctx, SSL_MODE_AUTO_RETRY);

        int certRet = OnLoadCertificates(m_ctx);
        if (certRet != UTE_SUCCESS) {
            SSL_CTX_free(m_ctx);
            m_ctx = NULL;
            return UTE_ERROR;
        }

        m_ssl = SSL_new(m_ctx);

        m_isSSL = true;

        return UTE_SUCCESS;
    }

    return UTE_SUCCESS;
}

int CUT_WSClient::SetSecurityMode(SSLMode mode){
    if (m_SSLconnected)
        return UTE_ERROR;   //cannot change mode when SSL is connected

    if (m_sslMode == mode)
        return OnError(UTE_SUCCESS);

    m_sslMode = mode;
    switch(mode) {
        case TLS:
        default:
            //all version specific methods are deprecated since openssl 1.1.0
            m_meth = TLS_client_method();
            break;
    }

    if (m_meth == NULL) //possible?
        return OnError(UTE_ERROR);
    else
        return ResetSSL();

    return UTE_SUCCESS;

}

int CUT_WSClient::ConnectSSL() {
    if (m_SSLconnected)
        return UTE_SUCCESS;

    SSL_set_fd(m_ssl, m_socket);

    //Session reuse mod: If m_reuseSession is set, set it as the current SSL session
    //m_reuseSession should be set just prior to a Connect() call.
    if (m_reuseSession != NULL) {
        SSL_set_session(m_ssl, m_reuseSession);
    }
    int rc = SSL_connect(m_ssl);
    if (rc < 1) {
        char buf[256];
        u_long err;

        while ((err = ERR_get_error()) != 0) {
            ERR_error_string_n(err, buf, sizeof(buf));
            printf("*** %s\n", buf);
        }

        return UTE_ERROR;
    }



    X509* cert = SSL_get_peer_certificate(m_ssl);
    int code = SSL_get_verify_result(m_ssl);

    int res = OnSSLCertificate(m_ssl, cert, code);
    if (cert)
        X509_free(cert);
    if (res == UTE_ERROR) {
        return UTE_ERROR;
    }

    m_SSLconnected = true;

    return UTE_SUCCESS;
}

int CUT_WSClient::DisconnectSSL() {
    if (!m_SSLconnected)
        return UTE_SUCCESS;

    //assuming shutdown fully blocks on blocking sockets
    int rc = SSL_shutdown(m_ssl);

    //wait for peer to disconnect
    if (rc == 0) {
        char buf[256];
        int read = SSL_read(m_ssl, &buf[0], 256);
        while (read > 0) {
            read = SSL_read(m_ssl, &buf[0], 256);
        }
        rc = SSL_shutdown(m_ssl);
    }

    m_SSLconnected = false;

    //all done
    if (rc == 1) {
        return UTE_SUCCESS;
    }

    //error
    if (rc < 0) {
        //this is risky, is it closed or not? Up to client to detect that
        char buf[256];
        u_long err;

        while ((err = ERR_get_error()) != 0) {
            ERR_error_string_n(err, buf, sizeof(buf));
            printf("*** %s\n", buf);
        }
        return UTE_ERROR;
    }

    return UTE_SUCCESS;
}

int CUT_WSClient::ResetSSL() {
    if (m_SSLconnected)
        DisconnectSSL();

    if (m_ssl)
        SSL_free(m_ssl);
    if (m_ctx)
        SSL_CTX_free(m_ctx);

    m_ssl = NULL;
    m_ctx = NULL;

    m_ctx = SSL_CTX_new(m_meth);
    if (!m_ctx) {
        EnableSSL(false);
        return UTE_ERROR;
    }

    int certRet = OnLoadCertificates(m_ctx);
    if (certRet != UTE_SUCCESS) {
        EnableSSL(false);
        return UTE_ERROR;
    }

    m_ssl = SSL_new(m_ctx);
    if (!m_ssl) {
        EnableSSL(false);
        return UTE_ERROR;
    }

    return UTE_SUCCESS;
}

int CUT_WSClient::OnLoadCertificates(SSL_CTX * ctx) {
    return UTE_SUCCESS;
}

int CUT_WSClient::OnSSLCertificate(const SSL * ssl, const X509* certificate, int verifyResult) {
    if (certificate == NULL) {
        return UTE_ERROR;
    } else {
        if (verifyResult == X509_V_OK) {
            return UTE_SUCCESS;
        } else {
            return UTE_ERROR;   //This should be properly handled by a subclass
        }
    }

    return UTE_SUCCESS;
}

int CUT_WSClient::SSLSend(LPCSTR data, int len) {
    if (m_isSSL && m_SSLconnected) {
        return SSL_write(m_ssl, (char *)data, len);
    } else {
        return SocketSend(m_socket, (char *)data, len, 0);
    }
}

int CUT_WSClient::SSLReceive(LPSTR buffer, int maxSize, bool peek) {
    if (m_isSSL && m_SSLconnected) {
        if (!peek) {
            int size = SSL_read(m_ssl, (char *)buffer, maxSize);
            return size;
        }
        else {
            //return SSL_peek(m_ssl, (char *)buffer, maxSize);
            int size = SSL_peek(m_ssl, (char *)buffer, maxSize);
            return size;
        }
    } else {
        return SocketRecv(m_socket, (char *)buffer, maxSize, peek?MSG_PEEK:0);
    }
}

SSL_SESSION * CUT_WSClient::SSLGetCurrentSession() {
    if (m_ssl)
        return SSL_get_session(m_ssl);
    else
        return NULL;
}

int CUT_WSClient::SSLSetReuseSession(SSL_SESSION * reuseSession) {
    m_reuseSession = reuseSession;

    return UTE_SUCCESS;
}

/***************************************************
Send
    Sends the given data to the client.
    if 'len' is 0 or less then the given data is
    assumed to be a NULL terminated string
Params
    data    - pointer to the data to send
    [len]   - length of the data to send

    Note that the widechar version translates to
    ANSI before sending.

Return
    number of bytes sent
    0   - on fail
****************************************************/
#if defined _UNICODE
int CUT_WSClient::Send(LPCWSTR data, int len) {
    return Send(AC(data), len);}
#endif
int CUT_WSClient::Send(LPCSTR data, int len)
{
    if(len <=0)
        len = (int)strlen((char *)data);

    int rt = SSLSend(data,len);

    //TODO: onerror socket io
    if(SOCKET_ERROR == rt)
        return 0;

    return rt;
}

/***************************************************
SendBlob
    Sends information to the currently connected
    client. The receive function can block until the
    specified timeout.  Unlike Receive, this method
    waits until all the data is sent or until an
    error occurs.
Params
    data    - pointer to data to send
    dataLen - buffer length
    timeOut - timeout in seconds ( 0- for no time-out)
Return
    number of bytes send
    0 if the connection was closed or timed-out
    any other error returns SOCKET_ERROR
****************************************************/
int CUT_WSClient::SendBlob(LPBYTE data,int dataLen){

    LPBYTE position = data;
    int dataRemain = dataLen;

    // Try to send all data
    while(dataRemain > 0){

        // Send as much data as we can
        int dataSent = Send((LPCSTR)position, dataRemain);

        if(dataSent == 0)
            break;

        // Move past the data sent
        position += dataSent;
        dataRemain -= dataSent;
    }

    return (int)(position - data);
}

/***************************************************
SendAsLine
    Sends the given data and breaks the data up into
    lines of the specified line lengths by adding
    '\r\n' to the data
Params
    data    - data to send
    len     - length of the data to send
    linelen - max length to send as a line

    Note that the widechar version translates to
    ANSI before sending.

Return
    number of bytes sent
    UTE_ERROR   - on fail
****************************************************/
#if defined _UNICODE
int CUT_WSClient::SendAsLine(LPCWSTR data, int len,int lineLen){
    return SendAsLine(AC(data), len, lineLen);}
#endif
int CUT_WSClient::SendAsLine(LPCSTR data, int len,int lineLen){

    int     pos,sendLen;
    int     rt;
    int     bytesSent =0;

    //if the length value is less than zero then find the length
    if(len < 0)
        len = (int)strlen(data);

    sendLen =lineLen;

    for(pos =0; pos < len && !IsAborted(); pos+= lineLen){

        if(sendLen > (len-pos))
            sendLen = (len-pos);

        rt = Send(&data[pos],sendLen);
        if(rt > 0)
            bytesSent += rt;
        else
            break;
        if(Send("\r\n") <=0)
            break;
    }
    return bytesSent;
}

/***************************************************
Send
    Sends data from the datasource across the connection
Params
    source  - data source
Return
    UTE_SUCCESS             - success
    UTE_ABORTED             - send aborted
    UTE_SOCK_NOT_OPEN       - socket is not set up
    UTE_DS_OPEN_FAILED      - unable to open specified data source
    UTE_SOCK_SEND_ERROR     - remote connection terminated
****************************************************/
int CUT_WSClient::Send(CUT_DataSource & source)
{

    //check to see if the socket is open
    if(m_socket == INVALID_SOCKET)
        return OnError(UTE_SOCK_NOT_OPEN);

    int         len;
    char        buf[WSC_BUFFER_SIZE];
    long        bytesSent = 0;

    // Open data source for reading
    if(source.Open(UTM_OM_READING) == -1)
        return OnError(UTE_DS_OPEN_FAILED);

    int   error = UTE_SUCCESS;

    // Send the file
  do{
        if((len = source.Read(buf, sizeof(buf)-1)) <= 0)
            break;

        if( Send(buf, len) == 0){
            error = UTE_SOCK_SEND_ERROR;
            break;
        }

        bytesSent += len;

        //send notify
        if(SendFileStatus(bytesSent) == FALSE || IsAborted()) {
            error = UTE_ABORTED;
            break;
        }
    }while (len > 0);

    // Close data source
    source.Close();

    //return success
    return OnError(error);
}

/***************************************************
Send
    Sends data from the queue across the connection
Params
    queue   - queue to send
Return
    UTE_SUCCESS             - success
    UTE_ABORTED             - send aborted
    UTE_SOCK_NOT_OPEN       - socket is not set up
    UTE_SOCK_SEND_ERROR     - remote connection terminated
****************************************************/
int CUT_WSClient::Send(CUT_Queue& queue)
{

    //check to see if the socket is open
    if(m_socket == INVALID_SOCKET)
        return OnError(UTE_SOCK_NOT_OPEN);

    int         len;
    char        buf[WSC_BUFFER_SIZE];
    long        bytesSent = 0;

    int   error = UTE_SUCCESS;

    // Send the file
    do{
        if((len = queue.Read((LPBYTE)buf, sizeof(buf)-1)) <= 0)
            break;

        if( Send(buf, len) == 0){
            error = UTE_SOCK_SEND_ERROR;
            break;
        }

        bytesSent += len;

        //send notify
        if(SendFileStatus(bytesSent) == FALSE || IsAborted()) {
            error = UTE_ABORTED;
            break;
        }
    }while (len > 0);

    //return success
    return OnError(error);
}

/***************************************************
SendFile
    Sends the specified file across the connection
Params
    filename - file to send
Return
    UTE_SUCCESS             - success
    UTE_ABORTED             - send aborted
    UTE_FILE_OPEN_ERROR     - unable to open specified file
    UTE_CONNECT_TERMINATED  - remote connection terminated
****************************************************/
int CUT_WSClient::SendFile(LPCTSTR filename) {
    CUT_FileDataSource ds(filename) ;

    return Send( ds);
}

/***************************************************
WaitForSend
    Waits up til the specified time to see if the
    connection is ready to send data if it is then
    it will return UTE_SUCCESS
Params
    secs  - max. seconds to wait
    uSecs - max. micro seconds to wait
Return
    UTE_SUCCESS     - error
    UTE_ERROR       - success
****************************************************/
int CUT_WSClient::WaitForSend(long secs,long uSecs){

    fd_set writeSet;
    struct timeval tv;

    tv.tv_sec = secs;
    tv.tv_usec =uSecs;

    FD_ZERO(&writeSet);

    FD_SET(m_socket,&writeSet);

    //wait up to the specified time to see if data is avail
    if( select(-1,NULL,&writeSet,NULL,&tv)!= 1){
        return OnError(UTE_ERROR);
    }
    return OnError(UTE_SUCCESS);
}

/***********************************************
SendTo
    Sends the specified data across the UDP
    connection (See CreateUDPSocket)
Params
    data - data to send
    dataLen - length of data to send
        Plus the length actually sent is
        returned in this varaible as well

    Note that the widechar version translates to
    ANSI before sending.

Return
    UTE_SOCK_NOT_OPEN   - socket is not set up
    UTE_SOCK_SEND_ERROR - send failure
    UTE_SUCCESS         - success
************************************************/
#ifdef CUT_UDP_SOCKET
#if defined _UNICODE
int CUT_WSClient::SendTo(LPCWSTR data,int &dataLen){
    return SendTo(AC(data), dataLen);}
#endif
int CUT_WSClient::SendTo(LPCSTR data,int &dataLen){

    //check to see if the socket is open
    if(m_socket == INVALID_SOCKET)
        return OnError(UTE_SOCK_NOT_OPEN);

    //Set up the SockAddr structure
    memset(&m_sockAddr, 0, sizeof(m_sockAddr));
    m_sockAddr.sin_port         = (unsigned short) htons((u_short  )m_nRemotePort);        // port
    m_sockAddr.sin_family       = (short) m_nFamily;                   // family
    m_sockAddr.sin_addr.s_addr  = inet_addr(m_szAddress);      // address

    //call the send to
    int rt = sendto(m_socket,data,dataLen,0,(LPSOCKADDR)&m_sockAddr,
        sizeof(m_sockAddr));
    dataLen = rt;

    if(rt != SOCKET_ERROR )
        return OnError(UTE_SUCCESS);

    return OnError(UTE_SOCK_SEND_ERROR);
}

/***************************************************
RecieveFrom
    Receives data from a UDP connection (see CreateUDPSocket)
Params
    data        - data buffer to receive data
    dataLen     - length of the data buffer the acutal
                  length of data received is returned
                  in this variable
    [timeout]   - max. time to wait for data (in seconds)
Return
    UTE_SOCK_NOT_OPEN       - socket is not set up
    UTE_SOCK_RECEIVE_ERROR  - receive failure
    UTE_SOCK_TIMEOUT        - a timeout occurred
    UTE_SUCCESS             - success
****************************************************/
int CUT_WSClient::ReceiveFrom(LPSTR data,int &dataLen,unsigned long timeout){

    //check to see if the socket is already open
    if(m_socket == INVALID_SOCKET)
        return OnError(UTE_SOCK_NOT_OPEN);

    //wait for a recive for upto the specified time
    if(WaitForReceive(timeout,0) != UTE_SUCCESS)
        return OnError(UTE_SOCK_TIMEOUT);

    //call the send to
    int size = sizeof(m_sockAddr);
    int rt = recvfrom(m_socket,data,dataLen,0,(LPSOCKADDR)&m_sockAddr,&size);
    dataLen = rt;

    if(rt != SOCKET_ERROR )
        return OnError(UTE_SUCCESS);

    return OnError(UTE_SOCK_RECEIVE_ERROR);
}
#endif //CUT_UDP_SOCKET
/***************************************************
WaitForReceive
    This function wiats up til the specified time
    to see if there is data to be received.
Params
    secs    - max. seconds to wait
    [uSecs] - max. micro seconds to wait
Return
    UTE_ERROR   - error
    UTE_SUCCESS - success
****************************************************/
int CUT_WSClient::WaitForReceive(long secs,long uSecs){
    if (m_SSLconnected) {
            if (SSL_pending(m_ssl)) {
                return UTE_SUCCESS;
            }
    }
    return OnError(SocketWaitForReceive(m_socket, secs, uSecs));
}

/***************************************************
Receive
    Receives data from the remote connection
Params
    data - data buffer for received data
    maxLen - length of data buffer
    timeOut - max. time in seconds to wait for incomming
        data to be received
Return
    the length of the data received or
    UTE_ERROR   - failure
****************************************************/
int CUT_WSClient::Receive(LPSTR data,int maxLen,int timeOut){
    if(timeOut >0){
        if(WaitForReceive(timeOut,0) != UTE_SUCCESS)
            return 0;
    }
    return SSLReceive(data,maxLen);

}

/***************************************************
ReceiveBlob
    Receives information for the currently connected
    client. The receive function can block until the
    specified timeout.  Unlike Receive, this method
    waits until all the data is received or until an error occurs.
Params
    data    - buffer for received data
    dataLen - buffer length
    timeOut - timeout in seconds ( 0- for no time-out)
Return
    number of bytes received
    0 if the connection was closed or timed-out
    any other error returns SOCKET_ERROR
****************************************************/
int CUT_WSClient::ReceiveBlob(LPBYTE data,int dataLen,int timeOut){

    LPBYTE position = data;
    int dataRemain = dataLen;

    // Try to get all the data
    while(dataRemain > 0){

        // Wait for any timeout until data is ready
        if(timeOut > 0)
            if(WaitForReceive(timeOut,0) != UTE_SUCCESS)
                break;

        // Receive any data that's available
        int received = SSLReceive((LPSTR)position,dataRemain);

        if(received < 0)
            break;

        // Move past the data returned
        position += received;
        dataRemain -= received;
    }

    return (int)(position - data);
}

/***************************************************
SetRecieveTimeOut
    Sets the maximum number of millisecs to wait
    for data when receiving data from a remote
    connection - this function may not work with
    all stack implementations.

    The SO_RCVTIMEO option should be available if
    using Winsock 2.

Params
    milisecs - miliseconds to wait
Return
    UTE_SUCCESS - success
    UTE_ERROR   - failure
****************************************************/
int CUT_WSClient::SetReceiveTimeOut(int milisecs){

    int     result;

    #ifdef WIN32
        result = setsockopt(m_socket,SOL_SOCKET,SO_RCVTIMEO,(char *)&milisecs,sizeof(milisecs));
        if(result == UTE_SUCCESS)
            m_lRecvTimeOut = milisecs;
        return OnError(result);
    #else
        return OnError(UTE_ERROR);
    #endif

}

/***************************************************
GetRecieveTimeOut
    Gets the maximum number of millisecs to wait
    for data when receiving data from a remote
    connection - this function may not work with
    all stack implementations
Params
    none
Return
    Receive time out in miliseconds
****************************************************/
int CUT_WSClient::GetReceiveTimeOut() const
{
    return m_lRecvTimeOut;
}

/***************************************************
SetSendTimeOut
    Sets the maximum number of millisecs to wait
    for data when sending data to a remote
    connection - this function may not work with
    all stack implementations
Params
    milisecs - miliseconds to wait
Return
    UTE_SUCCESS     - success
    UTE_ERROR       - failure
****************************************************/
int CUT_WSClient::SetSendTimeOut(int milisecs){

    int     result;

    #ifdef WIN32
        result = setsockopt(m_socket,SOL_SOCKET,SO_SNDTIMEO,(char *)&milisecs,sizeof(milisecs));
        if(result == UTE_SUCCESS)
            m_lSendTimeOut = milisecs;
        return OnError(result);
    #else
        return OnError(UTE_ERROR);
    #endif
}

/***************************************************
GetSendTimeOut
    Gets the maximum number of millisecs to wait
    for data when sending data to a remote
    connection - this function may not work properly
    with all stack implementations
Params
    none
Return
    Send time out in miliseconds
****************************************************/
int CUT_WSClient::GetSendTimeOut() const
{
    return m_lSendTimeOut;
}

/***************************************************
SetBlockingMode
    Blocking is on by default. But it may be turned off.
    If ansync functions are used then blocking is
    automatically turned off.
    When blocking is on certain functions will not return
    until they have performed their function or time-out.
    These include - sending functions, receiving functions
    and connecting function.

Params
    mode - CUT_BLOCKING  or CUT_NONBLOCKING
Return
    UTE_ERROR   - error
    UTE_SUCCESS - success
****************************************************/
int CUT_WSClient::SetBlockingMode(int mode){

    unsigned long lmode = mode;

    if(ioctlsocket(m_socket,FIONBIO,&lmode) == SOCKET_ERROR)
        return OnError(UTE_ERROR);

    m_nBlockingMode = mode;
    return OnError(UTE_SUCCESS);
}

/***************************************************
GetBlockingMode
    Returns current blocking mode

Params
    none
Return
    CUT_NONBLOCKING - non blocking mode
    CUT_BLOCKING    - blocking mode
****************************************************/
int CUT_WSClient::GetBlockingMode() const
{
    return m_nBlockingMode;
}

/***************************************************
ReceiveLine
    Receives a line of data. This function will receive data
    until the maxDataLen is reached or a '\n' is encountered.
    This function will also return when the timeOut has expired.
Params
    data       - data buffer for receiving data
    maxDataLen - length of the data buffer
    [timeOut]  - timeout in seconds
Return
    the length of the data received
****************************************************/
int CUT_WSClient::ReceiveLine(LPSTR data,int maxDataLen,int timeOut){

    int     rt, count = 0;
    char    *tempBuf = NULL;
    LPSTR   dataPtr = data; // point to the begining of the buffer

    maxDataLen --;

    tempBuf = new char[maxDataLen+1];

    while(maxDataLen > 0) {

        if(IsAborted()) {
            break;
        }

        // wait for a data up til the timeout value
        if(timeOut > 0) {
            if(WaitForReceive(timeOut,0) != UTE_SUCCESS)
                break;
        }

        // look at the data coming in
        rt = SSLReceive(tempBuf,maxDataLen, true);
        //error checking
        if(rt < 1 ) {
            break;
        }

        // find the first LF
        for(int x = 0; x < rt; x ++){
            if(tempBuf[x] == '\n'){
                rt = SSLReceive(dataPtr,x+1);
                dataPtr[x+1] = 0;
                count += x + 1;
                delete [] tempBuf;
                return count;
            }
        }

        // if the LF is not found then copy what is available
        rt = SSLReceive(dataPtr, rt);
        count += rt;
        dataPtr[rt] = 0;

        // move the current position of the pointer to the new point
        dataPtr = &dataPtr[rt];
        maxDataLen -= rt;
    }

    delete [] tempBuf;
    return count;
}
/***************************************************
IsDataWaiting
    Returns TRUE if data is immediately
    available for receiving.
Params
    none
Return
    TRUE - if data is waiting
    FALSE - if no data is available
See Also: WaitForReceive
****************************************************/
BOOL CUT_WSClient::IsDataWaiting() const
{
    return SocketIsDataWaiting(m_socket);
}

/***************************************************
ClearReceiveBuffer
    Clears any data in the current connections
    receive buffer
Params
    none
Return
    UTE_SUCCESS - success
    UTE_ERROR   - failure
****************************************************/
int CUT_WSClient::ClearReceiveBuffer(){

    char buf[WSC_BUFFER_SIZE];

    //remove all other data
    while(IsDataWaiting()) {
        if( Receive(buf,sizeof(buf)) <=0)
            break;
        }

    return OnError(UTE_SUCCESS);
}

/****************************************************
Receive
    Receives data from the current connection directly
    to a data source. This function will not return
    until the connection is terminated, timed-out or
    the max data has been received.
Params
    dest            - queue to copy received data into
    type            - the method of opening the data
                      source (write vs append)
    [timeOut]       - timeout in seconds
    [lMaxToReceive] - maximum data to receive
Return
    UTE_SUCCESS               - success
    UTE_SOCK_NOT_OPEN         - socket is not set up
    UTE_FILE_TYPE_ERROR       - invalid file type
    UTE_DS_OPEN_FAILED        - unable to open specified data source
    UTE_SOCK_TIMEOUT          - timeout
    UTE_SOCK_RECEIVE_ERROR    - receive socket error
    UTE_DS_WRITE_FAILED       - data source write error
    UTE_ABORTED               - aborted
*****************************************************/
int CUT_WSClient::Receive(CUT_DataSource & dest, OpenMsgType type, int timeOut, long lMaxToReceive)
{
    char        data[WSC_BUFFER_SIZE];
    int         count, nSize = sizeof(data);
    int         error = UTE_SUCCESS;
    long        bytesReceived = 0L;

    //check to see if the socket is open
    if(m_socket == INVALID_SOCKET)
        return OnError(UTE_SOCK_NOT_OPEN);

    // Check data source open type
    if(type != UTM_OM_APPEND && type != UTM_OM_WRITING)
        return OnError(UTE_FILE_TYPE_ERROR);

    // Open data source
    if(dest.Open(type) == -1)
        return OnError(UTE_DS_OPEN_FAILED);

    //start reading in the data
    do{
        if(timeOut > 0) {
            if(WaitForReceive(timeOut,0) != UTE_SUCCESS) {
                error = UTE_SOCK_TIMEOUT;
                break;
                }
            }

        if(lMaxToReceive > 0) {
            nSize = min((long)sizeof(data), lMaxToReceive - bytesReceived);
            if(nSize == 0)
                break;
            }

        count = Receive(data, nSize);

        // Check to see if the connection is closed
        if(count == 0)
            break;

        // If an error then break
        if(count == SOCKET_ERROR) {
            error = UTE_SOCK_RECEIVE_ERROR;
            break;
            }

        // Write the the data source
        if(dest.Write(data, count) != count) {
            error = UTE_DS_WRITE_FAILED;
            break;
            }

        // Count the bytes copied
        bytesReceived += count;

        //send notify
        if(ReceiveFileStatus(bytesReceived) == FALSE || IsAborted()) {
            error = UTE_ABORTED;
            break;
            }
        }while (count > 0 );

    // Close data source
    dest.Close();

    return OnError(error);
}

/****************************************************
Receive
    Receives data from the current connection directly
    to a queue. This function will not return until
    the connection is terminated, timed-out, the
    queue is full or the max data has been received.
Params
    dest            - queue to copy received data into
    [timeOut]       - timeout in seconds
    [lMaxToReceive] - maximum data to receive
Return
    UTE_SUCCESS               - success
    UTE_SOCK_NOT_OPEN         - socket is not set up
    UTE_FILE_TYPE_ERROR       - invalid file type
    UTE_SOCK_TIMEOUT          - timeout
    UTE_SOCK_RECEIVE_ERROR    - receive socket error
    UTE_DS_WRITE_FAILED       - failed writing to queue
    UTE_ABORTED               - aborted
*****************************************************/
int CUT_WSClient::Receive(CUT_Queue & dest, int timeOut, long lMaxToReceive){

    BYTE        data[WSC_BUFFER_SIZE];
    int         count, nSize = sizeof(data);
    int         error = UTE_SUCCESS;
    long        bytesReceived = 0L;

    //check to see if the socket is open
    if(m_socket == INVALID_SOCKET)
        return OnError(UTE_SOCK_NOT_OPEN);

    //we cannot receive more than the free size of the queue
    if(lMaxToReceive > 0)
        lMaxToReceive = min(lMaxToReceive, (long)dest.GetFreeSize());
    else
        lMaxToReceive = dest.GetFreeSize();

    //start reading in the data
    do{
        if(timeOut > 0) {
            if(WaitForReceive(timeOut,0) != UTE_SUCCESS) {
                error = UTE_SOCK_TIMEOUT;
                break;
                }
            }

        nSize = min((long)sizeof(data), lMaxToReceive - bytesReceived);
        if(nSize == 0)
            break;

        count = Receive((LPSTR)data, nSize);

        // Check to see if the connection is closed
        if(count == 0)
            break;

        // If an error then break
        if(count == SOCKET_ERROR) {
            error = UTE_SOCK_RECEIVE_ERROR;
            break;
            }

        // Write the the data source
        if(dest.Write(data, count) != count) {
            error = UTE_DS_WRITE_FAILED;
            break;
            }

        // Count the bytes copied
        bytesReceived += count;

        //send notify
        if(ReceiveFileStatus(bytesReceived) == FALSE || IsAborted()) {
            error = UTE_ABORTED;
            break;
            }
        }while (count > 0 );

    return OnError(error);
}

/****************************************************
ReceiveToFile
    Receives data from the current connection directly
    to a file. This function will not return until
    the connection is terminated or timed-out.
Params
    name          - name of file to copy received data into
    fileType      - UTM_OM_WRITING:create/overwrite
                    UTM_OM_APPEND:create/append
    [timeOut]     - timeout in seconds
Return
    UTE_SUCCESS             - success
    UTE_FILE_TYPE_ERROR     - invalid file type
    UTE_FILE_OPEN_ERROR     - unable to open or create file
    UTE_SOCK_TIMEOUT        - timeout
    UTE_SOCK_RECEIVE_ERROR  - receive socket error
    UTE_FILE_WRITE_ERROR    - file write error
    UTE_ABORTED             - aborted
*****************************************************/
int CUT_WSClient::ReceiveToFile(LPCTSTR name, OpenMsgType fileType, int timeOut){
    CUT_FileDataSource ds(name);
    return Receive( ds , fileType, timeOut);
}

/***************************************************
GetMaxSend
    Returns the maximum number of bytes that can be
    sent at once. By sending full packets total
    throughput is increased.
Params
    none
Return
    Returns the maximum number of bytes that can be
    sent at once.
****************************************************/
int CUT_WSClient::GetMaxSend() const
{
    int length;
    int size = sizeof(int);
    if (getsockopt(m_socket, SOL_SOCKET, SO_SNDBUF,(char*) &length, &size) ==0)
        return length;

    return -1;
}

/***************************************************
GetMaxReceive
    Returns the maximum number of bytes that can
    be received at once.
Params
    none
Return
    Returns the maximum number of bytes that can
    be received at once.
****************************************************/
int CUT_WSClient::GetMaxReceive() const
{
    int length;
    int size = sizeof(int);
    if (getsockopt(m_socket, SOL_SOCKET, SO_RCVBUF,(char*) &length, &size) == 0)
        return length;

    return -1;
}

/***************************************************
SetMaxSend
    Sets the maximum number of bytes that can be
    sent at once.

    Note that SetSockOpt may report success while
    setting less than size requested - call
    GetMaxSend to confirm.

Params
    length - number of bytes
Return
    UTE_SUCCESS - success
    otherwise failure
****************************************************/
int CUT_WSClient::SetMaxSend(int length){
    return setsockopt(m_socket,SOL_SOCKET,SO_SNDBUF,(char *)&length,sizeof(int));
}

/***************************************************
SetMaxReceive
    Sets the maximum number of bytes that can be
    received at once

    Note that SetSockOpt may report success while
    setting less than size requested - call
    GetMaxReceive to confirm.

Params
    length - number of bytes
Return
    UTE_SUCCESS - success
    otherwise failure
****************************************************/
int CUT_WSClient::SetMaxReceive(int length){
    return setsockopt(m_socket,SOL_SOCKET,SO_RCVBUF,(char *)&length,sizeof(int));
}

/***************************************************
GetNameFromAddress
    Returns the name associated with the given address
Params
    address - address string to lookup
    name    - buffer to return the name in
    maxLen  - length of the name buffer
Return
    UTE_SUCCESS             - success
    UTE_BUFFER_TOO_SHORT    - return string too short
    UTE_NAME_LOOKUP_FAILED  - name lookup failure
****************************************************/
#if defined _UNICODE
int CUT_WSClient::GetNameFromAddress(LPCWSTR address,LPWSTR name,int maxLen){
    char *nameA = (char*) alloca(maxLen);
    *nameA = '\0';
    int result = GetNameFromAddress(AC(address), nameA, maxLen);
    if(result == UTE_SUCCESS) {
        CUT_Str::cvtcpy(name, maxLen, nameA);
    }
    return result;}
#endif
int CUT_WSClient::GetNameFromAddress(LPCSTR address,LPSTR name,int maxLen){

    unsigned long   addr;
    hostent FAR *   host;
    int             len;
    addr =  inet_addr(address);

    host = gethostbyaddr ((const char *)&addr,4,PF_INET);

    if(host == NULL)
        return OnError(UTE_NAME_LOOKUP_FAILED);

    len = (int)strlen(host->h_name) +1;

    if(len > maxLen)
        return OnError(UTE_BUFFER_TOO_SHORT);

    strcpy(name,host->h_name);

    return OnError(UTE_SUCCESS);
}

/***************************************************
GetAddressFromName
    Returns the address associated with the given name
Params
    name - name to lookup
    address - buffer for the address
    maxLen - length of the address buffer
Return
    UTE_SUCCESS             - success
    UTE_BUFFER_TOO_SHORT    - return string too short
    UTE_NAME_LOOKUP_FAILED  - name lookup failure
****************************************************/
#if defined _UNICODE
int CUT_WSClient::GetAddressFromName(LPCWSTR name,LPWSTR address,int maxLen){
    char * addressA = (char*) alloca(maxLen);
    *addressA = '\0';
    int result = GetAddressFromName(AC(name), addressA, maxLen);
    if(result == UTE_SUCCESS) {
        CUT_Str::cvtcpy(address, maxLen, addressA);
    }
    return result;}
#endif
int CUT_WSClient::GetAddressFromName(LPCSTR name,LPSTR address,int maxLen){

    in_addr         addr;
    hostent FAR *   host;
    int             len;
    char *          pChar;
    int             position;

    //check for @ symbol
    len = (int)strlen(name);
    for(position = len; position >0;position --){
        if(name[position]=='@'){
            position++;
            break;
        }
    }

    host = gethostbyname (&name[position]);

    if(host == NULL)
        return OnError(UTE_NAME_LOOKUP_FAILED);

    addr.S_un.S_un_b.s_b1 = host->h_addr_list[0][0];
    addr.S_un.S_un_b.s_b2 = host->h_addr_list[0][1];
    addr.S_un.S_un_b.s_b3 = host->h_addr_list[0][2];
    addr.S_un.S_un_b.s_b4 = host->h_addr_list[0][3];

    pChar = inet_ntoa (addr);

    if(pChar == NULL)
        return OnError(UTE_NAME_LOOKUP_FAILED);

    len = (int)strlen(pChar) +1;

    if(len > maxLen)
        return OnError(UTE_BUFFER_TOO_SHORT);

    strcpy(address,pChar);

    return OnError(UTE_SUCCESS);
}

/***************************************************
IsIPAddress
    Checks to see if the given string has a vaild
    IP address format. Note: it does not check the
    IP value itself only the format.
Param
    address - string of the address to check
Return
    TRUE - success
    FALSE - failure
****************************************************/
#if defined _UNICODE
int CUT_WSClient::IsIPAddress(LPCWSTR address) const {
    return IsIPAddress(AC(address));}
#endif
int CUT_WSClient::IsIPAddress(LPCSTR address) const
{
    if (inet_addr(address) == INADDR_NONE)
        return FALSE;
    return TRUE;
}

/***************************************************
IsConnected
    Checks to see if a connection is still alive

    This method relies on a recv with MSG_PEEK, and will
    not be terribly efficient.  Use with ptotocols subject
    to resets and disconnects not initiated by the client.
    (eg TELNET)

Params
    none
Return
    TRUE - success
    FALSE - failure
****************************************************/
BOOL CUT_WSClient::IsConnected(){

    int rt1, rt2, error;

    fd_set readSet;
    struct timeval tv;

    tv.tv_sec = 0;      // do not block - for polling
    tv.tv_usec = 0;

    FD_ZERO(&readSet);  // always reinit

    FD_SET(m_socket,&readSet);

    rt1 = select(-1,&readSet,NULL,NULL,&tv);

    if(rt1 == SOCKET_ERROR) {
        int err = WSAGetLastError();
        switch (err) {
        case WSANOTINITIALISED:
        case WSAENETDOWN:
        case WSAENOTSOCK:
            return FALSE;
            break;
        default:
            break;
        }                   // other possible errors:
    }                       // WSAEFAULT - WSAEINVAL - WSAEINTR(ws1.1)


    char data = 0;

    SetBlockingMode(CUT_NONBLOCKING);   //no blocking


    // the security version of Ultimate TCP/IP uses diffrent implementation
    // of Receive and in order to avoid calling it's error checking we will call the base
    // winsock recv
    // if you use your own class or stream for using you can either
    // change this code to call your own receive function
    // or expose recv function of your base class that will fulfil this
    // recv function requirment

    rt2 = recv(m_socket,(char *)&data,1, MSG_PEEK);

    if(rt2 == SOCKET_ERROR){
        error = WSAGetLastError();
        switch(error) {
        case WSAEWOULDBLOCK:
            SetBlockingMode(CUT_BLOCKING);  // back to blocking mode
            //NppFTP: settings this to 0 will cause a disconnected state to be reported
            //even if the connection is still alive and waiting for data
            //set to 1 instead
            rt2 = 1;
            //rt2 = 0;            // no data - set rt2 to 0 and continue
                                // Alternative - use a small
                                // timeout value in tv ....
            break;
        case WSAECONNRESET:
        case WSAECONNABORTED:
            SetBlockingMode(CUT_BLOCKING);  // back to blocking mode
            return  FALSE;
            break;
        case WSAEINPROGRESS:
            SetBlockingMode(CUT_BLOCKING);  // back to blocking mode
            return  TRUE;
            break;
        case WSAESHUTDOWN:
        case WSAENETDOWN:
        case WSAENETRESET:
        case WSAETIMEDOUT:
            SetBlockingMode(CUT_BLOCKING);  // back to blocking mode
            return  FALSE;
            break;
        case WSAEISCONN:
        case WSAENOTSOCK:
        case WSAEFAULT:
        case WSAEINTR:
        case WSAEINVAL:
        case WSAEOPNOTSUPP:
        case WSAEMSGSIZE:
            SetBlockingMode(CUT_BLOCKING);  // back to blocking mode
            return FALSE;
            break;
        default:
            break;      // should never happen.
        }

    }

    SetBlockingMode(CUT_BLOCKING);  //blocking

    // final check - if we made it through the recv, we know the connection
    // was not reset (RST) - but no error will show if the remote has initiated
    // a graceful close (FIN).

    // the select with readfds will return with 1 (rt1) if data is available or
    // the close is in progress.  The call to recv will be 0 if no data
    // waiting, so we examine rt1 and rt2 together to determine closure.
    // For protocols using WSASendDisconnect() this behavior may need to be
    // reexamined.

    if(0 == rt2 && 1 == rt1)
        return FALSE;

    return TRUE;

}

/***************************************************
Reset
    Shuts down and restarts the winsock DLL
Params
    none
Return
    UTE_SUCCESS - success
    UTE_ERROR   - error
****************************************************/
int CUT_WSClient::Reset(){

    //shut down the winsock DLL
    WSACleanup();

    strcpy(m_szAddress,"");
    m_nRemotePort = 0;
    m_nLocalPort = 0;
    m_nAcceptPort = 0;

    m_socket = m_serverSocket = INVALID_SOCKET;


    //re -init the Windows Socket DLL
    if( WSAStartup(WINSOCKVER, &m_data) !=0)
        return OnError(UTE_ERROR);
    else
        return OnError(UTE_SUCCESS);


}

/***************************************************
GetHostAddress
    Returns the address of the local machine.
    It is possible for this function to fail
    since it is not guaranteed to be available
    in a winsock implementation ( since it is not
    part of the winsock specification)
Params
    address                 - buffer to return the address in
    maxLen                  - length of the buffer
    [useCurrentConnectAddr] - use current connect address flag
Return
    UTE_SUCCESS - success
    UTE_ERROR   - error
****************************************************/
#if defined _UNICODE
int CUT_WSClient::GetHostAddress(LPWSTR address, int maxLen, BOOL useCurrentConnectAddr){
    char *addressA = (char*) alloca(maxLen);
    *addressA = '\0';
    int result = GetHostAddress(addressA, maxLen, useCurrentConnectAddr);
    if( result == UTE_SUCCESS) {
        CUT_Str::cvtcpy(address, maxLen, addressA);
    }
    return result;}
#endif
int CUT_WSClient::GetHostAddress(LPSTR address, int maxLen, BOOL useCurrentConnectAddr){

    char name[WSC_BUFFER_SIZE];

    if(useCurrentConnectAddr == FALSE){
        if( gethostname(name,sizeof(name)) != SOCKET_ERROR){
            if(GetAddressFromName(name,address,maxLen) == UTE_SUCCESS)
                return OnError(UTE_SUCCESS);
        }
    }
    if(m_socket != INVALID_SOCKET){
        SOCKADDR_IN addr;
        int len = sizeof(SOCKADDR_IN);
        getsockname(m_socket,(LPSOCKADDR)&addr,&len);
        maxLen --;
        strncpy(address,inet_ntoa(addr.sin_addr),maxLen);
        address[maxLen] =0;
        return OnError(UTE_SUCCESS);
    }
    return OnError(UTE_ERROR);
}

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
BOOL CUT_WSClient::ReceiveFileStatus(long /*  bytesReceived */ ){
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
BOOL CUT_WSClient::SendFileStatus(long /* bytesSent */ ){
    return !IsAborted();
}

/***************************************************
Initasync
    Initializes the class for use with async winsock
    functions.
Params
    hInstance - instance handle of the application
Return
    UTE_SUCCESS - success
    UTE_ERROR   - error
****************************************************/
int CUT_WSClient::InitAsync( HINSTANCE hInstance){


    WNDCLASS wndclass;

    //if the window class is not yet registered then register it
    if(GetClassInfo(hInstance,_T("CUT_WSClient"),&wndclass)==FALSE){

        //register this new window class
        wndclass.style          = 0;
        wndclass.lpfnWndProc    = WndProc;
        wndclass.cbClsExtra     = 0 ;
        wndclass.cbWndExtra     = 20;
        wndclass.hInstance      = hInstance ;
        wndclass.hIcon          = NULL;
        wndclass.hCursor        = NULL;
        wndclass.hbrBackground  = NULL;
        wndclass.lpszMenuName   = NULL;
        wndclass.lpszClassName  = _T("CUT_WSClient");

        RegisterClass(&wndclass);

        if(GetClassInfo(hInstance,_T("CUT_WSClient"),&wndclass)==FALSE)
            return OnError(UTE_ERROR);
    }

    //create the window
    m_hAsyncWnd = CreateWindow(_T("CUT_WSClient"),
                _T(""),
                0,
                0,0,0,0,
                NULL,
                NULL,
                hInstance,
                NULL);

    if(m_hAsyncWnd == NULL)
        return UTE_ERROR;

    SendMessage(m_hAsyncWnd,CUT_SET_THIS_PTR,0,(LPARAM)this);

    return OnError(UTE_SUCCESS);
}

/***************************************************
WndProc
    Internal function
    This function is used to process async winsock
    messages.
Params
    n/a
Return
    n/a
****************************************************/
LRESULT CALLBACK CUT_WSClient::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message) {

        case WM_NCCREATE:
            {
                SetWindowLong(hwnd,0,(LPARAM)NULL);
                return 1;
            }
        case CUT_SET_THIS_PTR:
            {
                //store the pointer to the calling class
                // CUT_WSClient *_this = (CUT_WSClient*)lParam;
                SetWindowLong(hwnd,0,(LONG)lParam);
                return 1;
            }
    }

    // Get the pointer to the calling class
    CUT_WSClient *_this = (CUT_WSClient*)GetWindowLongPtr(hwnd,0);

    // Call the functions that match in incoming message
    if (_this == NULL)
        return DefWindowProc(hwnd, message, wParam, lParam);

    if (message == CUT_ASYNC_NOTIFY_MSG) {
        _this->OnAsyncNotify(WSAGETSELECTEVENT(lParam), WSAGETSELECTERROR(lParam));
        return 0;
    }
    else if (message == CUT_ASYNC_ATON_MSG) {
        _this->OnAsyncGetNameNotify(WSAGETASYNCERROR(lParam));
        return 0;
    }
    else if (message == CUT_ASYNC_NTOA_MSG) {
        _this->OnAsyncGetAddressNotify(WSAGETASYNCERROR(lParam));
        return 0;
    }

    // Degenerate case:
    return DefWindowProc(hwnd, message, wParam, lParam);
}

/***************************************************
OnAsyncNotify
    This virtual function is called when an async
    function has completed. The event param contains
    which function that just completed.
Params
    event - FD_READ FD_WRITE FD_OOB FD_ACCEPT FD_CONNECT FD_CLOSE
    error - the error code
return
    none
****************************************************/
void CUT_WSClient::OnAsyncNotify(int /* event */ ,int /* error */){
}
/***************************************************
****************************************************/
void CUT_WSClient::OnAsyncGetNameNotify(int /* error */){
}
/***************************************************
****************************************************/
void CUT_WSClient::OnAsyncGetAddressNotify(int /* error */){
}
/***************************************************
SetAsyncNotify
    Sets which functions that will have an async
    notification if used.
Param
    events - FD_READ FD_WRITE FD_OOB FD_ACCEPT FD_CONNECT FD_CLOSE
     (one or more of the above)
Return
    UTE_SUCCESS - success
    UTE_ERROR   - error
****************************************************/
int CUT_WSClient::SetAsyncNotify(long events){

    int iResult;

    if (m_socket != INVALID_SOCKET)
        iResult = WSAAsyncSelect(m_socket,m_hAsyncWnd,CUT_ASYNC_NOTIFY_MSG,events);
    else if (m_serverSocket != INVALID_SOCKET)
        iResult = WSAAsyncSelect(m_serverSocket,m_hAsyncWnd,CUT_ASYNC_NOTIFY_MSG,events);
    else
    {
        return OnError(UTE_ERROR);  // Call on of the socket creating functions
                                    // before calling SetAsyncNotify().
    }

    if(iResult == SOCKET_ERROR)
        return OnError(UTE_ERROR);

    // Changes !!! AG 14 April 99 !!!
    // Call to WSAAsyncSelect function automatically set a
    // socket to nonblocking mode. To set the socket back to
    // blocking mode, an application must first disable
    // WSAAsyncSelect by calling WSAAsyncSelect with the events
    // parameter equal to zero.
    m_nBlockingMode = CUT_NONBLOCKING;

    return OnError(UTE_SUCCESS);
}
/***************************************************
AsyncGetNameFromAddress
    Async version of GetNameFromAddress.
Params
    address - address to lookup the name on
    name - buffer for the name
    maxLen - buffer length
Return
    UTE_SUCCESS - success
    UTE_ERROR   - error
****************************************************/
int CUT_WSClient::AsyncGetNameFromAddress(LPCSTR address,LPSTR name,int maxLen){
    UNREFERENCED_PARAMETER(name);
    UNREFERENCED_PARAMETER(maxLen);

    unsigned long addr =  inet_addr(address);

    HANDLE h = WSAAsyncGetHostByAddr(m_hAsyncWnd,CUT_ASYNC_ATON_MSG,
        (const char *)&addr,sizeof(addr),
        PF_INET,(char FAR * )&m_hostent,MAXGETHOSTSTRUCT);


    if(h == NULL)
        return OnError(UTE_ERROR);

    return OnError(UTE_SUCCESS);
}

/***************************************************
AsyncGetAddressFromName
    Async Version of GetAddressFromName
Params
    name - name to lookup address on
    address - buffer for the address
    maxLen - buffer length
Return
    UTE_SUCCESS - success
    UTE_ERROR   - error
****************************************************/
int CUT_WSClient::AsyncGetAddressFromName(LPCSTR name,LPSTR address,int maxLen){

    UNREFERENCED_PARAMETER(address);
    UNREFERENCED_PARAMETER(maxLen);

    int             len,position;

    //check for @ symbol
    len = (int)strlen(name);
    for(position = len; position >0;position --){
        if(name[position]=='@'){
            position++;
            break;
        }
    }

    HANDLE h= WSAAsyncGetHostByName(m_hAsyncWnd,CUT_ASYNC_NTOA_MSG,
        &name[position],
        (char FAR * )&m_hostent,MAXGETHOSTSTRUCT);

    if(h == NULL)
        return OnError(UTE_ERROR);

    return OnError(UTE_SUCCESS);
}

/***************************************************
GetRemotePort
    returns the port number that the client is currently
    connect to on the remote host.  May be 0 if not connected.
Params
    none
Return
    port number
****************************************************/
int CUT_WSClient::GetRemotePort() const
{
    return m_nRemotePort;
}

/***************************************************
GetLocalPort
    returns the port number that the client is currently
    connect to on the local host.  May be 0 if not connected.
Params
    none
Return
    port number
****************************************************/
int CUT_WSClient::GetLocalPort() const
{
    return m_nLocalPort;
}

/***************************************************
GetAcceptPort
    returns the port number that the 'sever' socket
    is listening on for connections.  May be 0 if
    the server socket is not listening (WaitForConnect
    has not been called.)
Params
    none
Return
    port number
****************************************************/
int CUT_WSClient::GetAcceptPort() const
{
    return m_nAcceptPort;
}

/***************************************************
GetSocket
    returns the socket the the client is currently
    using
Params
    none
Return
    socket handle (SOCKET)
****************************************************/
SOCKET CUT_WSClient::GetSocket() const
{
    return m_socket;
}

/************************************************
SetSocketOption()
    Sets the current value for the socket option
    associated with the client socket, in any state, and stores
    the result in optval. Options may exist at multiple protocol levels,
    but they are always present at the uppermost "socket" level.
    Options affect socket operations, such as whether an operation
    blocks or not, the routing of packets, out-of-band data transfer, etc.
PARAM
    The optionValue parameter for this function depends on the option
    - option            - void *optionValue
    SO_ACCEPTCONN       Socket is listen()ing.
    SO_BROADCAST        Socket is configured for the transmission of broadcast messages.
    SO_DEBUG            Debugging is enabled.
    SO_DONTLINGER       If true, the SO_LINGER option is disabled.
    SO_DONTROUTE        Routing is disabled.
    SO_ERROR            Retrieve error status and clear.
    SO_KEEPALIVE        Keepalives are being sent.
    SO_LINGER           struct linger FAR *  Returns the current linger options.
    SO_OOBINLINE        Out-of-band data is being received in the normal data stream.
    SO_RCVBUF           int Buffer size for receives
    SO_REUSEADDR        The socket may be bound to an address which is already in use.
    SO_SNDBUF           int Buffer size for sends
    SO_TYPE             int The type of the socket (e.g. SOCK_STREAM).
    TCP_NODELAY         BOOL Disables the Nagle algorithm for send coalescing.
RETURN
    int     - 0 if success otherwise SOCKET_ERROR
***********************************************/
int CUT_WSClient::SetSocketOption(int option,void *optionValue, int iBufferSize)
{
    return setsockopt(m_socket,SOL_SOCKET,option,(char *)optionValue, iBufferSize);
}

/***********************************************************
GetSocketOption()
    retrieves the current value for the socket option
    associated with the client socket, in any state, and stores
    the result in optval. Options may exist at multiple protocol levels,
    but they are always present at the uppermost "socket" level.
    Options affect socket operations, such as whether an operation
    blocks or not, the routing of packets, out-of-band data transfer, etc.
PARAM
    The optionValue parameter for this function depends on the option
    - option            - void *optionValue
    SO_ACCEPTCONN       Socket is listen()ing.
    SO_BROADCAST        Socket is configured for the transmission of broadcast messages.
    SO_DEBUG            Debugging is enabled.
    SO_DONTLINGER       If true, the SO_LINGER option is disabled.
    SO_DONTROUTE        Routing is disabled.
    SO_ERROR            Retrieve error status and clear.
    SO_KEEPALIVE        Keepalives are being sent.
    SO_LINGER           struct linger FAR *  Returns the current linger options.
    SO_OOBINLINE        Out-of-band data is being received in the normal data stream.
    SO_RCVBUF           int Buffer size for receives
    SO_REUSEADDR        The socket may be bound to an address which is already in use.
    SO_SNDBUF           int Buffer size for sends
    SO_TYPE             int The type of the socket (e.g. SOCK_STREAM).
    TCP_NODELAY         BOOL Disables the Nagle algorithm for send coalescing.
RETURN
    int     - 0 if success otherwise SOCKET_ERROR
************************************************************/
int CUT_WSClient::GetSocketOption(int option,void *optionValue) const
{
    int size = sizeof(int);
    return getsockopt(m_socket, SOL_SOCKET, option, (char*) optionValue, &size);
}

/***************************************************
OnError
    This virtual function is called each time we return
    a value. It's a good place to put in an error messages
    or trace.
Params
    error - error code
Return
    error code
****************************************************/
int CUT_WSClient::OnError(int error)
{
    return error;
}

/***************************************************
IsAborted
    This virtual function is called during time consuming
    operations to check if we want to abort an operation
Params
    none
Return
    TRUE    - abort operation
    FALSE   - continue
****************************************************/
BOOL CUT_WSClient::IsAborted() {
    return FALSE;
}
/***************************************************
CreateSocket
Params
    sock            - reference for result SOCKET
    addressFamily   - protocol family: AF_INET, AF_AF_IPX, etc. Default AF_INET
    [sockType]      - SOCK_STREAM (TCP) or SOCK_DGRAM (UDP) Default is SOCK_STREAM
    [protocol]      - protocol
Return
    socket
    INVALID_SOCKET  - error
****************************************************/
int CUT_WSClient::CreateSocket(SOCKET &sock, int addressFamily, int socketType, int protocol)
{
    sock = socket(addressFamily, socketType, protocol);
    if(sock == INVALID_SOCKET) {
        return OnError(UTE_ERROR);
    }
    return OnError(UTE_SUCCESS);
}


/****************************************
 Inet_Addr
 Convert string representation of dotted
 ip address to unsigned long equivalent.

 RETURN
    unsigned long equivalent of address
    or INADDR_NONE
*****************************************/
unsigned long CUT_WSClient::Inet_Addr(LPCSTR string) {
    return inet_addr (string);
}
#if defined _UNICODE
unsigned long CUT_WSClient::Inet_Addr(LPCWSTR string) {
    if(string != NULL) {
        size_t len = wcslen(string);
        char * str = new char[len+1];
        CUT_Str::cvtcpy(str, len+1, string);
        return inet_addr(str);
    }
    else {
        return INADDR_NONE;
    }
}
#endif // _UNICODE

