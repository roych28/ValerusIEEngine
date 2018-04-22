#pragma once

#include <MsHTML.h>
#include "ClickEvents.h"

class ClickEvents : public IDispatch {
private:
	//BrowserWindow * browserWindow_;
public:
	ClickEvents();
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
