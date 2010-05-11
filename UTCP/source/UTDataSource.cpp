//=================================================================
//  class: CUT_DataSource, CUT_FileDataSource, CUT_BufferDataSource
//  File:  UTDataSource.h
//
//  Purpose:
//
//		CUT_DataSource		- Data source abstract class
//		CUT_FileDataSource	- File based data source
//		CUT_BufferDataSource- Memory buffer data source
//
//=================================================================
// Ultimate TCP/IP v4.2
// This software along with its related components, documentation and files ("The Libraries")
// is © 1994-2007 The Code Project (1612916 Ontario Limited) and use of The Libraries is
// governed by a software license agreement ("Agreement").  Copies of the Agreement are
// available at The Code Project (www.codeproject.com), as part of the package you downloaded
// to obtain this file, or directly from our office.  For a copy of the license governing
// this software, you may contact us at legalaffairs@codeproject.com, or by calling 416-849-8900.
//=================================================================

#include "stdafx.h"

#include "UTDataSource.h"


// v4.2 * All datasource read/write fns now take size_t params *
// =================================================================
//	CUT_DataSource class
//
//	Data source ABC
// =================================================================

/********************************
FindMatchingString
	This function will scan a data source and locate
	the desired string. It will return the offset to
	the matched string.
PARAM:
	szStrToFind		- string to find
	nStartOffset	- starting from
	bCaseSensitive	- if case sensitive
	nMaxSearchLength- maximum length to search
RETURN:
	-1		- errror
	string offset if found
*********************************/
long CUT_DataSource::Find(LPCSTR szStrToFind, size_t nStartOffset, BOOL bCaseSensitive, size_t nMaxSearchLength)
{
	char		szBuffer[FIND_BUFFER_SIZE + 1];			// Temporary buffer
	char		szSearchStr[FIND_BUFFER_SIZE + 1];		// Copy of the search string
	size_t		nBytesRead		= 0;					// Number of bytes read
	size_t		nCurPosition	= nStartOffset;			// Current offset position

	// Wrong search string
	if(szStrToFind == NULL)
		return -1;

	// Wrong parameters
	size_t		nStrLength = strlen(szStrToFind);
	if(nStrLength == 0 || (nMaxSearchLength > 0 && nMaxSearchLength < nStrLength) )
		return -1;

	// Search string is too big
	if(nStrLength >= FIND_BUFFER_SIZE)
		return -1;

	// Make a copy of the search string and convert to upper case if case insensitive search
	strcpy(szSearchStr, szStrToFind);
	if(!bCaseSensitive)
		_strupr(szSearchStr);

    // Seek to the beginning of the search
    if(Seek((long)nCurPosition, SEEK_SET) == -1)
		return -1;

	// Read data and try to find the string
	while( (nBytesRead = Read(szBuffer, FIND_BUFFER_SIZE - 1)) > nStrLength ) {

		// Terminate buffer with NULL
		szBuffer[nBytesRead] = NULL;

		// Convert to upper case if case insensitive search
		if(!bCaseSensitive)
			_strupr(szBuffer);

		// Try to find the string
		LPSTR	szStrPos = NULL;
		if( (szStrPos = strstr(szBuffer, szSearchStr)) != NULL )
		{
			// String found !!!
			/*
				Mod by Nish - April 21, 2005
				A check was added to make sure we don't return a pos
				beyond the specified limit (nMaxSearchLength)
			*/
			long retval = (long)(nCurPosition + (szStrPos - szBuffer));
			if(nMaxSearchLength == 0)//No limit
				return retval;
			else
			{
				if( ( retval - nStartOffset) <= nMaxSearchLength )
					return retval;
				else
					return -1;
			}
		}

		// It's the last block of data read
		if( nBytesRead == nStrLength )
			return -1;

		// Seek back on the length of the buffer minus search string length
		nCurPosition += nBytesRead - nStrLength;
	    if(Seek((long)nCurPosition, SEEK_SET) == -1)
			return -1;

		// Check for the search length
		if(nMaxSearchLength > 0 && (nCurPosition - nStartOffset) > nMaxSearchLength)
			return -1;
		}

	return -1;
}

// =================================================================
//	CUT_FileDataSource class
//
//	Data source class based on the file
// =================================================================

