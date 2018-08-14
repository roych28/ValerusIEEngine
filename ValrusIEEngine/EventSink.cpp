/*
 Copyright (C) 2009 Moutaz Haq <cefarix@gmail.com>
 This file is released under the Code Project Open License <http://www.codeproject.com/info/cpol10.aspx>

 This file contains our implementation of the DWebBrowserEvents2 dispatch interface.
*/

#include "stdafx.h"
#include "EventSink.h"
#include <atlcomcli.h>
#include <mshtml.h>
#include <comdef.h>
#include "Utils.h"
#include "json\json.h"
#include "EventLog.h"

// The single global object of CEventSink
CEventSink EventSink;

STDMETHODIMP CEventSink::QueryInterface(REFIID riid,void **ppvObject)
{
	// Check if ppvObject is a valid pointer
	if(IsBadWritePtr(ppvObject,sizeof(void*))) return E_POINTER;
	// Set *ppvObject to NULL
	(*ppvObject)=NULL;
	// See if the requested IID matches one that we support
	// If it doesn't return E_NOINTERFACE
	if(!IsEqualIID(riid,IID_IUnknown) && !IsEqualIID(riid,IID_IDispatch) && !IsEqualIID(riid,DIID_DWebBrowserEvents2)) return E_NOINTERFACE;
	// If it's a matching IID, set *ppvObject to point to the global EventSink object
	(*ppvObject)=(void*)&EventSink;
	return S_OK;
}

STDMETHODIMP_(ULONG) CEventSink::AddRef()
{
	return 1; // We always have just one static object
}

STDMETHODIMP_(ULONG) CEventSink::Release()
{
	return 1; // Ditto
}

// We don't need to implement the next three methods because we are just a pure event sink
// We only care about Invoke() which is what IE calls to notify us of events
STDMETHODIMP CEventSink::GetTypeInfoCount(UINT *pctinfo)
{
	UNREFERENCED_PARAMETER(pctinfo);

	return E_NOTIMPL;
}

STDMETHODIMP CEventSink::GetTypeInfo(UINT iTInfo,LCID lcid,ITypeInfo **ppTInfo)
{
	UNREFERENCED_PARAMETER(iTInfo);
	UNREFERENCED_PARAMETER(lcid);
	UNREFERENCED_PARAMETER(ppTInfo);

	return E_NOTIMPL;
}

STDMETHODIMP CEventSink::GetIDsOfNames(REFIID riid,LPOLESTR *rgszNames,UINT cNames,LCID lcid,DISPID *rgDispId)
{
	UNREFERENCED_PARAMETER(riid);
	UNREFERENCED_PARAMETER(rgszNames);
	UNREFERENCED_PARAMETER(cNames);
	UNREFERENCED_PARAMETER(lcid);
	UNREFERENCED_PARAMETER(rgDispId);

	return E_NOTIMPL;
}

void updateRegSessionStorage(std::wstring key, LPCWSTR val)
{
	std::wstring fullPathKey = L"Software\\Vicon\\ChromeExt\\";
	Utils::SetStringVal(HKEY_CURRENT_USER, fullPathKey.c_str(), key.c_str(), val);
}

