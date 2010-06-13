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
#include "DragDropSupport.h"

//----------------------------------------DropTargetWindow-------------------------------------------------
DropTargetWindow::DropTargetWindow() :
	m_dropHwnd(NULL),
	m_dropTarget(this)
{
}

DropTargetWindow::~DropTargetWindow() {
}

bool DropTargetWindow::AcceptType(LPDATAOBJECT /*pDataObj*/) {
	return false;
}

HRESULT DropTargetWindow::OnDragEnter(LPDATAOBJECT /*pDataObj*/, DWORD /*grfKeyState*/, POINTL /*pt*/, LPDWORD /*pdwEffect*/) {
	return E_NOTIMPL;
}

HRESULT DropTargetWindow::OnDragOver(DWORD /*grfKeyState*/, POINTL /*pt*/, LPDWORD /*pdwEffect*/) {
	return E_NOTIMPL;
}

HRESULT DropTargetWindow::OnDragLeave() {
	return E_NOTIMPL;
}

HRESULT DropTargetWindow::OnDrop(LPDATAOBJECT /*pDataObj*/, DWORD /*grfKeyState*/, POINTL /*pt*/, LPDWORD /*pdwEffect*/) {
	return E_NOTIMPL;
}

int DropTargetWindow::DoRegisterDragDrop(HWND hwnd) {
	if (hwnd == NULL)
		hwnd = m_dropHwnd;
	::RegisterDragDrop(hwnd, &m_dropTarget);
	return 0;
}

int DropTargetWindow::DoRevokeDragDrop(HWND hwnd) {
	if (hwnd == NULL)
		hwnd = m_dropHwnd;
	::RevokeDragDrop(hwnd);
	return 0;
}

//----------------------------------------DropTarget-------------------------------------------------
CDropTarget::CDropTarget(DropTargetWindow * targetWindow) :
	m_refs(1),
	m_dragging(false),
	m_targetWindow(targetWindow)
{
}

CDropTarget::~CDropTarget() {
}

STDMETHODIMP CDropTarget::QueryInterface(REFIID iid, void ** ppv) {
	if(iid == IID_IUnknown || iid == IID_IDropTarget) {
		*ppv = this;
		AddRef();
		return NOERROR;
	}
	*ppv = NULL;
	return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG) CDropTarget::AddRef(void) {
	return ++m_refs;
}

STDMETHODIMP_(ULONG) CDropTarget::Release(void) {
	if(--m_refs == 0) {
		delete this;
		return 0;
	}
	return m_refs;
}

STDMETHODIMP CDropTarget::DragEnter(LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect) {
	m_dragging = m_targetWindow->AcceptType(pDataObj);

	// Does the drag source provide a suitable type? Callback if so
	if (m_dragging) {
		*pdwEffect= DROPEFFECT_NONE;	//security
		m_targetWindow->OnDragEnter(pDataObj, grfKeyState, pt, pdwEffect);
		m_targetWindow->OnDragOver(grfKeyState, pt, pdwEffect);
	} else {
		*pdwEffect= DROPEFFECT_NONE;
	}
	return NOERROR;
}

STDMETHODIMP CDropTarget::DragOver(DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect) {
	//if accepted format do callback
	if (m_dragging) {
		*pdwEffect= DROPEFFECT_NONE;
		m_targetWindow->OnDragOver(grfKeyState, pt, pdwEffect);
	} else {
		*pdwEffect= DROPEFFECT_NONE;
	}
	return NOERROR;
}

STDMETHODIMP CDropTarget::DragLeave() {
	//Cancel drag and drop operation
	if (m_dragging) {
		m_targetWindow->OnDragLeave();
	}
	m_dragging = false;

	return NOERROR;
}

STDMETHODIMP CDropTarget::Drop(LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect) {
	if (m_dragging) {	  //check if valid drop
		*pdwEffect = DROPEFFECT_NONE;
		m_targetWindow->OnDrop(pDataObj, grfKeyState, pt, pdwEffect);
	} else {
		*pdwEffect = DROPEFFECT_NONE;
	}
	return NOERROR;
}

