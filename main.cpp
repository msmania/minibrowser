//
// main.cpp
//

#include <windows.h>
#include "resource.h"
#include "minihost.h"

INT_PTR CALLBACK DlgProc(HWND Dlg, UINT Msg, WPARAM w, LPARAM l) {
    switch (Msg) {
    case WM_INITDIALOG:
        return 1;

    case WM_COMMAND:
        switch (LOWORD(w)) {
        case IDOK:
            PostMessage(GetDlgItem(Dlg, IDC_WEBOC), WEBOC_START, 0, 0);
            break;
        case IDCANCEL:
            EndDialog(Dlg, LOWORD(w));
            break;
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