// This is called by IE to notify us of events
// Full documentation about all the events supported by DWebBrowserEvents2 can be found at
//  http://msdn.microsoft.com/en-us/library/aa768283(VS.85).aspx
STDMETHODIMP CEventSink::Invoke(DISPID dispIdMember,REFIID riid,LCID lcid,WORD wFlags,DISPPARAMS *pDispParams,VARIANT *pVarResult,EXCEPINFO *pExcepInfo,UINT *puArgErr)
{
	UNREFERENCED_PARAMETER(lcid);
	UNREFERENCED_PARAMETER(wFlags);
	UNREFERENCED_PARAMETER(pVarResult);
	UNREFERENCED_PARAMETER(pExcepInfo);
	UNREFERENCED_PARAMETER(puArgErr);
	VARIANT v[5];					
	int n;

	std::wstring logmsg = L"CEventSink::Invoke" + std::to_wstring(dispIdMember) + L"\r\n";
	OutputDebugString(logmsg.c_str());
	if(!IsEqualIID(riid,IID_NULL)) 
		return DISP_E_UNKNOWNINTERFACE;

	for(n=0;n<5;n++) 
		VariantInit(&v[n]);

	//this code is an event after web browser complete  
	//this is a test for get the session storage and save 
	if (dispIdMember == DISPID_DOCUMENTCOMPLETE)
	{
		if (!webBrowser) 
			return E_FAIL;
		IDispatch *idisp; webBrowser->get_Document(&idisp);
		IWebBrowser2* webBrowser2 = webBrowser;
		IDispatchPtr documentDispatch;
		HRESULT hr = webBrowser2->get_Document(&documentDispatch);
		if (FAILED(hr) || !documentDispatch) {
			return E_FAIL;
		}
		IHTMLDocument2Ptr htmlDocument2;
		hr = documentDispatch->QueryInterface(IID_IHTMLDocument2,
			(void**)&htmlDocument2);
		if (FAILED(hr) || !htmlDocument2) {
			return E_FAIL;
		}
		IHTMLWindow2Ptr htmlWindow2;
		hr = htmlDocument2->get_parentWindow(
			static_cast<IHTMLWindow2**>(&htmlWindow2));
		if (FAILED(hr) || !htmlWindow2) {
			return E_FAIL;
		}

		CComQIPtr<IHTMLWindow6, &IID_IHTMLWindow6> spHTMLWin6 = htmlWindow2;
		CComQIPtr<IHTMLStorage, &IID_IHTMLStorage> spHTMLStorage;
		hr = spHTMLWin6->get_sessionStorage(&spHTMLStorage);

		if (FAILED(hr) || !spHTMLStorage) {
			return E_FAIL;
		}

		_bstr_t hrefAttr(L"token");
		VARIANT attrValue;
		VariantInit(&attrValue);

		long p;
		hr = spHTMLStorage->get_length(&p);
		for (int i = 0; i < (int)p; i++)
		{
			BSTR key;
			spHTMLStorage->key(i, &key);
			
			hr = spHTMLStorage->getItem(key, &attrValue);

			if (SUCCEEDED(hr))
			{
				TCHAR tokenStr[4096];
				wsprintf(tokenStr, _T("%ls"), (LPOLESTR)attrValue.bstrVal);
				Json::Value root;
				Json::Reader reader;

				std::wstring wStr = tokenStr;
				std::string str = std::string(wStr.begin(), wStr.end());
				bool parsingSuccessfull = reader.parse(str, root);

				if (wcslen(tokenStr) > 1 && parsingSuccessfull)
				{
					//TODO : check if need to save token in registry - this is midwork for session storage POC
					//::SendMessage(m_parentHWND, WM_WEB_CONTROL_MESSAGE, dispIdMember, (LPARAM)tokenStr);
					updateRegSessionStorage(key, attrValue.bstrVal);
					std::wstring strKey = key;
					std::wstring logMsg = std::wstring(L"CEventSink::Invoke - save ") + key + std::wstring(L" : ") + wStr;
					log_event_log_message(logMsg, EVENTLOG_INFORMATION_TYPE, event_log_source_name);
				}
			}
		}

		return NOERROR;
	}
	//this code is an event when a new window is open from the ie itself(link etc...)
	//here we prevent from the new IE to launch and send message to MainFrame class 
	//there it notify the chrome ext about the new window via the native messaging pipe
	else if (dispIdMember == DISPID_NEWWINDOW3) 
	{
		VariantChangeType(&v[0], &pDispParams->rgvarg[0], 0, VT_BSTR);			// url

		*(pDispParams->rgvarg[3].pboolVal) = VARIANT_TRUE;						//prevent new window in explorer
		
		TCHAR urlStr[1024];
		wsprintf(urlStr, _T("%ls"), (LPOLESTR)v[0].bstrVal);

		::SendMessage(m_parentHWND, WM_WEB_CONTROL_MESSAGE, dispIdMember, (LPARAM)urlStr);
	}
	//this code is an event before web browser navigates 
	//this is a test for set the session storage 
	else if (dispIdMember == DISPID_BEFORENAVIGATE2)
	{
		VariantChangeType(&v[0], &pDispParams->rgvarg[5], 0, VT_BSTR);			// url

		TCHAR urlBuf[1024];
		wsprintf(urlBuf, _T("%ls"), (LPOLESTR)v[0].bstrVal);
		std::wstring szUrl = urlBuf;

		std::wstring wTokenStr			= Utils::GetStringVal(HKEY_CURRENT_USER, L"Software\\Vicon\\ChromeExt\\", L"token");
		std::wstring devSettings		= Utils::GetStringVal(HKEY_CURRENT_USER, L"Software\\Vicon\\ChromeExt\\", L"devSettings");
		std::wstring perm					= Utils::GetStringVal(HKEY_CURRENT_USER, L"Software\\Vicon\\ChromeExt\\", L"perm");
		std::wstring isOpenPlayerInstall	= Utils::GetStringVal(HKEY_CURRENT_USER, L"Software\\Vicon\\ChromeExt\\", L"ViconVMS.Web.Portal.WebStorage_demo_isOpenPlayerInstall");
		std::wstring repositoryHandle		= Utils::GetStringVal(HKEY_CURRENT_USER, L"Software\\Vicon\\ChromeExt\\", L"ViconVMS.Web.Portal.WebStorage_demo_repositoryHandle");

		if (wTokenStr.length() < 10)
		{
			return S_OK;
		}

		if (!webBrowser)
			return E_FAIL;
		IDispatch *idisp; webBrowser->get_Document(&idisp);
		IWebBrowser2* webBrowser2 = webBrowser;
		IDispatchPtr documentDispatch;
		HRESULT hr = webBrowser2->get_Document(&documentDispatch);
		if (FAILED(hr) || !documentDispatch) {
			return E_FAIL;
		}
		IHTMLDocument2Ptr htmlDocument2;
		hr = documentDispatch->QueryInterface(IID_IHTMLDocument2,
			(void**)&htmlDocument2);
		if (FAILED(hr) || !htmlDocument2) {
			return E_FAIL;
		}
		IHTMLWindow2Ptr htmlWindow2;
		hr = htmlDocument2->get_parentWindow(
			static_cast<IHTMLWindow2**>(&htmlWindow2));
		if (FAILED(hr) || !htmlWindow2) {
			return E_FAIL;
		}

		CComQIPtr<IHTMLWindow6, &IID_IHTMLWindow6> spHTMLWin6 = htmlWindow2;
		CComQIPtr<IHTMLStorage, &IID_IHTMLStorage> spHTMLStorage;
		hr = spHTMLWin6->get_sessionStorage(&spHTMLStorage);

		if (FAILED(hr) || !spHTMLStorage) {
			return E_FAIL;
		}

		long p;
		hr = spHTMLStorage->get_length(&p);
		if (p == 0)
		{
			_bstr_t bstrSessionDataToken(wTokenStr.c_str());
			hr = spHTMLStorage->setItem(L"token", bstrSessionDataToken);	//TODO : check if need to get token from registry - this is midwork for session storage POC

			if (SUCCEEDED(hr))
			{
				std::wstring logMsg = L"CEventSink::Invoke - set session token : " + wTokenStr;
				log_event_log_message(logMsg, EVENTLOG_INFORMATION_TYPE, event_log_source_name);
			}

			_bstr_t bstrSessionDatadevSettings(devSettings.c_str());
			hr = spHTMLStorage->setItem(L"devSettings", bstrSessionDatadevSettings);

			if (SUCCEEDED(hr))
			{
				std::wstring logMsg = L"CEventSink::Invoke - set session devSettings : " + devSettings;
				log_event_log_message(logMsg, EVENTLOG_INFORMATION_TYPE, event_log_source_name);
			}

			_bstr_t bstrSessionDataperm(perm.c_str());
			hr = spHTMLStorage->setItem(L"perm", bstrSessionDataperm);

			if (SUCCEEDED(hr))
			{
				std::wstring logMsg = L"CEventSink::Invoke - set session perm : " + perm;
				log_event_log_message(logMsg, EVENTLOG_INFORMATION_TYPE, event_log_source_name);
			}

			_bstr_t bstrSessionDataisOpenPlayerInstall(isOpenPlayerInstall.c_str());
			hr = spHTMLStorage->setItem(L"ViconVMS.Web.Portal.WebStorage_demo_isOpenPlayerInstall", bstrSessionDataisOpenPlayerInstall);

			if (SUCCEEDED(hr))
			{
				std::wstring logMsg = L"CEventSink::Invoke - set session ViconVMS.Web.Portal.WebStorage_demo_isOpenPlayerInstall : " + isOpenPlayerInstall;
				log_event_log_message(logMsg, EVENTLOG_INFORMATION_TYPE, event_log_source_name);
			}

			_bstr_t bstrSessionDatarepositoryHandle(repositoryHandle.c_str());
			hr = spHTMLStorage->setItem(L"ViconVMS.Web.Portal.WebStorage_demo_repositoryHandle", bstrSessionDatarepositoryHandle);

			if (SUCCEEDED(hr))
			{
				std::wstring logMsg = L"CEventSink::Invoke - set session repositoryHandle : " + repositoryHandle;
				log_event_log_message(logMsg, EVENTLOG_INFORMATION_TYPE, event_log_source_name);
			}
		}
	}

	for(n=0;n<5;n++) 
		VariantClear(&v[n]);

	return S_OK;
}

