//
// minihost.cpp
//
// How to disable the default pop-up menu for CHtmlView in Visual C++
// https://support.microsoft.com/en-us/kb/236312
//
// Use an ActiveX control in your Win32 Project without MFC with CreateWindowEx or in a dialog box - CodeProject
// http://www.codeproject.com/Articles/18417/Use-an-ActiveX-control-in-your-Win-Project-witho
//
// WebBrowser Customization (Internet Explorer)
// https://msdn.microsoft.com/en-us/library/aa770041(v=vs.85).aspx
//
// IDocHostUIHandler interface (Windows)
// https://msdn.microsoft.com/en-us/library/aa753260(v=vs.85).aspx
//

// For testing: verify these destructors are called on exit
// minib!MiniBrowserSite::`scalar deleting destructor'
// minib!CExternalDispatch::`scalar deleting destructor'

#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <Mshtml.h> // IHTMLDocument
#include "minihost.h"

void Log(LPCWSTR Format, ...) {
    WCHAR LineBuf[1024];
    va_list v;
    va_start(v, Format);
    StringCbVPrintf(LineBuf, sizeof(LineBuf), Format, v);
    OutputDebugString(LineBuf);
}

LRESULT CALLBACK MiniBrowserSite::MiniHostProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    LRESULT lr = 0;
    MiniBrowserSite *BrowserSite = nullptr;

    switch (m) {
    case WM_CREATE:
        // https://msdn.microsoft.com/en-us/library/windows/desktop/ms632619(v=vs.85).aspx
        //
        lr = -1;
        BrowserSite = new MiniBrowserSite(h);
        if (BrowserSite) {
            if (SUCCEEDED(BrowserSite->InPlaceActivate())) {
                SetLastError(0);
                SetWindowLongPtr(h, GWLP_USERDATA, (LONG_PTR)BrowserSite);
                if (GetLastError() == 0) {
                    lr = 0;
                }
            }
            if (lr != 0) {
                BrowserSite->Cleanup();
                BrowserSite->Release();
            }
        }
        break;

    case WM_DESTROY:
        BrowserSite = MiniBrowserSite::GetFromHWND(h);
        if (BrowserSite) {
            BrowserSite->Cleanup();
            BrowserSite->Release(); // do not 'delete BrowserSite' directly
        }
        break;

    default:
        lr = DefWindowProc(h, m, w, l);
        break;
    }

    return lr;
}

MiniBrowserSite *MiniBrowserSite::GetFromHWND(HWND h) {
    return h ? (MiniBrowserSite*)GetWindowLongPtr(h, GWLP_USERDATA) : nullptr;
}

MiniBrowserSite::MiniBrowserSite(HWND h) : _ulRefs(1), _WebOC(h) {
    _External.Attach(new CExternalDispatch(h));
}

MiniBrowserSite::~MiniBrowserSite() {}

HRESULT MiniBrowserSite::Navigate(LPCWSTR url) {
    HRESULT hr = E_POINTER;
    if (_WebBrowser != nullptr) {
        CComBSTR UrlCopy(url);
        hr = _WebBrowser->Navigate(UrlCopy, nullptr, nullptr, nullptr, nullptr);
    }
    return hr;
}

HRESULT MiniBrowserSite::Stop() {
    HRESULT hr = E_POINTER;
    if (_WebBrowser != nullptr) {
        hr = _WebBrowser->Stop();
    }
    return hr;
}

HRESULT MiniBrowserSite::Refresh(RefreshConstants Level) {
    HRESULT hr = E_POINTER;
    if (_WebBrowser != nullptr) {
        VARIANT var;
        VariantInit(&var);
        var.vt = VT_I4;
        var.lVal = Level;
        hr = _WebBrowser->Refresh2(&var);
    }
    return hr;
}

