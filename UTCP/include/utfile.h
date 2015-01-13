/////////////////////////////////////////////////////////////////////////////
//
//  Class: CUT_File
//  File:  UTFile.h
//
//  Synopsis:
//   The CUT_File wraps access to the Win32 file handle kernel 
//  object.
//
/////////////////////////////////////////////////////////////////////////////
//
// Ultimate TCP/IP v4.2
// This software along with its related components, documentation and files ("The Libraries")
// is © 1994-2007 The Code Project (1612916 Ontario Limited) and use of The Libraries is
// governed by a software license agreement ("Agreement").  Copies of the Agreement are
// available at The Code Project (www.codeproject.com), as part of the package you downloaded
// to obtain this file, or directly from our office.  For a copy of the license governing
// this software, you may contact us at legalaffairs@codeproject.com, or by calling 416-849-8900.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __UTFILE_H__INCLUDED
#define __UTFILE_H__INCLUDED

class CUT_File
{
public:
// Constructors / Destructors
	CUT_File();
	CUT_File(LPCTSTR lpszFileName, UINT nOpenMode = GENERIC_WRITE, DWORD dwCreationDisposition = OPEN_EXISTING, BOOL bAppend = false);
	virtual ~CUT_File();
  
// Operations
	virtual int Open(LPCTSTR lpszFileName, UINT nOpenMode = GENERIC_WRITE, DWORD dwCreationDisposition = OPEN_EXISTING, BOOL bAppend = false);
	virtual void Close();
	virtual DWORD Read(LPVOID lpBuffer, DWORD dwNumberOfBytesToRead);
	virtual DWORD Write(LPCVOID lpBuffer, DWORD dwNumberOfBytesToWrite);
	virtual LONG Seek(LONG lDistanceToMove, DWORD dwMoveMethod);
	virtual DWORD GetLength() const;

private:
	CUT_File(const CUT_File&);             // no implementation
	CUT_File& operator=(const CUT_File&);  // no implementation

private:
	HANDLE m_hFile;
	BOOL m_bOpenAppending;
};


#endif // __UTFILE_H__INCLUDED






