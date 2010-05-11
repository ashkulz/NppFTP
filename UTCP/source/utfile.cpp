/////////////////////////////////////////////////////////////////////////////
//
//  Class: CUT_File
//  File:  UTFile.cpp
//
//  Synopsis:
//   The CUT_File class wraps access to the Win32 file handle kernel
//  object.
//
/////////////////////////////////////////////////////////////////////////////
// Ultimate TCP/IP v4.2
// This software along with its related components, documentation and files ("The Libraries")
// is © 1994-2007 The Code Project (1612916 Ontario Limited) and use of The Libraries is
// governed by a software license agreement ("Agreement").  Copies of the Agreement are
// available at The Code Project (www.codeproject.com), as part of the package you downloaded
// to obtain this file, or directly from our office.  For a copy of the license governing
// this software, you may contact us at legalaffairs@codeproject.com, or by calling 416-849-8900.
/////////////////////////////////////////////////////////////////////////////

/*
NppFTP:
Modification made April 2010:
-remove pragma statements
*/

#include "stdafx.h"
#include "tchar.h"
#include "UTFile.h"


CUT_File::CUT_File()
	: m_hFile(INVALID_HANDLE_VALUE), m_bOpenAppending(false)
{
}

CUT_File::CUT_File(LPCTSTR lpszFileName, UINT nOpenMode,
									 DWORD dwCreationDisposition, BOOL bAppend)
	: m_hFile(INVALID_HANDLE_VALUE), m_bOpenAppending(bAppend)
{
	Open(lpszFileName, nOpenMode, dwCreationDisposition, bAppend);
}

CUT_File::~CUT_File()
{
	Close();
}

