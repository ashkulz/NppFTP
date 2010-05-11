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

#ifndef MONITOR_H
#define MONITOR_H

class Monitor {	//Should actually be: Half-assed monitor
public:
							Monitor(int nrConditions);
	virtual					~Monitor();	//assumes no events and CSs are in use

	virtual int				Enter();
	virtual int				Exit();

	virtual int				Wait(int condition);
	virtual int				Signal(int condition);
private:
	int						m_nrConditions;
	CRITICAL_SECTION		m_critMonitor;
	HANDLE*					m_conditions;

	int						m_enterCount;
};

#endif //MONITOR_H