//----------------------------------------DropSource-------------------------------------------------
CDropSource::CDropSource() :
	m_refs(1)
{
}

CDropSource::~CDropSource() {
}

STDMETHODIMP CDropSource::QueryInterface(REFIID iid, void ** ppv) {
	if(iid == IID_IUnknown || iid == IID_IDropSource)
	{
	  *ppv = this;
	  ++m_refs;
	  return NOERROR;
	}
	*ppv = NULL;
	return ResultFromScode(E_NOINTERFACE);
}


STDMETHODIMP_(ULONG) CDropSource::AddRef(void) {
	return ++m_refs;
}


STDMETHODIMP_(ULONG) CDropSource::Release(void) {
	if(--m_refs == 0)
	{
	  delete this;
	  return 0;
	}
	return m_refs;
}

STDMETHODIMP CDropSource::QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState) {
	if (fEscapePressed)
		return ResultFromScode(DRAGDROP_S_CANCEL);
	else if (!(grfKeyState & MK_LBUTTON))
		return ResultFromScode(DRAGDROP_S_DROP);
	else
		return NOERROR;
}

STDMETHODIMP CDropSource::GiveFeedback(DWORD /*dwEffect*/) {
	return ResultFromScode(DRAGDROP_S_USEDEFAULTCURSORS);
}

//----------------------------------------DataObject--------------------------------------------------
CDataObject::CDataObject(DropDataWindow * dataWindow) {
	m_refs = 1;
	m_filedescriptorID = RegisterClipboardFormat(CFSTR_FILEDESCRIPTOR);
	m_filecontentsID = RegisterClipboardFormat(CFSTR_FILECONTENTS);

	m_dataWindow = dataWindow;
}

CDataObject::~CDataObject() {
}

STDMETHODIMP CDataObject::QueryInterface(REFIID iid, void ** ppv) {
	if(iid == IID_IUnknown || iid == IID_IDataObject) {
	  *ppv = this;
	  AddRef();
	  return NOERROR;
	}
	*ppv = NULL;
	return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG) CDataObject::AddRef(void) {
	return ++m_refs;
}

STDMETHODIMP_(ULONG) CDataObject::Release(void) {
	if(--m_refs == 0) {
	  delete this;
	  return 0;
	}
	return m_refs;
}

STDMETHODIMP CDataObject::GetData(LPFORMATETC pformatetc, LPSTGMEDIUM pmedium) {
	pmedium->tymed = NULL;
	pmedium->pUnkForRelease = NULL;
	pmedium->hGlobal = NULL;

	//pformatetc->lindex == -1
	//pformatetc->ptd == NULL
	//pformatetc->dwAspect == DVASPECT_CONTENT

	if (pformatetc->cfFormat == m_filedescriptorID && pformatetc->tymed & TYMED_HGLOBAL) {


		int files = m_dataWindow->GetNrFiles();
		if (files <= 0)
			return ResultFromScode(E_FAIL);	//TODO: not really nice error code

		HGLOBAL globalFGD = GlobalAlloc(GMEM_MOVEABLE, sizeof(FILEGROUPDESCRIPTOR) + (files-1)*sizeof(FILEDESCRIPTOR));
		if (globalFGD == NULL)
			return ResultFromScode(E_OUTOFMEMORY);

		FILEGROUPDESCRIPTOR * fgd = (FILEGROUPDESCRIPTOR*)GlobalLock(globalFGD);
		ZeroMemory(fgd, sizeof(FILEGROUPDESCRIPTOR) + (files-1)*sizeof(FILEDESCRIPTOR));

		fgd->cItems = files;
		for(int i = 0; i < files; i++) {
			int fdres = m_dataWindow->GetFileDescriptor(&(fgd->fgd[i]), i);
			if (fdres == -1) {
				GlobalUnlock(globalFGD);
				GlobalFree(globalFGD);
				return ResultFromScode(E_FAIL);	//TODO: not really nice error code
			}
		}

		GlobalUnlock(globalFGD);
		pmedium->hGlobal = globalFGD;
		pmedium->tymed = TYMED_HGLOBAL;
		return ResultFromScode(S_OK);
	} else if (pformatetc->cfFormat == m_filecontentsID && pformatetc->tymed & TYMED_ISTREAM) {
		int index = (int)pformatetc->lindex;
		if (index >= m_dataWindow->GetNrFiles())
			return ResultFromScode(E_INVALIDARG);

		FILEDESCRIPTOR fd;
		int fdres = m_dataWindow->GetFileDescriptor(&fd, index);
		if (fdres == -1)
			return ResultFromScode(E_FAIL);	//TODO: not really nice error code

		CStreamData * stream = new CStreamData(&fd);
		if (!stream)
			return ResultFromScode(E_OUTOFMEMORY);

		pmedium->pstm = stream;
		pmedium->tymed = TYMED_ISTREAM;

		m_dataWindow->StreamData(stream, index);

		return ResultFromScode(S_OK);
	} else {
		return ResultFromScode(DATA_E_FORMATETC);
	}

	return ResultFromScode(S_OK);
}

