// ===================================================================
//     Class: CUT_ERR
//      File: UTERR.h
//   Created: March 16, 1999
//   Revised:
//   Purpose: Provide localized init for error constants and
//              error string associations.  Avoids preprocessor
//              defines for these values.
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
Remove pragma statements
*/


#ifndef UTERR_H_INCLUDED
#define UTERR_H_INCLUDED

#include <tchar.h>


// =========================================================
//          WINSOCK LIBRARY & HEADER
// =========================================================
#include <winsock2.h>   //use for Winsock v2.x


//standard success and error codes
#ifndef CUT_SUCCESS
    #define CUT_SUCCESS 0
#endif

#ifndef CUT_ERROR
    #define CUT_ERROR   1
#endif

typedef enum {
        UTE_SUCCESS                 = CUT_SUCCESS,
        UTE_ERROR                   = CUT_ERROR,

        // Connection errors
        UTE_CONNECT_FAILED,
        UTE_CONNECT_REJECTED,
        UTE_CONNECT_TERMINATED,
        UTE_CONNECT_TIMEOUT,
        UTE_NOCONNECTION,
        UTE_NAME_LOOKUP_FAILED,
        UTE_DATAPORT_FAILED,
        UTE_ACCEPT_FAILED,

        // Server errors
        UTE_SVR_REQUEST_DENIED,
        UTE_SVR_NOT_SUPPORTED,
        UTE_SVR_NO_RESPONSE,
        UTE_SVR_ACCESS_DENIED,
        UTE_SVR_DATA_CONNECT_FAILED,

        // Socket errors
        UTE_SOCK_NOT_OPEN,
        UTE_SOCK_ALREADY_OPEN,
        UTE_SOCK_CREATE_FAILED,
        UTE_SOCK_BIND_FAILED,
        UTE_SOCK_CONNECT_FAILED,
        UTE_SOCK_TIMEOUT,
        UTE_SOCK_RECEIVE_ERROR,
        UTE_SOCK_SEND_ERROR,
        UTE_SOCK_LISTEN_ERROR,
        UTE_CLIENT_RESET,
        UTE_SERVER_RESET,

        // File errors
        UTE_FILE_TYPE_ERROR,
        UTE_FILE_OPEN_ERROR,
        UTE_FILE_CREATE_ERROR,
        UTE_FILE_READ_ERROR,
        UTE_FILE_WRITE_ERROR,
        UTE_FILE_CLOSE_ERROR,
        UTE_FILE_ERROR,
        UTE_FILE_FORMAT_ERROR,
        UTE_FILE_TMP_NAME_FAILED,

        // Buffer errors
        UTE_BUFFER_TOO_SHORT,
        UTE_NULL_PARAM,

        // Response errors
        UTE_INVALID_RESPONSE,
        UTE_NO_RESPONSE,

        // Index errors
        UTE_INDEX_OUTOFRANGE,

        // User validation errors
        UTE_USER_ERROR,
        UTE_PASSWORD_ERROR,

        // Message errors
        UTE_INVALID_MESSAGE,
        UTE_INVALID_FORMAT,
        UTE_FILE_NOT_MIME,

        // URL errors
        UTE_BAD_URL,

        // Command errors
        UTE_INVALID_COMMAND,
        UTE_MAIL_FAILED,
        UTE_RETR_FAILED,
        UTE_PORT_FAILED,
        UTE_LIST_FAILED,
        UTE_STOR_FAILED,
        UTE_DATA_FAILED,
        UTE_USER_FAILED,
        UTE_HELLO_FAILED,
        UTE_PASS_FAILED,
        UTE_STAT_FAILED,
        UTE_TOP_FAILED,
        UTE_UIDL_FAILED,
        UTE_DELE_FAILED,
        UTE_RSET_FAILED,
        UTE_XOVER_FAILED,
        UTE_USER_NA,
        UTE_PASS_NA,
        UTE_ACCT_NA,
        UTE_RNFR_NA,
        UTE_RNTO_NA,
        UTE_RCPT_FAILED,
        UTE_NNTP_BAD_ARTICLE,
        UTE_NNTP_NOPOSTING,
        UTE_NNTP_POST_FAILED,
        UTE_NNTP_AUTHINFO_USER_FAILED,
        UTE_NNTP_AUTHINFO_PASS_FAILED,
        UTE_XOVER_COMMAND_FAILED,

        // Message errors
        UTE_MSG_OPEN_FAILED,
        UTE_MSG_CLOSE_FAILED,
        UTE_MSG_WRITE_LINE_FAILED,
        UTE_MSG_READ_LINE_FAILED,
        UTE_MSG_NO_ATTACHMENTS,
        UTE_MSG_BODY_TOO_BIG,
        UTE_MSG_ATTACHMENT_ADD_FAILED,

        // Data source errors
        UTE_DS_OPEN_FAILED,
        UTE_DS_CLOSE_FAILED,
        UTE_DS_WRITE_FAILED,

        // Encoding errors
        UTE_ENCODING_INVALID_CHAR,
        UTE_ENCODING_LINE_TOO_LONG,

        // IMAP4 errors
        UTE_LOGIN_FAILED,
        UTE_NOOP_FAILED,
        UTE_UNKNOWN_COMMAND,
        UTE_UNKNOWN_RESPONSE,
        UTE_AUTH_OR_SELECTED_STATE_REQUIRED,
        UTE_SELECTED_STATE_REQUIRED,


        //RAS Errors
        UTE_RAS_LOAD_ERROR,
        UTE_RAS_DIAL_ERROR,
        UTE_RAS_DIALINIT_ERROR,
        UTE_RAS_HANDLE_ERROR,
        UTE_RAS_ENUM_ERROR,
        UTE_RAS_ENTRYNAME_ERROR,

        // Unclassified errors
        UTE_ABORTED,
        UTE_BAD_HOSTNAME,
        UTE_INVALID_ADDRESS,
        UTE_INVALID_ADDRESS_FORMAT,
        UTE_USER_TERMINATED,
        UTE_ANS_NOT_FOUND,
        UTE_SERVER_SET_NAME_FAILED,
        UTE_PARAMETER_TOO_LONG,
        UTE_PARAMETER_INVALID_VALUE,
        UTE_TEMP_FILENAME_FAILED,
        UTE_OUT_OF_MEMORY,
        UTE_GROUP_INFO_UPDATE_FAILED,
        UTE_GROUP_NOT_SELECTED,
        UTE_INTERNAL_ERROR,
        UTE_ALREADY_IN_USE,
        UTE_NO_CURRENT_MSG_SET,
        UTE_QUOTE_LINE_IS_EMPTY,
        UTE_REST_COMMAND_NOT_SUPPORTED,
        UTE_SYSTEM_INFO_LOAD_FAILED,
        UTE_USER_INFO_LOAD_FAILED,
        UTE_USER_NAME_ALREDY_EXIST,
        UTE_MAILBOX_NAME_ALREDY_EXIST,

        UTE_LOAD_SECURITY_LIBRARIES_FAILED,
        UTE_OPEN_CERTIFICATE_STORE_FAILED,
        UTE_FAILED_TO_FIND_CERTIFICATE,
        UTE_FAILED_TO_CREATE_SECURITY_CREDENTIALS,
        UTE_FAILED_TO_INITIALIZE_SECURITY_CONTEXT,
        UTE_FAILED_TO_ACCEPT_SECURITY_CONTEXT,
        UTE_HANDSHAKE_FAILED,
        UTE_FAILED_TO_QUERY_CERTIFICATE,
        UTE_FAILED_TO_GET_CERTIFICATE_CHAIN,
        UTE_FAILED_TO_VERIFY_CERTIFICATE_CHAIN,
        UTE_FAILED_TO_VERIFY_CERTIFICATE_TRUST,
        UTE_FAILED_TO_GET_SECURITY_STREAM_SIZE,
        UTE_FAILED_TO_APPLY_CONTROL_TOKEN,
        UTE_FAILED_TO_RECEIVE_SECURITY_MESSAGE,
        UTE_SECURITY_CONTEXT_EXPIRED,
        UTE_FAILED_TO_DECRYPT_SECURITY_MESSAGE,
        UTE_FAILED_TO_ENCRYPT_SECURITY_MESSAGE,
        UTE_INVALID_CHARS_IN_STRING_PARAM,
        UTE_FAILED_TO_GENERATE_SECURITY_KEY,
        UTE_FAILED_TO_EXPORT_SECURITY_KEY,
        UTE_OPEN_CERTIFICATE_STORE_FIRST,
        UTE_FAILED_CREATE_ICENROLL,
        UTE_CONNECT_FAIL_NO_SSL_SUPPORT,
        UTE_CERTIFICATE_INVALID_DATE,

        UTE_IMAP4_TLS_NOT_SUPPORTED,
        UTE_SMTP_TLS_NOT_SUPPORTED,
        UTE_POP3_TLS_NOT_SUPPORTED,

        UTE_UNSUPPORTED_KEY_SIZE,

        UTE_UNSUPPORTED_ENCODING_TYPE,
        UTE_INVALID_CHARACTER_IN_CHARSET,
        UTE_CHARSET_TOO_BIG,
        UTE_INVALID_ENCODING_FORMAT,


        // Include extended error
        #include "UTExtErr.h"

        // always leave this last - doubles as constant for string
        // array declaration.
        CUT_MAX_ERROR
    } CUT_ERROR_CONSTANTS;       // this variable assingment gives the
                                 // user something to click on
