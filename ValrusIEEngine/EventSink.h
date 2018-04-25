#include <Exdisp.h>
#include <Exdispid.h>

class CEventSink : public DWebBrowserEvents2 {
public:
	// No constructor or destructor is needed
	// IUnknown methods
	STDMETHODIMP QueryInterface(REFIID riid,void **ppvObject);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();
	// IDispatch methods
	STDMETHODIMP GetTypeInfoCount(UINT *pctinfo);
	STDMETHODIMP GetTypeInfo(UINT iTInfo,LCID lcid,ITypeInfo **ppTInfo);
	STDMETHODIMP GetIDsOfNames(REFIID riid,LPOLESTR *rgszNames,UINT cNames,LCID lcid,DISPID *rgDispId);
	STDMETHODIMP Invoke(DISPID dispIdMember,REFIID riid,LCID lcid,WORD wFlags,DISPPARAMS *pDispParams,VARIANT *pVarResult,EXCEPINFO *pExcepInfo,UINT *puArgErr);
	// DWebBrowserEvents2 does not have any methods, IE calls our Invoke() method to notify us of events
	void SetParentHWND(HWND p);
protected:
	
	HWND m_parentHWND;
};

// We only have one global object of this
extern CEventSink EventSink;