void CEventSink::SetParentHWND(HWND p)
{
	m_parentHWND = p;
}

void CEventSink::SetParentIWebBrowser2(IWebBrowser2* browser)
{
	webBrowser = browser;
}
/*

// Return true to prevent the url from being opened
bool CEventSink::Event_BeforeNavigate2(LPOLESTR url,LONG Flags,LPOLESTR TargetFrameName,PUCHAR PostData,LONG PostDataSize,LPOLESTR Headers,bool Cancel)
{
// Do whatever you like here
// This is just an example
TCHAR msg[1024];
wsprintf(msg,_T("url=%ls\nFlags=0x%08X\nTargetFrameName=%ls\nPostData=%hs\nPostDataSize=%d\nHeaders=%ls\nCancel=%s"),url,Flags,TargetFrameName,(char*)PostData,PostDataSize,Headers,((Cancel)?(_T("true")):(_T("false"))));
//MessageBox(NULL,msg,_T("BeforeNavigate2 event fired"),MB_OK);
return Cancel;
}

bool b;
PVOID pv;
LONG lbound,ubound,sz;

if (dispIdMember == DISPID_BEFORENAVIGATE2) { // Handle the BeforeNavigate2 event
	VariantChangeType(&v[0], &pDispParams->rgvarg[5], 0, VT_BSTR);			// url
	VariantChangeType(&v[1], &pDispParams->rgvarg[4], 0, VT_I4);			// Flags
	VariantChangeType(&v[2], &pDispParams->rgvarg[3], 0, VT_BSTR);			// TargetFrameName
	VariantChangeType(&v[3], &pDispParams->rgvarg[2], 0, VT_UI1 | VT_ARRAY); // PostData
	VariantChangeType(&v[4], &pDispParams->rgvarg[1], 0, VT_BSTR);			// Headers
	if (v[3].vt != VT_EMPTY) {
		SafeArrayGetLBound(v[3].parray, 0, &lbound);
		SafeArrayGetUBound(v[3].parray, 0, &ubound);
		sz = ubound - lbound + 1;
		SafeArrayAccessData(v[3].parray, &pv);
	}
	else {
		sz = 0;
		pv = NULL;
	}
	//b=Event_BeforeNavigate2((LPOLESTR)v[0].bstrVal,v[1].lVal,(LPOLESTR)v[2].bstrVal,(PUCHAR)pv,sz,(LPOLESTR)v[4].bstrVal,((*(pDispParams->rgvarg[0].pboolVal))!=VARIANT_FALSE));
	//if(v[3].vt!=VT_EMPTY) 
	//	SafeArrayUnaccessData(v[3].parray);
	//if(b) 
	//	*(pDispParams->rgvarg[0].pboolVal)=VARIANT_TRUE;
	//else 
	//	*(pDispParams->rgvarg[0].pboolVal)=VARIANT_FALSE;
}
else*/

