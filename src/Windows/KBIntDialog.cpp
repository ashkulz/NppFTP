#include "../StdInc.h"
#include "KBIntDialog.h"

#include "resource.h"

const int MAXPROMPT = 20;

KBIntDialog::KBIntDialog() :
	Dialog(IDD_DIALOG_KBINT),
	m_session(NULL),
	m_nrPrompt(0)
{
}

KBIntDialog::~KBIntDialog() {
	m_session = NULL;
}

int KBIntDialog::Create(HWND hParent, ssh_session session) {
	m_session = NULL;
	m_nrPrompt = 0;
	if (!session)
		return -1;

	m_session = session;
	m_nrPrompt = ssh_userauth_kbdint_getnprompts(m_session);
	if (m_nrPrompt == 0)	//nothing to answer, don't show dialog
		return 0;
	if (m_nrPrompt > MAXPROMPT)
		return -1;		//unable to show that many prompts

	return Dialog::Create(hParent, true, NULL);
}

INT_PTR KBIntDialog::DlgMsgProc(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (uMsg == WM_CLOSE) {
		OnCancel();
		EndDialog(m_hwnd, 2);
		return TRUE;
	}

	return Dialog::DlgMsgProc(uMsg, wParam, lParam);
}

inline void ScreenRectToClient(HWND hWnd, RECT * rect) {
	POINT pt;

	pt.x = rect->left;
	pt.y = rect->top;
	ScreenToClient(hWnd, &pt);
	rect->left = pt.x;
	rect->top = pt.y;

	pt.x = rect->right;
	pt.y = rect->bottom;
	ScreenToClient(hWnd, &pt);
	rect->right = pt.x;
	rect->bottom = pt.y;

}

