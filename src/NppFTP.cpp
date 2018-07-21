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

#include "AboutDialog.h"
#include "InputDialog.h"
#include "Encryption.h"
#include "DragDropWindow.h"
#include "FTPSettings.h"
#include <commctrl.h>

HWND _MainOutputWindow = NULL;
char * _HostsFile = NULL;
TCHAR * _ConfigPath = NULL;

NppFTP::NppFTP() :
	m_ftpSettings(NULL),
	m_ftpSession(NULL),
	m_ftpWindow(NULL),
	m_activeSession(false),
	m_configStore(NULL)
{
	m_ftpSettings = new FTPSettings();
}

NppFTP::~NppFTP() {
	for(size_t i = 0; i < m_profiles.size(); i++) {
		m_profiles[i]->Release();
		//delete m_profiles[i];
	}

	SSLCertificates::FreeX509Vector(m_certificates);

	if (m_configStore)
		delete [] m_configStore;

	delete m_ftpSettings;
}

int NppFTP::Start(NppData nppData, TCHAR * nppConfigStore, int id, FuncItem funcItem) {
	m_nppData = nppData;

	PF::Init();
	Encryption::Init();

	m_configStore = new TCHAR[MAX_PATH];
	lstrcpy(m_configStore, nppConfigStore);
	::PathAppend(m_configStore, TEXT("NppFTP"));
	int res = PU::CreateLocalDir(m_configStore);
	if (res == -1)
		return -1;

	_ConfigPath = new TCHAR[MAX_PATH];
	lstrcpy(_ConfigPath, nppConfigStore);
	::PathAppend(_ConfigPath, TEXT("NppFTP\\"));

	_HostsFile = new char[MAX_PATH];
	char * utf8Store = SU::TCharToCP(m_configStore, CP_ACP);
	strcpy(_HostsFile, utf8Store);
	::PathCombineA(_HostsFile, utf8Store, "known_hosts");
	SU::FreeChar(utf8Store);

	_MainOutputWindow = nppData._nppHandle;

	res = LoadSettings();
	if (res == -1)
		return -1;

	m_ftpWindow = new FTPWindow();
	m_ftpSession = new FTPSession();

	res = m_ftpWindow->Create(nppData._nppHandle, nppData._nppHandle, id, funcItem._cmdID);
	if (res == -1) {
		return -1;
	}

	res = m_ftpSession->Init(m_ftpWindow, m_ftpSettings);
	if (res == -1) {
		m_ftpWindow->Destroy();
		return -1;
	}

	res = m_ftpWindow->Init(m_ftpSession, &m_profiles, m_ftpSettings);
	if (res == -1) {
		m_ftpSession->Deinit();
		m_ftpWindow->Destroy();
		return -1;
	}

	res = m_ftpSession->SetCertificates(&m_certificates);

	OutMsg("[NppFTP] Everything initialized");

	return 0;
}

int NppFTP::Stop() {
	SaveSettings();

	//delete m_ftpWindow;
	//delete m_ftpSession;

	m_ftpWindow = NULL;
	m_ftpSession = NULL;

	if (_HostsFile) {
		delete [] _HostsFile;
		_HostsFile = NULL;
	}

	if (_ConfigPath) {
		delete [] _ConfigPath;
		_ConfigPath = NULL;
	}

	PF::Deinit();
	Encryption::Deinit();

	return 0;
}

int NppFTP::ShowFTPWindow() {
	bool show = !(m_ftpWindow->IsVisible());
	m_ftpWindow->Show(show);

	return 0;
}

int NppFTP::FocusFTPWindow() {
	bool shown = (m_ftpWindow->IsVisible());
	if (!shown)
		m_ftpWindow->Show(true);
	m_ftpWindow->Focus();

	return 0;
}



int NppFTP::ShowAboutDialog() {
	AboutDialog ab;
	ab.Create(m_nppData._nppHandle);
	return 0;
}

int NppFTP::OnSave(const TCHAR* path) {
	if (!path || !m_ftpSession)
		return -1;

	if (!m_ftpSession->IsConnected())
	{
		AttemptToAutoConnect(path);
	}

	if (m_ftpSession->IsConnected()) {
		m_ftpSession->UploadFileCache(path);
	}

	return 0;
}

int NppFTP::OnActivateLocalFile(const TCHAR* path) {
	if (!path || !m_ftpWindow)
		return -1;

	return m_ftpWindow->OnActivateLocalFile(path);
}

int NppFTP::InitAll(HINSTANCE hInst) {
	Window::SetDefaultInstance(hInst);
	FTPWindow::RegisterClass();
	OutputWindow::RegisterClass();
	DragDropWindow::RegisterClass();

    INITCOMMONCONTROLSEX icce;
    icce.dwSize = sizeof(icce);
    icce.dwICC = ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icce);

	return 0;
}

