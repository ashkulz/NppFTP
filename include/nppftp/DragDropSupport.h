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

//Based on microsoft classes from DragDrop example files from FTP, altered for personal use
//These classes represent various DnD items in NppFTP, and the means to support them

//callback function definitions
typedef void (*enterCallback) (CLIPFORMAT, IDataObject *, void *);							//type, object, custom data
typedef void (*dragCallback) (DWORD, POINTL, DWORD *, void *);								//keystate, position, effect, custom data
typedef void (*dropCallback) (CLIPFORMAT, IDataObject *, DWORD, POINTL, DWORD *, void *);	//type, object, keystate, position, effect, custom data
typedef void (*cancelCallback) (void *);													//custom data

typedef HRESULT (*feedbackCall)(DWORD, void *);												//current dropstate, custom data

typedef void* (*renderData)(LPFORMATETC pformatetc, void *);								//format to render, custom data, return pointer to rendered data, if HGLOBAL its an address of a handle to GlobalAlloc data, or NULL if failure (MUST be GMEM_SHARE)

struct DropHandler {
	CLIPFORMAT type;
	void * customData;
	enterCallback enterCall;
	dragCallback dragCall;
	dropCallback dropCall;
	cancelCallback cancelCall;
};

class DropTargetWindow {	//does _not_ derive Window
public:
							DropTargetWindow();
	virtual					~DropTargetWindow();

	bool					AcceptType(CLIPFORMAT type);
	HRESULT					OnDragEnter(LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);
	HRESULT					OnDragOver(DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);
	HRESULT					OnDragLeave();
	HRESULT					OnDrop(LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);
private:
};

//Class used to allow items to be dragged INTO the window
class CDropTarget : public IDropTarget {
//Register a type and callbackfunction (static) with custom data to support different droppable stuff
public:
	CDropTarget();

	/* IUnknown methods */
	STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppvObj);
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);

	/* IDropTarget methods */
	STDMETHOD(DragEnter)(LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);
	STDMETHOD(DragOver)(DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);
	STDMETHOD(DragLeave)();
	STDMETHOD(Drop)(LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);

	/* Client interface methods */
	bool addType(CLIPFORMAT type, void * custom, enterCallback enter, dragCallback drag, dropCallback drop, cancelCallback cancel);

private:
	ULONG m_refs;
	BOOL m_bAcceptFmt;
	int m_currentAcceptedType;
	std::vector<DropHandler> m_supportedTypes;
	int size;
};

//Class used to allow files to be dragged OUT the window
class CDropSource : public IDropSource {
public:
	CDropSource();

	/* IUnknown methods */
	STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppvObj);
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);

	/* IDropSource methods */
	STDMETHOD(QueryContinueDrag)(BOOL fEscapePressed, DWORD grfKeyState);
	STDMETHOD(GiveFeedback)(DWORD dwEffect);

	void setCallback(feedbackCall, void *);
private:
	ULONG m_refs;
	feedbackCall callback;
	void * customData;
};

//Class used when dragging object OUT of window
class CDataObject : public IDataObject {
public:
	CDataObject();
	~CDataObject();

   /* IUnknown methods */
	STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppvObj);
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);

	/* IDataObject methods */
	STDMETHOD(GetData)(LPFORMATETC pformatetcIn,  LPSTGMEDIUM pmedium );
	STDMETHOD(GetDataHere)(LPFORMATETC pformatetc, LPSTGMEDIUM pmedium );
	STDMETHOD(QueryGetData)(LPFORMATETC pformatetc );
	STDMETHOD(GetCanonicalFormatEtc)(LPFORMATETC pformatetc, LPFORMATETC pformatetcOut);
	STDMETHOD(SetData)(LPFORMATETC pformatetc, STGMEDIUM FAR * pmedium, BOOL fRelease);
	STDMETHOD(EnumFormatEtc)(DWORD dwDirection, LPENUMFORMATETC FAR* ppenumFormatEtc);
	STDMETHOD(DAdvise)(FORMATETC FAR* pFormatetc, DWORD advf, LPADVISESINK pAdvSink, DWORD FAR* pdwConnection);
	STDMETHOD(DUnadvise)(DWORD dwConnection);
	STDMETHOD(EnumDAdvise)(LPENUMSTATDATA FAR* ppenumAdvise);

	void addType(FORMATETC format);		//TARGETDEVICE MUST be NULL;
	void setCallback(renderData, void *);
private:
	ULONG m_refs;
	CLIPFORMAT currentFormat;
	renderData callback;
	void * customData;
	std::vector<FORMATETC> formats;
	//return -1 if not found
	bool getFormatIndex(LPFORMATETC pformatetc, size_t * offset);
};

//Class used in conjunction with CDataObject to allow to enumerate all of its dataformats
class CEnumFormatEtc : public IEnumFORMATETC {
public:
	CEnumFormatEtc(std::vector<FORMATETC> * formats);
	CEnumFormatEtc(FORMATETC *pFormatEtc, int nNumFormats);
	~CEnumFormatEtc();

	/* IUnknown methods */
	STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppvObj);
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);

	/* IEnumFORMATETC methods */
	STDMETHOD(Next)(ULONG celt, FORMATETC * rgelt, ULONG * pceltFetched);
	STDMETHOD(Skip)(ULONG celt);
	STDMETHOD(Reset)(void);
	STDMETHOD(Clone)(IEnumFORMATETC ** ppEnumFormatEtc);
private:
	ULONG m_refs;
	ULONG m_nIndex;
	ULONG m_nNumFormats;
	FORMATETC * m_pFormatEtc;
};

//Class to stream network contents to explorer
class CStreamData : public IStream {
	//Supports only 4GB streamsize, no more
public:
	CStreamData();
	~CStreamData();

	/* IUnknown methods */
	STDMETHOD(QueryInterface)(REFIID iid, void FAR* FAR* ppv);
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);

	/* ISequentialStream methods */
	STDMETHOD(Read)(void* pv, ULONG cb, ULONG* pcbRead);
	STDMETHOD(Write)(void const* pv, ULONG cb, ULONG* pcbWritten);

	/* IStream methods */
	STDMETHOD(SetSize)(ULARGE_INTEGER);
	STDMETHOD(CopyTo)(IStream*, ULARGE_INTEGER, ULARGE_INTEGER*, ULARGE_INTEGER*);
	STDMETHOD(Commit)(DWORD);
	STDMETHOD(Revert)(void);
	STDMETHOD(LockRegion)(ULARGE_INTEGER, ULARGE_INTEGER, DWORD);
	STDMETHOD(UnlockRegion)(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) ;
	STDMETHOD(Clone)(IStream **);
	STDMETHOD(Seek)(LARGE_INTEGER liDistanceToMove, DWORD dwOrigin, ULARGE_INTEGER* lpNewFilePointer);
	STDMETHOD(Stat)(STATSTG* pStatstg, DWORD grfStatFlag);

	HANDLE getWriteHandle() {
		return writeHandle;
	}
	void close() {
		CloseHandle(writeHandle);
	}
private:
    ULONG m_refs;

	HANDLE readHandle;
	HANDLE writeHandle;
	bool closedStream;
};

#endif //DRAGDROPSUPPORT_H
