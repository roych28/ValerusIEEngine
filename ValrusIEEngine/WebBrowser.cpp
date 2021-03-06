#include "stdafx.h"
#include "WebBrowser.h"
#include "EventLog.h"
#include <atlcomcli.h>
#include <mshtml.h>
#include <comdef.h>
#include "EventSink.h"
#include <Wininet.h>
#include "Utils.h"

_COM_SMARTPTR_TYPEDEF(IHTMLDocument2, IID_IHTMLDocument2);
_COM_SMARTPTR_TYPEDEF(IHTMLDocument3, IID_IHTMLDocument3);
_COM_SMARTPTR_TYPEDEF(IHTMLElement, IID_IHTMLElement);
_COM_SMARTPTR_TYPEDEF(IHTMLElement2, IID_IHTMLElement2);

WebBrowser::WebBrowser(HWND _hWndParent)
{
	iComRefCount = 0;
	::SetRect(&rObject, -300, -300, 300, 300);
	m_hWndParent = _hWndParent;

	if (CreateBrowser() == FALSE)
	{
		return;
	}

	ShowWindow(GetControlWindow(), SW_SHOW);

	this->Navigate(_T("about:blank"));
}

bool WebBrowser::CreateBrowser()
{
	HRESULT hr;
	hr = ::OleCreate(CLSID_WebBrowser,
		IID_IOleObject, OLERENDER_DRAW, 0, this, this,
		(void**)&oleObject);

	if (FAILED(hr))
	{
		MessageBox(NULL, _T("Cannot create oleObject CLSID_WebBrowser"),
			_T("Error"),
			MB_ICONERROR);
		return FALSE;
	}

	hr = oleObject->SetClientSite(this);
	hr = OleSetContainedObject(oleObject, TRUE);

	RECT posRect;
	::SetRect(&posRect, -300, -300, 300, 300);
	hr = oleObject->DoVerb(OLEIVERB_INPLACEACTIVATE,
		NULL, this, -1, m_hWndParent, &posRect);
	if (FAILED(hr))
	{
		MessageBox(NULL, _T("oleObject->DoVerb() failed"),
			_T("Error"),
			MB_ICONERROR);
		return FALSE;
	}

	hr = oleObject->QueryInterface(&webBrowser2);
	if (FAILED(hr))
	{
		MessageBox(NULL, _T("oleObject->QueryInterface(&webBrowser2) failed"),
			_T("Error"),
			MB_ICONERROR);
		return FALSE;
	}

	this->webBrowser2->put_Silent(VARIANT_TRUE);
	//this->webBrowser2->put_RegisterAsBrowser(VARIANT_TRUE);

	ConnectEventSink();
	return TRUE;
}

void WebBrowser::ConnectEventSink()
{
	HRESULT hr;
	IConnectionPointContainer* pCPC;

	if (webBrowser2 == NULL) 
		return; // If we don't have a site, don't do anything
	// Get an IConnectionPointContainer interface pointer from the site
	hr = webBrowser2->QueryInterface(IID_IConnectionPointContainer, (void**)&pCPC);
	if (FAILED(hr)) 
		return; // If we couldn't get it, abort
	
	// Now we use the IConnectionPointContainer interface to get an IConnectionPoint interface pointer that will handle DWebBrowserEvents2 "dispatch interface" events.
	// That means we have to plug our implementation of DWebBrowserEvents2 into the returned IConnectionPoint interface using its Advise() method, as below
	hr = pCPC->FindConnectionPoint(DIID_DWebBrowserEvents2, &pCP);
	if (FAILED(hr)) { // If it failed, release the pCPC interface pointer and abort
		pCPC->Release();
		return;
	}

	EventSink.SetParentHWND(m_hWndParent);
	EventSink.SetParentIWebBrowser2(webBrowser2);
	// Finally we can plug our event handler object EventSink into the connection point and start receiving IE events
	// The advise cookie is just a return value we use when we want to "unplug" our event handler object from the connection point
	pCP->Advise((IUnknown*)&EventSink, &adviseCookie);
}

