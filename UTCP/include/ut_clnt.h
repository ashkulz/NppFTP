// =================================================================
//  class: CUT_WSClient
//  File:  UT_Clnt.h
//
//  Purpose:
//
//  Class declaration of the base winsock client class
//
//  This class encapsulates the functionality of winsock.dll in order
//  to give the user an object oriented framework for Winsock programming
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
NppFTP modifications:
simplify WINSOCKVER define
remove pragma statements+comments
Remove existing secure functionality
Add OpenSSL secure functionality
*/

#ifndef IncludeCUT_WSClient
#define IncludeCUT_WSClient

// =========================================================
//          WINSOCK LIBRARY & HEADER
#include <winsock2.h>     //use for Winsock v2.x
#define WINSOCKVER    MAKEWORD(2,0)

#include "uterr.h"
#include "utfile.h"
#include "utstrlst.h"
#include "UTDataSource.h"
#include "UT_Queue.h"

#include "openssl/ssl.h"
#include "openssl/err.h"

//async notification defines
#define CUT_SET_THIS_PTR        WM_USER+127
#define CUT_ASYNC_NOTIFY_MSG    WM_USER+128
#define CUT_ASYNC_ATON_MSG      WM_USER+129
#define CUT_ASYNC_NTOA_MSG      WM_USER+130

// blocking defines
#ifndef CUT_BLOCKING
#define CUT_BLOCKING        0
#define CUT_NONBLOCKING     1
#endif

#define WSC_BUFFER_SIZE     256


class CUT_Socket
{
public:
    CUT_Socket() {}
    virtual ~CUT_Socket() {}

    virtual int SocketOnConnected(SOCKET s, const char * lpszName)
    {
        UNREFERENCED_PARAMETER(s);
        UNREFERENCED_PARAMETER(lpszName);
        return UTE_SUCCESS;
    }

    virtual int SocketClose(SOCKET s)
    { return closesocket(s); }

    virtual int SocketShutDown(SOCKET s, int how)
    { return shutdown(s, how); }

    virtual int SocketRecv(SOCKET s, char FAR* buf, int len, int flags)
    { return  recv(s, buf, len, flags); }

    virtual int SocketSend(SOCKET s, char FAR* buf, int len, int flags)
    { return send(s, buf, len, flags); }

    virtual BOOL SocketIsDataWaiting(SOCKET s) const
    {
        unsigned long val;
        if (ioctlsocket(s, FIONREAD, &val) != 0)
            return FALSE;
        return (val > 0) ? TRUE : FALSE;
    }

    virtual int SocketWaitForReceive(SOCKET s, long secs, long uSecs)
    {
        fd_set readSet;
        FD_ZERO(&readSet);

        FD_SET(s, &readSet);

        struct timeval tv;
        tv.tv_sec = secs;
        tv.tv_usec = uSecs;

        // Wait up to the specified time to see if data is avail
        if( select(-1,&readSet,NULL,NULL,&tv)!= 1)
            return UTE_ERROR;

        return UTE_SUCCESS;
    }

};

// Define base class for the client
#ifndef CUT_CLIENT_BASE_CLASS
    #define CUT_CLIENT_BASE_CLASS   CUT_Socket
#endif

class CUT_WSClient : public CUT_CLIENT_BASE_CLASS
{
public:
    enum SSLMode {NONE, TLS, SSLv2, SSLv3, SSLv23};
    virtual int EnableSSL(bool enable);

    virtual int SetSecurityMode(SSLMode mode);

    virtual int ConnectSSL();
    virtual int DisconnectSSL();
    virtual int ResetSSL();
protected:
    virtual int OnLoadCertificates(SSL_CTX * ctx);
    virtual int OnSSLCertificate(const SSL * ssl, const X509* certificate, int verifyResult);   //return UTE_ERROR if certificate not accepted, UTE_SUCCESS if accepted. certificate can be NULL
protected:
    ////////////////////////////////////////////////////////////////////////////
    //
    // Data
    //

    unsigned int m_nRemotePort;  // port connected to on remote host
    unsigned int m_nLocalPort;   // port connected to on local host
    unsigned int m_nAcceptPort;  // port for listen/accept (WaitForConnect)

    char m_szAddress[30];        // IP address connected to
    int m_nProtocol;             // Protocol
    int m_nFamily;               // Address family - AF_INET
    int m_nSockType;             // Socket Type SOCK_STREAM (TCP) or SOCK_DGRAM (UDP)

    WSADATA m_data;              // loaded library  data
    SOCKET m_socket;             // socket for connections
    SOCKADDR_IN m_sockAddr;      // socket address

    SOCKET m_serverSocket;       // socket for basic server operations
    SOCKADDR_IN m_serverSockAddr;// server socket address

    int m_nBlockingMode;         // Blocking mode
    int m_nSuccessful;           // constructor success value

    HWND m_hAsyncWnd;            // window handle for async calls

    // timeout variables
    long m_lSendTimeOut;         // milli sec for a timeout
    long m_lRecvTimeOut;         // milli sec for a timeout

    BYTE m_hostent[MAXGETHOSTSTRUCT];

