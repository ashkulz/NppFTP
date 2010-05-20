#include "StdInc.h"
//#include "WinPlatform.h"

PF::ETDTProc PF::m_etdt = NULL;
PF::SWTProc PF::m_swt = NULL;
HMODULE PF::m_hUxtheme = NULL;

bool PF::Init() {
	//Just load the DLL, no platform version checks. If it works, it works
	m_hUxtheme = ::LoadLibrary(TEXT("UxTheme.dll"));
	if (m_hUxtheme) {
		m_etdt = (ETDTProc)::GetProcAddress(m_hUxtheme, "EnableThemeDialogTexture");
		m_swt = (SWTProc)::GetProcAddress(m_hUxtheme, "SetWindowTheme");
	}
	return true;
}

bool PF::Deinit() {
	m_etdt = NULL;
	m_swt = NULL;
	if (m_hUxtheme) {
		::FreeLibrary(m_hUxtheme);
		m_hUxtheme = NULL;
	}
	return true;
}

HRESULT PF::EnableThemeDialogTexture(HWND hwnd, DWORD uFlags) {
	if (m_etdt) {
		return m_etdt(hwnd, uFlags);
	} else {
		return E_NOTIMPL;
	}
}

HRESULT PF::SetWindowTheme(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList) {
	if (m_swt) {
		return m_swt(hwnd, pszSubAppName, pszSubIdList);
	} else {
		return E_NOTIMPL;
	}
}
