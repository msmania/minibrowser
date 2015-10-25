//
// main.cpp
//

#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include "resource.h"
#include "minihost.h"

HRESULT OnCommand(HWND Dlg, WORD Cmd) {
    HRESULT hr = E_INVALIDARG;
    CComPtr<MiniBrowserSite> BrowserSite(MiniBrowserSite::GetFromHWND(GetDlgItem(Dlg, IDC_WEBOC)));
    if (BrowserSite != nullptr) {
        WCHAR Url[MAX_PATH];
        switch (Cmd) {
        case IDC_GO:
            if (!GetDlgItemText(Dlg, IDC_URL, Url, MAX_PATH)) {
                WCHAR ModuleName[MAX_PATH];
                if  (GetModuleFileName(nullptr, ModuleName, sizeof(ModuleName))) {
                    StringCchPrintf(Url, MAX_PATH, L"res://%s/bounce.htm", ModuleName);
                }
            }
            hr = BrowserSite->Navigate(Url);
            break;
        case IDC_STOP:
            hr = BrowserSite->Stop();
            break;
        case IDC_REFRESH:
            hr = BrowserSite->Refresh(REFRESH_COMPLETELY);
            break;
        case IDC_INFO:
            hr = BrowserSite->DumpInfo();
            break;
        }
    }
    return hr;
}

INT_PTR CALLBACK DlgProc(HWND Dlg, UINT Msg, WPARAM w, LPARAM l) {
    switch (Msg) {
    case WM_INITDIALOG:
        return 1;

    case WM_COMMAND:
        if (LOWORD(w) == IDCANCEL) {
            EndDialog(Dlg, LOWORD(w));
        }
        else {
            OnCommand(Dlg, LOWORD(w));
        }
        break;
    }
    return 0;
}

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPWSTR lpCmdLine, int nCmdShow) {
    OleInitialize(nullptr);

    WNDCLASSEXA wc;
    ZeroMemory(&wc, sizeof(WNDCLASSEX));
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hInstance = hInstance;
    wc.lpfnWndProc = MiniBrowserSite::MiniHostProc;
    wc.lpszClassName = MINIHOST;

    if (RegisterClassExA(&wc)) {
        DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc);
    }

    return 0;
}