RECT WebBrowser::PixelToHiMetric(const RECT& _rc)
{
	static bool s_initialized = false;
	static int s_pixelsPerInchX, s_pixelsPerInchY;
	if(!s_initialized)
	{
		HDC hdc = ::GetDC(0);
		s_pixelsPerInchX = ::GetDeviceCaps(hdc, LOGPIXELSX);
		s_pixelsPerInchY = ::GetDeviceCaps(hdc, LOGPIXELSY);
		::ReleaseDC(0, hdc);
		s_initialized = true;
	}

	RECT rc;
	rc.left = MulDiv(2540, _rc.left, s_pixelsPerInchX);
	rc.top = MulDiv(2540, _rc.top, s_pixelsPerInchY);
	rc.right = MulDiv(2540, _rc.right, s_pixelsPerInchX);
	rc.bottom = MulDiv(2540, _rc.bottom, s_pixelsPerInchY);
	return rc;
}

void WebBrowser::SetRect(const RECT& _rc)
{
	rObject = _rc;

	{
		RECT hiMetricRect = PixelToHiMetric(rObject);
		SIZEL sz;
		sz.cx = hiMetricRect.right - hiMetricRect.left;
		sz.cy = hiMetricRect.bottom - hiMetricRect.top;
		oleObject->SetExtent(DVASPECT_CONTENT, &sz);
	}

	if(oleInPlaceObject != 0)
	{
		oleInPlaceObject->SetObjectRects(&rObject, &rObject);
	}
}

// ----- Control methods -----

void WebBrowser::GoBack()
{
	this->webBrowser2->GoBack();
}

void WebBrowser::GoForward()
{
	this->webBrowser2->GoForward();
}

void WebBrowser::Refresh()
{
	this->webBrowser2->Refresh();
}

void WebBrowser::Navigate(wstring szUrl)
{
	/*std::wstring domain = szUrl;
	std::string::size_type i = szUrl.find(L"http://");

	if (i != std::string::npos)
		domain.erase(i, 7); //remove http://

	//unsigned long rescd;
	if (!InternetSetPerSiteCookieDecision(domain.c_str(), COOKIE_STATE_DOWNGRADE))
	{
		DWORD e = GetLastError();
		log_event_log_message(L"WebBrowser::Navigate InternetSetPerSiteCookieDecision: ", EVENTLOG_INFORMATION_TYPE, event_log_source_name);
	}

	/*HRESULT hr;
	BOOL bProtectedMode = FALSE;

	hr = IEIsProtectedModeProcess(&bProtectedMode);

	if (SUCCEEDED(hr) && bProtectedMode)
	{
		// IE is running in protected mode
	}
	else 
	{
		// IE isn't running in protected mode
	}*/
		

	bstr_t url(szUrl.c_str());
	variant_t flags(0x02u); //navNoHistory
    HRESULT res = this->webBrowser2->Navigate(url, 0, 0, 0, 0);

	this->webBrowser2->put_Visible(VARIANT_TRUE);

	std::wstring resStr = std::to_wstring(res);
	log_event_log_message(L"WebBrowser::Navigate res: " + resStr, EVENTLOG_INFORMATION_TYPE, event_log_source_name);
}

// ----- IUnknown -----

