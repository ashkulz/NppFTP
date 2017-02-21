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
#include "FTPClientWrapper.h"

#include "SSLCertificates.h"
#include "MessageDialog.h"
#include <algorithm>

FTPClientWrapperZOS::FTPClientWrapperZOS(const char * host, int port, const char * user, const char * password) :
	FTPClientWrapperSSL(host, port, user, password)
{

}

FTPClientWrapper* FTPClientWrapperZOS::Clone() {
	FTPClientWrapperZOS* wrapper = new FTPClientWrapperZOS(m_hostname, m_port, m_username, m_password);
	wrapper->SetMode(m_mode);
	if (m_ftpListParams)
		wrapper->m_ftpListParams = SU::strdup(m_ftpListParams);
	wrapper->SetTimeout(m_timeout);
	wrapper->SetProgressMonitor(m_progmon);
	wrapper->SetCertificates(m_certificates);

	wrapper->m_client.SetFireWallMode(m_client.GetFireWallMode());
	wrapper->m_client.SetTransferType(m_client.GetTransferType());

	return wrapper;
}

char* FTPClientWrapperZOS::CleanToZOSPath(const char * ftpfile) {
	char* path = strdup(ftpfile);

	// Remove first '/' if exists (no '/' in ZOS)
	if (path[1] == '\'') {
		path++;
	}

	// Transform UNIX path to ZOS path
	int i = 1;
	while (path[i]) {
		if (path[i] == '/')
			path[i] = '.';
		if (path[i] == '\'') {
			memmove(&path[i], &path[i + 1], strlen(path) - i);
			i--;
		}
		i++;
	}

	// Append ' to string (string must finish by ' but not by .)
	if (path[strlen(path) - 1] == '.')
		path[strlen(path) - 1] = '\'';
	else
		strcat(path, "'");

	return path;
}

BOOL FTPClientWrapperZOS::AddParenthesis(char* path) {
	// Check if file is in root or in container
	int res = m_client.GetDirInfo(path);
	if (res != UTE_SUCCESS) {
		// Add parenthesis to last part (separated by .)
		char* m_name = strrchr(path, '.');
        if (m_name == NULL)
            return FALSE;

		m_name[0] = '(';
		m_name[strlen(m_name) - 1] = ')';
		strcat(m_name, "'");

		return TRUE;
	}

	return FALSE;
}

char* FTPClientWrapperZOS::GetZOSPath(const char * ftpfile) {
	char* path = CleanToZOSPath(ftpfile);
	AddParenthesis(path);
	return path;
}

int FTPClientWrapperZOS::GetDir(const char * path, FTPFile** files) {
	return FTPClientWrapperSSL::GetDir(GetZOSPath(path), files);
}

int FTPClientWrapperZOS::SendFile(const TCHAR * localfile, const char * ftpfile) {
	return FTPClientWrapperSSL::SendFile(localfile, GetZOSPath(ftpfile));
}

int FTPClientWrapperZOS::ReceiveFile(const TCHAR * localfile, const char * ftpfile) {
	return FTPClientWrapperSSL::ReceiveFile(localfile, GetZOSPath(ftpfile));
}

int FTPClientWrapperZOS::Rename(const char * from, const char * to) {
	char* pathFrom = CleanToZOSPath(from);
	char* pathTo = CleanToZOSPath(to);
	BOOL hasParenthesis = AddParenthesis(pathFrom);

	if (hasParenthesis)
		AddParenthesis(pathTo);

	return FTPClientWrapperSSL::Rename(pathFrom, pathTo);
}

int FTPClientWrapperZOS::MkFile(const char * path) {
	char* zospath = CleanToZOSPath(path);
	char* zospathFlat = CleanToZOSPath(path);
	BOOL hasParenthesis = AddParenthesis(zospath);
	int res;

	res = FTPClientWrapperSSL::MkFile(zospath); // try with parenthesis
	if (hasParenthesis && res != UTE_SUCCESS) {
		res = FTPClientWrapperSSL::MkFile(zospathFlat); // try without parenthesis
	}

	return res;
}

int	 FTPClientWrapperZOS::DeleteFile(const char * path) {
	char* zospath = CleanToZOSPath(path);
	char* zospathFlat = CleanToZOSPath(path);
	BOOL hasParenthesis = AddParenthesis(zospath);
	int res;

	res = FTPClientWrapperSSL::DeleteFile(zospath); // try with parenthesis
	if (hasParenthesis && res != UTE_SUCCESS) {
		res = FTPClientWrapperSSL::DeleteFile(zospathFlat); // try without parenthesis
	}

	return res;
}

int FTPClientWrapperZOS::MkDir(const char * path) {
	char* zospath = CleanToZOSPath(path);
	return FTPClientWrapperSSL::MkDir(zospath);
}

int FTPClientWrapperZOS::RmDir(const char * path) {
	char* zospath = CleanToZOSPath(path);
	return FTPClientWrapperSSL::RmDir(zospath);
}