STDMETHODIMP CDataObject::GetDataHere(LPFORMATETC /*pformatetc*/, LPSTGMEDIUM /*pmedium*/) {
	return ResultFromScode(DATA_E_FORMATETC);
}

STDMETHODIMP CDataObject::QueryGetData(LPFORMATETC pformatetc) {
	// This method is called by the drop target to check whether the source
	// provides data is a format that the target accepts.
	if (pformatetc->cfFormat == m_filedescriptorID && pformatetc->tymed & TYMED_HGLOBAL)
		return ResultFromScode(S_OK);
	if (pformatetc->cfFormat == m_filecontentsID && pformatetc->tymed == TYMED_ISTREAM)
		return ResultFromScode(S_OK);

	return ResultFromScode(S_FALSE);
}

STDMETHODIMP CDataObject::GetCanonicalFormatEtc(LPFORMATETC /*pformatetc*/, LPFORMATETC pformatetcOut) {
	pformatetcOut->ptd = NULL;
	return ResultFromScode(E_NOTIMPL);
}

STDMETHODIMP CDataObject::SetData(LPFORMATETC /*pformatetc*/, STGMEDIUM * /*pmedium*/, BOOL /*fRelease*/) {
	return ResultFromScode(E_NOTIMPL);
}

STDMETHODIMP CDataObject::EnumFormatEtc(DWORD dwDirection, LPENUMFORMATETC * ppenumFormatEtc) {
	SCODE sc = S_OK;

	FORMATETC formats[2];
	formats[0].cfFormat = m_filedescriptorID;
	formats[0].ptd = NULL;
	formats[0].dwAspect = DVASPECT_CONTENT;
	formats[0].lindex = -1;
	formats[0].tymed = TYMED_HGLOBAL;

	formats[1].cfFormat = m_filecontentsID;
	formats[1].ptd = NULL;
	formats[1].dwAspect = DVASPECT_CONTENT;
	formats[1].lindex = -1;
	formats[1].tymed = TYMED_ISTREAM;


	if (dwDirection == DATADIR_GET){
		*ppenumFormatEtc = (IEnumFORMATETC*) new CEnumFormatEtc(formats, 2);
		if (*ppenumFormatEtc == NULL)
			sc = E_OUTOFMEMORY;
	} else if (dwDirection == DATADIR_SET) {
		sc = E_NOTIMPL;
	} else {
		sc = E_INVALIDARG;
	}

	return ResultFromScode(sc);
}