#ifndef __midl

#ifndef _TCHAR          // redundant include guard. (added for console app)
    #include <tchar.h>
#endif

class CUT_ERR {

private:

    // Note that CTOR and DTOR are private - we do not
    // instantiate this class.
    CUT_ERR();      // unimplemented
    ~CUT_ERR();     // unimplemented

    // Note that both copy constuctor and assignment op
    // private - probably overkill, since class can never
    // be instantiated, but safer.
    CUT_ERR(const CUT_ERR& );           // unimplemented
    CUT_ERR &operator=(const CUT_ERR&);  // unimplemented


public:
    // enum for UT error codes.
    //
    // We stay out of the of the winsock SOCKET_ERROR, which is #defined as -1
    //
    // Note that we don't need the assignments here - and in fact it leaves
    // us prone to errors - but it may be helpful to be able to know what the enumeration
    // value is without counting or using the debugger.
private:

    /******************************************************************
    // InitErrorString is declared here in the header to facilitate
    // setup.

    // Adding error constants and their related strings is a two stage
    // process - adding another enum and setting a string value for it.
    ******************************************************************/
    static void InitErrorStrings(const _TCHAR **pStrings) {
        pStrings[UTE_ERROR]                 = _T("Operation failed.");
        pStrings[UTE_SUCCESS]               = _T("Operation completed successfully.");

        // Connection errors
        pStrings[UTE_CONNECT_FAILED]        = _T("The connection failed.");
        pStrings[UTE_CONNECT_REJECTED]      = _T("The connection was rejected.");
        pStrings[UTE_CONNECT_TERMINATED]    = _T("The connection was terminated.");
        pStrings[UTE_CONNECT_TIMEOUT]       = _T("The connection timed-out.");
        pStrings[UTE_NOCONNECTION]          = _T("There was no connection.");
        pStrings[UTE_NAME_LOOKUP_FAILED]    = _T("Name lookup failed.");
        pStrings[UTE_DATAPORT_FAILED]       = _T("Data port could not be opened.");
        pStrings[UTE_ACCEPT_FAILED]         = _T("Accept failed.");

        // Server errors
        pStrings[UTE_SVR_REQUEST_DENIED]    = _T("The request was denied by the server.");
        pStrings[UTE_SVR_NOT_SUPPORTED]     = _T("The request is not supported by the server.");
        pStrings[UTE_SVR_NO_RESPONSE]       = _T("There was no response from the server.");
        pStrings[UTE_SVR_ACCESS_DENIED]     = _T("Permission denied by the server.");
        pStrings[UTE_SVR_DATA_CONNECT_FAILED] = _T("The Server failed to connect on the data port.");

        // Socket errors
        pStrings[UTE_SOCK_NOT_OPEN]         = _T("The socket is not opened.");
        pStrings[UTE_SOCK_ALREADY_OPEN]     = _T("The socket is already opened or in use.");
        pStrings[UTE_SOCK_CREATE_FAILED]    = _T("The socket creation failed.");
        pStrings[UTE_SOCK_BIND_FAILED]      = _T("The socket binding to local port failed.");
        pStrings[UTE_SOCK_CONNECT_FAILED]   = _T("The socket connection failed.");
        pStrings[UTE_SOCK_TIMEOUT]          = _T("A timeout occurred.");
        pStrings[UTE_SOCK_RECEIVE_ERROR]    = _T("A receive socket error occurred.");
        pStrings[UTE_SOCK_SEND_ERROR]       = _T("Winsock send command failed.");
        pStrings[UTE_SOCK_LISTEN_ERROR]     = _T("The listen process failed.");
        pStrings[UTE_CLIENT_RESET]          = _T("A client socket closure  caused a failure.");
        pStrings[UTE_SERVER_RESET]          = _T("A server socket closure caused failure.");

        // File errors
        pStrings[UTE_FILE_TYPE_ERROR]       = _T("An error occurred as a result of an invalid file type.");
        pStrings[UTE_FILE_OPEN_ERROR]       = _T("An error occurred as a result of not being able to open a file.");
        pStrings[UTE_FILE_CREATE_ERROR]     = _T("An error occurred as a result of not being able to create a file.");
        pStrings[UTE_FILE_READ_ERROR]       = _T("A file read error occurred.");
        pStrings[UTE_FILE_WRITE_ERROR]      = _T("A file write error occurred.");
        pStrings[UTE_FILE_CLOSE_ERROR]      = _T("Error trying to close file.");
        pStrings[UTE_FILE_ERROR]            = _T("There was no output file name provided, and no output file name was included with the attachment.");
        pStrings[UTE_FILE_FORMAT_ERROR]     = _T("File format error.");
        pStrings[UTE_FILE_TMP_NAME_FAILED]  = _T("Get temporary file name failed.");

        // Buffer errors
        pStrings[UTE_BUFFER_TOO_SHORT]      = _T("An error resulted due to the buffer being too short.");
        pStrings[UTE_NULL_PARAM]            = _T("An error resulted due to a NULL buffer");

        // Response errors
        pStrings[UTE_INVALID_RESPONSE]      = _T("An error occurred as a result of an invalid or negative response.");
        pStrings[UTE_NO_RESPONSE]           = _T("There was no response.");

        // Index errors
        pStrings[UTE_INDEX_OUTOFRANGE]      = _T("The index value was out of range.");

        // User validation errors
        pStrings[UTE_USER_ERROR]            = _T("The user name was invalid.");
        pStrings[UTE_PASSWORD_ERROR]        = _T("The password was invalid.");

        // Message errors
        pStrings[UTE_INVALID_MESSAGE]       = _T("This is a malformed message.");
        pStrings[UTE_INVALID_FORMAT]        = _T("Invalid format.");
        pStrings[UTE_FILE_NOT_MIME]         = _T("Not a MIME file.");

        // URL errors
        pStrings[UTE_BAD_URL]               = _T("A bad URL was given.");

        // Command errors
        pStrings[UTE_INVALID_COMMAND]       = _T("An invalid command.");
        pStrings[UTE_MAIL_FAILED]           = _T("MAIL command failed.");
        pStrings[UTE_RETR_FAILED]           = _T("The RETR command failed.");
        pStrings[UTE_PORT_FAILED]           = _T("The PORT command failed.");
        pStrings[UTE_LIST_FAILED]           = _T("The LIST command failed.");
        pStrings[UTE_STOR_FAILED]           = _T("The STOR command failed.");
        pStrings[UTE_DATA_FAILED]           = _T("The DATA command failed.");
        pStrings[UTE_USER_FAILED]           = _T("The USER command failed.");
        pStrings[UTE_HELLO_FAILED]          = _T("The HELLO command failed.");
        pStrings[UTE_PASS_FAILED]           = _T("The PASS command failed.");
        pStrings[UTE_STAT_FAILED]           = _T("The STAT command failed.");
        pStrings[UTE_TOP_FAILED]            = _T("The TOP command failed.");
        pStrings[UTE_UIDL_FAILED]           = _T("The UIDL command failed.");
        pStrings[UTE_DELE_FAILED]           = _T("The DELE command failed.");
        pStrings[UTE_RSET_FAILED]           = _T("The RSET command failed.");
        pStrings[UTE_XOVER_FAILED]          = _T("The XOVER command failed.");
        pStrings[UTE_USER_NA]               = _T("The USER command was not accepted.");
        pStrings[UTE_PASS_NA]               = _T("The PASS command was not accepted.");
        pStrings[UTE_ACCT_NA]               = _T("The ACCT command was not accepted.");
        pStrings[UTE_RNFR_NA]               = _T("The RNFR command not accepted.");
        pStrings[UTE_RNTO_NA]               = _T("The RNTO command not accepted.");
        pStrings[UTE_RCPT_FAILED]           = _T("The RCPT command failed. The specified account does not exsist.");
        pStrings[UTE_NNTP_BAD_ARTICLE]      = _T("Bad article, posting rejected by server");
        pStrings[UTE_NNTP_NOPOSTING]        = _T("Posting is not allowed.");
        pStrings[UTE_NNTP_POST_FAILED]      = _T("Posting rejected by server");
        pStrings[UTE_NNTP_AUTHINFO_USER_FAILED]= _T("The AUTHINFO USER command failed.");
        pStrings[UTE_NNTP_AUTHINFO_PASS_FAILED]= _T("The AUTHINFO PASS command failed.");
        pStrings[UTE_XOVER_COMMAND_FAILED] = _T("The XOVER command failed.");


        // Message errors
        pStrings[UTE_MSG_OPEN_FAILED]       = _T("Can't open message data source.");
        pStrings[UTE_MSG_CLOSE_FAILED]      = _T("Close message data source failed.");
        pStrings[UTE_MSG_READ_LINE_FAILED]  = _T("Failed to read a line from the message data source.");
        pStrings[UTE_MSG_WRITE_LINE_FAILED] = _T("Failed to write a line to the message data source.");
        pStrings[UTE_MSG_NO_ATTACHMENTS]    = _T("There is no any attachments in the message data source.");
        pStrings[UTE_MSG_BODY_TOO_BIG]      = _T("Message body exceeds maximum length.");
        pStrings[UTE_MSG_ATTACHMENT_ADD_FAILED]= _T("Failed to add attachment to the message.");

        // Data source errors
        pStrings[UTE_DS_OPEN_FAILED]        = _T("Failed to open data source.");
        pStrings[UTE_DS_CLOSE_FAILED]       = _T("Failed to close data source.");
        pStrings[UTE_DS_WRITE_FAILED]       = _T("Failed to write to the data source.");

        // Encoding errors
        pStrings[UTE_ENCODING_INVALID_CHAR] = _T("Invalid character in the stream.");
        pStrings[UTE_ENCODING_LINE_TOO_LONG]= _T("Too many characters on one line.");

        // IMAP4 errors
        pStrings[UTE_LOGIN_FAILED]          = _T("Server login failed. Username or password invalid.");
        pStrings[UTE_NOOP_FAILED]           = _T("NOOP command failed.");
        pStrings[UTE_UNKNOWN_COMMAND]       = _T("Command unknown or arguments invalid.");
        pStrings[UTE_UNKNOWN_RESPONSE]      = _T("Unknown response.");
        pStrings[UTE_AUTH_OR_SELECTED_STATE_REQUIRED]   = _T("You must login first.");
        pStrings[UTE_SELECTED_STATE_REQUIRED]   = _T("You must select the mailbox first.");

        //RAS Errors
        pStrings[UTE_RAS_LOAD_ERROR]        = _T("Unable to load the required RAS DLLs");
        pStrings[UTE_RAS_DIAL_ERROR]        = _T("An error occurred during the dialing process");
        pStrings[UTE_RAS_DIALINIT_ERROR]        = _T("An error occurred when attempting to dial");
        pStrings[UTE_RAS_HANDLE_ERROR]      = _T("An invalid RAS handle was given");
        pStrings[UTE_RAS_ENUM_ERROR]        = _T("An error occurred when performing a RAS enumeration");
        pStrings[UTE_RAS_ENTRYNAME_ERROR]       = _T("An invalid or non-existant RAS entry name was given");

        // Unclassified errors
        pStrings[UTE_ABORTED]               = _T("Aborted by user.");
        pStrings[UTE_BAD_HOSTNAME]          = _T("A bad hostname format.");
        pStrings[UTE_INVALID_ADDRESS]       = _T("The address is not valid.");
        pStrings[UTE_INVALID_ADDRESS_FORMAT]= _T("The address format is not valid.");
        pStrings[UTE_USER_TERMINATED]       = _T("User terminated the process.");
        pStrings[UTE_ANS_NOT_FOUND]         = _T("The authorized name server was not found.");
        pStrings[UTE_SERVER_SET_NAME_FAILED]= _T("Unable to set the server name.");
        pStrings[UTE_PARAMETER_TOO_LONG]    = _T("Parameter too long.");
        pStrings[UTE_PARAMETER_INVALID_VALUE]= _T("Invalid value of the parameter.");
        pStrings[UTE_TEMP_FILENAME_FAILED]  = _T("Get temorary filename failed.");
        pStrings[UTE_OUT_OF_MEMORY]         = _T("Out of memory.");
        pStrings[UTE_GROUP_INFO_UPDATE_FAILED]= _T("Update of group information failed.");
        pStrings[UTE_GROUP_NOT_SELECTED]    = _T("No selected news group.");
        pStrings[UTE_INTERNAL_ERROR]        = _T("Internal error.");
        pStrings[UTE_ALREADY_IN_USE]        = _T("Already in use.");
        pStrings[UTE_NO_CURRENT_MSG_SET]    = _T("No current message set.");
        pStrings[UTE_QUOTE_LINE_IS_EMPTY]   = _T("The Quote Command was empty");
        pStrings[UTE_REST_COMMAND_NOT_SUPPORTED]=_T("The REST command is not supported by the server");
        pStrings[UTE_SYSTEM_INFO_LOAD_FAILED]=_T("Failed to load system information.");
        pStrings[UTE_USER_INFO_LOAD_FAILED] =_T("Failed to load user information.");
        pStrings[UTE_USER_NAME_ALREDY_EXIST]=_T("User with this name is alredy existing.");
        pStrings[UTE_MAILBOX_NAME_ALREDY_EXIST]=_T("Mailbox with this name is alredy existing.");

        pStrings[UTE_LOAD_SECURITY_LIBRARIES_FAILED]=_T("Can't load the security libraries.");
        pStrings[UTE_OPEN_CERTIFICATE_STORE_FAILED]=_T("Can't open the certificate store.");
        pStrings[UTE_FAILED_TO_FIND_CERTIFICATE]=_T("Can't find the certificate.");
        pStrings[UTE_FAILED_TO_CREATE_SECURITY_CREDENTIALS]=_T("Can't create security credentials.");
        pStrings[UTE_FAILED_TO_INITIALIZE_SECURITY_CONTEXT]=_T("Failed to initialize security context.");
        pStrings[UTE_FAILED_TO_ACCEPT_SECURITY_CONTEXT]=_T("Faile to accept security context.");
        pStrings[UTE_HANDSHAKE_FAILED]          =_T("Security handshake failed.");
        pStrings[UTE_FAILED_TO_QUERY_CERTIFICATE] =_T("Failed to query the certificate.");
        pStrings[UTE_FAILED_TO_GET_CERTIFICATE_CHAIN] =_T("Failed to get the certificate chain.");
        pStrings[UTE_FAILED_TO_VERIFY_CERTIFICATE_CHAIN] =_T("Failed to verify the certificate chain.");
        pStrings[UTE_FAILED_TO_VERIFY_CERTIFICATE_TRUST] =_T("Failed to verify the certificate trust.");
        pStrings[UTE_FAILED_TO_GET_SECURITY_STREAM_SIZE] =_T("Failed to get the security stream sizes.");
        pStrings[UTE_FAILED_TO_APPLY_CONTROL_TOKEN] =_T("Failed to apply control token.");
        pStrings[UTE_FAILED_TO_RECEIVE_SECURITY_MESSAGE] =_T("Failed to receive the security message.");
        pStrings[UTE_SECURITY_CONTEXT_EXPIRED] =_T("Security context expired.");
        pStrings[UTE_FAILED_TO_DECRYPT_SECURITY_MESSAGE] =_T("Failed to decrypt security message.");
        pStrings[UTE_FAILED_TO_ENCRYPT_SECURITY_MESSAGE] =_T("Failed to encrypt security message.");
        pStrings[UTE_INVALID_CHARS_IN_STRING_PARAM] =_T("String parameter contains invalid characters.");
        pStrings[UTE_FAILED_TO_GENERATE_SECURITY_KEY] =_T("Failed to create security key.");
        pStrings[UTE_FAILED_TO_EXPORT_SECURITY_KEY] =_T("Failed to export security key.");
        pStrings[UTE_OPEN_CERTIFICATE_STORE_FIRST]=_T("The certificate store is not opened.");
        pStrings[UTE_FAILED_CREATE_ICENROLL]=_T("Failed to create the Certificate Enrollment object.");
        pStrings[UTE_CONNECT_FAIL_NO_SSL_SUPPORT]=_T("Server does not support SSL. Connection failed.");
        pStrings[UTE_CERTIFICATE_INVALID_DATE]=_T("Security certificate date is not valid.");
        pStrings[UTE_IMAP4_TLS_NOT_SUPPORTED]=_T("STARTTLS not supported by the IMAP4 server.");
        pStrings[UTE_SMTP_TLS_NOT_SUPPORTED]=_T("STARTTLS not supported by the SMTP server.");
        pStrings[UTE_POP3_TLS_NOT_SUPPORTED]=_T("TLS not supported by the POP3 server.");

        pStrings[UTE_UNSUPPORTED_KEY_SIZE]=_T("Base Cryptographic Provider does not support Key Lengths more than 512 bits.");

        pStrings[UTE_UNSUPPORTED_ENCODING_TYPE] =_T("Wrong encoding type. Must be \'Q\' or \'B\'.");
        pStrings[UTE_INVALID_CHARACTER_IN_CHARSET]  =_T("Invalid character in the CharSet string.");
        pStrings[UTE_CHARSET_TOO_BIG]           =_T("Character set name is too big.");
        pStrings[UTE_INVALID_ENCODING_FORMAT]   =_T("Invalid encoding format.");

        // Include extended error strings
        #define _EXT_ERR_STRINGS
        #include "UTExtErr.h"
    }


public:


