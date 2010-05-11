//=================================================================
//  class: CUT_Queue, CUT_FIFO_Queue
//  File:  UT_Queue.cpp
//  
//  Purpose:
//	
//		CUT_Queue		- Queue abstract class
//		CUT_FIFO_Queue	- FIFO queue
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

#include "UT_Queue.h"


// =================================================================
//	CUT_FIFO_Queue class
//
//	Queue class based on FIFO set
// =================================================================

/***********************************************
CUT_FIFO_Queue
	Constructor
PARAM:
	size - size for circular queue
RETURN:
	none
************************************************/
CUT_FIFO_Queue::CUT_FIFO_Queue(unsigned int size, unsigned int growSize) {

	m_cbBuffer = size + 1;
	m_cbGrow = growSize;
	m_pbBuffer = new BYTE[m_cbBuffer];
	m_iReadPointer = 0;
	m_iWritePointer = 0;
}

/***********************************************
CUT_FIFO_Queue
	Destructor
RETURN:
	none
************************************************/
CUT_FIFO_Queue::~CUT_FIFO_Queue() {

	if(m_pbBuffer != NULL)
		delete [] m_pbBuffer;
}

/***********************************************
GetDataSize
	Returns the number of bytes of data 
	available in the queue
RETURN:
	The size of the available data in the queue
************************************************/
int CUT_FIFO_Queue::GetDataSize() {

	if(m_iReadPointer == m_iWritePointer)
		return 0;
	
	if(m_iReadPointer < m_iWritePointer)
		return (m_iWritePointer - m_iReadPointer);

	return ((m_iWritePointer + m_cbBuffer) - m_iReadPointer);
}

/***********************************************
GetFreeSize
	Returns the number of bytes of free space
	available in the queue
RETURN:
	The size of the free space in the queue
************************************************/
int CUT_FIFO_Queue::GetFreeSize() {

	return m_cbBuffer - GetDataSize() - 1;
}

/***********************************************
Grow
	Grows the size of the queue if the queue
	can be grown.
RETURN:
	true if successful, else false
NOTE:
	this is terribly inefficient and should
	only be used when absolutely necessary.
************************************************/
bool CUT_FIFO_Queue::Grow() {

	if(m_cbGrow == 0)
		return false;

	LPBYTE pbNew = new BYTE[m_cbBuffer + m_cbGrow];

	if(pbNew == NULL)
		return false;

	int curSize = GetDataSize();

	if(Peek(pbNew, curSize) != curSize) {
		delete [] pbNew;

		return false;
	}

	delete [] m_pbBuffer;
	m_pbBuffer = pbNew;
	m_cbBuffer += m_cbGrow;
	m_iReadPointer = 0;
	m_iWritePointer = curSize;

	return true;
}

/***********************************************
Read
	Reads data from the read pointer and adjusts
	the read pointer.
PARAM:
	pbBuffer - pointer to the buffer to receive
			   read data
	count    - the number of bytes to read
RETURN:
	The actual number of bytes read
************************************************/
int CUT_FIFO_Queue::Read(LPBYTE pbBuffer, unsigned int count) {

	int nNumToRead = min(GetDataSize(), (int)count);
	int nNumRead = 0;

	if(m_iReadPointer + nNumToRead > m_cbBuffer) {

		int nNumAtEnd = m_cbBuffer - m_iReadPointer;

		memcpy(pbBuffer, m_pbBuffer + m_iReadPointer, nNumAtEnd);
		nNumRead += nNumAtEnd;
		nNumToRead -= nNumAtEnd;
		m_iReadPointer = 0 ;
	}

	memcpy(pbBuffer + nNumRead, m_pbBuffer + m_iReadPointer, nNumToRead);

	nNumRead += nNumToRead;
	m_iReadPointer += nNumToRead;

	return nNumRead;
}

/***********************************************
Peek
	Reads data from the read pointer but does 
	not adjust the read pointer.
PARAM:
	pbBuffer - pointer to the buffer to receive
			   read data
	count    - the number of bytes to read
RETURN:
	The actual number of bytes read
************************************************/
int CUT_FIFO_Queue::Peek(LPBYTE pbBuffer, unsigned int count) {

	int iReadPointer = m_iReadPointer;
	int nNumToRead = min(GetDataSize(), (int)count);
	int nNumRead = 0;

	if(iReadPointer + nNumToRead > m_cbBuffer) {

		int nNumAtEnd = m_cbBuffer - iReadPointer;

		memcpy(pbBuffer, m_pbBuffer + iReadPointer, nNumAtEnd);
		nNumRead += nNumAtEnd;
		nNumToRead -= nNumAtEnd;
		iReadPointer = 0 ;
	}

	memcpy(pbBuffer + nNumRead, m_pbBuffer + iReadPointer, nNumToRead);

	nNumRead += nNumToRead;

	return nNumRead;
}

/***********************************************
Write
	Writes data to the write pointer and adjusts
	the write pointer.
PARAM:
	pbBuffer - pointer to the buffer to receive
			   write data
	count    - the number of bytes to write
RETURN:
	The actual number of bytes written
************************************************/
int CUT_FIFO_Queue::Write(LPBYTE pbBuffer, unsigned int count) {

	int nNumToWrite = min(GetFreeSize(), (int)count);
	int nNumWritten = 0;

	if(m_iWritePointer + nNumToWrite > m_cbBuffer) {

		int nNumAtEnd = m_cbBuffer - m_iWritePointer;

		memcpy(m_pbBuffer + m_iWritePointer, pbBuffer, nNumAtEnd);
		nNumWritten += nNumAtEnd;
		nNumToWrite -= nNumAtEnd;
		m_iWritePointer = 0 ;
	}

	memcpy(m_pbBuffer + m_iWritePointer, pbBuffer + nNumWritten, nNumToWrite);

	nNumWritten += nNumToWrite;
	m_iWritePointer += nNumToWrite;

	return nNumWritten;
}
