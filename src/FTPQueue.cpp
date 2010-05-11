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

#include "StdInc.h"
#include "FTPQueue.h"

const int ConditionQueueOps = 0;
const int ConditionQueueStop = 1;
const int ConditionQueueAcked = 2;
const int ConditionCount = 3;

FTPQueue::FTPQueue(FTPClientWrapper* wrapper) :
	m_wrapper(wrapper),
	m_running(false),
	m_stopping(false),
	m_performing(false),
	m_activeOp(NULL)
{
	m_monitor = new Monitor(ConditionCount);
	m_wrapper->SetProgressMonitor(this);
}

FTPQueue::~FTPQueue() {
	delete m_monitor;
	if (m_running)
		OutErr("[Queue] Error: queue still running\n");
}

int FTPQueue::Initialize() {
	if (m_running)
		return 0;

	m_stopping = false;
	m_running = true;

	::CreateThread(NULL, 0, &ThreadProc, this, 0, NULL);

	return 0;
}

int FTPQueue::Deinitialize() {
	if (!m_running)
		return 0;

	//Notifications sent from this function are handled immediatly, since they're from the local thread.
	//Therefore, peekmessage won't remove any messages that this function sent, as they won;t be queued

	m_monitor->Enter();
		m_stopping = true;

		if (m_performing) {
			m_queue.front()->Terminate();
			m_queue.front()->SendNotification(QueueOperation::QueueEventEnd);
			//m_queue.front()->SendNotification(QueueOperation::QueueEventRemove);
		}

		m_monitor->Signal(ConditionQueueOps);
		m_monitor->Wait(ConditionQueueStop);
	m_monitor->Exit();

	while (!m_queue.empty()) {
		//Remove any remaining messages (most notably Progress messages)
		m_queue.front()->ClearPendingNotifications();
		m_queue.front()->SendNotification(QueueOperation::QueueEventRemove);
		delete m_queue.front();
		m_queue.pop();
	}

	m_running = false;
	m_stopping = false;
	m_performing = false;

	return 0;
}

int FTPQueue::AddQueueOp(QueueOperation * op) {
	op->SetClient(m_wrapper);

	//Can safely inform queueWindow, Add is called by window thread
	op->SendNotification(QueueOperation::QueueEventAdd);

	m_monitor->Enter();
		m_queue.push(op);
		if (m_queue.size() == 1)
			m_monitor->Signal(ConditionQueueOps);
	m_monitor->Exit();

	return 0;
}

int FTPQueue::GetQueueSize() const {
	int res = 0;

	m_monitor->Enter();
	res = m_queue.size();
	m_monitor->Exit();

	return res;
}

int FTPQueue::ClearQueue() {
	QueueOperation * op = NULL;

	m_monitor->Enter();
		if (m_performing) {
			op = m_queue.front();
			m_queue.pop();
		}
		while (!m_queue.empty()) {
			m_queue.front()->SendNotification(QueueOperation::QueueEventRemove);
			delete m_queue.front();
			m_queue.pop();
		}
		if (m_performing) {
			m_queue.push(op);
		}
	m_monitor->Exit();

	return 0;
}

int FTPQueue::CancelQueueOp(QueueOperation * op) {
	m_monitor->Enter();
		if (m_performing) {
			if (op == m_queue.front()) {
				m_monitor->Exit();
				return -1;		//Cannot cancel running operation, only abort
			}
		}

		size_t size = m_queue.size();
		QueueOperation * queueop = NULL;
		for(size_t i = 0; i < size; i++) {
			queueop = m_queue.front();
			m_queue.pop();
			if (queueop == op) {
				queueop->SendNotification(QueueOperation::QueueEventRemove);
				delete queueop;
			} else {
				m_queue.push(queueop);
			}
		}
	m_monitor->Exit();

	return 0;
}

int FTPQueue::QueueLoop() {
	QueueOperation * op = NULL;

	while(!m_stopping) {

		m_monitor->Enter();
			while (m_queue.empty() && !m_stopping)
				m_monitor->Wait(ConditionQueueOps);

			if (m_stopping) {
				m_monitor->Exit();
				break;
			}

			op = m_queue.front();
			m_activeOp = op;
			m_performing = true;
		m_monitor->Exit();

		op->SendNotification(QueueOperation::QueueEventStart);
		op->SetRunning(true);
		op->Perform();
		op->SetRunning(false);
		op->SendNotification(QueueOperation::QueueEventEnd);

		if (m_stopping)
			break;

		op->SendNotification(QueueOperation::QueueEventRemove);

		m_monitor->Enter();
			m_activeOp = NULL;
			m_performing = false;
			m_queue.pop();
			delete op;
		m_monitor->Exit();
	}

	m_monitor->Enter();
		m_performing = false;
		m_monitor->Signal(ConditionQueueStop);
	m_monitor->Exit();

	return 0;
}

int FTPQueue::OnDataReceived(long received, long total) {
	m_monitor->Enter();
		if (!m_performing || !m_activeOp) {	//not called from performing thread?
			m_monitor->Exit();
			return -1;
		}
	m_monitor->Exit();


	if (total == -1) {
		m_activeOp->SetProgress(-1.0f);
	} else {
		float percentage = (float)received/(float)total * 100.0f;
		m_activeOp->SetProgress(percentage);
	}
	m_activeOp->SendNotification(QueueOperation::QueueEventProgress);
	return 0;
}

int FTPQueue::OnDataSent(long sent, long total) {
	m_monitor->Enter();
		if (!m_performing || !m_activeOp) {	//not called from performing thread?
			m_monitor->Exit();
			return -1;
		}
	m_monitor->Exit();


	if (total == -1) {
		m_activeOp->SetProgress(-1.0f);
	} else {
		float percentage = (float)sent/(float)total * 100.0f;
		m_activeOp->SetProgress(percentage);
	}
	m_activeOp->SendNotification(QueueOperation::QueueEventProgress);
	return 0;
}

int FTPQueue::QueueThread(FTPQueue* queue) {
	return queue->QueueLoop();
}

DWORD WINAPI ThreadProc(LPVOID param) {
	FTPQueue* queue = (FTPQueue*)param;
	return FTPQueue::QueueThread(queue);
}