HRESULT MiniBrowserSite::DumpInfo() {
    HRESULT hr = E_POINTER;
    if (_WebBrowser != nullptr) {
        LPVOID p = nullptr;
        CComPtr<IDispatch> DocDispatch;
        hr = _WebBrowser->get_Document(&DocDispatch); // return S_FALSE if doc is empty
        if (hr == S_OK) {
            CComPtr<IHTMLDocument> Doc;
            hr = DocDispatch->QueryInterface(&Doc);
            if (SUCCEEDED(hr)) {
                p = (IHTMLDocument*)Doc;
            }
        }
        Log(L"IHTMLDocument* = %p\n", p);
    }
    return hr;
}

void MiniBrowserSite::Cleanup() {
    if (_WebBrowser != nullptr) {
        _WebBrowser->Quit();

        CComQIPtr<IOleObject> OleObject(_WebBrowser);
        if (OleObject != nullptr) {
            OleObject->SetClientSite(nullptr);
            OleObject->Close(OLECLOSE::OLECLOSE_NOSAVE);
        }
    }
    if (_External != nullptr) {
        _External.Release();
    }
}

HRESULT MiniBrowserSite::InPlaceActivate() {
    HRESULT hr = CoCreateInstance(CLSID_WebBrowser, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&_WebBrowser));
    if (SUCCEEDED(hr)) {
        CComPtr<IOleObject> OleObject;
        hr = _WebBrowser->QueryInterface(IID_PPV_ARGS(&OleObject));
        if (SUCCEEDED(hr)) {
            hr = OleSetContainedObject(OleObject, TRUE);
            if (SUCCEEDED(hr)) {
                hr = OleObject->SetClientSite(this);
                if (SUCCEEDED(hr)) {
                    RECT rc = { 0 };
                    GetClientRect(_WebOC, &rc);
                    hr = OleObject->DoVerb(OLEIVERB_INPLACEACTIVATE, nullptr, this, -1, _WebOC, &rc);
                }
            }
        }
    }
    return hr;
}

// ---------- IUnknown ----------

#pragma warning(push)
#pragma warning(disable : 4838)
// warning C4838: conversion from 'DWORD' to 'int' requires a narrowing conversion
HRESULT STDMETHODCALLTYPE MiniBrowserSite::QueryInterface(REFIID riid, void **ppvObject) {
    const QITAB QITable[] = {
        QITABENT(MiniBrowserSite, IOleClientSite),
        QITABENT(MiniBrowserSite, IOleWindow),
        QITABENT(MiniBrowserSite, IOleInPlaceSite),
        QITABENT(MiniBrowserSite, IDocHostUIHandler),
        { 0 },
    };
    return QISearch(this, QITable, riid, ppvObject);
}
#pragma warning(pop)

STDMETHODIMP_(ULONG) MiniBrowserSite::AddRef(void) {
    return InterlockedIncrement(&_ulRefs);
}

STDMETHODIMP_(ULONG) MiniBrowserSite::Release(void) {
    ULONG cref = (ULONG)InterlockedDecrement(&_ulRefs);
    if (cref == 0) {
        delete this;
    }
    return cref;
}

// ---------- IOleClientSite ----------

HRESULT STDMETHODCALLTYPE MiniBrowserSite::ShowObject(void) {
    return S_OK;
}

HRESULT STDMETHODCALLTYPE MiniBrowserSite::OnShowWindow(
        /* [in] */ BOOL fShow) {
    return S_OK;
}

// ---------- IOleWindow ----------

HRESULT STDMETHODCALLTYPE MiniBrowserSite::GetWindow(
        /* [out] */ __RPC__deref_out_opt HWND *phwnd) {
    (*phwnd) = _WebOC;
    return S_OK;
}

// ---------- IOleInPlaceSite ----------

HRESULT STDMETHODCALLTYPE MiniBrowserSite::CanInPlaceActivate(void) {
    return S_OK;
}

HRESULT STDMETHODCALLTYPE MiniBrowserSite::OnInPlaceActivate(void) {
    return S_OK;
}

HRESULT STDMETHODCALLTYPE MiniBrowserSite::OnUIActivate(void) {
    return S_OK;
}

