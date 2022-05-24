// ImageGridCtrlSample.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "ImageGridCtrlSample.h"
#include "STXImageGridCtrl.h"

#define MAX_LOADSTRING 100
#define MAX_PATH_LENGTH 4096

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
int g_maxLoadImageCount = 32;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

typedef BOOL(CALLBACK* PFNENUMFILEPROC)(LPCTSTR, DWORD_PTR);

CSTXImageGridCtrl* pImageGrid = nullptr;

void EnumFiles(LPCTSTR lpszFilter, PFNENUMFILEPROC pfnEnumFunc, DWORD_PTR dwUserData)
{
    if (pfnEnumFunc == NULL)
        return;

    TCHAR szStartPath[MAX_PATH_LENGTH];
    size_t nLen = 0;
    LPCTSTR pszLastFlash = _tcsrchr(lpszFilter, _T('\\'));
    LPCTSTR pszFilterOnly = _T("*");
    if (pszLastFlash != NULL)
    {
        pszFilterOnly = pszLastFlash + 1;
        _tcsncpy_s(szStartPath, MAX_PATH_LENGTH, lpszFilter, pszFilterOnly - lpszFilter);
        nLen = _tcslen(szStartPath);
    }
    else
    {
        _tcscpy_s(szStartPath, MAX_PATH_LENGTH, lpszFilter);
        nLen = _tcslen(szStartPath);
        if (nLen > 0 && szStartPath[nLen - 1] != _T('\\'))
        {
            szStartPath[nLen] = _T('\\');
            nLen++;
        }
    }

    TCHAR szTempPathName[MAX_PATH_LENGTH];
    _tcscpy_s(szTempPathName, MAX_PATH_LENGTH, szStartPath);
    LPTSTR pszTempPathNameFileName = szTempPathName + nLen;


    WIN32_FIND_DATA FindFileData;
    HANDLE hFind = INVALID_HANDLE_VALUE;

    TCHAR szStartFilter[MAX_PATH_LENGTH];
    _tcscpy_s(szStartFilter, MAX_PATH_LENGTH, szStartPath);
    _tcscat_s(szStartFilter, MAX_PATH_LENGTH, pszFilterOnly);
    TCHAR szSubFilter[MAX_PATH_LENGTH];

    // Find the first file in the directory.
    hFind = FindFirstFile(szStartFilter, &FindFileData);

    if (hFind == INVALID_HANDLE_VALUE)
    {
        _tprintf(TEXT("Invalid file handle. Error is %u.\n"), GetLastError());
        return;
    }
    else
    {
        if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            if (_tcscmp(FindFileData.cFileName, _T(".")) != 0 && _tcscmp(FindFileData.cFileName, _T("..")))
            {
                _tcscpy_s(szSubFilter, MAX_PATH_LENGTH, szStartPath);
                _tcscat_s(szSubFilter, MAX_PATH_LENGTH, FindFileData.cFileName);
                _tcscat_s(szSubFilter, MAX_PATH_LENGTH, _T("\\"));
                _tcscat_s(szSubFilter, MAX_PATH_LENGTH, pszFilterOnly);
                EnumFiles(szSubFilter, pfnEnumFunc, dwUserData);
            }
        }
        else
        {
            _tcscpy_s(pszTempPathNameFileName, MAX_PATH_LENGTH - nLen, FindFileData.cFileName);
            if (!pfnEnumFunc(szTempPathName, dwUserData))
            {
                FindClose(hFind);
                return;
            }
        }

        // List all the other files in the directory.
        while (FindNextFile(hFind, &FindFileData) != 0)
        {
            if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if (_tcscmp(FindFileData.cFileName, _T(".")) != 0 && _tcscmp(FindFileData.cFileName, _T("..")))
                {
                    _tcscpy_s(szSubFilter, MAX_PATH_LENGTH, szStartPath);
                    _tcscat_s(szSubFilter, MAX_PATH_LENGTH, FindFileData.cFileName);
                    _tcscat_s(szSubFilter, MAX_PATH_LENGTH, _T("\\"));
                    _tcscat_s(szSubFilter, MAX_PATH_LENGTH, pszFilterOnly);
                    EnumFiles(szSubFilter, pfnEnumFunc, dwUserData);
                }
            }
            else
            {
                _tcscpy_s(pszTempPathNameFileName, MAX_PATH_LENGTH - nLen, FindFileData.cFileName);
                if (!pfnEnumFunc(szTempPathName, dwUserData))
                {
                    FindClose(hFind);
                    return;
                }
            }
        }
        FindClose(hFind);
    }
}

BOOL CALLBACK FileEnumFunc(LPCTSTR lpszFile, DWORD_PTR dwUserData)
{
    if (g_maxLoadImageCount <= 0)
        return FALSE;

    LPCTSTR pExt = PathFindExtension(lpszFile);
    if (pExt == nullptr)
        return TRUE;

    if (_tcsicmp(pExt, _T(".JPG")) == 0)
    {
        CComPtr<IStream> spStream;
        SHCreateStreamOnFile(lpszFile, STGM_READ, &spStream);
        LPCTSTR lpszName = PathFindFileName(lpszFile);
        int nCount = pImageGrid->GetItemCount();
        pImageGrid->Internal_InsertItem(lpszName, spStream, rand() % (nCount + 1));

        --g_maxLoadImageCount;
    }

    return TRUE;
}

void FillImageGrid()
{
    TCHAR chWinDir[MAX_PATH_LENGTH];
    GetWindowsDirectory(chWinDir, MAX_PATH_LENGTH);
    _tcscat_s(chWinDir, MAX_PATH_LENGTH, _T("\\Web\\*.*"));
    EnumFiles(chWinDir, FileEnumFunc, 0);

    if (g_maxLoadImageCount > 0)
    {
        TCHAR chWinDir[MAX_PATH_LENGTH];
        GetWindowsDirectory(chWinDir, MAX_PATH_LENGTH);
        _tcscat_s(chWinDir, MAX_PATH_LENGTH, _T("\\*.*"));
        EnumFiles(chWinDir, FileEnumFunc, 0);
    }
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    CoInitialize(nullptr);
    ULONG_PTR gdiplusToken = 0;
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_IMAGEGRIDCTRLSAMPLE, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        Gdiplus::GdiplusShutdown(gdiplusToken);
        CoUninitialize();
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_IMAGEGRIDCTRLSAMPLE));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    delete pImageGrid;

    Gdiplus::GdiplusShutdown(gdiplusToken);
    CoUninitialize();
    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_IMAGEGRIDCTRLSAMPLE));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_IMAGEGRIDCTRLSAMPLE);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }
   CSTXImageGridCtrl::RegisterAnimatedTreeCtrlClass();

   RECT rcWindow;
   GetClientRect(hWnd, &rcWindow);

   pImageGrid = new CSTXImageGridCtrl();
   pImageGrid->Create(szTitle, WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
       rcWindow.left, rcWindow.top, rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top, hWnd, 1002);
   
   pImageGrid->SetItemHeight(150);
   pImageGrid->SetItemWidth(140);

   FillImageGrid();


   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_SIZE:
    {
        if (pImageGrid)
        {
            RECT rcWindow;
            GetClientRect(hWnd, &rcWindow);
            MoveWindow(pImageGrid->GetSafeHwnd(), rcWindow.left, rcWindow.top, rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top, TRUE);
        }
    }
    break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