int NppFTP::LoadSettings() {
	int result = 0;

	char xmlPath[MAX_PATH];
	char * utf8Store = SU::TCharToCP(m_configStore, CP_ACP);
	strcpy(xmlPath, utf8Store);
	::PathCombineA(xmlPath, utf8Store, "NppFTP.xml");

	TiXmlDocument settingsDoc = TiXmlDocument(xmlPath);
	settingsDoc.LoadFile();

	strcpy(xmlPath, utf8Store);
	::PathCombineA(xmlPath, utf8Store, "Certificates.xml");
	SU::FreeChar(utf8Store);

	TiXmlDocument certificatesDoc = TiXmlDocument(xmlPath);
	certificatesDoc.LoadFile();

	TiXmlElement* ftpElem = settingsDoc.FirstChildElement("NppFTP");
	if (!ftpElem) {
		result = 1;
		return result;
	}

	m_ftpSettings->LoadSettings(ftpElem);

	TiXmlElement * profilesElem = ftpElem->FirstChildElement(FTPProfile::ProfilesElement);
	if (!profilesElem) {
		m_profiles.clear();
		result = 1;
	} else {
		m_profiles = FTPProfile::LoadProfiles(profilesElem);
		for(size_t i = 0; i < m_profiles.size(); i++) {
			m_profiles.at(i)->AddRef();
			m_profiles.at(i)->SetCacheParent(m_ftpSettings->GetGlobalCache());
		}
	}


	ftpElem = certificatesDoc.FirstChildElement("NppFTP");
	if (!ftpElem) {
		m_certificates.clear();
		result = 1;
	} else {
		TiXmlElement * dersElem = ftpElem->FirstChildElement(SSLCertificates::DERsElem);
		if (!dersElem) {
			m_certificates.clear();
			result = 1;
		} else {
			vDER derVect = SSLCertificates::LoadDER(dersElem);
			m_certificates = SSLCertificates::ConvertDERVector(derVect);
			SSLCertificates::FreeDERVector(derVect);
		}
	}

	return result;
}

int NppFTP::SaveSettings() {

	char xmlPath[MAX_PATH];
	char * utf8Store = SU::TCharToCP(m_configStore, CP_ACP);
	strcpy(xmlPath, utf8Store);
	::PathCombineA(xmlPath, utf8Store, "NppFTP.xml");

	TiXmlDocument * settingsDoc = new TiXmlDocument(xmlPath);
	TiXmlDeclaration * decl = new TiXmlDeclaration("1.0", "UTF-8", "");
	settingsDoc->LinkEndChild(decl);

	strcpy(xmlPath, utf8Store);
	::PathCombineA(xmlPath, utf8Store, "Certificates.xml");
	SU::FreeChar(utf8Store);

	TiXmlDocument * certificatesDoc = new TiXmlDocument(xmlPath);
	decl = new TiXmlDeclaration("1.0", "UTF-8", "");
	certificatesDoc->LinkEndChild(decl);

	TiXmlElement * ftpElem = new TiXmlElement("NppFTP");
	settingsDoc->LinkEndChild(ftpElem);

	m_ftpSettings->SaveSettings(ftpElem);

	TiXmlElement * profilesElem = FTPProfile::SaveProfiles(m_profiles);
	ftpElem->LinkEndChild(profilesElem);

	ftpElem = new TiXmlElement("NppFTP");
	certificatesDoc->LinkEndChild(ftpElem);

	vDER derVect = SSLCertificates::ConvertX509Vector(m_certificates);
	TiXmlElement * dersElem = SSLCertificates::SaveDER(derVect);
	SSLCertificates::FreeDERVector(derVect);
	ftpElem->LinkEndChild(dersElem);

	settingsDoc->SaveFile();
	certificatesDoc->SaveFile();

	delete settingsDoc;
	delete certificatesDoc;

	return 0;
}

//  TODO this method could probably be expanded to automatically connect to correct FTP server even when other connection is already present
void NppFTP::AttemptToAutoConnect(const TCHAR* path)
{
	if (!path)
		return;

	FTPProfile* profile = nullptr;

	// we're going to parse the current username from the file path
	TCHAR* charScanner = const_cast<TCHAR*>(path);
	int atSymbolIndex = 0;
	int previousBackslashIndex = 0;
	while (*charScanner != '\0')
	{
		if (charScanner[atSymbolIndex] == (TCHAR)'\\')
			previousBackslashIndex = atSymbolIndex + 1;
		if (charScanner[atSymbolIndex] == (TCHAR)'@')
			break;
		atSymbolIndex++;
	}

	// we store that parsed username into the variable username below
	if (atSymbolIndex>0)
	{
		TCHAR username[FILENAME_MAX];	// assume username+path is smaller than largest allowed filename
		memset(username, '\0', FILENAME_MAX);
		int nameLength = (atSymbolIndex - previousBackslashIndex - 1);
		memcpy(username, &path[previousBackslashIndex], nameLength * 2);	// TCHAR is 2 bytes so we have to double the nameLength to be correct in byte-space

		// now we try to find a reasonable partial match given all of our available saved ftp account names
		for (u_int i = 0; i < m_profiles.size() && !profile; i++)
		{
			const TCHAR* profileName = m_profiles.at(i)->GetName();
			bool notWrongYet = true;	// as long as we're matching char-by-char, this stays true
			while (profileName && !profile)
			{
				for (int j = 0; j < nameLength && !profile && notWrongYet; j++)
				{
					if (notWrongYet && profileName + j)
					{
						if (username[j] != profileName[j])
						{
							notWrongYet = false;
						}
					}
					if (notWrongYet && *(profileName + j + 1) == '\0')
					{
						// we found a good enough match
						profile = m_profiles.at(i);
						break;
					}
				}
				profileName++;
			}
		}

		if (profile)
		{
			int ret = m_ftpSession->StartSession(profile);
			if (ret == -1) {
				OutErr("[NppFTP] Cannot start FTP session");
			}
			m_ftpSession->Connect();
		}
	}
}