INT_PTR KBIntDialog::OnInitDialog() {
	//TODO: fix the tab order
	const char * name = ssh_userauth_kbdint_getname(m_session);
	if (!name)
		name = "";
	::SetDlgItemTextA(m_hwnd, IDC_STATIC_NAMEFIELD, name);

	if (m_nrPrompt > 1) {	//for each prompt, clone prompt and input fields

		HWND hPrompt1 = ::GetDlgItem(m_hwnd, IDC_EDIT_PROMPT1);
		HWND hAnswer1 = ::GetDlgItem(m_hwnd, IDC_EDIT_ANSWER1);
		HWND hMarker = ::GetDlgItem(m_hwnd, IDC_STATIC_MARKER);
		RECT promptRect;
		RECT answerRect;
		RECT markerRect;
		::GetWindowRect(hPrompt1, &promptRect);
		ScreenRectToClient(m_hwnd, &promptRect);
		::GetWindowRect(hAnswer1, &answerRect);
		ScreenRectToClient(m_hwnd, &answerRect);
		::GetWindowRect(hMarker, &markerRect);
		ScreenRectToClient(m_hwnd, &markerRect);

		int deltaY = markerRect.top - promptRect.top;
		//int deltaX = 0;	//unused


		int curPromptID = IDC_EDIT_PROMPT1;
		int curAnswerID = IDC_EDIT_ANSWER1;

		for(int i = 1; i < m_nrPrompt; i++) {
			curPromptID = curPromptID+2;
			curAnswerID = curAnswerID+2;

			promptRect.top += deltaY;
			promptRect.bottom += deltaY;
			answerRect.top += deltaY;
			answerRect.bottom += deltaY;

			::CreateWindowEx(0, WC_EDIT, TEXT(""),
							WS_CHILD | WS_VSCROLL | ES_AUTOHSCROLL | ES_MULTILINE | ES_READONLY | WS_VISIBLE,
							promptRect.left, promptRect.top, promptRect.right-promptRect.left, promptRect.bottom-promptRect.top,
							m_hwnd, (HMENU)curPromptID, m_hInstance, NULL);

			::CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, TEXT(""),
							WS_CHILD | WS_BORDER | ES_AUTOHSCROLL | WS_VISIBLE,
							answerRect.left, answerRect.top, answerRect.right-answerRect.left, answerRect.bottom-answerRect.top,
							m_hwnd, (HMENU)curPromptID, m_hInstance, NULL);
		}

		int totalDeltaY = deltaY * (m_nrPrompt-1);

		HWND hBtnOK = ::GetDlgItem(m_hwnd, IDOK);
		HWND hBtnCancel = ::GetDlgItem(m_hwnd, IDCANCEL);
		RECT btnRect;
		::GetWindowRect(hBtnOK, &btnRect);
		ScreenRectToClient(m_hwnd, &btnRect);
		::MoveWindow(hBtnOK, btnRect.left, btnRect.top+totalDeltaY, btnRect.right-btnRect.left, btnRect.bottom-btnRect.top, TRUE);

		::GetWindowRect(hBtnCancel, &btnRect);
		ScreenRectToClient(m_hwnd, &btnRect);
		::MoveWindow(hBtnCancel, btnRect.left, btnRect.top+totalDeltaY, btnRect.right-btnRect.left, btnRect.bottom-btnRect.top, TRUE);

		RECT winRect;
		::GetWindowRect(m_hwnd, &winRect);
		Resize(winRect.right-winRect.left, winRect.bottom-winRect.top+totalDeltaY);
	}

	for(int i = 0; i < m_nrPrompt; i++) {
		char echo = 0;
		const char * prompt = ssh_userauth_kbdint_getprompt(m_session, i, &echo);
		::SetDlgItemTextA(m_hwnd, IDC_EDIT_PROMPT1+(i*2), prompt);

		if (!echo) {
			HWND hCurAnswer = ::GetDlgItem(m_hwnd, IDC_EDIT_ANSWER1+(i*2));
			//LONG_PTR style = ::GetWindowLongPtr(hCurAnswer, GWL_STYLE);
			//::SetWindowLongPtr(hCurAnswer, GWL_STYLE, style|ES_PASSWORD);
			::SendMessage(hCurAnswer, EM_SETPASSWORDCHAR, (WPARAM)'*', 0);
		}

	}

	Dialog::OnInitDialog();
	::SetFocus(::GetDlgItem(m_hwnd, IDC_EDIT_ANSWER1));

	return FALSE;
}

INT_PTR KBIntDialog::OnCommand(int ctrlId, int notifCode, HWND idHwnd) {
	if (ctrlId == IDOK) {
		int ret = OnAccept();
		if (ret == 0)
			EndDialog(m_hwnd, 1);
		else
			EndDialog(m_hwnd, 2);
		return TRUE;
	} else if (ctrlId == IDCANCEL) {
		OnCancel();
		EndDialog(m_hwnd, 2);
		return TRUE;
	}

	return Dialog::OnCommand(ctrlId, notifCode, idHwnd);
}

INT_PTR KBIntDialog::OnNotify(NMHDR * pnmh) {
	return Dialog::OnNotify(pnmh);
}

int KBIntDialog::OnAccept() {
	int ctrlID = IDC_EDIT_ANSWER1;
	char buffer[MAX_PATH];
	bool failed = false;

	for(int i = 0; i < m_nrPrompt; i++) {
		HWND hEditAnswer = ::GetDlgItem(m_hwnd, ctrlID);
		if (!hEditAnswer) {
			ssh_userauth_kbdint_setanswer(m_session, i, "");
			failed = true;
			continue;
		}

		GetWindowTextA(hEditAnswer, buffer, MAX_PATH);
		ssh_userauth_kbdint_setanswer(m_session, i, buffer);
		ctrlID += 2;
	}

	if (failed)
		return -1;

	return 0;
}

int KBIntDialog::OnCancel() {
	for(int i = 0; i < m_nrPrompt; i++) {
		ssh_userauth_kbdint_setanswer(m_session, i, "");
	}
	return 0;
}
