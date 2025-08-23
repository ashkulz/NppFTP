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
#include "Npp/PluginInterface.h"

#include "NppFTP.h"
#include "resource.h"
#include <stdlib.h>
#include <fcntl.h>

typedef void* BufferID;

const int	nrFuncItem = 3;
FuncItem	funcItems[nrFuncItem];
NppData		nppData;
NppFTP		nppFTP;
HBITMAP		hFTPBitmap;
toolbarIcons	tbiFtp;

bool		show = false;
bool		isStarted = false;

void __cdecl ShowFTPWindow();
void __cdecl FocusFTPWindow();
void __cdecl ShowAboutDialog();
void __cdecl FakeItem();

extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD  fdwReason, LPVOID /*lpvReserved*/) {
	BOOL result = TRUE;

	switch(fdwReason ) {
		case DLL_PROCESS_ATTACH: {
			NppFTP::InitAll(hinstDLL);
			tbiFtp.hToolbarBmp = CreateMappedBitmap(hinstDLL, IDB_BITMAP_FOLDERS, 0, 0, 0);
			tbiFtp.hToolbarIcon = (HICON)::LoadImage(hinstDLL, MAKEINTRESOURCE(IDI_ICON_MAINFOLDERS), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);

			result = TRUE;
			break; }
		case DLL_PROCESS_DETACH: {
			DeleteObject(tbiFtp.hToolbarBmp);
			DeleteObject(tbiFtp.hToolbarIcon);
			break; }
		case DLL_THREAD_ATTACH: {
			break; }
		case DLL_THREAD_DETACH: {
			break; }
	}

	return result;
}

void setInfo(NppData _nppData) {
	nppData = _nppData;
}

const TCHAR * getName() {
	return TEXT("NppFTP");
}

FuncItem * getFuncsArray(int * arraysize) {
	*arraysize = nrFuncItem;

	funcItems[0]._pFunc = &ShowFTPWindow;
	funcItems[0]._init2Check = false;
	funcItems[0]._pShKey = NULL;
	lstrcpyn(funcItems[0]._itemName, TEXT("Show NppFTP Window"), menuItemSize);

	funcItems[1]._pFunc = &FocusFTPWindow;
	funcItems[1]._init2Check = false;
	funcItems[1]._pShKey = NULL;
	lstrcpyn(funcItems[1]._itemName, TEXT("Focus NppFTP Window"), menuItemSize);

	funcItems[2]._pFunc = &ShowAboutDialog;
	funcItems[2]._init2Check = false;
	funcItems[2]._pShKey = NULL;
	lstrcpyn(funcItems[2]._itemName, TEXT("About NppFTP"), menuItemSize);

	return &funcItems[0];
}

void beNotified(SCNotification * scNotification) {
	switch(scNotification->nmhdr.code) {
		case NPPN_TBMODIFICATION: {
			SendMessage(nppData._nppHandle, NPPM_ADDTOOLBARICON_DEPRECATED, (WPARAM)funcItems[0]._cmdID, (LPARAM)&tbiFtp);
			break; }
		case NPPN_READY: {
			TCHAR configStore[MAX_PATH]{};
			configStore[0] = 0;
			SendMessage(nppData._nppHandle, NPPM_GETPLUGINSCONFIGDIR, (WPARAM)MAX_PATH, (LPARAM)configStore);
			int res = nppFTP.Start(nppData, configStore, 0, funcItems[0]);
			if (res == -1) {
				break;
			}
			isStarted = true;
			if (show)
				nppFTP.ShowFTPWindow();
			break; }
		case NPPN_SHUTDOWN: {
			nppFTP.Stop();
			break; }
		case NPPN_FILESAVED: {
			BufferID buf = (BufferID)scNotification->nmhdr.idFrom;
			int size = ::SendMessage(nppData._nppHandle, NPPM_GETFULLPATHFROMBUFFERID, (WPARAM)buf, (LPARAM)NULL);
			if (size == -1)
				break;
			TCHAR * path = new TCHAR[size+1];
			::SendMessage(nppData._nppHandle, NPPM_GETFULLPATHFROMBUFFERID, (WPARAM)buf, (LPARAM)path);
			nppFTP.OnSave(path);
			delete [] path;
			break; }
		case NPPN_BUFFERACTIVATED: {
			BufferID buf = (BufferID)scNotification->nmhdr.idFrom;
			int size = ::SendMessage(nppData._nppHandle, NPPM_GETFULLPATHFROMBUFFERID, (WPARAM)buf, (LPARAM)NULL);
			if (size == -1)
				break;
			TCHAR * path = new TCHAR[size+1];
			::SendMessage(nppData._nppHandle, NPPM_GETFULLPATHFROMBUFFERID, (WPARAM)buf, (LPARAM)path);
			nppFTP.OnActivateLocalFile(path);
			delete [] path;
			break; }
	}
}

LRESULT messageProc(UINT /*Message*/, WPARAM /*wParam*/, LPARAM /*lParam*/) {
	return TRUE;
}

#ifdef UNICODE
BOOL isUnicode() {
	return TRUE;
}
#endif //UNICODE


/////
//Menu Functions
/////

void __cdecl ShowFTPWindow() {
	show = !show;
	if (isStarted)
		nppFTP.ShowFTPWindow();
}

void __cdecl FocusFTPWindow() {
	show = !show;
	if (isStarted)
		nppFTP.FocusFTPWindow();
}

void __cdecl ShowAboutDialog() {
	if (isStarted)
		nppFTP.ShowAboutDialog();
}

void __cdecl FakeItem() {
}
