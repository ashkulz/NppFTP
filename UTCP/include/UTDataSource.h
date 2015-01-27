// =================================================================
//  class: CUT_DataSource, CUT_FileDataSource, CUT_BufferDataSource
//  File:  UTDataSource.h
//
//  Purpose:
//
//		CUT_DataSource		- Data source abstract class
//		CUT_FileDataSource	- File based data source
//		CUT_BufferDataSource- Memory buffer data source
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
Remove pragma statements
*/


#ifndef UTDATASOURCE_H
#define UTDATASOURCE_H

#include <stdio.h>
#include "utstrlst.h"
#include "utfile.h"

#define	FIND_BUFFER_SIZE	4096

// Open data source type enumeration
enum OpenMsgType {
	UTM_OM_READING,					// Open data source for reading
	UTM_OM_WRITING,					// Open data source for writing
	UTM_OM_APPEND					// Open data source for appending
};

// =================================================================
//	CUT_DataSource abstract class
//
//	Data source class for CUT_Msg class
// =================================================================
class CUT_DataSource {

public:
	CUT_DataSource() {}
	virtual ~CUT_DataSource() {}

	// Virtual clone constructor
	virtual CUT_DataSource * clone() = 0;

	// Opens data file type == UTM_OM_READING, UTM_OM_WRITING, UTM_OM_APPEND
	virtual int		Open(OpenMsgType type) = 0;

	// Close message
	virtual int		Close() = 0;

	// Read one line
	virtual int		ReadLine(LPSTR buffer, size_t maxsize) = 0;

	// Write one line
	virtual int		WriteLine(LPCSTR buffer) = 0;

	// Read data
	virtual int		Read(LPSTR buffer, size_t count) = 0;

	// Write data
	virtual int		Write(LPCSTR buffer, size_t count) = 0;

	// Move a current pointer to the specified location.
	virtual long	Seek(long offset, int origin) = 0;

	// Find a string in the data source
	virtual long	Find(LPCSTR	szStrToFind, size_t lStartOffset = 0, BOOL bCaseSensitive = FALSE, size_t lMaxSearchLength = 0);
};

// =================================================================
//	CUT_FileDataSource class
//
//	Data source class for CUT_Msg class based on the file
// =================================================================
class CUT_FileDataSource : public CUT_DataSource {

protected:
	CUT_File	m_file;						// File class
	LPSTR		m_lpszBuffer;				// File Reading buffer
	int			m_nBufferSize;				// File Reading buffer szie
	int			m_nLoadedDataSize;			// Number of bytes read into buffer
	_TCHAR		m_szFileName[MAX_PATH+1];	// File name

public:

	CUT_FileDataSource(LPCTSTR filename, int nBufferSize = 4096);
	virtual ~CUT_FileDataSource();

	// Virtual clone constructor
	virtual CUT_DataSource * clone();

	// Opens data file type == UTM_OM_READING, UTM_OM_WRITING, UTM_OM_APPEND
	virtual int		Open(OpenMsgType type);

	// Close data file
	virtual int		Close();

	// Read one line from the data file
	virtual int		ReadLine(LPSTR buffer, size_t maxsize);

	// Write one line to the data file
	virtual int		WriteLine(LPCSTR buffer);

	// Read data
	virtual int		Read(LPSTR buffer, size_t count);

	// Write data
	virtual int		Write(LPCSTR buffer, size_t count);

	// Move a current pointer to the specified location.
	virtual long	Seek(long offset, int origin);

};

// =================================================================
//	CUT_BufferDataSource class
//
//	Data source class for CUT_Msg class based on the file
// =================================================================
class CUT_BufferDataSource : public CUT_DataSource {
protected:
	LPSTR			m_lpszBuffer;				// Pointer to the buffer
	// v4.2 changed to size_t
	size_t			m_nSize;					// Buffer or data size
	size_t			m_nDataSize;				// Buffer or data size
	char			m_szName[MAX_PATH+1];		// Datasource name
	size_t			m_nCurPosition;				// Current reading/writing position
	BOOL			m_bCleanUp;					// TRUE if we need to cleen up the buffer ourselfs

public:
	// v4.2 buffer sizes changed to size_t
	CUT_BufferDataSource(LPSTR buffer, size_t size, LPCSTR name = NULL);
	virtual ~CUT_BufferDataSource();

	// Virtual clone constructor
	virtual CUT_DataSource * clone();

	// Opens data file type == UTM_OM_READING, UTM_OM_WRITING, UTM_OM_APPEND
	virtual int		Open(OpenMsgType type);

	// Close data file
	virtual int		Close();

	// Read one line from the data file
	virtual int		ReadLine(LPSTR buffer, size_t maxsize);

	// Write one line to the data file
	virtual int		WriteLine(LPCSTR buffer);

	// Read data
	virtual int		Read(LPSTR buffer, size_t count);

	// Write data
	virtual int		Write(LPCSTR buffer, size_t count);

	// Move a current pointer to the specified location.
	virtual long	Seek(long offset, int origin);

};


// =================================================================
//	CUT_MapFileDataSource
//
//	Data source class for CUT_Msg class based on the mapped file
// =================================================================
class CUT_MapFileDataSource : public CUT_DataSource {
protected:
	_TCHAR			m_szFileName[MAX_PATH + 1];	// File name
	char			m_szName[MAX_PATH + 1];		// Datasource name

	HANDLE			m_hFile;						// File handle
	HANDLE			m_hMapFile;						// File map handle
	LPSTR			m_lpMapAddress;					// Pointer to the data

	LARGE_INTEGER	m_lnSize;						// Allocated data size
	LARGE_INTEGER	m_lnPosition;					// Current position
	LARGE_INTEGER	m_lnActualSize;					// Actual size of data
	LARGE_INTEGER	m_lnIncrement;					// Increment size

	OpenMsgType		m_OpenType;						// Last open type

	BOOL			m_bTempFileName;				// If TRUE delete file while closing

public:
	CUT_MapFileDataSource(DWORD SizeHigh, DWORD SizeLow, LPCSTR name = NULL, LPCTSTR filename = NULL);
	virtual ~CUT_MapFileDataSource();

	// Virtual clone constructor
	virtual CUT_DataSource * clone();

	// Opens data file type == UTM_OM_READING, UTM_OM_WRITING, UTM_OM_APPEND
	virtual int		Open(OpenMsgType type);

	// Close data file
	virtual int		Close();

	// Read one line from the data file
	virtual int		ReadLine(LPSTR buffer, size_t maxsize);

	// Write one line to the data file
	virtual int		WriteLine(LPCSTR buffer);

	// Read data
	virtual int		Read(LPSTR buffer, size_t count);

	// Write data
	virtual int		Write(LPCSTR buffer, size_t count);

	// Move a current pointer to the specified location.
	virtual long	Seek(long offset, int origin);

};

#endif
