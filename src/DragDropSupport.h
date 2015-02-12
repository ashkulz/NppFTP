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

#ifndef DRAGDROPSUPPORT_H
#define DRAGDROPSUPPORT_H

#include <shlobj.h>

//Based on microsoft classes from DragDrop example files from FTP, altered for personal use
//These classes represent various DnD items in NppFTP, and the means to support them

class DropTargetWindow;
class DropDataWindow;
class CStreamData;

//Class used to allow items to be dragged INTO the window
class CDropTarget : public IDropTarget {
//Register a type and callbackfunction (static) with custom data to support different droppable stuff
public:
							CDropTarget(DropTargetWindow * targetWindow);
	virtual					~CDropTarget();

	/* IUnknown methods */
	STDMETHOD				(QueryInterface)(REFIID riid, void FAR* FAR* ppvObj);
	STDMETHOD_				(ULONG, AddRef)(void);
	STDMETHOD_				(ULONG, Release)(void);

	/* IDropTarget methods */
	STDMETHOD				(DragEnter)(LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);
	STDMETHOD				(DragOver)(DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);
	STDMETHOD				(DragLeave)();
	STDMETHOD				(Drop)(LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);
private:
	ULONG					m_refs;
	bool					m_dragging;
	DropTargetWindow*		m_targetWindow;
};

class DropTargetWindow {	//does _not_ derive Window
public:
							DropTargetWindow();
	virtual					~DropTargetWindow();

	virtual bool			AcceptType(LPDATAOBJECT pDataObj);
	virtual HRESULT			OnDragEnter(LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);
	virtual HRESULT			OnDragOver(DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);
	virtual HRESULT			OnDragLeave();
	virtual HRESULT			OnDrop(LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);
protected:
	virtual int				DoRegisterDragDrop(HWND hwnd = NULL);
	virtual int				DoRevokeDragDrop(HWND hwnd = NULL);

	HWND					m_dropHwnd;
	CDropTarget				m_dropTarget;
};

//Class used to allow files to be dragged OUT the window
class CDropSource : public IDropSource {
public:
							CDropSource();
	virtual					~CDropSource();

	/* IUnknown methods */
	STDMETHOD				(QueryInterface)(REFIID riid, void FAR* FAR* ppvObj);
	STDMETHOD_				(ULONG, AddRef)(void);
	STDMETHOD_				(ULONG, Release)(void);

	/* IDropSource methods */
	STDMETHOD				(QueryContinueDrag)(BOOL fEscapePressed, DWORD grfKeyState);
	STDMETHOD				(GiveFeedback)(DWORD dwEffect);
private:
	ULONG m_refs;
};

//Class used when dragging object OUT of window
class CDataObject : public IDataObject {
public:
							CDataObject(DropDataWindow * dataWindow);
							virtual ~CDataObject();

   /* IUnknown methods */
	STDMETHOD				(QueryInterface)(REFIID riid, void FAR* FAR* ppvObj);
	STDMETHOD_				(ULONG, AddRef)(void);
	STDMETHOD_				(ULONG, Release)(void);

	/* IDataObject methods */
	STDMETHOD				(GetData)(LPFORMATETC pformatetcIn,  LPSTGMEDIUM pmedium);
	STDMETHOD				(GetDataHere)(LPFORMATETC pformatetc, LPSTGMEDIUM pmedium);
	STDMETHOD				(QueryGetData)(LPFORMATETC pformatetc );
	STDMETHOD				(GetCanonicalFormatEtc)(LPFORMATETC pformatetc, LPFORMATETC pformatetcOut);
	STDMETHOD				(SetData)(LPFORMATETC pformatetc, STGMEDIUM FAR * pmedium, BOOL fRelease);
	STDMETHOD				(EnumFormatEtc)(DWORD dwDirection, LPENUMFORMATETC FAR* ppenumFormatEtc);
	STDMETHOD				(DAdvise)(FORMATETC FAR* pFormatetc, DWORD advf, LPADVISESINK pAdvSink, DWORD FAR* pdwConnection);
	STDMETHOD				(DUnadvise)(DWORD dwConnection);
	STDMETHOD				(EnumDAdvise)(LPENUMSTATDATA FAR* ppenumAdvise);
private:
	ULONG					m_refs;
	CLIPFORMAT				m_filedescriptorID;
	CLIPFORMAT				m_filecontentsID;

