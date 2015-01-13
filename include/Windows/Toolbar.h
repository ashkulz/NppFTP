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

#ifndef TOOLBAR_H
#define TOOLBAR_H

#include "Window.h"
#include <commctrl.h>

#define REBAR_BAR_TOOLBAR		0
#define REBAR_BAR_EXTERNAL		10

//#if _MSC_VER > 1400 // MS Compiler > VS 2005
//#define REBARBAND_SIZE REBARBANDINFO_V3_SIZE
//#else
#define REBARBAND_SIZE sizeof(REBARBANDINFO)
//#endif

class Rebar;

class Toolbar : public Window {
public :
							Toolbar();
	virtual					~Toolbar();

	virtual int				Create(HWND hParent);
	virtual int				Destroy();

	virtual int				Enable(int cmdID, bool enabled) const;

	virtual int				GetWidth() const;
	virtual int				GetHeight() const;

	virtual bool			GetChecked(int cmdID) const;
	virtual int				SetChecked(int cmdID, BOOL check);

	virtual int				SetMenu(int cmdID, HMENU menu);

	virtual int				DoPopop(POINT chevPoint);	//show the popup if buttons are hidden
	virtual const TCHAR*	GetTooltip(int cmdID) const;
	virtual int				DoDropDown(int buttonID);

	virtual int				AddToRebar(Rebar * rebar);
private :
	size_t					m_nrButtons;
	TBBUTTON*				m_buttons;
	HMENU*					m_buttonMenus;
	int						m_connectBitmapIndex;
	int						m_disconnectBitmapIndex;

	Rebar *					m_rebar;
	REBARBANDINFO			m_rbBand;
};

class Rebar : public Window {
public :
							Rebar();
	virtual					~Rebar();

	virtual int				Create(HWND hParent);
	virtual int				Destroy();

	virtual int				AddBand(REBARBANDINFO * rBand, bool useID);	//useID true if ID from info should be used (false for plugins). wID in bandinfo will be set to used ID
	virtual int				Update(int id, REBARBANDINFO * rBand);		//wID from bandinfo is used for update
	virtual int				RemoveBand(int id);

	virtual int				SetIDVisible(int id, bool show);
	virtual int				GetIDVisible(int id) const;
private:
	virtual int				GetNewID();
	virtual int				ReleaseID(int id);
	virtual bool			IsIDTaken(int id);

	std::vector<int>		m_usedIDs;
};

#endif //TOOLBAR_H