/***********************************************
CUT_FileDataSource
	Constructor
PARAM:
	filename		- file name
	[nBufferSize]	- buffer size for reading (def = 4096)
RETURN:
	none
************************************************/
CUT_FileDataSource::CUT_FileDataSource(LPCTSTR filename, int nBufferSize) :
				m_nBufferSize(nBufferSize),
				m_nLoadedDataSize(0)
{
	// Allocate reading buffer
	m_lpszBuffer = new char [nBufferSize];

	// Remember the file name
	m_szFileName[0] = _T('\0');
	if(filename)
		_tcsncpy(m_szFileName, filename, MAX_PATH);
}


/***********************************************
clone
	File data source copy constructor
PARAM:
	none
RETURN:
	pointer to the newly c reated object
************************************************/
CUT_DataSource * CUT_FileDataSource::clone() {
	return new CUT_FileDataSource(m_szFileName, m_nBufferSize);
}

/***********************************************
~CUT_FileDataSource
	Destructor
PARAM:
	none
RETURN:
	none
************************************************/
CUT_FileDataSource::~CUT_FileDataSource() {
	// Destroy reading buffer
	delete [] m_lpszBuffer;
}

/***********************************************
Open
	Opens data file for reading or writing
PARAM:
	type	- UTM_OM_READING, UTM_OM_WRITING or UTM_OM_APPEND
RETURN:
	-1 if error
************************************************/
int	CUT_FileDataSource::Open(OpenMsgType type)
{
	if (m_szFileName == NULL || _tcslen(m_szFileName) == 0)
		return -1;

	// Open file for either reading or writing depending
	// on the OpenMsgType
	//
	// nOpenMode specifies the open mode for the file. This can be either
	// GENERIC_READ (to open file for reading) or GENERIC_WRITE (to open
	// the file for writing. You may specify both of these using the bit-
	// wise OR operator (|); this indicates the file is open for both
	// reading and writing.
	//
	// dwCreationDisposition specifies what action to take when creating
	// the file. See CUT_File::Open for more information
	//
	// bAppend specifies if the file should be open for appending. If a file
	// is open for appending, *every* write operation occurs at the end of
	// the file no matter where the file pointer is currently set.

	UINT nOpenMode;
	DWORD dwCreationDisposition = OPEN_EXISTING;
	BOOL bAppend = false;

	switch(type) {

		case UTM_OM_READING:
			nOpenMode = GENERIC_READ;
			break;

		case UTM_OM_WRITING:
			nOpenMode = GENERIC_WRITE | GENERIC_READ;
			dwCreationDisposition = CREATE_ALWAYS;
			break;

		case UTM_OM_APPEND:
			nOpenMode = GENERIC_WRITE | GENERIC_READ;
			dwCreationDisposition = OPEN_ALWAYS;
			bAppend = true;
			break;

		default:
			return -1;
	}

	return m_file.Open(m_szFileName, nOpenMode, dwCreationDisposition, bAppend);
}

/***********************************************
Close
	Close data file
PARAM:
	none
RETURN:
	-1 if error
************************************************/
int	CUT_FileDataSource::Close() {
	// Close file
	m_file.Close();
	return UTE_SUCCESS;
}

