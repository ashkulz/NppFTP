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

#ifndef COMMANDS_H
#define COMMANDS_H

#include <windows.h>

//Custom window messages
#define	WM_DLGEND			WM_USER + 500
#define EN_RETURN			WM_USER + 501
#define WM_MOVESPLITTER		WM_USER + 503
#define WM_DND				WM_USER + 504
#define WM_OUTPUTSHOWN		WM_USER + 505

//output popup menu
#define IDM_POPUP_COPY				10001
#define IDM_POPUP_CLEAR				10002
#define IDM_POPUP_SELECTALL			10003
//file popup menu
#define IDM_POPUP_DOWNLOADFILE		10004
#define IDM_POPUP_DLDTOLOCATION		10005
#define IDM_POPUP_RENAMEFILE		10006
#define IDM_POPUP_DELETEFILE		10007
#define IDM_POPUP_PERMISSIONFILE	10008
#define IDM_POPUP_PROPSFILE			10009
//directory popup menu
#define IDM_POPUP_NEWDIR			10010
#define IDM_POPUP_NEWFILE			10011
#define IDM_POPUP_RENAMEDIR			10012
#define IDM_POPUP_DELETEDIR			10013
#define IDM_POPUP_UPLOADFILE		10014
#define IDM_POPUP_UPLOADOTHERFILE	10015
#define IDM_POPUP_REFRESHDIR		10016
#define IDM_POPUP_PERMISSIONDIR		10017
#define IDM_POPUP_PROPSDIR			10018
//link popup menu
#define IDM_POPUP_LINKTYPE			10019
//queue popup menus
#define IDM_POPUP_QUEUE_ABORT		10020
#define IDM_POPUP_QUEUE_CANCEL		10021
//settings popup menus
#define IDM_POPUP_SETTINGSGENERAL	10022
#define IDM_POPUP_SETTINGSPROFILE	10023
//profile in treeview menus
#define IDM_POPUP_PROFILE_CONNECT	10024
#define IDM_POPUP_PROFILE_EDIT		10025
#define IDM_POPUP_PROFILE_DELETE	10026
#define IDM_POPUP_PROFILE_CREATE		10027
#define IDM_POPUP_PROFILE_FOLDER_CREATE 10028
#define IDM_POPUP_CUT				10030
#define IDM_POPUP_PASTE				10031
//Range for profile items in popupmenu. Go over 1000 profiles and the menu will not work anymore
#define IDM_POPUP_PROFILE_FIRST		11000
#define IDM_POPUP_PROFILE_MAX		12000

//output window context menu
#define IDM_POPUP_OUTPUT_COPY		12001
#define IDM_POPUP_OUTPUT_CLEAR		12002

//Toolbar button IDs
#define IDB_BUTTON_TOOLBAR_CONNECT	501
#define IDB_BUTTON_TOOLBAR_UPLOAD	502
#define IDB_BUTTON_TOOLBAR_DOWNLOAD	503
#define IDB_BUTTON_TOOLBAR_ABORT	504
#define IDB_BUTTON_TOOLBAR_SETTINGS	505
#define IDB_BUTTON_TOOLBAR_MESSAGES	506
#define IDB_BUTTON_TOOLBAR_RAWCMD	507
#define IDB_BUTTON_TOOLBAR_REFRESH	508
#define IDB_BUTTON_TOOLBAR_OPENDIR	509

#endif //COMMANDS_H