	DropDataWindow*			m_dataWindow;
};

class DropDataWindow {
public:
							DropDataWindow();
	virtual					~DropDataWindow();

	virtual int				GetNrFiles();
	virtual int				GetFileDescriptor(FILEDESCRIPTOR * fd, int index);
	virtual int				StreamData(CStreamData * stream, int index);

	virtual int				OnEndDnD();
private:
};

//Class used in conjunction with CDataObject to allow to enumerate all of its dataformats
class CEnumFormatEtc : public IEnumFORMATETC {
public:
							CEnumFormatEtc(FORMATETC *pFormatEtc, int nNumFormats);
							virtual ~CEnumFormatEtc();

	/* IUnknown methods */
	STDMETHOD				(QueryInterface)(REFIID riid, void FAR* FAR* ppvObj);
	STDMETHOD_				(ULONG, AddRef)(void);
	STDMETHOD_				(ULONG, Release)(void);

	/* IEnumFORMATETC methods */
	STDMETHOD				(Next)(ULONG celt, FORMATETC * rgelt, ULONG * pceltFetched);
	STDMETHOD				(Skip)(ULONG celt);
	STDMETHOD				(Reset)(void);
	STDMETHOD				(Clone)(IEnumFORMATETC ** ppEnumFormatEtc);
private:
	ULONG					m_refs;
	ULONG					m_nIndex;
	ULONG					m_nNumFormats;
	FORMATETC*				m_pFormatEtc;
};

//Class to stream network contents to explorer
class CStreamData : public IStream {
	//Supports only 4GB streamsize, no more
public:
							CStreamData(FILEDESCRIPTOR * fd);
							virtual ~CStreamData();

	/* IUnknown methods */
	STDMETHOD				(QueryInterface)(REFIID iid, void FAR* FAR* ppv);
	STDMETHOD_				(ULONG, AddRef)(void);
	STDMETHOD_				(ULONG, Release)(void);

	/* ISequentialStream methods */
	STDMETHOD				(Read)(void* pv, ULONG cb, ULONG* pcbRead);
	STDMETHOD				(Write)(void const* pv, ULONG cb, ULONG* pcbWritten);

	/* IStream methods */
	STDMETHOD				(SetSize)(ULARGE_INTEGER);
	STDMETHOD				(CopyTo)(IStream*, ULARGE_INTEGER, ULARGE_INTEGER*, ULARGE_INTEGER*);
	STDMETHOD				(Commit)(DWORD);
	STDMETHOD				(Revert)(void);
	STDMETHOD				(LockRegion)(ULARGE_INTEGER, ULARGE_INTEGER, DWORD);
	STDMETHOD				(UnlockRegion)(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) ;
	STDMETHOD				(Clone)(IStream **);
	STDMETHOD				(Seek)(LARGE_INTEGER liDistanceToMove, DWORD dwOrigin, ULARGE_INTEGER* lpNewFilePointer);
	STDMETHOD				(Stat)(STATSTG* pStatstg, DWORD grfStatFlag);

	HANDLE					GetWriteHandle() const;
	void					Close();
private:
    ULONG					m_refs;

    ULARGE_INTEGER			m_currentPointer;

	HANDLE					m_readHandle;
	HANDLE					m_writeHandle;
	bool					m_closedStream;

	FILEDESCRIPTOR			m_filedesc;
};

class DropHelper {
public:
							DropHelper(DropDataWindow * dataWindow);
	virtual					~DropHelper();

	virtual int				PerformDragDrop();
private:
	static DWORD WINAPI		StaticDragDropThread(LPVOID param);
	virtual int				DragDropThread();

	HANDLE					m_hThread;
	DropDataWindow*			m_dataWindow;
};

#endif //DRAGDROPSUPPORT_H