/***********************************************
ReadLine
	Reads next line from the message file.
PARAM:
	buffer		- buffer to read to
	maxsize		- size of the buffer
RETURN:
	0 - if no more data
************************************************/
int	CUT_FileDataSource::ReadLine(LPSTR buffer, size_t maxsize) {
	int		rt;
	LPSTR	pos = NULL;

	// Wrong parameter
	if(buffer == NULL)		return -1;

	// If there is no more data or we can't find \r\n - try to read more
	m_lpszBuffer[m_nLoadedDataSize] = 0;
	if(m_nLoadedDataSize == 0 || (pos = strstr(m_lpszBuffer, "\r\n")) == NULL) {
		if((rt = m_file.Read((m_lpszBuffer + m_nLoadedDataSize), m_nBufferSize - m_nLoadedDataSize - 1)) > 0)
			// Increase the number of bytes read
			m_nLoadedDataSize += rt;
		}

	// If we can find \r\n in the buffer - return one line
	if(pos == NULL && m_nLoadedDataSize > 0)
		pos = strstr(m_lpszBuffer, "\r\n");

	if(pos != NULL && (unsigned int)(pos - m_lpszBuffer) < (maxsize-3)) {
		// Copy line to buffer
		pos[0] = 0;
		strcpy(buffer, m_lpszBuffer);
		strcat(buffer, "\r\n");

		// Decrease the nuber of bytes read and
		// remove the first line from the buffer
		m_nLoadedDataSize -= (int)strlen(m_lpszBuffer) + 2;
		memmove(m_lpszBuffer, pos+2, m_nLoadedDataSize);
		m_lpszBuffer[m_nLoadedDataSize] = 0;
		return (int)strlen(buffer);
		}

	// If \r\n not found - return what we have
	else if(m_nLoadedDataSize > 0) {
		strncpy(buffer, m_lpszBuffer, maxsize-1);
		buffer[maxsize-1] = 0;

		// Decrease the nuber of bytes read and
		// remove the first line from the buffer
		m_nLoadedDataSize -= (int)strlen(buffer);
		memmove(m_lpszBuffer, m_lpszBuffer + strlen(buffer), m_nLoadedDataSize);
		m_lpszBuffer[m_nLoadedDataSize] = 0;
		return (int)strlen(buffer);
		}

	buffer[0] = 0;

	return 0; // no more data to read
}

/***********************************************
WriteLine
	Writes line to the message file
PARAM:
	buffer		- line to write
RETURN:
	-1 if error
************************************************/
int	CUT_FileDataSource::WriteLine(LPCSTR buffer) {

	// Wrong parameter
	if(buffer == NULL)		return -1;

	// Write line to the file
	int		rt = UTE_SUCCESS, len = (int)strlen(buffer);
	if(len > 0)
		rt = m_file.Write(buffer, len);

	// Write CRLF if the buffer doesn't have one
	if(!CUT_StrMethods::IsWithCRLF(buffer))
		m_file.Write("\r\n", 2);

	return rt;
}

/***********************************************
Read
	Reads specified number of bytes
PARAM:
	buffer		- buffer to read to
	count		- number of bytes to read
RETURN:
	number of bytes read
	-1 if error
************************************************/
int	CUT_FileDataSource::Read(LPSTR buffer, size_t count) {
	// Wrong parameter
	if(buffer == NULL)		return -1;

	m_nLoadedDataSize = 0;

	// Read data from the file
	return m_file.Read(buffer, (DWORD)count);
}

/***********************************************
Write
	Writes specified number of bytes
PARAM:
	buffer		- buffer to read to
	count		- number of bytes to read
RETURN:
	-1 if error
************************************************/
int	CUT_FileDataSource::Write(LPCSTR buffer, size_t count) {

	// Wrong parameter
	if(buffer == NULL)		return -1;

	// Write data to the file
	return m_file.Write(buffer, (DWORD)count);
}
/****************************************
Seek
    Moves the file pointer to a specified
	location
Params
	offset	- number of bytes from origin
	origin	- initial position
				SEEK_CUR
				SEEK_END
				SEEK_SET
Return
    offset, in bytes, of the new position
	from the beginning of the file
	-1		- error
*****************************************/
long CUT_FileDataSource::Seek(long offset, int origin)
{
	// When we reading the line of text we use buffering. Thats why
	// actual file position will not match. Fix this problem by subtractng
	// number of bytes read in buffer from current file position
	if(origin == SEEK_CUR && m_nLoadedDataSize > 0) {
		long	lCurPos = m_file.Seek(0, SEEK_CUR) - m_nLoadedDataSize;
		if(lCurPos > 0)
			m_file.Seek(lCurPos, SEEK_SET);
		}

	m_nLoadedDataSize = 0;

	return m_file.Seek(offset, origin);
}

// =================================================================
//	CUT_BufferDataSource class
//
//	Data source class based on the memory buffer
// =================================================================

/***********************************************
CUT_BufferDataSource
	Constructor
PARAM:
	buffer			- pointer to the buffer
	size			- buffer size
	[name]			- buffer name
RETURN:
	none
************************************************/
CUT_BufferDataSource::CUT_BufferDataSource(LPSTR buffer, size_t size, LPCSTR name) :
				m_nSize(size),
				m_nDataSize(size),
				m_nCurPosition(0),
				m_bCleanUp(FALSE)
{
	// Initialize buffer
	if(buffer == NULL) {
		m_lpszBuffer = new char[m_nSize + 1];
		*m_lpszBuffer = 0;
		m_bCleanUp = TRUE;
		}
	else
		m_lpszBuffer = buffer;

	// Remember the buffer name
	m_szName[0] = 0;
	if(name)
		strncpy(m_szName, name, MAX_PATH);
}

