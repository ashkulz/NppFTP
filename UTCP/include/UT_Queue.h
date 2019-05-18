// =================================================================
//  class: CUT_Queue, CUT_FIFO_Queue
//  File:  UT_Queue.h
//
//  Purpose:
//
//      CUT_Queue       - Queue abstract class
//      CUT_FIFO_Queue  - FIFO queue
//
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


#ifndef IncludeCUT_Queue
#define IncludeCUT_Queue

// =================================================================
//  CUT_Queue abstract class
//
//  Base queue class
// =================================================================
class CUT_Queue {

    public:

        CUT_Queue() {}
        virtual ~CUT_Queue() {}

        public:

        // Return the amount of data in the queue
        virtual int GetDataSize() = 0;

        // Return the amount of free space in the queue
        virtual int GetFreeSize() = 0;

        // Read data from the queue
        virtual int Read(LPBYTE buffer, unsigned int count) = 0;

        // Read data from the queue without moving the read pointer
        virtual int Peek(LPBYTE buffer, unsigned int count) = 0;

        // Write data to the queue
        virtual int Write(LPBYTE buffer, unsigned int count) = 0;
} ;


// =================================================================
//  CUT_FIFO_Queue class
//
//  FIFO queue class based on first-in-first-out set with separate
//  read and write pointers.
// =================================================================
class CUT_FIFO_Queue : public CUT_Queue {

    protected:

        int m_cbBuffer;
        int m_cbGrow;
        LPBYTE m_pbBuffer;
        int m_iReadPointer;
        int m_iWritePointer;

    public:

        CUT_FIFO_Queue(unsigned int size, unsigned int growSize = 0) ;
        virtual ~CUT_FIFO_Queue() ;

        public:

        // Return the amount of data in the queue
        virtual int GetDataSize();

        // Return the amount of free space in the queue
        virtual int GetFreeSize();

        // Grow the queue (increase free space in the queue)
        virtual bool Grow() ;

        // Read data from the queue
        virtual int Read(LPBYTE pbBuffer, unsigned int count);

        // Read data from the queue without moving the read pointer
        virtual int Peek(LPBYTE buffer, unsigned int count);

        // Write data to the queue
        virtual int Write(LPBYTE pbBuffer, unsigned int count);
} ;

#endif