STDMETHODIMP CDataObject::DAdvise(FORMATETC * /*pFormatetc*/, DWORD /*advf*/, LPADVISESINK /*pAdvSink*/, DWORD * /*pdwConnection*/) {
	return ResultFromScode(OLE_E_ADVISENOTSUPPORTED);
}

STDMETHODIMP CDataObject::DUnadvise(DWORD /*dwConnection*/) {
	return ResultFromScode(OLE_E_ADVISENOTSUPPORTED);
}

STDMETHODIMP CDataObject::EnumDAdvise(LPENUMSTATDATA FAR* /*ppenumAdvise*/) {
	return ResultFromScode(OLE_E_ADVISENOTSUPPORTED);
}

//--------------------


DropDataWindow::DropDataWindow() {
}

DropDataWindow::~DropDataWindow() {
}

int DropDataWindow::GetNrFiles() {
	return 0;
}

int DropDataWindow::GetFileDescriptor(FILEDESCRIPTOR * /*fd*/, int /*index*/) {
	return -1;
}

int DropDataWindow::StreamData(CStreamData * /*stream*/, int /*index*/) {
	return -1;
}

int DropDataWindow::OnEndDnD() {
	return -1;
}

//---------------------------------------EnumFORMATETC------------------------------------------------
CEnumFormatEtc::CEnumFormatEtc(FORMATETC * pFormatEtc, int nNumFormats) {
	m_refs			= 1;
	m_nIndex		= 0;
	m_nNumFormats	= nNumFormats;
	m_pFormatEtc	= new FORMATETC[nNumFormats];

	// copy the FORMATETC structures
	for(int i = 0; i < nNumFormats; i++) {
		m_pFormatEtc[i] = pFormatEtc[i];
		m_pFormatEtc[i].ptd = NULL;
	}
}

CEnumFormatEtc::~CEnumFormatEtc() {
	if(m_pFormatEtc) {
		delete[] m_pFormatEtc;
	}
}

STDMETHODIMP CEnumFormatEtc::QueryInterface(REFIID iid, void ** ppv) {
	if(iid == IID_IUnknown || iid == IID_IEnumFORMATETC) {
	  *ppv = this;
	  AddRef();
	  return NOERROR;
	}
	*ppv = NULL;
	return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG) CEnumFormatEtc::AddRef(void) {
	return ++m_refs;
}

STDMETHODIMP_(ULONG) CEnumFormatEtc::Release(void) {
	if(--m_refs == 0) {
	  delete this;
	  return 0;
	}
	return m_refs;
}


STDMETHODIMP CEnumFormatEtc::Next(ULONG celt, FORMATETC * pFormatEtc, ULONG * pceltFetched)
{
	ULONG copied  = 0;

	// validate arguments
	if(celt == 0 || pFormatEtc == 0)
		return E_INVALIDARG;

	// copy FORMATETC structures into caller's buffer
	while(m_nIndex < m_nNumFormats && copied < celt)
	{
		pFormatEtc[copied] = m_pFormatEtc[m_nIndex];
		pFormatEtc[copied].ptd = NULL;
		copied++;
		m_nIndex++;
	}

	// store result
	if(pceltFetched != 0)
		*pceltFetched = copied;

	// did we copy all that was requested?
	return (copied == celt) ? S_OK : S_FALSE;
}

STDMETHODIMP CEnumFormatEtc::Skip(ULONG celt)
{
	m_nIndex += celt;
	return (m_nIndex <= m_nNumFormats) ? S_OK : S_FALSE;
}

STDMETHODIMP CEnumFormatEtc::Reset(void)
{
	m_nIndex = 0;
	return S_OK;
}