/***********************************************
clone
	Virtual clone constructor
PARAM:
	none
RETURN:
	pointer to the newly created object
************************************************/
CUT_DataSource * CUT_BufferDataSource::clone()
{
	char *buffer = new char[m_nSize + 1];
	memcpy(buffer, m_lpszBuffer, m_nSize);
	*(buffer + m_nSize) = 0;
	CUT_BufferDataSource *ptr = new CUT_BufferDataSource(buffer, m_nSize, m_szName);
	ptr->m_bCleanUp = TRUE;
	ptr->m_nDataSize = m_nDataSize;
	return ptr;
}

/***********************************************
~CUT_BufferDataSource
	Destructor
PARAM:
	none
RETURN:
	none
************************************************/
CUT_BufferDataSource::~CUT_BufferDataSource() {
	if(m_bCleanUp && m_lpszBuffer != NULL)
		delete [] m_lpszBuffer;
}

/***********************************************
Open
	Opens buffer data source for reading or writing
PARAM:
	type	- UTM_OM_READING, UTM_OM_WRITING or UTM_OM_APPEND
RETURN:
	-1 if error
************************************************/
int	CUT_BufferDataSource::Open(OpenMsgType type) {
	long	nBufferSize = (long)strlen(m_lpszBuffer);

	if(type == UTM_OM_APPEND) {
		m_nCurPosition = nBufferSize;
		}
	else
		m_nCurPosition = 0;

	if(type == UTM_OM_APPEND || type == UTM_OM_READING)
				m_nDataSize = nBufferSize;
	else
				m_nDataSize = 0;

	return UTE_SUCCESS;
}

/***********************************************
Close
	Close data buffer
PARAM:
	none
RETURN:
	-1 if error
************************************************/
int	CUT_BufferDataSource::Close() {
	return UTE_SUCCESS;
}

/***********************************************
ReadLine
	Reads next line from the buffer
PARAM:
	buffer		- buffer to read to
	maxsize		- size of the buffer
RETURN:
	0 - if no more data
************************************************/
int	CUT_BufferDataSource::ReadLine(LPSTR buffer, size_t maxsize) {

	// Wrong parameter
	if(buffer == NULL)		return -1;

	// Read to the buffer
	int	nBytesRead;
	if((nBytesRead = Read(buffer, maxsize - 1)) <= 0)
		return 0;

	// Find CrLf
	LPSTR	szCrLf;
	long	nLineSize = nBytesRead;
	if((szCrLf = strchr(buffer, '\r')) != NULL) {
		++szCrLf;
		if(*szCrLf == '\n')
			++szCrLf;
		*szCrLf = 0;
		nLineSize = (long)(szCrLf - buffer);
		}

	// Adjust current position
	if(nLineSize > 0)
		m_nCurPosition -= (nBytesRead - nLineSize);

	return nLineSize;
}

/***********************************************
WriteLine
	Writes line to the buffer
PARAM:
	buffer		- line to write
RETURN:
	-1 if error
************************************************/
int	CUT_BufferDataSource::WriteLine(LPCSTR buffer) {

	// Wrong parameter
	if(buffer == NULL)		return -1;

	// Write line to the buffer
	int			rt = 0;
	size_t		len = strlen(buffer);
	size_t		nBytesNumber = min(len, (m_nSize - m_nCurPosition));

	if(len > 0)
		if((rt = Write(buffer, (unsigned int)nBytesNumber)) == -1)
			return -1;

	// Write CRLF if the buffer doesn't have one
	if(!CUT_StrMethods::IsWithCRLF(buffer))
		if((rt = Write("\r\n", 2)) == -1)
			return -1;

	return (int)nBytesNumber;
}

