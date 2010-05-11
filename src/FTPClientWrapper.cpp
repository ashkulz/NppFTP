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

FTPClientWrapper::FTPClientWrapper(Client_Type type, const char * host, int port, const char * user, const char * password) :
	m_type(type),
	m_connected(false),
	m_aborting(false),
	m_busy(false),
	m_timeout(30),
	m_progmon(NULL),
	m_certificates(NULL)
{
	m_hostname = SU::strdup(host);
	m_port = port;
	m_username = SU::strdup(user);
	m_password = SU::strdup(password);
}

FTPClientWrapper::~FTPClientWrapper() {
	if (m_connected)	//cannot simply call Disconnect as it's virtual
		OutErr("[FTPClientWrapper] connection active in destructor");

	free(m_hostname);
	free(m_username);
	free(m_password);
}

Client_Type FTPClientWrapper::GetType() {
	return m_type;
}

int FTPClientWrapper::SetTimeout(int timeout) {
	m_timeout = timeout;
	return 0;
}

int FTPClientWrapper::SetProgressMonitor(ProgressMonitor * progmon) {
	m_progmon = progmon;
	return 0;
}

int FTPClientWrapper::SetCertificates(vX509 * x509Vect) {
	m_certificates = x509Vect;
	return 0;
}

int FTPClientWrapper::ReleaseDir(FTPFile* files, int /*size*/) {
	if (!files)
		return -1;
	delete [] files;

	return 0;
}

bool FTPClientWrapper::IsConnected() {
	return m_connected;
}

int FTPClientWrapper::Abort() {
	m_aborting = true;
	return 0;
}

int FTPClientWrapper::OnReturn(int res) {
	m_aborting = false;
	return res;
}
