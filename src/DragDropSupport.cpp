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

//----------------------------------------DropTarget-------------------------------------------------
CDropTarget::CDropTarget() {
	m_refs = 1;
	m_bAcceptFmt = FALSE;
	m_currentAcceptedType = -1;
	m_supportedTypes.clear();
	size = 0;
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
	FORMATETC fmtetc;

	fmtetc.ptd	  = NULL;
	fmtetc.dwAspect = DVASPECT_CONTENT;
	fmtetc.lindex   = -1;
	fmtetc.tymed	= TYMED_HGLOBAL;

	m_bAcceptFmt = FALSE;

	for(int i = 0; i < size; i++) {
		fmtetc.cfFormat	= m_supportedTypes.at(i).type;
		if (NOERROR == pDataObj->QueryGetData(&fmtetc)) {	//found a match
			m_bAcceptFmt = true;
			m_currentAcceptedType = i;
			break;
		}
	}

	// Does the drag source provide a suitable type? Callback if so
	if (m_bAcceptFmt) {
		*pdwEffect= DROPEFFECT_NONE;	//security
		DropHandler & dh = m_supportedTypes.at(m_currentAcceptedType);
		dh.enterCall(dh.type, pDataObj, dh.customData);
		dh.dragCall(grfKeyState, pt, pdwEffect, dh.customData);
	} else {
		*pdwEffect= DROPEFFECT_NONE;
	}

	return NOERROR;
}

STDMETHODIMP CDropTarget::DragOver(DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect) {
	//if accepted format do callback
	if (m_bAcceptFmt) {
		*pdwEffect= DROPEFFECT_NONE;
		DropHandler & dh = m_supportedTypes.at(m_currentAcceptedType);
		dh.dragCall(grfKeyState, pt, pdwEffect, dh.customData);
	} else
		*pdwEffect= DROPEFFECT_NONE;
	return NOERROR;
}

STDMETHODIMP CDropTarget::DragLeave() {
	//Cancel drag and drop operation
	m_bAcceptFmt = FALSE;
	if (m_currentAcceptedType != -1) {
		DropHandler & dh = m_supportedTypes.at(m_currentAcceptedType);
		dh.cancelCall(dh.customData);
		m_currentAcceptedType = -1;
	}

	return NOERROR;
}

STDMETHODIMP CDropTarget::Drop(LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect) {
	if (m_bAcceptFmt && m_currentAcceptedType != -1) {	  //check if valid drop
		*pdwEffect = DROPEFFECT_NONE;
		DropHandler & dh = m_supportedTypes.at(m_currentAcceptedType);
		dh.dropCall(dh.type, pDataObj, grfKeyState, pt, pdwEffect, dh.customData);
	} else {
		*pdwEffect = DROPEFFECT_NONE;
	}
	return NOERROR;
}

void CDropTarget::addType(CLIPFORMAT type, void * custom, enterCallback enter, dragCallback drag, dropCallback drop, cancelCallback cancel) {
	DropHandler newHandler;
	newHandler.type = type;
	newHandler.customData = custom;
	newHandler.enterCall = enter;
	newHandler.dragCall = drag;
	newHandler.dropCall = drop;
	newHandler.cancelCall = cancel;
	m_supportedTypes.push_back(newHandler);
	size++;
}