/***********************************************
Read
	Reads specified number of bytes
PARAM:
	buffer		- buffer to read to
	count		- number of bytes to read
RETURN:
	number of bytes read
	-1 if error
************************************************/
int	CUT_BufferDataSource::Read(LPSTR buffer, size_t count) {
	// Wrong parameter
	if(buffer == NULL)		return -1;

	// Read data from the buffer
	int nBytesNumber = (int)min( count, (m_nDataSize - m_nCurPosition));
	memcpy(buffer, (m_lpszBuffer + m_nCurPosition), nBytesNumber);
	buffer[nBytesNumber] = 0;
	m_nCurPosition += nBytesNumber;

	return nBytesNumber;
}

/***********************************************
Write
	Writes specified number of bytes
PARAM:
	buffer		- buffer to read to
	count		- number of bytes to read
RETURN:
	-1 if error
************************************************/
int	CUT_BufferDataSource::Write(LPCSTR buffer, size_t count) {

	// Wrong parameter
	if(buffer == NULL)		return -1;

	// Write data to the buffer
	unsigned int nBytesNumber = (unsigned int)min(count, (m_nSize - m_nCurPosition));
	memcpy((m_lpszBuffer + m_nCurPosition), buffer, nBytesNumber);
	m_nCurPosition += nBytesNumber;
	m_lpszBuffer[m_nCurPosition] = 0;
	m_nDataSize = m_nCurPosition;

	return nBytesNumber;
}
/****************************************
Seek
    Moves the file pointer to a specified
	location
Params
	offset	- number of bytes from origin
	origin	- initial position
				SEEK_CUR
				SEEK_END
				SEEK_SET
Return
    offset, in bytes, of the new position
	from the beginning of the file
	-1		- error
*****************************************/
long CUT_BufferDataSource::Seek(long offset, int origin)
{
	long	nOffset = 0;

	switch(origin) {
		case(SEEK_CUR):
			if((m_nCurPosition + offset) <= m_nDataSize)
				nOffset = (long)(m_nCurPosition + offset);
			else
				return -1;
			break;
		case(SEEK_END):
			if(offset <= (long)m_nDataSize)
				nOffset = (long)(m_nDataSize - offset);
			else
				return -1;
			break;
		case(SEEK_SET):
			if(offset <= (long)m_nDataSize)
				nOffset = offset;
			else
				return -1;
			break;

		default:
			return -1;
		}

	m_nCurPosition = nOffset;

	return (long)m_nCurPosition;
}

// =================================================================
//	CUT_MapFileDataSource class
//
//	Data source class based on the mapped file
// =================================================================

/***********************************************
CUT_MapFileDataSource
	Constructor
PARAM:
	SizeHigh, SizeLow	- buffer size
	[name]				- buffer name
	[filename]			- buffer map file name
RETURN:
	none
************************************************/
CUT_MapFileDataSource::CUT_MapFileDataSource(DWORD SizeHigh, DWORD SizeLow, LPCSTR name, LPCTSTR filename) :
		m_hFile(INVALID_HANDLE_VALUE),
		m_hMapFile(NULL),
		m_lpMapAddress(NULL),
		m_bTempFileName(TRUE)
{
	// Initialize Size & Position
	m_lnSize.HighPart		= SizeHigh;
	m_lnSize.LowPart		= SizeLow;
	m_lnActualSize			= m_lnSize;

	// Initialize data source name
	m_szName[0]	= 0;
	if(name != NULL) {
		strncpy(m_szName, name,MAX_PATH);
		m_szName[MAX_PATH]	= 0;
		}

	// Initialize map file name
	m_szFileName[0]	= 0;
	if(filename != NULL) {
		m_bTempFileName = FALSE;
		_tcsncpy(m_szFileName, filename, MAX_PATH);
		m_szFileName[MAX_PATH]	= 0;
		}
	else {
		_TCHAR	szTmpPath[MAX_PATH + 1];
		GetTempPath(MAX_PATH, szTmpPath);
 		GetTempFileName(szTmpPath, _T("Map"), 0, m_szFileName);
		}

}