HRESULT STDMETHODCALLTYPE MiniBrowserSite::GetWindowContext(
        /* [out] */ __RPC__deref_out_opt IOleInPlaceFrame **ppFrame,
        /* [out] */ __RPC__deref_out_opt IOleInPlaceUIWindow **ppDoc,
        /* [out] */ __RPC__out LPRECT lprcPosRect,
        /* [out] */ __RPC__out LPRECT lprcClipRect,
        /* [out][in] */ __RPC__inout LPOLEINPLACEFRAMEINFO lpFrameInfo) {
    if (ppFrame != nullptr)
        *ppFrame = nullptr;

    if (ppDoc != nullptr)
        *ppDoc = nullptr;

    if (lprcPosRect)
        GetClientRect(_WebOC, lprcPosRect);

    if (lprcClipRect)
        GetClientRect(_WebOC, lprcClipRect);

    if (lpFrameInfo) {
        lpFrameInfo->fMDIApp = false;
        lpFrameInfo->hwndFrame = _WebOC;
        lpFrameInfo->haccel = nullptr;
        lpFrameInfo->cAccelEntries = 0;
    }

    return S_OK;
}

HRESULT STDMETHODCALLTYPE MiniBrowserSite::OnUIDeactivate(
        /* [in] */ BOOL fUndoable) {
    return S_OK;
}

HRESULT STDMETHODCALLTYPE MiniBrowserSite::OnInPlaceDeactivate(void) {
    _WebOC = nullptr;
    return S_OK;
}

// ---------- IDocHostUIHandler ----------

HRESULT STDMETHODCALLTYPE MiniBrowserSite::ShowContextMenu(
        /* [annotation][in] */ 
        _In_  DWORD dwID,
        /* [annotation][in] */ 
        _In_  POINT *ppt,
        /* [annotation][in] */ 
        _In_  IUnknown *pcmdtReserved,
        /* [annotation][in] */ 
        _In_  IDispatch *pdispReserved) {
    // We've shown our own context menu. MSHTML.DLL will no longer try to show its own.
    return S_OK;
}

