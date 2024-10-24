#ifndef KBINTDIALOG_H
#define KBINTDIALOG_H

#include "Dialog.h"
#include <libssh/libssh.h>

class KBIntDialog : public Dialog {
public:
							KBIntDialog();
	virtual					~KBIntDialog();

							//Return 1 if answers set, 2 if not
	virtual int				Create(HWND hParent, ssh_session session);	//if modal, returns 99 on close
protected:
	using Dialog::Create; //avoid compiler warning about hidden method

	virtual INT_PTR			DlgMsgProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual INT_PTR			OnInitDialog();	//DialogProc filters this one out, therefore calback

	virtual INT_PTR			OnCommand(int ctrlId, int notifCode, HWND idHwnd);
	virtual INT_PTR			OnNotify(NMHDR * pnmh);

	virtual int				OnAccept();
	virtual int				OnCancel();

	ssh_session				m_session;
	int						m_nrPrompt;
};

#endif //KBINTDIALOG_H