/***********************************************
clone
	Virtual clone constructor
PARAM:
	none
RETURN:
	pointer to the newly created object
************************************************/
CUT_DataSource * CUT_MapFileDataSource::clone()
{
	// Create object
	CUT_MapFileDataSource	*ptrNewDataSource = new CUT_MapFileDataSource(m_lnActualSize.HighPart, m_lnActualSize.LowPart, m_szName, (m_hFile == INVALID_HANDLE_VALUE) ? m_szFileName : NULL );

	// Copy the data if nessesary
	if(m_hMapFile != NULL && m_lpMapAddress)
		if(ptrNewDataSource->Open(UTM_OM_WRITING) != -1) {
			LARGE_INTEGER	lnIndex = { 0, 0 };
			for(/*lnIndex.QuadPart*/; lnIndex.QuadPart < m_lnActualSize.QuadPart; lnIndex.QuadPart++)
				*(ptrNewDataSource->m_lpMapAddress + lnIndex.QuadPart) = *(m_lpMapAddress + lnIndex.QuadPart);

			ptrNewDataSource->m_lnActualSize	= m_lnActualSize;
			ptrNewDataSource->Close();
			}

	ptrNewDataSource->m_bTempFileName	= m_bTempFileName;
	return ptrNewDataSource;
}

/***********************************************
~CUT_MapFileDataSource
	Destructor
PARAM:
	none
RETURN:
	none
************************************************/
CUT_MapFileDataSource::~CUT_MapFileDataSource()
{
	// Close everything
	Close();

	// Delete temp. file
	if(m_bTempFileName && m_szFileName[0] != 0)
		DeleteFile(m_szFileName);
}

