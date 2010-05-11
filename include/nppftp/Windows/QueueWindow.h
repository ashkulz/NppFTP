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

#ifndef QUEUEWINDOW_H
#define QUEUEWINDOW_H

#include "Window.h"

#include "QueueOperation.h"

class QueueWindow : public Window {
public:
							QueueWindow();
	virtual					~QueueWindow();

	virtual int				Create(HWND hParent);

	virtual int				PushQueueItem(QueueOperation * op);
	virtual int				PopQueueItem(QueueOperation * op);
	virtual int				RemoveQueueItem(QueueOperation * op);

	virtual QueueOperation*	GetSelectedQueueOperation();

	virtual int				ProgressQueueItem(QueueOperation * op);
private:
	virtual int				GetItemIndex(QueueOperation * op);
	virtual int				GetNrItems();

};

#endif //QUEUEWINDOW_H