/*hr = spHTMLStorage->getItem(L"token", &attrValue);

if (SUCCEEDED(hr))
{
TCHAR tokenStr[4096];
wsprintf(tokenStr, _T("%ls"), (LPOLESTR)attrValue.bstrVal);
Json::Value root;
Json::Reader reader;

std::wstring wStr = tokenStr;
std::string str = std::string(wStr.begin(), wStr.end());
bool parsingSuccessfull = reader.parse(str, root);

if (wcslen(tokenStr) > 1 && parsingSuccessfull)
{
//TODO : check if need to save token in registry - this is midwork for session storage POC
::SendMessage(m_parentHWND, WM_WEB_CONTROL_MESSAGE, dispIdMember, (LPARAM)tokenStr);
std::wstring logMsg = L"CEventSink::Invoke - save token : " + wStr;
log_event_log_message(logMsg, EVENTLOG_INFORMATION_TYPE, event_log_source_name);
}
}*/
/*
else if (dispIdMember == DISPID_COMMANDSTATECHANGE)
{
	IDispatch *idisp; webBrowser->get_Document(&idisp);
	IWebBrowser2* webBrowser2 = webBrowser;
	IDispatchPtr documentDispatch;
	HRESULT hr = webBrowser2->get_Document(&documentDispatch);
	if (FAILED(hr) || !documentDispatch) {
		return E_FAIL;
	}
	IHTMLDocument2Ptr htmlDocument2;
	hr = documentDispatch->QueryInterface(IID_IHTMLDocument2,
		(void**)&htmlDocument2);
	if (FAILED(hr) || !htmlDocument2) {
		return E_FAIL;
	}
	IHTMLWindow2Ptr htmlWindow2;
	hr = htmlDocument2->get_parentWindow(
		static_cast<IHTMLWindow2**>(&htmlWindow2));
	if (FAILED(hr) || !htmlWindow2) {
		return E_FAIL;
	}

	IHTMLEventObjPtr htmlEvent;
	hr = htmlWindow2->get_event(&htmlEvent);
	if (FAILED(hr) || !htmlEvent) {
		//	LOG_WARNING << "ClickEvents::Invoke() failed: "
		//		"IHTMLWindow2->get_event() failed";
		return S_OK;
	}
	else {

	}*/