    /******************************************************************
    NOTE: this is definitely not intended to be inline!
    ******************************************************************/
    static const _TCHAR* GetErrorString(int err) {
        // ErrorStrings is declared at function (not class) scope since
        // we have no constuctor to provide for initialization.

        // init all to 0
        static const _TCHAR *ErrorStrings[CUT_MAX_ERROR] = {0};

        static int initDone = 0;

        // has init been performed?
        if(0 == initDone) {
            // do this once... a bit of overhead, but not much.
            InitErrorStrings(&ErrorStrings[0]);
            initDone = 1;
            }

        // 'real' fn body...
        const _TCHAR* temp;
        if( -1 < err && CUT_MAX_ERROR > err) {
            if (*(ErrorStrings+err) == 0)
                temp =  _T("CUT_ERR::GetErrorString: ErrorString array improperly initialized");
            else
                temp =  *(ErrorStrings+err);
            }
        else {
             temp = _T("CUT_ERR::GetErrorString: Array out of bounds!");
             }


        return temp;
    }


/**********************************************
    GetSocketError

    Rather than litter the code with switches
    for debugging SOCKET_ERROR return codes
    from the SDK, use this generic routine.

    In:     the socket in question, and an
            optional address of a integer to
            hold the winsock error code on return.

    Return: A string constant representing a
            readable message associated with the
            error.

    NOTES:  this uses getsockopt rather than
            WSAGetLastError so the return is specific
            to this socket.)

            No data other than the optional result
            param is written, so threading should
            not be an issue, at the expense of a
            multitude of return statements.

            When using an API debugging tool
            you may find last errors have been cleared.
            (E.G. BoundsChecker (R))

  TODO: isolate wsock 2 specific and extension codes.
 **********************************************/
    static const _TCHAR *GetSocketError(SOCKET * s, int * result=NULL)
    {
		int  value = 0;
        int  len = sizeof(int);

        int error = getsockopt(*s, SOL_SOCKET, SO_ERROR, (char*)&value, &len);

        if (SOCKET_ERROR == error) {

            error = WSAGetLastError();
            if(NULL != result){
                *result = error;
            }

            switch (error) {
            case WSANOTINITIALISED:
                return _T("GetSockOpt error: WSANOTINITIALISED");
            case WSAENETDOWN:
                return _T("GetSockOpt error: WSAENETDOWN");
            case WSAEINPROGRESS:
                return _T("GetSockOpt error: WSAEINPROGRESS");
            case WSAEINVAL:
                return _T("GetSockOpt error: WSAEINVAL");
            case WSAENOPROTOOPT:
                return _T("GetSockOpt error: WSAENOPROTOOPT");
            case WSAENOTSOCK:
                return _T("GetSockOpt error: WSAENOTSOCK");
            default:
                return _T("GetSockOpt error.");
            }
        }
        else
        {
            error = (int) value;
            if(NULL != result) {
                *result = error;
            }

            switch (error) {
            case WSAEACCES:
                return _T("Permission denied");
            case WSAEADDRINUSE:
                return _T("Address already in use.");
            case WSAEADDRNOTAVAIL:
                return _T("Cannot assign requested address.");
            case WSAEAFNOSUPPORT:
                return _T("Address family not supported by protocol family.");
            case WSAEALREADY:
                return _T("Operation already in progress.");
            case WSAECONNABORTED:
                return _T("Software caused connection abort.");
            case WSAECONNREFUSED:
                return _T("Connection refused.");
            case WSAECONNRESET:
                return _T("Connection reset by peer.");
            case WSAEDESTADDRREQ:
                return _T("Destination address required.");
            case WSAEFAULT:
                return _T("Invalid pointer or buffer too small");
            case WSAEHOSTDOWN:
                return _T("Host is down.");
            case WSAEHOSTUNREACH:
                return _T("No route to host.");
            case WSAEINPROGRESS:
                return _T("Operation now in progress.");
            case WSAEINTR:
                return _T("Interrupted function call.");
            case WSAEINVAL:
                return _T("Invalid argument");
            case WSAEISCONN:
                return _T("Socket is already connected.");
            case WSAEMFILE:
                return _T("Too many open files.");
            case WSAEMSGSIZE:
                return _T("Message too long.");
            case WSAENETDOWN:
                return _T("Network is down.");
            case WSAENETRESET:
                return _T("Network dropped connection on reset.");
            case WSAENETUNREACH:
                return _T("Network is unreachable.");
            case WSAENOBUFS:
                return _T("No buffer space available.");
            case WSAENOPROTOOPT:
                return _T("Bad protocol option.");
            case WSAENOTCONN:
                return _T("Socket is not connected.");
            case WSAENOTSOCK:
                return _T("Socket operation on non-socket.");
            case WSAEOPNOTSUPP:
                return _T("Operation not supported.");
            case WSAEPFNOSUPPORT:
                return _T("Protocol family not supported.");
            case WSAEPROCLIM:
                return _T("Too many processes.");
            case WSAEPROTONOSUPPORT:
                return _T("Protocol not supported.");
            case WSAEPROTOTYPE:
                return _T("Protocol wrong type for socket.");
            case WSAESHUTDOWN:
                return _T("Cannot send after socket shutdown.");
            case WSAESOCKTNOSUPPORT:
                return _T("Socket type not supported.");
            case WSAETIMEDOUT:
                return _T("Connection timed out.");
            case WSAEWOULDBLOCK:
                return _T("Resource temporarily unavailable.");
            case WSAHOST_NOT_FOUND:
                return _T("Host not found.");
            case WSANOTINITIALISED:
                return _T("Successful WSAStartup not yet performed.");
            case WSANO_DATA:
                return _T("Valid name, no data record of requested type.");
            case WSANO_RECOVERY:
                return _T("Non-recoverable error.");
            case WSASYSNOTREADY:
                return _T("Network subsystem is unavailable.");
            case WSATRY_AGAIN:
                return _T("Non-authoritative host not found");
            case WSAEDISCON:
                return _T("Graceful shutdown in progress.");
#ifdef _WINSOCK_2_0_
            case WSATYPE_NOT_FOUND:
                return _T("Class type not found.");
            case WSA_INVALID_HANDLE:
                return _T("Specified event object handle is invalid.");
            case WSA_INVALID_PARAMETER:
                return _T("One or more parameters are invalid.");
            case WSA_IO_INCOMPLETE:
                return _T("Overlapped I/O event object not in signaled state.");
            case WSA_IO_PENDING:
                return _T("Overlapped operations will complete later.");
            case WSASYSCALLFAILURE:
                return _T("System call failure.");
            case WSA_NOT_ENOUGH_MEMORY:
                return _T("Insufficient memory available.");
            case WSA_OPERATION_ABORTED:
                return _T("Overlapped operation aborted.");
#endif
            // these are listed as being in ws2spi.h, but...
            //        case WSAPROVIDERFAILEDINIT:
            //            return _T("Unable to initialize a service provider.");
            //        case WSAINVALIDPROCTABLE:
            //            return _T("Invalid procedure table from service provider");
            //        case WSAINVALIDPROVIDER:
            //            return _T("Invalid service provider version number.");

            default:
                if(0 == error)
                    // may happen if debug tool in use
                    return _T("Last error not available");
                else
                    return _T("Unknown socket error.");
            }
        }
    }

};

#endif      // #ifndef __midl

#endif      // #ifndef UTERR_H_INCLUDED