    ////////////////////////////////////////////////////////////////////////////
    //
    // SSL specific
    //

    bool            m_isSSL;
    bool            m_SSLconnected;
    SSLMode         m_sslMode;

    const SSL_METHOD *  m_meth;
    SSL_CTX *       m_ctx;
    SSL *           m_ssl;
    SSL_SESSION *   m_reuseSession;

    virtual int SSLSend(LPCSTR data, int len);
    virtual int SSLReceive(LPSTR buffer, int maxSize, bool peek = false);
    virtual SSL_SESSION * SSLGetCurrentSession();
    virtual int SSLSetReuseSession(SSL_SESSION * reuseSession); //use NULL to disable

    // This function is used to process async winsock
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

public:
    ////////////////////////////////////////////////////////////////////////////
    //
    // Object construct/destructor
    //

    CUT_WSClient();
    virtual ~CUT_WSClient();

    ////////////////////////////////////////////////////////////////////////////
    //
    // Connect / Create methods
    //

    // Connect. default type is TCP Stream, and family AF_INET
    virtual int Connect(unsigned int port, LPCSTR address, long timeout = -1,
        int family = AF_INET, int sockType = SOCK_STREAM);
#if defined _UNICODE
    virtual int Connect(unsigned int port, LPCWSTR address, long timeout = -1,
        int family = AF_INET, int sockType = SOCK_STREAM);
#endif

    // Calls bind prior to calling connect to associate the local address with the socket.
    virtual int ConnectBound(unsigned int localPort, unsigned int remotePort,
        LPSTR localAddress, LPSTR remoteAddress, int family, int sockType);
#if defined _UNICODE
    virtual int ConnectBound(unsigned int localPort, unsigned int remotePort,
        LPWSTR localAddress, LPWSTR remoteAddress, int family, int sockType);
#endif

#ifdef CUT_UDP_SOCKET
    // Creates a socket for UDP use, the socket is  setup and bound to the
    // given ports, but no connection is made yet
    virtual int CreateUDPSocket(unsigned int localPort, unsigned int remotePort,
        LPSTR remoteAddress, LPSTR localAddress = NULL);
#if defined _UNICODE
    virtual int CreateUDPSocket(unsigned int localPort, unsigned int remotePort,
        LPWSTR remoteAddress, LPWSTR localAddress = NULL);
#endif
#endif //CUT_UDP_SOCKET

    // Creates a socket
    virtual int CreateSocket(SOCKET &socket, int addressFamily,
        int socketType = SOCK_STREAM, int protocol = 0);

    // Terminate current connection
    virtual int CloseConnection();

    ////////////////////////////////////////////////////////////////////////////
    //
    // Initialization functions
    //

    // Shuts down and restarts the winsock DLL
    int Reset();

    //  Initializes the class for use with async winsock functions.
    virtual int InitAsync(HINSTANCE hInstance);

    ////////////////////////////////////////////////////////////////////////////
    //
    // Wait / Accpet methods
    //

    // Creates a socket and binds it to the given port.
    // Then puts the socket in a listening state
    virtual int WaitForConnect(unsigned short port, int queSize = 1,short family = AF_INET,
        unsigned long address = htonl(INADDR_ANY));

    //  Waits until the specified time to see if there
    //  is an incomming connection waiting with a read set
    virtual int WaitForAccept(long secs);

    // once a socket is set up for listening
    // this function is used to accept a connection
    virtual int AcceptConnection();

public:
    ////////////////////////////////////////////////////////////////////////////
    //
    // Accessor / Mutator methods
    //

    int IsIPAddress(LPCSTR szAddress) const;
#if defined _UNICODE
    int IsIPAddress(LPCWSTR szAddress) const;
#endif

        // added for v4.2 - winsock equivalent takes only char
        static unsigned long Inet_Addr(LPCSTR string);
#if defined _UNICODE
        static unsigned long Inet_Addr(LPCWSTR string);
#endif // _UNICODE

    // Checks to see if a connection is still alive
    virtual BOOL IsConnected();

    // Access functions
    SOCKET GetSocket() const;

    // current port number
    int GetRemotePort() const;
    int GetLocalPort() const;
    int GetAcceptPort() const;

    // is any data avialaable on the receive buffer
    BOOL IsDataWaiting() const;

    // Set/Get Blocking mode
    int SetBlockingMode(int mode);
    int GetBlockingMode() const;

    // Set/Get the timout value for the send operation
    int SetSendTimeOut(int milisecs);
    int GetSendTimeOut() const;

    // Set/Get the timout value for the send operation
    int SetReceiveTimeOut(int milisecs);
    int GetReceiveTimeOut() const;

    // Set/Get a socket option for the client socket
    int SetSocketOption(int option, void *optionValue, int iBufferSize = 4);
    int GetSocketOption(int option, void *optionValue) const;

    // Get the send buffer size for the client socket
    int GetMaxSend() const;
    // Set the send buffer size for the client socket
    int SetMaxSend(int length);

    //  Get/Set the Buffer size for receives for the client socket
    int SetMaxReceive(int length);
    int GetMaxReceive() const;

