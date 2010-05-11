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
#include "RefObject.h"

RefObject::RefObject() : m_refcounter(0)
{
}

RefObject::~RefObject() {
}

int RefObject::AddRef() {
	m_refcounter++;
	return m_refcounter;
}

int RefObject::Release() {
	m_refcounter--;
	if (!m_refcounter)
		delete this;
	return m_refcounter;
}