//----------------------------------------DropSource-------------------------------------------------
CDropSource::CDropSource() {
	m_refs = 1;
	callback = NULL;
	customData = NULL;
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

STDMETHODIMP CDropSource::GiveFeedback(DWORD dwEffect) {
	if (callback)
		return callback(dwEffect, customData);
	return ResultFromScode(DRAGDROP_S_USEDEFAULTCURSORS);
}

void CDropSource::setCallback(feedbackCall feed, void * custom) {
	callback = feed;
	customData = custom;
}

//----------------------------------------DataObject--------------------------------------------------
CDataObject::CDataObject() {
	m_refs = 1;
	currentFormat = 0;
	callback = NULL;
	customData = NULL;
	formats.clear();
}

CDataObject::~CDataObject() {
	formats.clear();
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
	void * pRendered;

	pmedium->tymed = NULL;
	pmedium->pUnkForRelease = NULL;
	pmedium->hGlobal = NULL;

	size_t offset = 0;
	if (this->getFormatIndex(pformatetc, &offset)) {	//query ourself if request is valid
		if (!callback)
			return ResultFromScode(E_FAIL);	//This may NEVER HAPPEN!
		pRendered = callback(&(formats.at(offset)), customData);
		if (!pRendered)
			return ResultFromScode(E_OUTOFMEMORY);

		pmedium->tymed = formats.at(offset).tymed;
		switch(pmedium->tymed) {
			case TYMED_HGLOBAL:
				pmedium->hGlobal = (HGLOBAL)*((HGLOBAL*)(pRendered));
				break;
			case TYMED_ISTREAM:
				pmedium->pstm = (IStream*)pRendered;
				break;
			default:
				pmedium->hGlobal = (HGLOBAL)*((HGLOBAL*)(pRendered));
				break;
		}
		return ResultFromScode(S_OK);
	}
	return ResultFromScode(DATA_E_FORMATETC);
}

STDMETHODIMP CDataObject::GetDataHere(LPFORMATETC /*pformatetc*/, LPSTGMEDIUM /*pmedium*/) {
	return ResultFromScode(DATA_E_FORMATETC);
}

STDMETHODIMP CDataObject::QueryGetData(LPFORMATETC pformatetc) {
	// This method is called by the drop target to check whether the source
	// provides data is a format that the target accepts.
	if (getFormatIndex(pformatetc, NULL) == true)
		return ResultFromScode(S_OK);
	else
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

	if (dwDirection == DATADIR_GET){
		*ppenumFormatEtc = (IEnumFORMATETC*) new CEnumFormatEtc(&formats);
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

void CDataObject::addType(FORMATETC format) {
	formats.push_back(format);
}

void CDataObject::setCallback(renderData call, void * custom) {
	callback = call;
	customData = custom;
}

bool CDataObject::getFormatIndex(LPFORMATETC pformatetc, size_t * offset) {
	size_t size = formats.size();
	if (offset == NULL)
		offset = &size;	//size unused when setting offset anyway

	//Iterate over the COMPLETE set of formats as there is no guaranteed order
	//If a partial match doesnt end up being a full match it doesnt have to mean the format isnt there
	for(size_t i = 0; i < size; i++) {
		FORMATETC & fmt = formats.at(i);
		if (pformatetc->cfFormat == fmt.cfFormat
		 //&& pformatetc->lindex == fmt.lindex
		 && pformatetc->tymed & fmt.tymed) {
			*offset = i;
			return true;
		}
	}
	*offset = 0;
	return false;	//not found
}
//---------------------------------------EnumFORMATETC------------------------------------------------
CEnumFormatEtc::CEnumFormatEtc(std::vector<FORMATETC> * formats) {
	m_refs			= 1;
	m_nIndex		= 0;
	m_nNumFormats	= (ULONG)formats->size();
	m_pFormatEtc	= new FORMATETC[m_nNumFormats];

	// copy the FORMATETC structures
	for(ULONG i = 0; i < m_nNumFormats; i++) {
		m_pFormatEtc[i] = formats->at(i);
		m_pFormatEtc[i].ptd = NULL;
	}
}

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
CStreamData::CStreamData() {
    m_refs = 1;
	closedStream = false;
	if (!CreatePipe(&readHandle, &writeHandle, NULL, 2048)) {
		OutErr("[DnD] Unable to create pipe for stream\n");
		closedStream = true;
	}
}

CStreamData::~CStreamData() {
	if (!closedStream) {
		CloseHandle(readHandle);
		CloseHandle(writeHandle);
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
	if (!pv || !pcbRead) {
		return STG_E_INVALIDPOINTER;
	}

	if (!cb || closedStream) {
		*pcbRead = 0;
		return S_OK;
	}

	DWORD bytesRead = 0;
	BOOL res = ReadFile(readHandle, pv, cb, &bytesRead, NULL);
	if (res) {
		*pcbRead = bytesRead;
	} else {
		*pcbRead = 0;
		closedStream = true;
		DWORD err = GetLastError();
		if (err == ERROR_BROKEN_PIPE) {
			return S_OK;
		} else {
			OutErr("[DnD] Stream unexpected error: %d\n", err);
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

STDMETHODIMP CStreamData::Seek(LARGE_INTEGER /*liDistanceToMove*/, DWORD dwOrigin, ULARGE_INTEGER * /*lpNewFilePointer*/) {
	DWORD dwMoveMethod;

	switch(dwOrigin) {
	case STREAM_SEEK_SET:
		dwMoveMethod = FILE_BEGIN;
		break;
	case STREAM_SEEK_CUR:
		dwMoveMethod = FILE_CURRENT;
		break;
	case STREAM_SEEK_END:
		dwMoveMethod = FILE_END;
		break;
	default:
		return STG_E_INVALIDFUNCTION;
		break;
	}

	return E_NOTIMPL;
}

STDMETHODIMP CStreamData::Stat(STATSTG * pStatstg, DWORD grfStatFlag) {
	//Doesnt seem to get called by explorer
	FILETIME currentTime;
	if (CoFileTimeNow(&currentTime) != S_OK) {
		currentTime.dwHighDateTime = 0;
		currentTime.dwLowDateTime = 0;
	}

	if (!pStatstg)
		return STG_E_INVALIDPOINTER;

	if (!(grfStatFlag & STATFLAG_NONAME))  {	//allocate memory for name
		pStatstg->pwcsName = (LPOLESTR)CoTaskMemAlloc(11*sizeof(WCHAR));
		lstrcpynW(pStatstg->pwcsName, L"FTP_Stream", 11);
	}
	pStatstg->type = STGTY_STREAM;
	pStatstg->cbSize.LowPart = 0;	//FILESIZE!!
	pStatstg->cbSize.HighPart = 0;
	pStatstg->mtime = currentTime;
	pStatstg->ctime = currentTime;
	pStatstg->atime = currentTime;
	pStatstg->grfMode = STGM_READ;
	pStatstg->grfLocksSupported = 0;
	//pStatstg->clsid = ;
	//pStatstg->grfStateBits = ;
	return S_OK;
}
