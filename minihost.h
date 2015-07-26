//
// minihost.h
//

#include <atlbase.h>
#include <Exdisp.h> // IWebBrowser2
#include <Mshtmhst.h> // IDocHostUIHandler

#define WEBOC_START (WM_USER + 1)

#define METHOD_CLOSEDIALOG L"closedialog"
#define DISP_ID_CLOSEDIALOG 100

class CExternalDispatch : public IDispatch {
protected:
    ULONG _ulRefs;
    HWND _WebOC;

    virtual ~CExternalDispatch();

public:
    CExternalDispatch(HWND h);

    // IUnknown
    STDMETHOD(QueryInterface)(REFIID riid, void **ppvObject);
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

    //IDispatch
    STDMETHODIMP GetTypeInfoCount(
        /* [out] */ __RPC__out UINT *pctinfo) { return E_NOTIMPL; }
    STDMETHODIMP GetTypeInfo(
        /* [in] */ UINT iTInfo,
        /* [in] */ LCID lcid,
        /* [out] */ __RPC__deref_out_opt ITypeInfo **ppTInfo) { return E_NOTIMPL; }
    STDMETHODIMP GetIDsOfNames(
        /* [in] */ __RPC__in REFIID riid,
        /* [size_is][in] */ __RPC__in_ecount_full(cNames) LPOLESTR *rgszNames,
        /* [range][in] */ __RPC__in_range(0,16384) UINT cNames,
        /* [in] */ LCID lcid,
        /* [size_is][out] */ __RPC__out_ecount_full(cNames) DISPID *rgDispId);
    STDMETHODIMP Invoke(
        /* [annotation][in] */ 
        _In_  DISPID dispIdMember,
        /* [annotation][in] */ 
        _In_  REFIID riid,
        /* [annotation][in] */ 
        _In_  LCID lcid,
        /* [annotation][in] */ 
        _In_  WORD wFlags,
        /* [annotation][out][in] */ 
        _In_  DISPPARAMS *pDispParams,
        /* [annotation][out] */ 
        _Out_opt_  VARIANT *pVarResult,
        /* [annotation][out] */ 
        _Out_opt_  EXCEPINFO *pExcepInfo,
        /* [annotation][out] */ 
        _Out_opt_  UINT *puArgErr);
};