    ////////////////////////////////////////////////////////////////////////////
    //
    // Database functions
    //

    int GetNameFromAddress(LPCSTR address, LPSTR name, int maxLen);
    int GetAddressFromName(LPCSTR name, LPSTR address, int maxLen);
#if defined _UNICODE
    int GetNameFromAddress(LPCWSTR address, LPWSTR name, int maxLen);
    int GetAddressFromName(LPCWSTR name, LPWSTR address, int maxLen);
#endif
    // Get the local machine address - set useCurrentConnectAddr
    // if you need the IP assigned by an ISP you are connected to
    // rather than the IP of your LAN.
    int GetHostAddress(LPSTR address, int maxLen, BOOL useCurrentConnectAddr = TRUE);
#if defined _UNICODE
    int GetHostAddress(LPWSTR address, int maxLen, BOOL useCurrentConnectAddr = TRUE);
#endif

    //  Sets the events which will have an async notification for if used.
    // use | to combine the events
    int SetAsyncNotify(long events);

    int AsyncGetNameFromAddress(LPCSTR address, LPSTR name, int maxLen);
    int AsyncGetAddressFromName(LPCSTR name, LPSTR address, int maxLen);

    ////////////////////////////////////////////////////////////////////////////
    //
    // Send / Receive methods
    //

    // Send functions
    // changed to a default parameter instead of overloaded functions
    virtual int Send(LPCSTR data, int len = -1);
#if defined _UNICODE
    virtual int Send(LPCWSTR data, int len = -1);
#endif

    // Send entire blob of data
    virtual int SendBlob(LPBYTE data,int dataLen);

    // Send from the data source
    virtual int Send(CUT_DataSource & source);

    // Send from the queue
    virtual int Send(CUT_Queue & source);

    // Sends the given data and breaks the data up into
    // lines of the specified line lengths by adding
    // \r\n to the data
    virtual int SendAsLine(LPCSTR data, int len, int lineLen);
#if defined _UNICODE
    virtual int SendAsLine(LPCWSTR data, int len, int lineLen);
#endif

    // Send a file
    virtual int SendFile(LPCTSTR path);

#ifdef CUT_UDP_SOCKET
    // Sends the specified data accross the UDP
    // connection (See CreateUDPSocket)
    virtual int SendTo(LPCSTR data, int &dataLen);
#if defined _UNICODE
    virtual int SendTo(LPCWSTR data, int &dataLen);
#endif
#endif //CUT_UDP_SOCKET
    //  Waits up til the specified time to see if the
    //  connection is ready to send data if it is then
    //  it will return CUT_SUCCESS
    virtual int WaitForSend(long secs, long uSecs);

    // Receive functions
    virtual int Receive(LPSTR data, int maxLen, int timeOut = 0);

    // Receive entire blob of data
    virtual int ReceiveBlob(LPBYTE data, int dataLen, int timeOut = 0);

    // Receive from the receive buffer of the connected socket to a data source
    virtual int Receive(CUT_DataSource & dest, OpenMsgType type, int timeOut = 0, long lMaxToReceive = 0);

    // Receive from the receive buffer of the connected socket to a queue
    virtual int Receive(CUT_Queue & dest, int timeOut = 0, long lMaxToReceive = 0);

    // Receive from the receive buffer of the connected socket to a disk file
    virtual int ReceiveToFile(LPCTSTR name, OpenMsgType type, int timeOut = 0);

    // receive as a line searching for \r\n
    // changed to default value instead of overloaded functions (MARCH 1999)
    virtual int ReceiveLine(LPSTR data, int maxDataLen, int timeOut = 0);

    // clear receive buffer from any data waiting to be received
    virtual int ClearReceiveBuffer();

#ifdef CUT_UDP_SOCKET
    //  Receives data from a UDP connection (see CreateUDPSocket)
    virtual int ReceiveFrom(LPSTR data, int &dataLen, unsigned long timeout);
#endif //CUT_UDP_SOCKET

    // Wait until the specified time
    // to see if there is data to be received.
    virtual int WaitForReceive(long secs,long uSecs = 0);

protected:
    ////////////////////////////////////////////////////////////////////////////
    //
    // Overridables
    //

    // Return the error code passed. Called each time an error occurs
    virtual int OnError(int error);

    // Monitor progress and/or cancel a receive operation
    virtual BOOL ReceiveFileStatus(long bytesReceived);

    // Monitor progress and/or cancel a send operation
    virtual BOOL SendFileStatus(long bytesSent);

    // Return TRUE if we are going to abort operation
    virtual BOOL IsAborted();

    virtual void OnAsyncNotify(int event,int error);
    virtual void OnAsyncGetNameNotify(int error);
    virtual void OnAsyncGetAddressNotify(int error);

// v4.2 Removed these to kill 4511 and 4512 warnings for derived classes (VC6)
//private:
//    CUT_WSClient& operator=(const CUT_WSClient& rhs); // no implementation
//    CUT_WSClient(const CUT_WSClient& src);            // no implementation

};


#endif // IncludeCUT_WSClient


