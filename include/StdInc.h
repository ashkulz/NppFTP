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

//System headers
#define _WIN32_IE 0x0501
#define _WIN32_WINNT 0x0501
#include <winsock2.h>
#include <windows.h>
#include <shlwapi.h>
#include <tchar.h>
#include <uxtheme.h>
#include <string>
#include <vector>
#include <deque>

//Library headers
#include "tinyxml.h"

//Common project headers
#include "Output.h"
#include "StringUtils.h"
#include "PathUtils.h"
#include "WinPlatform.h"
#include "RefObject.h"

#include "FileObject.h"

//Missing functions