HRESULT STDMETHODCALLTYPE WebBrowser::QueryInterface(REFIID riid,
													 void**ppvObject)
{
	if (riid == __uuidof(IUnknown))
	{
		(*ppvObject) = static_cast<IOleClientSite*>(this);
	}
	else if (riid == __uuidof(IOleInPlaceSite))
	{
		(*ppvObject) = static_cast<IOleInPlaceSite*>(this);
	}
	else
	{
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}

ULONG STDMETHODCALLTYPE WebBrowser::AddRef(void)
{
	iComRefCount++; 
	return iComRefCount;
}

ULONG STDMETHODCALLTYPE WebBrowser::Release(void)
{
	iComRefCount--; 
	return iComRefCount;
}

// ---------- IOleWindow ----------

HRESULT STDMETHODCALLTYPE WebBrowser::GetWindow( 
	__RPC__deref_out_opt HWND *phwnd)
{
	(*phwnd) = m_hWndParent;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE WebBrowser::ContextSensitiveHelp(
	BOOL fEnterMode)
{
	return E_NOTIMPL;
}

// ---------- IOleInPlaceSite ----------

HRESULT STDMETHODCALLTYPE WebBrowser::CanInPlaceActivate(void)
{
	return S_OK;
}

HRESULT STDMETHODCALLTYPE WebBrowser::OnInPlaceActivate(void)
{
	OleLockRunning(oleObject, TRUE, FALSE);
	oleObject->QueryInterface(&oleInPlaceObject);
	oleInPlaceObject->SetObjectRects(&rObject, &rObject);

	return S_OK;

}

HRESULT STDMETHODCALLTYPE WebBrowser::OnUIActivate(void)
{
	return S_OK;
}

HRESULT STDMETHODCALLTYPE WebBrowser::GetWindowContext( 
	__RPC__deref_out_opt IOleInPlaceFrame **ppFrame,
	__RPC__deref_out_opt IOleInPlaceUIWindow **ppDoc,
	__RPC__out LPRECT lprcPosRect,
	__RPC__out LPRECT lprcClipRect,
	__RPC__inout LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
	HWND hwnd = m_hWndParent;

	(*ppFrame) = NULL;
	(*ppDoc) = NULL;
	(*lprcPosRect).left = rObject.left;
	(*lprcPosRect).top = rObject.top;
	(*lprcPosRect).right = rObject.right;
	(*lprcPosRect).bottom = rObject.bottom;
	*lprcClipRect = *lprcPosRect;

	lpFrameInfo->fMDIApp = false;
	lpFrameInfo->hwndFrame = hwnd;
	lpFrameInfo->haccel = NULL;
	lpFrameInfo->cAccelEntries = 0;

	return S_OK;
}

HRESULT STDMETHODCALLTYPE WebBrowser::Scroll( 
	SIZE scrollExtant)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE WebBrowser::OnUIDeactivate( 
	BOOL fUndoable)
{
	return S_OK;
}

HWND WebBrowser::GetControlWindow()
{
	if(m_hWndControl != 0)
		return m_hWndControl;

	if(oleInPlaceObject == 0)
		return 0;

	oleInPlaceObject->GetWindow(&m_hWndControl);
	return m_hWndControl;
}

HRESULT STDMETHODCALLTYPE WebBrowser::OnInPlaceDeactivate(void)
{
	m_hWndControl = 0;
	oleInPlaceObject = 0;

	return S_OK;
}

HRESULT STDMETHODCALLTYPE WebBrowser::DiscardUndoState(void)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE WebBrowser::DeactivateAndUndo(void)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE WebBrowser::OnPosRectChange( 
	__RPC__in LPCRECT lprcPosRect)
{
	return E_NOTIMPL;
}

// ---------- IOleClientSite ----------

HRESULT STDMETHODCALLTYPE WebBrowser::SaveObject(void)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE WebBrowser::GetMoniker( 
	DWORD dwAssign,
	DWORD dwWhichMoniker,
	__RPC__deref_out_opt IMoniker **ppmk)
{
	if((dwAssign == OLEGETMONIKER_ONLYIFTHERE) &&
		(dwWhichMoniker == OLEWHICHMK_CONTAINER))
		return E_FAIL;

	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE WebBrowser::GetContainer( 
	__RPC__deref_out_opt IOleContainer **ppContainer)
{
	return E_NOINTERFACE;
}

HRESULT STDMETHODCALLTYPE WebBrowser::ShowObject(void)
{
	ShowWindow(GetControlWindow(), SW_SHOW);
	return S_OK;
}

HRESULT STDMETHODCALLTYPE WebBrowser::OnShowWindow( 
	BOOL fShow)
{
	return S_OK;
}

HRESULT STDMETHODCALLTYPE WebBrowser::RequestNewObjectLayout(void)
{
	return E_NOTIMPL;
}

// ----- IStorage -----

HRESULT STDMETHODCALLTYPE WebBrowser::CreateStream( 
	__RPC__in_string const OLECHAR *pwcsName,
	DWORD grfMode,
	DWORD reserved1,
	DWORD reserved2,
	__RPC__deref_out_opt IStream **ppstm)
{
	log_event_log_message(L"WebBrowser::Navigate InternetSetPerSiteCookieDecision: ", EVENTLOG_INFORMATION_TYPE, event_log_source_name);
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE WebBrowser::OpenStream( 
	const OLECHAR *pwcsName,
	void *reserved1,
	DWORD grfMode,
	DWORD reserved2,
	IStream **ppstm)
{
	log_event_log_message(L"WebBrowser::OpenStream: ", EVENTLOG_INFORMATION_TYPE, event_log_source_name);
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE WebBrowser::CreateStorage( 
	__RPC__in_string const OLECHAR *pwcsName,
	DWORD grfMode,
	DWORD reserved1,
	DWORD reserved2,
	__RPC__deref_out_opt IStorage **ppstg)
{
	log_event_log_message(L"WebBrowser::CreateStorage: ", EVENTLOG_INFORMATION_TYPE, event_log_source_name);
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE WebBrowser::OpenStorage( 
	__RPC__in_opt_string const OLECHAR *pwcsName,
	__RPC__in_opt IStorage *pstgPriority,
	DWORD grfMode,
	__RPC__deref_opt_in_opt SNB snbExclude,
	DWORD reserved,
	__RPC__deref_out_opt IStorage **ppstg)
{
	log_event_log_message(L"WebBrowser::OpenStorage: ", EVENTLOG_INFORMATION_TYPE, event_log_source_name);
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE WebBrowser::CopyTo( 
	DWORD ciidExclude,
	const IID *rgiidExclude,
	__RPC__in_opt  SNB snbExclude,
	IStorage *pstgDest)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE WebBrowser::MoveElementTo( 
	__RPC__in_string const OLECHAR *pwcsName,
	__RPC__in_opt IStorage *pstgDest,
	__RPC__in_string const OLECHAR *pwcsNewName,
	DWORD grfFlags)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE WebBrowser::Commit( 
	DWORD grfCommitFlags)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE WebBrowser::Revert(void)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE WebBrowser::EnumElements( 
	DWORD reserved1,
	void *reserved2,
	DWORD reserved3,
	IEnumSTATSTG **ppenum)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE WebBrowser::DestroyElement( 
	__RPC__in_string const OLECHAR *pwcsName)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE WebBrowser::RenameElement( 
	__RPC__in_string const OLECHAR *pwcsOldName,
	__RPC__in_string const OLECHAR *pwcsNewName)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE WebBrowser::SetElementTimes( 
	__RPC__in_opt_string const OLECHAR *pwcsName,
	__RPC__in_opt const FILETIME *pctime,
	__RPC__in_opt const FILETIME *patime,
	__RPC__in_opt const FILETIME *pmtime)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE WebBrowser::SetClass(
	__RPC__in REFCLSID clsid)
{
	return S_OK;
}

HRESULT STDMETHODCALLTYPE WebBrowser::SetStateBits( 
	DWORD grfStateBits,
	DWORD grfMask)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE WebBrowser::Stat( 
	__RPC__out STATSTG *pstatstg,
	DWORD grfStatFlag)
{
	return E_NOTIMPL;
}

void WebBrowser::InjectMessage(LPMSG msg)
{
	IOleInPlaceActiveObject* pIOIPAO;
	HRESULT hr = this->webBrowser2->QueryInterface(IID_IOleInPlaceActiveObject, (void**)&pIOIPAO);
	if (SUCCEEDED(hr))
	{
		/*MSG msg;
		msg.message = uMsg;
		msg.wParam = wParam;
		msg.lParam = lParam;*/

		pIOIPAO->TranslateAccelerator(msg);
		pIOIPAO->Release();
	}
}


//clickEvents_.reset(new ClickEvents(webBrowser2));

//clickDispatch_.vt = VT_DISPATCH;
//clickDispatch_.pdispVal = static_cast<IDispatch*>(clickEvents_.get());

/*
bool WebBrowser::TryAttachClickEvents() {
	// Attach OnClick event - to catch clicking any external
	// links. Returns whether succeeded to attach click events,
	// it is required for the DOM to be ready, call this
	// function in a timer until it succeeds.After browser
	// navigation these click events need to be re-attached.

	if (!webBrowser2) {
		// Web-browser control might be closing.
		return false;
	}
	HRESULT hr;
	VARIANT_BOOL isBusy;
	hr = webBrowser2->get_Busy(&isBusy);
	// This may fail when window is loading/unloading.
	if (FAILED(hr) || isBusy == VARIANT_TRUE) {
		return false;
	}
	IDispatchPtr dispatch;
	hr = webBrowser2->get_Document(&dispatch);
	// This may fail when window is loading.
	if (FAILED(hr) || !dispatch) {
		return false;
	}
	IHTMLDocument3Ptr htmlDocument3;
	hr = dispatch->QueryInterface(IID_IHTMLDocument3,
		(void**)&htmlDocument3);
	if (FAILED(hr) || !htmlDocument3) {
		//LOG_WARNING << "BrowserWindow::TryAttachClickEvents() failed "
		//"QueryInterface(IHTMLDocument3) failed";
		return false;
	}
	IHTMLElementPtr htmlElement;
	hr = htmlDocument3->get_documentElement(&htmlElement);
	if (FAILED(hr) || !htmlElement) {
		//LOG_WARNING << "BrowserWindow::TryAttachClickEvents() failed "
		//	"get_documentElement() failed";
		return false;
	}
	_bstr_t documentID;
	hr = htmlElement->get_id(&documentID.GetBSTR());
	if (FAILED(hr)) {
		//LOG_WARNING << "BrowserWindow::TryAttachClickEvents() failed "
		//	"htmlElement->get_id() failed";
		return false;
	}
	if (documentID.length() && documentID == documentUniqueID_) {
		return true;
	}
	else {
		// Document's identifier changed, browser navigated.
		this->clickEventsAttached_ = false;
		_bstr_t uniqueID;
		hr = htmlDocument3->get_uniqueID(&uniqueID.GetBSTR());
		if (FAILED(hr)) {
			//LOG_WARNING << "BrowserWindow::TryAttachClickEvents() "
			//	"failed: htmlDocument3->get_uniqueID() failed";
			return false;
		}
		hr = htmlElement->put_id(uniqueID.GetBSTR());
		if (FAILED(hr)) {
			//LOG_WARNING << "BrowserWindow::TryAttachClickEvents() "
			//	"failed: htmlElement->put_id() failed";
			return false;
		}
		documentUniqueID_.Assign(uniqueID.GetBSTR());
	}
	if (this->clickEventsAttached_) {
		return true;
	}
	IHTMLDocument2Ptr htmlDocument2;
	hr = dispatch->QueryInterface(IID_IHTMLDocument2,
		(void**)&htmlDocument2);
	if (FAILED(hr) || !htmlDocument2) {
		//LOG_WARNING << "BrowserWindow::TryAttachClickEvents() failed: "
		//	"QueryInterface(IHTMLDocument2)";
		return false;
	}
	hr = htmlDocument2->put_onclick(clickDispatch_);
	if (FAILED(hr)) {
		//LOG_WARNING << "BrowserWindow::TryAttachClickEvents() failed: "
		//	"htmlDocument2->put_onclick() failed";
		return false;
	}
	this->clickEventsAttached_ = true;
	return true;
}*/