HRESULT STDMETHODCALLTYPE MiniBrowserSite::GetHostInfo(
        /* [annotation][out][in] */ 
        _Inout_  DOCHOSTUIINFO *pInfo) {
    pInfo->dwDoubleClick = DOCHOSTUIDBLCLK_DEFAULT;
    pInfo->dwFlags = DOCHOSTUIFLAG_NO3DBORDER;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE MiniBrowserSite::ShowUI(
        /* [annotation][in] */ 
        _In_  DWORD dwID,
        /* [annotation][in] */ 
        _In_  IOleInPlaceActiveObject *pActiveObject,
        /* [annotation][in] */ 
        _In_  IOleCommandTarget *pCommandTarget,
        /* [annotation][in] */ 
        _In_  IOleInPlaceFrame *pFrame,
        /* [annotation][in] */ 
        _In_  IOleInPlaceUIWindow *pDoc) {
    // We've already got our own UI in place so just return S_OK
    return S_OK;
}

HRESULT STDMETHODCALLTYPE MiniBrowserSite::HideUI(void) {
    return S_OK;
}

HRESULT STDMETHODCALLTYPE MiniBrowserSite::UpdateUI(void) {
    return S_OK;
}

HRESULT STDMETHODCALLTYPE MiniBrowserSite::TranslateAccelerator(
        /* [in] */ LPMSG lpMsg,
        /* [in] */ const GUID *pguidCmdGroup,
        /* [in] */ DWORD nCmdID) {
    return S_OK; // Blocking the default behavior
}

HRESULT STDMETHODCALLTYPE MiniBrowserSite::GetExternal(
        /* [annotation][out] */ 
        _Outptr_result_maybenull_  IDispatch **ppDispatch) {
    return _External.CopyTo(ppDispatch);
}

CExternalDispatch::CExternalDispatch(HWND h) : _ulRefs(1), _WebOC(h) {}

CExternalDispatch::~CExternalDispatch() {}

// ---------- IUnknown ----------

#pragma warning(push)
#pragma warning(disable : 4838)
// warning C4838: conversion from 'DWORD' to 'int' requires a narrowing conversion
HRESULT STDMETHODCALLTYPE CExternalDispatch::QueryInterface(REFIID riid, void **ppvObject) {
    const QITAB QITable[] = {
        QITABENT(CExternalDispatch, IDispatch),
        { 0 },
    };
    return QISearch(this, QITable, riid, ppvObject);
}
#pragma warning(pop)

STDMETHODIMP_(ULONG) CExternalDispatch::AddRef(void) {
    return InterlockedIncrement(&_ulRefs);
}

STDMETHODIMP_(ULONG) CExternalDispatch::Release(void) {
    ULONG cref = (ULONG)InterlockedDecrement(&_ulRefs);
    if (cref == 0) {
        delete this;
    }
    return cref;
}

// ---------- IDispatch ----------

#define DISP_ID_CLOSEDIALOG 100
#define DISP_ID_LOG         101
#define DISP_ID_DPI         102

STDMETHODIMP CExternalDispatch::GetIDsOfNames(
        /* [in] */ __RPC__in REFIID riid,
        /* [size_is][in] */ __RPC__in_ecount_full(cNames) LPOLESTR *rgszNames,
        /* [range][in] */ __RPC__in_range(0,16384) UINT cNames,
        /* [in] */ LCID lcid,
        /* [size_is][out] */ __RPC__out_ecount_full(cNames) DISPID *rgDispId) {
    // Assume some degree of success
    HRESULT hr = NOERROR;

    struct {
        DISPID id;
        LPCWSTR name;
    } const Mapping[] = {
        {DISP_ID_CLOSEDIALOG, L"closedialog"},
        {DISP_ID_LOG, L"log"},
        {DISP_ID_DPI, L"dpi"},
    };

    // Hardcoded mapping for this sample
    // A more usual procedure would be to use a TypeInfo
    for (UINT i = 0; i < cNames; ++i) {
        bool Found = false;
        for (int j = 0; j < ARRAYSIZE(Mapping); ++j) {
            if (CompareString(lcid, NORM_IGNOREWIDTH, Mapping[j].name, -1, rgszNames[i], -1) == CSTR_EQUAL) {
                Found = true;
                rgDispId[i] = Mapping[j].id;
                break;
            }
        }
        if (!Found) {
            // One or more are unknown so set the return code accordingly
            rgDispId[i] = DISPID_UNKNOWN;
            hr = DISP_E_UNKNOWNNAME;
        }
    }

    return hr;
}

STDMETHODIMP CExternalDispatch::Invoke(
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
        _Out_opt_  UINT *puArgErr) {
    HRESULT hr = E_INVALIDARG;

    if (wFlags & DISPATCH_METHOD) {
        switch (dispIdMember) {
        case DISP_ID_CLOSEDIALOG:
            EndDialog(GetParent(_WebOC), IDOK);
            hr = S_OK;
            break;
        case DISP_ID_LOG:
            hr = E_INVALIDARG;
            if (pDispParams && pDispParams->cArgs == 1 &&
                               pDispParams->rgvarg[0].vt == VT_BSTR) {
                Log(L"%s\n", pDispParams->rgvarg[0].bstrVal);
                hr = S_OK;
            }
            break;
        case DISP_ID_DPI:
            hr = E_INVALIDARG;
            if (pDispParams
                /*&& pDispParams->cArgs == 2
                && pDispParams->rgvarg[0].vt == VT_I4
                && pDispParams->rgvarg[1].vt == VT_I4*/) {
                RECT r;
                GetWindowRect(GetParent(_WebOC), &r);
                SetWindowPos(_WebOC, nullptr,
                    0, 0,
                    r.right - r.left - 30,
                    r.bottom - r.top - 90,
                    SWP_NOZORDER | SWP_NOMOVE);
                RedrawWindow(GetParent(_WebOC), nullptr, nullptr, RDW_ERASE | RDW_INVALIDATE);
                hr = S_OK;
            }
            break;
        default:
            hr = E_INVALIDARG;
            break;
        }
    }

    return hr;
}