/***********************************************
Open
	Opens data file for reading or writing
PARAM:
	type	- UTM_OM_READING, UTM_OM_WRITING or UTM_OM_APPEND
RETURN:
	-1 if error
************************************************/
int	CUT_MapFileDataSource::Open(OpenMsgType type)
{
	// Save opening type
	m_OpenType = type;

	// If something is already opened - close it
	Close();

	// Create/Open map file
	if((m_hFile = CreateFile(m_szFileName, GENERIC_READ | GENERIC_WRITE, 0, NULL,
						(type == UTM_OM_READING) ?  OPEN_EXISTING : OPEN_ALWAYS,
						FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
		return -1;

	// If size is not set - try to get the file size
	if(m_lnSize.HighPart == 0 && m_lnSize.LowPart == 0) {
		m_lnSize.LowPart = GetFileSize(m_hFile, (ULONG *)&m_lnSize.HighPart);
		if(m_lnSize.LowPart == 0xFFFFFFFF && GetLastError() != NO_ERROR)
			return -1;
		}

	// Initialize Actual Size
	if(type == UTM_OM_WRITING) {
		m_lnActualSize.HighPart	= 0;
		m_lnActualSize.LowPart	= 0;
		}
	else if(type == UTM_OM_APPEND) {
		m_lnActualSize.LowPart = GetFileSize(m_hFile, (ULONG *)&m_lnActualSize.HighPart);
		if(m_lnActualSize.LowPart == 0xFFFFFFFF && GetLastError() != NO_ERROR)
			return -1;

		// Size can't be less then file size when adding
		if(m_lnActualSize.LowPart < m_lnSize.LowPart)
			m_lnSize.HighPart = m_lnActualSize.LowPart;
		if(m_lnActualSize.HighPart < m_lnSize.HighPart)
			m_lnSize.HighPart = m_lnActualSize.HighPart;
		}
	else
		m_lnActualSize.QuadPart = m_lnSize.QuadPart;

	// Initialize increment value. Size divided by 8 ( or shifted by 3)
	m_lnIncrement.QuadPart = max(Int64ShraMod32(m_lnSize.QuadPart, 3), 4096);

	// If file is opened in append mode - increase it size
	if(type == UTM_OM_APPEND || (type == UTM_OM_WRITING && m_lnSize.HighPart == 0 && m_lnSize.LowPart == 0 ))
		m_lnSize.QuadPart += m_lnIncrement.QuadPart;

	// Set file size
	if(SetFilePointer(m_hFile, m_lnSize.LowPart, (long *)&m_lnSize.HighPart, FILE_BEGIN ) == 0xFFFFFFFF && GetLastError() != NO_ERROR)
		return -1;
	if(!SetEndOfFile(m_hFile))
		return -1;

	// Create file mapping
	if((m_hMapFile = CreateFileMapping(m_hFile, NULL, (type == UTM_OM_READING) ? PAGE_READONLY : PAGE_READWRITE, m_lnSize.HighPart, m_lnSize.LowPart, NULL)) == NULL || GetLastError() == ERROR_ALREADY_EXISTS)
		return -1;

	// Map view of file
	if((m_lpMapAddress = (LPSTR)MapViewOfFile(m_hMapFile, (type == UTM_OM_READING) ? FILE_MAP_READ : FILE_MAP_WRITE, 0, 0, 0)) == NULL)
		return -1;

	// Initialize position
	if(type == UTM_OM_APPEND)
		m_lnPosition.QuadPart = m_lnActualSize.QuadPart;
	else {
		m_lnPosition.HighPart	= 0;
		m_lnPosition.LowPart	= 0;
		}

	return UTE_SUCCESS;
}

/***********************************************
Close
	Close data file
PARAM:
	none
RETURN:
	-1 if error
************************************************/
int	CUT_MapFileDataSource::Close() {
	int		rt = UTE_SUCCESS;

	m_lnSize = m_lnActualSize;

	// Flush file
	if(m_lpMapAddress != NULL)
		if(!FlushViewOfFile(m_lpMapAddress, 0))
			rt = -1;

	// Unmap
	if(m_lpMapAddress != NULL) {
		if(!UnmapViewOfFile(m_lpMapAddress))
			rt = -1;
		m_lpMapAddress = NULL;
		}

	// Close file mapping
	if(m_hMapFile != NULL) {
		if(!CloseHandle(m_hMapFile))
			rt = -1;
		m_hMapFile = NULL;
		}


	// Close file
	if(m_hFile != INVALID_HANDLE_VALUE) {
		// Set Actual file size
		if(SetFilePointer(m_hFile, m_lnActualSize.LowPart, (long *)&m_lnActualSize.HighPart, FILE_BEGIN ) == 0xFFFFFFFF && GetLastError() != NO_ERROR)
			return -1;
		if(!SetEndOfFile(m_hFile))
			return -1;

		// Close file
		if(!CloseHandle(m_hFile))
			rt = -1;
		m_hFile = INVALID_HANDLE_VALUE;
		}

	return rt;
}

/***********************************************
ReadLine
	Reads next line from the message file.
PARAM:
	buffer		- buffer to read to
	maxsize		- size of the buffer
RETURN:
	0 - if no more data
************************************************/
int	CUT_MapFileDataSource::ReadLine(LPSTR buffer, size_t maxsize)
{
	// Wrong parameter
	if(buffer == NULL || m_lpMapAddress == NULL)		return -1;

	// Read to the buffer
	int	nBytesRead;
	if((nBytesRead = Read(buffer, maxsize - 1)) <= 0)
		return 0;

	// Find CrLf
	LPSTR	szCrLf;
	int		nLineSize = nBytesRead;
	if((szCrLf = strchr(buffer, '\r')) != NULL) {
		++szCrLf;
		if(*szCrLf == '\n')
			++szCrLf;
		*szCrLf = 0;
		nLineSize = (int)(szCrLf - buffer);
		}

	// Adjust current position
	if(nLineSize > 0)
		m_lnPosition.QuadPart -= (nBytesRead - nLineSize);

	return nLineSize;
}

/***********************************************
WriteLine
	Writes line to the message file
PARAM:
	buffer		- line to write
RETURN:
	-1 if error
************************************************/
int	CUT_MapFileDataSource::WriteLine(LPCSTR buffer) {

	// Wrong parameter
	if(buffer == NULL || m_lpMapAddress == NULL)		return -1;

	// Check if it was opened for writing
	if(m_OpenType == UTM_OM_READING)					return -1;

	// Write line to the buffer
	int		rt = 0, nBytesNumber = (int)strlen(buffer);

	if(nBytesNumber > 0)
		if((rt = Write(buffer, nBytesNumber)) == -1)
			return -1;

	// Write CRLF if the buffer doesn't have one
	if(!CUT_StrMethods::IsWithCRLF(buffer))
		if(Write("\r\n", 2) == -1)
			return -1;

	return rt;
}

/***********************************************
Read
	Reads specified number of bytes
PARAM:
	buffer		- buffer to read to
	count		- number of bytes to read
RETURN:
	number of bytes read
	-1 if error
************************************************/
int	CUT_MapFileDataSource::Read(LPSTR buffer, size_t count)
{
	// Wrong parameter
	if(buffer == NULL || m_lpMapAddress == NULL)		return -1;

	LARGE_INTEGER	lnBufferSize	= {(DWORD)count, 0};
	LARGE_INTEGER	lnDiff;
	LARGE_INTEGER	lnBytesToRead;

	// Calculate number of bytes to read
	lnDiff.QuadPart			= m_lnActualSize.QuadPart - m_lnPosition.QuadPart;
	lnBytesToRead.QuadPart	= min(lnBufferSize.QuadPart, lnDiff.QuadPart);

	// Read data
	if(lnBytesToRead.QuadPart > 0) {
		memcpy(buffer, (m_lpMapAddress + m_lnPosition.QuadPart), (unsigned int)lnBytesToRead.QuadPart);
		m_lnPosition.QuadPart += lnBytesToRead.QuadPart;
		*(buffer + lnBytesToRead.QuadPart) = 0;
		}
	else
		*buffer = 0;

	return (int)lnBytesToRead.QuadPart;
}

/***********************************************
Write
	Writes specified number of bytes
PARAM:
	buffer		- buffer to read to
	count		- number of bytes to read
RETURN:
	-1 if error
************************************************/
int	CUT_MapFileDataSource::Write(LPCSTR buffer, size_t count) {

	// Wrong parameter
	if(buffer == NULL || m_lpMapAddress == NULL)		return -1;

	// Check if it was opened for writing
	if(m_OpenType == UTM_OM_READING)					return -1;

	// Check if we'll overflow current buffer size
	LARGE_INTEGER	lnDataSize = { (DWORD)count + 1, 0 };
	if((lnDataSize.QuadPart + m_lnPosition.QuadPart) >= m_lnSize.QuadPart) {
		// We need more space

		BOOL			bOldTempFile;
		LARGE_INTEGER	lnOldActualSize, lnOldPosition;

		// Save current value
		lnOldActualSize.QuadPart	= m_lnActualSize.QuadPart;
		lnOldPosition.QuadPart		= m_lnPosition.QuadPart;
		bOldTempFile				= m_bTempFileName;

		// Set to force flashing view into the file
		m_bTempFileName = FALSE;

		// Increase buffer size
		m_lnActualSize.QuadPart = m_lnSize.QuadPart + max(m_lnIncrement.QuadPart, lnDataSize.QuadPart+100);

		// Reopen
		int rt = Open(m_OpenType);

		// Restore old values
		m_lnActualSize.QuadPart		= lnOldActualSize.QuadPart;
		m_lnPosition.QuadPart		= lnOldPosition.QuadPart;
		m_bTempFileName				= bOldTempFile;

		// Error while reopening
		if(rt == -1)
			return UTE_SUCCESS;
		}

	// Write data
	memcpy((m_lpMapAddress + m_lnPosition.QuadPart), buffer, count);
	m_lnPosition.QuadPart += count;
	m_lnActualSize.QuadPart = max(m_lnActualSize.QuadPart, m_lnPosition.QuadPart);
	*(m_lpMapAddress + m_lnPosition.QuadPart) = 0;

	return (int)count;
}
/****************************************
Seek
    Moves the file pointer to a specified
	location
Params
	offset	- number of bytes from origin
	origin	- initial position
				SEEK_CUR
				SEEK_END
				SEEK_SET
Return
    offset, in bytes, of the new position
	from the beginning of the file
	-1		- error
*****************************************/
long CUT_MapFileDataSource::Seek(long offset, int origin)
{
	LARGE_INTEGER	lnOffset = {0, 0};

	if(m_lpMapAddress == NULL)	return -1;

	switch(origin) {
		case(SEEK_CUR):
			if((m_lnPosition.QuadPart + offset) <= m_lnActualSize.QuadPart)
				lnOffset.QuadPart = m_lnPosition.QuadPart + offset;
			else
				return -1;
			break;
		case(SEEK_END):
			if(offset <= m_lnActualSize.QuadPart)
				lnOffset.QuadPart = m_lnActualSize.QuadPart - offset;
			else
				return -1;
			break;
		case(SEEK_SET):
			if(offset <= m_lnActualSize.QuadPart)
				lnOffset.QuadPart = offset;
			else
				return -1;
			break;

		default:
			return -1;
		}

	m_lnPosition.QuadPart = lnOffset.QuadPart;

	return (long)m_lnPosition.QuadPart;
}