/////////////////////////////////////////////////////////////////////////////
//
// Function: CUT_File::Open
//
// Description:
//    Call this function to open the specified by `lpszFileName' for I/O
//    operations.
//
// Parameters:
//
//    LPCSTR lpszFileName:
//      A pointer to a null-terminated string specifying the full path
//      of the file.
//
//    UINT nOpenMode:
//      The open mode (desired file access). This can be one or
//      more of the following flags:
//
//       GENERIC_READ  - Specifies read access to the object. Data can be read
//                       from the file and the file pointer can be moved.
//                       Combine with GENERIC_WRITE for read-write access.
//
//       GENERIC_WRITE - Specifies write access to the object. Data can be written
//                       to the file and the file pointer can be moved. Combine
//                       with GENERIC_READ for read-write access.
//
//    DWORD dwCreationDisposition
//      Specifies how to create the file. This must be one of the following flags:
//
//       CREATE_NEW - Creates a new file. The function fails if the specified file
//                    already exists.
//
//       CREATE_ALWAYS - Creates a new file. If the file exists, the function
//                       overwrites the file and clears the existing attributes.
//
//       OPEN_EXISTING - Opens the file. The function fails if the file does
//                       not exist.
//
//       OPEN_ALWAYS - Opens the file, if it exists. If the file does not exist, the
//                     function creates the file as if dwCreationDisposition
//                     were CREATE_NEW.
//
//       TRUNCATE_EXISTING - Opens the file. Once opened, the file is truncated so
//                           that its size is zero bytes. The calling process must
//                           open the file with at least GENERIC_WRITE access.
//                           The function fails if the file does not exist.
//    BOOL bAppend
//      Flag indicating whether this file is open for appending (bAppend = true) or
//      for default open (bAppend = false). If bAppend is true, after each
//      write operation the file pointer is set to the end of the file.
//
// Return:
//    0 on success
//   -1 on failure; Call GetLastError for extended information
//
/////////////////////////////////////////////////////////////////////////////
int CUT_File::Open(LPCTSTR lpszFileName, UINT nOpenMode, DWORD dwCreationDisposition, BOOL bAppend)
{
	// if the file is already open, close it first
	if (m_hFile != INVALID_HANDLE_VALUE)
		Close();

	_TCHAR  szBuffer[MAX_PATH + 1];
	LPCTSTR pFront = NULL, pBack = NULL;
	if (lpszFileName[0] == ' ' || lpszFileName[_tcslen(lpszFileName) - 1] == _T(' ')) {

		// throughout the kit there are occassions where the file names passed
		// to this open function are padded with spaces on either the left
		// or right side. CreateFile will fail to open a file name with padding;
		// thus, we must remove any spaces from the beginning and end of the file
		// name

		pFront = lpszFileName; // pointer to front
		pBack = lpszFileName + _tcslen(lpszFileName); // pointer to back

		while (*pFront == ' ')
			pFront++;

		while (*(pBack-1) == ' ')
			pBack--;

		_tcsncpy(szBuffer, pFront, _tcslen(pFront) - _tcslen(pBack) + 1);
		lpszFileName = szBuffer;
	}

	m_hFile = ::CreateFile(lpszFileName, nOpenMode, FILE_SHARE_READ, NULL,
		dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, NULL);

	if (m_hFile == INVALID_HANDLE_VALUE)
		return -1;

	m_bOpenAppending = bAppend;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
//
// Function: CUT_File::Close
//
// Description:
//    Call this function to close an open file object. If the object is
//    not open, this call has no effect.
//
// Parameters:
//    None
//
// Return:
//    None
//
/////////////////////////////////////////////////////////////////////////////
void CUT_File::Close()
{
	// Closes an open file
	if (m_hFile != INVALID_HANDLE_VALUE) {
		::CloseHandle(m_hFile);
		m_hFile = INVALID_HANDLE_VALUE;
	}
}

/////////////////////////////////////////////////////////////////////////////
//
// Function: CUT_File::Read
//
// Description:
//    The Read function reads data from a file. After the read operation
//    has been completed, the file pointer is adjusted by the number of
//    bytes actually read.
//
// Parameters:
//
//    LPVOID lpBuffer:
//      Specifies a pointer to a buffer that receives the data.
//
//    DWORD dwNumberOfBytesToRead:
//      Specifies the number of bytes to read.
//
// Return:
//    Returns the number of bytes read on success, or
//    -1 on failure; Call GetLastError for extended information
//
/////////////////////////////////////////////////////////////////////////////
DWORD CUT_File::Read(LPVOID lpBuffer, DWORD dwNumberOfBytesToRead)
{
	if (m_hFile == INVALID_HANDLE_VALUE)
		return (DWORD)-1;

	DWORD dwNumberOfBytesRead = 0;
	if (!::ReadFile(m_hFile, lpBuffer, dwNumberOfBytesToRead, &dwNumberOfBytesRead, NULL))
		return (DWORD) -1;

	return dwNumberOfBytesRead;
}

/////////////////////////////////////////////////////////////////////////////
//
// Function: CUT_File::Write
//
// Description:
//    The Write function writes data to a file. After the write operations
//    has been completed, the file pointer is adjusted by the number of
//    bytes acutally written. If the file is open for appending, the
//    file pointer is moved to the end of the file before the write operation
//    occurs.
//
// Parameters:
//
//    LPCVOID lpBuffer:
//      Specifies a pointer to the data to write to file.
//
//    DWORD dwNumberOfBytesToWrite:
//      Specifies the number of bytes to write.
//
// Return:
//    Returns the number of bytes written on success, or
//    -1 on failure; Call GetLastError for extended information
//
/////////////////////////////////////////////////////////////////////////////
DWORD CUT_File::Write(LPCVOID lpBuffer, DWORD dwNumberOfBytesToWrite)
{
	if (m_hFile == INVALID_HANDLE_VALUE)
		return (DWORD)-1;

	if (m_bOpenAppending)
		Seek(0, FILE_END);

	DWORD dwNumberOfBytesWritten = 0;
	if (!::WriteFile(m_hFile, lpBuffer, dwNumberOfBytesToWrite, &dwNumberOfBytesWritten, NULL))
		return (DWORD)-1;

	return dwNumberOfBytesWritten;
}

/////////////////////////////////////////////////////////////////////////////
//
// Function: CUT_File::Seek
//
// Description:
//    Call Seek to move the file pointer of an open file.
//
// Parameters:
//
//    LONG lDistanceToMove:
//      Specifies the number of bytes to move file pointer.
//
//    DWORD dwMoveMethod:
//      Specifies the move method. This can be one of the following
//
//        SEEK_CUR or FILE_CURRENT - The starting point is the current
//                                   value of the file pointer.
//
//        SEEK_END or FILE_END - The starting point is the current
//                               end-of-file position.
//
//        SEEK_SET or FILE_BEGIN - The starting point is the beginning
//                                 of the file.
//
//      NOTE: SEEK_CUR and FILE_CURRENT are defined as the same value as
//            are SEEK_END and FILE_END and SEEK_SET and FILE_BEGIN. Thus,
//            you may call this function using either the RTL-style SEEK_CUR,
//            SEEK_END, and SEEK_SET symbols or the Win32-style FILE_CURRENT,
//            FILE_END, and FILE_BEGIN symbols.
//
// Return:
//    Returns the low-order DWORD of the new file pointer on success, or
//    -1 on failure; Call GetLastError for extended information
//
/////////////////////////////////////////////////////////////////////////////
LONG CUT_File::Seek(LONG lDistanceToMove, DWORD dwMoveMethod)
{
	if (m_hFile == INVALID_HANDLE_VALUE)
		return -1;

	// We must maintain compatibility with the old RTL move
	// methods including SEEK_CUR, SEEK_END and SEEK_SET

	DWORD dwResult;

	dwResult = ::SetFilePointer(m_hFile, lDistanceToMove, NULL, dwMoveMethod);
	return dwResult;
}

/////////////////////////////////////////////////////////////////////////////
//
// Function: CUT_File::GetLength
//
// Description:
//    Call this function to retrieve the file size, in bytes.
//
// Parameters:
//    None
//
// Return:
//    Returns the low-order DWORD of the new file pointer on success, or
//    -1 on failure; Call GetLastError for extended information
//
/////////////////////////////////////////////////////////////////////////////
DWORD CUT_File::GetLength() const
{
	if (m_hFile == INVALID_HANDLE_VALUE)
		return (DWORD) -1;

	DWORD dwReturn = GetFileSize(m_hFile, NULL);

	if (dwReturn == 0xFFFFFFFF)
		return (DWORD) -1; // failed;

	return dwReturn;
}
