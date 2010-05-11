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
#include <commctrl.h>

HWND _MainOutputWindow = NULL;
char * _HostsFile = NULL;
TCHAR * _ConfigPath = NULL;

NppFTP::NppFTP() :
	m_ftpSession(NULL),
	m_ftpWindow(NULL),
	m_outputShown(false),
	m_activeSession(false),
	m_configStore(NULL)
{
	PathMap globalPathmap;
	globalPathmap.localpath = SU::DupString(TEXT("%CONFIGDIR%\\Cache\\%USERNAME%@%HOSTNAME%"));
	globalPathmap.externalpath = SU::strdup("/");
	m_globalCache.Clear();
	m_globalCache.AddPathMap(globalPathmap);
}

NppFTP::~NppFTP() {
	for(size_t i = 0; i < m_profiles.size(); i++) {
		m_profiles[i]->Release();
		//delete m_profiles[i];
	}

	SSLCertificates::FreeX509Vector(m_certificates);

	if (m_configStore)
		delete [] m_configStore;
}

int NppFTP::Start(NppData nppData, TCHAR * nppConfigStore, int id, FuncItem funcItem) {
	m_nppData = nppData;

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
	SU::FreeUtf8(utf8Store);

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

	res = m_ftpSession->Init(m_ftpWindow, &m_globalCache);
	if (res == -1) {
		m_ftpWindow->Destroy();
		return -1;
	}

	res = m_ftpWindow->SetSession(m_ftpSession);
	if (res == -1) {
		m_ftpSession->Deinit();
		m_ftpWindow->Destroy();
		return -1;
	}

	res = m_ftpWindow->SetProfilesVector(&m_profiles);
	res = m_ftpWindow->SetGlobalCache(&m_globalCache);
	m_ftpWindow->m_outputShown = m_outputShown;

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

	return 0;
}

int NppFTP::ShowFTPWindow() {
	bool show = !(m_ftpWindow->IsVisible());
	m_ftpWindow->Show(show);

	return 0;
}

int NppFTP::ShowAboutDialog() {
	AboutDialog ab;
	ab.Create(m_nppData._nppHandle);
	return 0;
}

int NppFTP::OnSave(const TCHAR* path) {
	if (!path)
		return -1;

	if (m_ftpSession->IsConnected()) {
		m_ftpSession->UploadFileCache(path);
	}

	return 0;
}

int NppFTP::InitAll(HINSTANCE hInst) {
	Window::SetDefaultInstance(hInst);
	FTPWindow::RegisterClass();
	OutputWindow::RegisterClass();

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
	SU::FreeUtf8(utf8Store);

	TiXmlDocument certificatesDoc = TiXmlDocument(xmlPath);
	certificatesDoc.LoadFile();

	TiXmlElement* ftpElem = settingsDoc.FirstChildElement("NppFTP");
	if (!ftpElem) {
		result = 1;
		return result;
	}

	int outState = 0;
	const char * outstr = ftpElem->Attribute("outputShown", &outState);
	if (!outstr) {
		outState = 0;
	}
	m_outputShown = (outState != 0);

	const char * defaultCacheutf8 = ftpElem->Attribute("defaultCache");
	TCHAR * defaultCache;
	if (defaultCacheutf8) {
		defaultCache = SU::Utf8ToTChar(defaultCacheutf8);
	} else {
		TCHAR * defaultCacheTmp = new TCHAR[MAX_PATH];
		lstrcpy(defaultCacheTmp, m_configStore);
		::PathAppend(defaultCacheTmp, TEXT("Cache"));
		defaultCache = SU::DupString(defaultCacheTmp);
		delete [] defaultCacheTmp;
	}

	PathMap globalPathmap;
	globalPathmap.localpath = SU::DupString(defaultCache);
	globalPathmap.externalpath = SU::strdup("/");
	m_globalCache.Clear();
	m_globalCache.AddPathMap(globalPathmap);

	SU::FreeTChar(defaultCache);

	TiXmlElement * profilesElem = ftpElem->FirstChildElement(FTPProfile::ProfilesElement);
	if (!profilesElem) {
		m_profiles.clear();
		result = 1;
	} else {
		m_profiles = FTPProfile::LoadProfiles(profilesElem);
		for(size_t i = 0; i < m_profiles.size(); i++) {
			m_profiles.at(i)->AddRef();
			m_profiles.at(i)->SetCacheParent(&m_globalCache);
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
	SU::FreeUtf8(utf8Store);

	TiXmlDocument * certificatesDoc = new TiXmlDocument(xmlPath);
	decl = new TiXmlDeclaration("1.0", "UTF-8", "");
	certificatesDoc->LinkEndChild(decl);

	TiXmlElement * ftpElem = new TiXmlElement("NppFTP");
	settingsDoc->LinkEndChild(ftpElem);

	if (m_globalCache.GetPathMapCount() > 0) {
		const PathMap & map = m_globalCache.GetPathMap(0);
		char * defaultCacheutf8 = SU::TCharToUtf8(map.localpath);
		ftpElem->SetAttribute("defaultCache", defaultCacheutf8);
		SU::FreeUtf8(defaultCacheutf8);
	}

	bool shown = (m_ftpWindow != NULL)?m_ftpWindow->m_outputShown:false;
	ftpElem->SetAttribute("outputShown", shown?1:0);

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
