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
#include "Monitor.h"

Monitor::Monitor(int nrConditions) :
	m_nrConditions(nrConditions),
	m_enterCount(0)
{
	InitializeCriticalSection(&m_critMonitor);
	m_conditions = new HANDLE[m_nrConditions];
	for(int i = 0; i < m_nrConditions; i++) {
		m_conditions[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
	}
}
Monitor::~Monitor() {
	DeleteCriticalSection(&m_critMonitor);
	for(int i = 0; i < m_nrConditions; i++) {
		CloseHandle(m_conditions[i]);
	}
	delete [] m_conditions;
}

int Monitor::Enter() {
	EnterCriticalSection(&m_critMonitor);

	m_enterCount++;

	return 0;
}

int Monitor::Exit() {
	if (m_enterCount > 0) {
		m_enterCount--;
		LeaveCriticalSection(&m_critMonitor);
	} else {
		return -1;
	}
	return 0;
}

int Monitor::Wait(int condition) {
	//This can cause a deadlock in rare cases. Just dont do an Enter();Signal(); combo
	ResetEvent(m_conditions[condition]);

	Exit();

	WaitForSingleObject(m_conditions[condition], INFINITE);

	Enter();

	return 0;
}

int Monitor::Signal(int condition) {
	SetEvent(m_conditions[condition]);

	return 0;
}
