/*
    NppFTP: FTP/SFTP functionality for Notepad++
    Copyright (C) 2010  Harry (harrybharry@users.sourceforge.net)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef FTPQUEUE_H
#define FTPQUEUE_H

#include "FTPClientWrapper.h"
#include "Monitor.h"
#include "QueueOperation.h"

typedef std::queue<QueueOperation*> VQueue;

/*
Some notes about threading:
- It is very well possible for End/Remove messages to be sent twice
- If Terminate() is called on a queueoperation, it will not sendn otifications to another thread, but it will to the same thread
*/

class FTPQueue : public ProgressMonitor {
public:
							FTPQueue(FTPClientWrapper* wrapper);
	virtual					~FTPQueue();

	//Only to be called by creating thread
	virtual int				Initialize();
	virtual int				Deinitialize();

	virtual int				AddQueueOp(QueueOperation * op);
	virtual int				GetQueueSize() const;
	virtual int				ClearQueue();
	virtual int				CancelQueueOp(QueueOperation * op);

	virtual int				QueueLoop();

	virtual int				OnDataReceived(long received, long total);
	virtual int				OnDataSent(long sent, long total);

	static int				QueueThread(FTPQueue* queue);
private:
	Monitor*				m_monitor;
	FTPClientWrapper*		m_wrapper;
	bool					m_running;
	bool					m_stopping;
	bool					m_performing;
	QueueOperation*			m_activeOp;

	VQueue					m_queue;
};

DWORD WINAPI ThreadProc(LPVOID param);

#endif //FTPQUEUE_H
