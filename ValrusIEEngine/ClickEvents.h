#pragma once

#include <MsHTML.h>
#include <Exdisp.h>

class ClickEvents : public IDispatch {
private:
	IWebBrowser2 * mWebBrowser2;
public:
	ClickEvents(IWebBrowser2* webBrowser2);
	// IUnknown
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject);
	ULONG STDMETHODCALLTYPE AddRef(void);
	ULONG STDMETHODCALLTYPE Release(void);
	// IDispatch
	HRESULT STDMETHODCALLTYPE GetTypeInfoCount(
		/* [out] */ UINT *pctinfo);
	HRESULT STDMETHODCALLTYPE GetTypeInfo(
		/* [in] */ UINT iTInfo,
		/* [in] */ LCID lcid,
		/* [out] */ ITypeInfo **ppTInfo);
	HRESULT STDMETHODCALLTYPE GetIDsOfNames(
		/* [in] */ REFIID riid,
		/* [in] */ LPOLESTR *rgszNames,
		/* [in] */ UINT cNames,
		/* [in] */ LCID lcid,
		/* [out] */ DISPID *rgDispId);
	HRESULT STDMETHODCALLTYPE Invoke(
		/* [in] */ DISPID dispId,
		/* [in] */ REFIID riid,
		/* [in] */ LCID lcid,
		/* [in] */ WORD wFlags,
		/* [out][in] */ DISPPARAMS *pDispParams,
		/* [out] */ VARIANT *pVarResult,
		/* [out] */ EXCEPINFO *pExcepInfo,
		/* [out] */ UINT *puArgErr);
}; 