class MiniBrowserSite : public IOleClientSite,
                        public IOleInPlaceSite,
                        public IDocHostUIHandler {
protected:
    ULONG _ulRefs;
    HWND _WebOC;
    CComPtr<IWebBrowser2> _WebBrowser;
    CComPtr<IDispatch> _External;

    virtual ~MiniBrowserSite();

    void Cleanup();
    HRESULT InPlaceActivate();

public:
    static LRESULT CALLBACK MiniHostProc(HWND h, UINT m, WPARAM w, LPARAM l);

    MiniBrowserSite(HWND h);

    // IUnknown
    STDMETHOD(QueryInterface)(REFIID riid, void **ppvObject);
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

    // IOleClientSite
    IFACEMETHODIMP SaveObject(void) { return E_NOTIMPL; }
    IFACEMETHODIMP GetMoniker(
        /* [in] */ DWORD dwAssign,
        /* [in] */ DWORD dwWhichMoniker,
        /* [out] */ __RPC__deref_out_opt IMoniker **ppmk) { return E_NOTIMPL; }
    IFACEMETHODIMP GetContainer(
        /* [out] */ __RPC__deref_out_opt IOleContainer **ppContainer) { return E_NOTIMPL; }
    IFACEMETHODIMP ShowObject(void);
    IFACEMETHODIMP OnShowWindow(
        /* [in] */ BOOL fShow);
    IFACEMETHODIMP RequestNewObjectLayout(void) { return E_NOTIMPL; }

    // IOleWindow (via IOleInPlaceSite)
    IFACEMETHODIMP GetWindow(
        /* [out] */ __RPC__deref_out_opt HWND *phwnd);
    IFACEMETHODIMP ContextSensitiveHelp(
        /* [in] */ BOOL fEnterMode) { return E_NOTIMPL; }

    // IOleInPlaceSite
    IFACEMETHODIMP CanInPlaceActivate(void);
    IFACEMETHODIMP OnInPlaceActivate(void);
    IFACEMETHODIMP OnUIActivate(void);
    IFACEMETHODIMP GetWindowContext(
        /* [out] */ __RPC__deref_out_opt IOleInPlaceFrame **ppFrame,
        /* [out] */ __RPC__deref_out_opt IOleInPlaceUIWindow **ppDoc,
        /* [out] */ __RPC__out LPRECT lprcPosRect,
        /* [out] */ __RPC__out LPRECT lprcClipRect,
        /* [out][in] */ __RPC__inout LPOLEINPLACEFRAMEINFO lpFrameInfo);
    IFACEMETHODIMP Scroll(
        /* [in] */ SIZE scrollExtant) { return E_NOTIMPL; }
    IFACEMETHODIMP OnUIDeactivate(
        /* [in] */ BOOL fUndoable);
    IFACEMETHODIMP OnInPlaceDeactivate(void);
    IFACEMETHODIMP DiscardUndoState(void) { return E_NOTIMPL; }
    IFACEMETHODIMP DeactivateAndUndo(void) { return E_NOTIMPL; }
    IFACEMETHODIMP OnPosRectChange(
        /* [in] */ __RPC__in LPCRECT lprcPosRect) { return E_NOTIMPL; }

    // IDocHostUIHandler
    IFACEMETHODIMP ShowContextMenu(
        /* [annotation][in] */ 
        _In_  DWORD dwID,
        /* [annotation][in] */ 
        _In_  POINT *ppt,
        /* [annotation][in] */ 
        _In_  IUnknown *pcmdtReserved,
        /* [annotation][in] */ 
        _In_  IDispatch *pdispReserved);
    IFACEMETHODIMP GetHostInfo(
        /* [annotation][out][in] */ 
        _Inout_  DOCHOSTUIINFO *pInfo) ;
    IFACEMETHODIMP ShowUI(
        DWORD dwID,
        _In_ IOleInPlaceActiveObject *pActiveObject,
        _In_ IOleCommandTarget *pCommandTarget,
        _In_ IOleInPlaceFrame *pFrame,
        _In_ IOleInPlaceUIWindow *pDoc);
    IFACEMETHODIMP HideUI(void);
    IFACEMETHODIMP UpdateUI(void);
    IFACEMETHODIMP EnableModeless(
        /* [in] */ BOOL fEnable) { return E_NOTIMPL; }
    IFACEMETHODIMP OnDocWindowActivate(
        /* [in] */ BOOL fActivate) { return E_NOTIMPL; }
    IFACEMETHODIMP OnFrameWindowActivate(
        /* [in] */ BOOL fActivate) { return E_NOTIMPL; }
    IFACEMETHODIMP ResizeBorder(
        /* [annotation][in] */ 
        _In_  LPCRECT prcBorder,
        /* [annotation][in] */ 
        _In_  IOleInPlaceUIWindow *pUIWindow,
        /* [annotation][in] */ 
        _In_  BOOL fRameWindow) { return E_NOTIMPL; }
    IFACEMETHODIMP TranslateAccelerator(
        /* [in] */ LPMSG lpMsg,
        /* [in] */ const GUID *pguidCmdGroup,
        /* [in] */ DWORD nCmdID);
    IFACEMETHODIMP GetOptionKeyPath(
        /* [annotation][out] */ 
        _Out_  LPOLESTR *pchKey,
        /* [in] */ DWORD dw) { return E_NOTIMPL; }
    IFACEMETHODIMP GetDropTarget(
        /* [annotation][in] */ 
        _In_  IDropTarget *pDropTarget,
        /* [annotation][out] */ 
        _Outptr_  IDropTarget **ppDropTarget) { return E_NOTIMPL; }
    IFACEMETHODIMP GetExternal(
        /* [annotation][out] */ 
        _Outptr_result_maybenull_  IDispatch **ppDispatch);
    IFACEMETHODIMP TranslateUrl(
        /* [in] */ DWORD dwTranslate,
        /* [annotation][in] */ 
        _In_  LPWSTR pchURLIn,
        /* [annotation][out] */ 
        _Outptr_  LPWSTR *ppchURLOut) { return E_NOTIMPL; }
    IFACEMETHODIMP FilterDataObject(
        /* [annotation][in] */ 
        _In_  IDataObject *pDO,
        /* [annotation][out] */ 
        _Outptr_result_maybenull_  IDataObject **ppDORet) { return E_NOTIMPL; }
};
