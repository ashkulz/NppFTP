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
#include "QueueWindow.h"

#include <commctrl.h>

QueueWindow::QueueWindow() :
	Window(NULL, WC_LISTVIEW)
{
	m_style = WS_CHILD | LVS_REPORT | LVS_NOSORTHEADER | LVS_SINGLESEL;
	m_exStyle = WS_EX_CLIENTEDGE;
}

QueueWindow::~QueueWindow() {
}

int QueueWindow::Create(HWND hParent) {
	int ret = Window::Create(hParent);
	if (ret == -1)
		return -1;

	ListView_SetExtendedListViewStyle(m_hwnd, LVS_EX_FULLROWSELECT);

	LVCOLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
	lvc.fmt = LVCFMT_LEFT;

	lvc.cx = 50;
	TCHAR strAction[] = TEXT("Action");
	lvc.pszText = strAction;
	ListView_InsertColumn(m_hwnd, 0, &lvc);

	lvc.cx = 60;
	TCHAR strProgress[] = TEXT("Progress");
	lvc.pszText = strProgress;
	ListView_InsertColumn(m_hwnd, 1, &lvc);

	lvc.cx = 250;
	TCHAR strFile[] = TEXT("File");
	lvc.pszText = strFile;
	ListView_InsertColumn(m_hwnd, 2, &lvc);

	return 0;
}

int QueueWindow::PushQueueItem(QueueOperation * op) {
	QueueOperation::QueueType type = op->GetType();
	if (!ValidType(type))
		return -1;

	LVITEM lvi;
	lvi.mask = LVIF_TEXT | LVIF_PARAM;
	lvi.iItem = GetNrItems();
	lvi.iSubItem = 0;
	lvi.lParam = (LPARAM)op;
	lvi.pszText = (TCHAR*)(type==QueueOperation::QueueTypeDownload||type==QueueOperation::QueueTypeDownloadHandle?TEXT("Download"):TEXT("Upload"));

	int index = ListView_InsertItem(m_hwnd,  &lvi);
	if (index == -1)
		return -1;

	ListView_SetItemText(m_hwnd, index, 1, TEXT("0.0%") );

	TCHAR * path = NULL;
	if (type == QueueOperation::QueueTypeDownload) {
		QueueDownload * qdld = (QueueDownload*)op;
		path = SU::Utf8ToTChar(qdld->GetExternalPath());
	} else if (type == QueueOperation::QueueTypeDownloadHandle) {
		QueueDownloadHandle * qdld = (QueueDownloadHandle*)op;
		path = SU::Utf8ToTChar(qdld->GetExternalPath());
	} else if (type == QueueOperation::QueueTypeUpload) {
		QueueUpload * quld = (QueueUpload*)op;
		path = SU::Utf8ToTChar(quld->GetExternalPath());
	}

	ListView_SetItemText(m_hwnd, index, 2, path );
	SU::FreeTChar(path);

	return 0;
}

int QueueWindow::PopQueueItem(QueueOperation * op) {
	if (!ValidType(op->GetType()))
		return -1;

	int index = GetNrItems()-1;
	if (index == -1)
		return -1;

	LVITEM lvi;
	lvi.iItem = 0;
	lvi.iSubItem = 0;
	lvi.mask = LVIF_PARAM;
	BOOL res = ListView_GetItem(m_hwnd, &lvi);
	if (res == FALSE)
		return -1;

	if ((QueueOperation*)lvi.lParam != op)
		return -1;

	res = ListView_DeleteItem(m_hwnd,index);
	if (res == FALSE)
		return -1;

	return 0;
}

int QueueWindow::RemoveQueueItem(QueueOperation * op) {
	if (!ValidType(op->GetType()))
		return -1;

	int index = GetItemIndex(op);
	if (index == -1)
		return -1;

	BOOL res = ListView_DeleteItem(m_hwnd,index);
	if (res == FALSE)
		return -1;

	return -1;
}

QueueOperation*	QueueWindow::GetSelectedQueueOperation() {
	int selectedindex = ListView_GetNextItem(m_hwnd, -1, LVNI_ALL|LVNI_SELECTED);
	if (selectedindex == -1)
		return NULL;

	LVITEM lvi;
	lvi.iItem = selectedindex;
	lvi.iSubItem = 0;
	lvi.mask = LVIF_PARAM;
	BOOL res = ListView_GetItem(m_hwnd, &lvi);
	if (res == FALSE)
		return NULL;

	return (QueueOperation*)lvi.lParam;
}

bool QueueWindow::GetSelectedQueueRect(RECT * pRect) {
	int selectedindex = ListView_GetNextItem(m_hwnd, -1, LVNI_ALL|LVNI_SELECTED);
	if (selectedindex == -1)
		return false;

	BOOL res = ListView_GetItemRect(m_hwnd, selectedindex, pRect, LVIR_LABEL);
	return (res == TRUE);
}

int QueueWindow::ProgressQueueItem(QueueOperation * op) {
	if (!ValidType(op->GetType()))
		return -1;

	int index = GetItemIndex(op);
	if (index == -1)
		return -1;

	if (index != 0) {
		return -1;
	}

	TCHAR buffer[10];
	float progress = op->GetProgress();
	if (progress > 100.0f || progress < 0.0f) {
		lstrcpy(buffer, TEXT("??"));
	} else {
		SU::TSprintf(buffer, 10, TEXT("%.1f%%"), progress);
	}

	ListView_SetItemText(m_hwnd, index, 1, buffer );

	return 0;
}

int QueueWindow::GetItemIndex(QueueOperation * op) {
	if (!ValidType(op->GetType()))
		return -1;

	int index = -1;
	int nritems = GetNrItems();

	LVITEM lvi;
	lvi.iItem = -1;
	lvi.iSubItem = 0;
	lvi.mask = LVIF_PARAM;
	lvi.lParam = 0;

	while((QueueOperation*)lvi.lParam != op && lvi.iItem < nritems) {
		lvi.iItem++;
		index++;
		BOOL res = ListView_GetItem(m_hwnd, &lvi);
		if (res == FALSE)
			return -1;
	}

	if (lvi.iItem == nritems)
		return -1;

	return index;
}

int QueueWindow::GetNrItems() {
	int res = ListView_GetItemCount(m_hwnd);

	return res;
}

bool QueueWindow::ValidType(QueueOperation::QueueType type) {
	return (
		type == QueueOperation::QueueTypeDownload ||
		type == QueueOperation::QueueTypeDownloadHandle ||
		type == QueueOperation::QueueTypeUpload
		);
}
