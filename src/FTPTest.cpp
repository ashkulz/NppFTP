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
#include "NppFTP.h"

void printFileTree(int level, FileObject* fo);
void Error(LPTSTR lpszFunction);

int main(int argc, char* argv[]) {
	NppFTP::InitAll(GetModuleHandle(NULL));
	NppFTP nppftp;
	NppData nppData;
	nppData._nppHandle = HWND_DESKTOP;
	FuncItem fItem;
	int res = nppftp.Start(nppData, 0, fItem);

	if (res == -1) {
		printf("Lasterr: %d\n", GetLastError());
		return 1;
	}

	BOOL msgres = 1;
	MSG msg;

	while(msgres != 0)
	{
		msgres = ::GetMessage(&msg, NULL, 0, 0);
		if (msgres == 0)
			break;
		if (msgres == -1) {
			printf("GetMessage error\n");
			break;
		}

		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}

	nppftp.Stop();

	return 0;
}

void printFileTree(int level, FileObject* fo) {
	for(int i = 0; i < level; i++)
		printf(" ");

	printf("%s\n", fo->GetName());

	if (fo->isDir()) {
		for(int i = 0; i < fo->GetChildCount(); i++) {
			printFileTree(level+1, fo->GetChild(i));
		}
	}
}


void Error(LPCTSTR lpszFunction) {
	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	if (lpszFunction == NULL) {
		lpszFunction = TEXT("Unknown function");
	}
	DWORD dw = GetLastError();

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,NULL,dw,MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &lpMsgBuf,0, NULL );

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,(lstrlen((LPCTSTR)lpMsgBuf)+lstrlen((LPCTSTR)lpszFunction)+40)*sizeof(TCHAR));
	wsprintf((LPTSTR)lpDisplayBuf,TEXT("%s failed with error %d: %s"),lpszFunction, dw, lpMsgBuf);

	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
}
