//
// main.cpp
//

#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <ShellScalingApi.h>
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
        case IDC_UPDATE_DPI:
            hr = BrowserSite->UpdateDpi();
            break;
        }
    }
    return hr;
}

INT_PTR CALLBACK DlgProc(HWND Dlg, UINT Msg, WPARAM w, LPARAM l) {
    switch (Msg) {
    case WM_INITDIALOG:
        SetDlgItemText(Dlg, IDC_URL, L"res://minib.exe/test.htm");
        return 1;

    case WM_COMMAND:
        if (LOWORD(w) == IDCANCEL) {
            EndDialog(Dlg, LOWORD(w));
        }
        else {
            OnCommand(Dlg, LOWORD(w));
        }
        break;

    case WM_WINDOWPOSCHANGED:
    case WM_DISPLAYCHANGE:
        // OnCommand(Dlg, IDC_UPDATE_DPI);
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

    if (lpCmdLine[0] >= L'0' && lpCmdLine[0] <= L'2') {
        SetProcessDpiAwareness((PROCESS_DPI_AWARENESS)(lpCmdLine[0] - L'0'));
    }

    if (RegisterClassExA(&wc)) {
        DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc);
    }

    return 0;
}
