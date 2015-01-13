#ifndef WINPLATFORM_H
#define WINPLATFORM_H

class PF {
private:
	typedef HRESULT (WINAPI * ETDTProc) (HWND, DWORD);
	typedef HRESULT (WINAPI * SWTProc) (HWND, LPCWSTR, LPCWSTR);
public:
	static bool				Init();
	static bool				Deinit();
	static HRESULT			EnableThemeDialogTexture(HWND, DWORD);
	static HRESULT			SetWindowTheme(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList);
private:
	static ETDTProc			m_etdt;
	static SWTProc			m_swt;
	static HMODULE			m_hUxtheme;
};

#endif //WINPLATFORM_H