STDMETHODIMP CEnumFormatEtc::Clone(IEnumFORMATETC ** ppEnumFormatEtc) {

	*ppEnumFormatEtc = (IEnumFORMATETC*) new CEnumFormatEtc(m_pFormatEtc, m_nNumFormats);

	if(*ppEnumFormatEtc != NULL) {
		// manually set the index state
		((CEnumFormatEtc *) *ppEnumFormatEtc)->m_nIndex = m_nIndex;
		return S_OK;
	} else {
		return E_OUTOFMEMORY;
	}
}
//----------------------------------------Stream-------------------------------------------------
CStreamData::CStreamData(FILEDESCRIPTOR * fd) {
    m_refs = 1;
    m_currentPointer.QuadPart = 0;
	m_closedStream = false;
	m_readHandle = NULL;
	m_writeHandle = NULL;

	m_filedesc = *fd;

	if (!CreatePipe(&m_readHandle, &m_writeHandle, NULL, 2048)) {
		OutErr("[DnD] Unable to create pipe for stream\n");
		m_closedStream = true;
	}
}

CStreamData::~CStreamData() {
	if (!m_closedStream) {
		Close();
	}
}

STDMETHODIMP CStreamData::QueryInterface(REFIID iid, void ** ppv) {
	if(iid == IID_IUnknown || iid == IID_IStream || iid == IID_ISequentialStream) {
		*ppv = this;
		AddRef();
		return NOERROR;
	}
	*ppv = NULL;
	return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG) CStreamData::AddRef(void) {
	return ++m_refs;
}

STDMETHODIMP_(ULONG) CStreamData::Release(void) {
	if(--m_refs == 0) {
		delete this;
		return 0;
	}
	return m_refs;
}

STDMETHODIMP CStreamData::Read(void * pv, ULONG cb, ULONG * pcbRead) {
	if (!pv) {
		return STG_E_INVALIDPOINTER;
	}

	if (!cb || m_closedStream) {
		*pcbRead = 0;
		return S_OK;
	}

	DWORD bytesRead = 0;
	DWORD totalBytesRead = 0;
	BOOL res = TRUE;
	char * dataBuffer = (char*)pv;
	while(res == TRUE && cb > 0) {
		res = ReadFile(m_readHandle, dataBuffer, cb, &bytesRead, NULL);
		cb -= bytesRead;
		dataBuffer += bytesRead;
		totalBytesRead += bytesRead;
		m_currentPointer.QuadPart += bytesRead;
	}

	if (res) {
		if (pcbRead)
			*pcbRead = totalBytesRead;
	} else {
		if (pcbRead)
			*pcbRead = 0;
		DWORD err = GetLastError();
		if (err == ERROR_BROKEN_PIPE) {
			Close();
			return S_OK;
		} else {
			OutErr("[DnD] Stream unexpected error: %d", err);
			Close();
			return S_FALSE;
		}
	}

	return S_OK;
}

STDMETHODIMP CStreamData::Write(void const * /*pv*/, ULONG /*cb*/, ULONG * /*pcbWritten*/) {
    return E_NOTIMPL;
}

STDMETHODIMP CStreamData::SetSize(ULARGE_INTEGER /*newsize*/) {
    return E_NOTIMPL;
}

STDMETHODIMP CStreamData::CopyTo(IStream *, ULARGE_INTEGER, ULARGE_INTEGER *, ULARGE_INTEGER *) {
    return E_NOTIMPL;
}

STDMETHODIMP CStreamData::Commit(DWORD) {
    return E_NOTIMPL;
}

STDMETHODIMP CStreamData::Revert(void) {
    return E_NOTIMPL;
}

STDMETHODIMP CStreamData::LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) {
    return E_NOTIMPL;
}

STDMETHODIMP CStreamData::UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) {
    return E_NOTIMPL;
}

STDMETHODIMP CStreamData::Clone(IStream **) {
    return E_NOTIMPL;
}

STDMETHODIMP CStreamData::Seek(LARGE_INTEGER liDistanceToMove, DWORD dwOrigin, ULARGE_INTEGER * lpNewFilePointer) {
	switch(dwOrigin) {
		case STREAM_SEEK_SET:
			return E_NOTIMPL;
			break;
		case STREAM_SEEK_CUR:
			if (liDistanceToMove.QuadPart != 0)
				return E_NOTIMPL;
			break;
		case STREAM_SEEK_END:
			return E_NOTIMPL;
			break;
		default:
			return E_INVALIDARG;
			break;
	}

	if (lpNewFilePointer)
		*lpNewFilePointer = m_currentPointer;

	return S_OK;
}

STDMETHODIMP CStreamData::Stat(STATSTG * pStatstg, DWORD grfStatFlag) {
	//Doesnt seem to get called by explorer
	if (!pStatstg)
		return STG_E_INVALIDPOINTER;

	if (!(grfStatFlag & STATFLAG_NONAME))  {	//allocate memory for name
#ifdef UNICODE
		int len = lstrlen(m_filedesc.cFileName)+1;
		pStatstg->pwcsName = (LPOLESTR)CoTaskMemAlloc(len*sizeof(WCHAR));
		lstrcpyn(pStatstg->pwcsName, m_filedesc.cFileName, len);
#else
		WCHAR * unistring = SU::CharToWChar(m_filedesc.cFileName);
		int len = lstrlenW(unistring)+1;
		pStatstg->pwcsName = (LPOLESTR)CoTaskMemAlloc(len*sizeof(WCHAR));
		lstrcpynW(pStatstg->pwcsName, unistring, len);
		SU::FreeWChar(unistring);
#endif
	}

	pStatstg->type = STGTY_STREAM;
	pStatstg->cbSize.LowPart = m_filedesc.nFileSizeLow;
	pStatstg->cbSize.HighPart = m_filedesc.nFileSizeHigh;
	pStatstg->mtime = m_filedesc.ftLastWriteTime;
	pStatstg->ctime = m_filedesc.ftCreationTime;
	pStatstg->atime = m_filedesc.ftLastAccessTime;
	pStatstg->grfMode = STGM_READ;
	pStatstg->grfLocksSupported = 0;
	//pStatstg->clsid = ;
	//pStatstg->grfStateBits = ;
	return S_OK;
}

HANDLE CStreamData::GetWriteHandle() const {
	return m_writeHandle;
}

void CStreamData::Close() {
	m_closedStream = true;
	::CloseHandle(m_readHandle);
	::CloseHandle(m_writeHandle);
}

//----------------------------------------DropHelper-------------------------------------------------


DropHelper::DropHelper(DropDataWindow * dataWindow) :
	m_hThread(NULL),
	m_dataWindow(dataWindow)
{
}

DropHelper::~DropHelper() {
}

int DropHelper::DropHelper::PerformDragDrop() {
	if (m_hThread != NULL)
		return -1;

	m_hThread = ::CreateThread(NULL, 0, &DropHelper::StaticDragDropThread, (LPVOID)this, 0, NULL);
	if (m_hThread == NULL)
		return -1;

	return 0;
}

DWORD WINAPI DropHelper::StaticDragDropThread(LPVOID param) {
	DropHelper * pDH = (DropHelper*)param;
	return pDH->DragDropThread();
}

int DropHelper::DragDropThread() {
	int result = 0;

	HRESULT res = OleInitialize(NULL);
	if (res != S_OK && res != S_FALSE) {
		OutErr("[DnD] Error initializing OLE (Thread): %d\n", res);
		return -1;
	}

	CDropSource * src = new CDropSource();
	if (!src) {
		return -1;
	}
	CDataObject * dat = new CDataObject(m_dataWindow);
	if (!dat) {
		src->Release();
		return -1;
	}

	DWORD resEffect = 0;
	OutMsg("[DnD] Begin DoDragDrop");
	res = DoDragDrop(dat, src, DROPEFFECT_COPY, &resEffect);
	OutMsg("[DnD] End DoDragDrop");

	src->Release();
	dat->Release();
	if (res == S_OK || res == DRAGDROP_S_DROP)
		result = TRUE;
	else
		result = FALSE;

	m_dataWindow->OnEndDnD();

	::OleUninitialize();
	m_hThread = NULL;

	return result;
}
