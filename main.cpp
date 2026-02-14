#include "ImageProcessor.h"
#include <windows.h>
#include <commctrl.h>
#include <gdiplus.h>
#include <shellapi.h>
#include <string>
#include <vector>

#ifndef WINVER
#define WINVER 0x0601
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif
#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x0601
#endif
#ifndef _WIN32_IE
#define _WIN32_IE 0x0700
#endif

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "gdiplus.lib")
#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

const wchar_t *STR_TITLE = L"Icon Maker - Professional Icon Converter";
const wchar_t *STR_SELECT_IMAGE = L"Select Image File";
const wchar_t *STR_CHOOSE_SIZES = L"Choose Icon Sizes:";
const wchar_t *STR_CONVERT_BTN = L"Convert to ICO";
const wchar_t *STR_SUCCESS = L"Conversion completed successfully!\nOutput folder created.";
const wchar_t *STR_ERROR = L"Sorry, an error occurred during conversion.";
const wchar_t *STR_BROWSE_BTN = L"Browse...";
const wchar_t *STR_NO_IMAGE = L"Please select an image file first.";
const wchar_t *STR_NO_SIZE = L"Please select at least one size.";
const wchar_t *STR_ABOUT_BTN = L"About";
const wchar_t *STR_ABOUT_TITLE = L"About Icon Maker";
const wchar_t *STR_ABOUT_DEVELOPER = L"Developer: Sajedur R. Roni";
const wchar_t *STR_ABOUT_APP = L"Icon Maker - Professional Icon Converter\nVersion 1.0";
const wchar_t *STR_GITHUB_BTN = L"Visit GitHub";
const wchar_t *STR_GITHUB_URL = L"https://github.com/Sajedur0";
const wchar_t *STR_SELECT_ALL = L"Select All";
const wchar_t *STR_CLEAR_ALL = L"Clear";
const wchar_t *STR_STATUS_READY = L"Ready: choose image and sizes, then convert.";
const wchar_t *STR_OUTPUT_INFO = L"Output: icon.ico with all selected sizes embedded";

enum ControlId {
  ID_BROWSE = 1,
  ID_CONVERT = 2,
  ID_ABOUT = 3,
  ID_SELECT_ALL = 4,
  ID_CLEAR_ALL = 5,
  ID_ABOUT_GITHUB = 101,
  ID_ABOUT_OK = 102,
  IDI_APP_ICON = 201
};

struct SizeOption {
  int size;
  HWND hwnd;
};

std::vector<SizeOption> g_sizeOptions = {{16, NULL}, {24, NULL}, {32, NULL}, {48, NULL},
                                         {64, NULL}, {128, NULL}, {256, NULL}, {512, NULL}};

HWND g_hInPath = NULL;
HWND g_hStatus = NULL;
std::wstring g_inputPath;
HFONT g_hUiFont = NULL;
HFONT g_hTitleFont = NULL;

HICON LoadAppIcon(int size) {
  HICON hIcon = (HICON)LoadImageW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(IDI_APP_ICON),
                                  IMAGE_ICON, size, size, LR_DEFAULTCOLOR | LR_SHARED);
  if (!hIcon) {
    hIcon = LoadIcon(NULL, IDI_APPLICATION);
  }
  return hIcon;
}

void BrowseFile(HWND hwnd) {
  WCHAR szFile[MAX_PATH] = {0};
  OPENFILENAMEW ofn = {0};
  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = hwnd;
  ofn.lpstrFile = szFile;
  ofn.nMaxFile = sizeof(szFile);
  ofn.lpstrFilter = L"Image Files\0*.PNG;*.JPG;*.JPEG;*.BMP;*.GIF\0All Files\0*.*\0";
  ofn.nFilterIndex = 1;
  ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

  if (GetOpenFileNameW(&ofn)) {
    g_inputPath = szFile;
    SetWindowTextW(g_hInPath, szFile);
    SetWindowTextW(g_hStatus, L"Image selected. Choose sizes and click Convert.");
  }
}

void OpenGitHubLink(HWND hwnd) {
  ShellExecuteW(hwnd, L"open", STR_GITHUB_URL, NULL, NULL, SW_SHOWNORMAL);
}

LRESULT CALLBACK AboutDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
  (void)lParam;
  switch (message) {
  case WM_COMMAND:
    switch (LOWORD(wParam)) {
    case ID_ABOUT_GITHUB:
      OpenGitHubLink(hDlg);
      break;
    case ID_ABOUT_OK:
      DestroyWindow(hDlg);
      break;
    }
    return 0;
  case WM_CLOSE:
    DestroyWindow(hDlg);
    return 0;
  }
  return DefWindowProcW(hDlg, message, wParam, lParam);
}

void ShowAboutDialog(HWND hwnd) {
  static bool aboutClassRegistered = false;
  if (!aboutClassRegistered) {
    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = AboutDialogProc;
    wc.hInstance = GetModuleHandleW(NULL);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = L"AboutDialogClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadAppIcon(32);
    wc.hIconSm = LoadAppIcon(16);
    RegisterClassExW(&wc);
    aboutClassRegistered = true;
  }

  HWND hAboutDlg = CreateWindowExW(WS_EX_DLGMODALFRAME, L"AboutDialogClass", STR_ABOUT_TITLE,
                                   WS_POPUP | WS_BORDER | WS_CAPTION | WS_SYSMENU,
                                   CW_USEDEFAULT, CW_USEDEFAULT, 360, 220,
                                   hwnd, NULL, GetModuleHandleW(NULL), NULL);
  if (!hAboutDlg) {
    return;
  }

  HFONT hFont = CreateFontW(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                            ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
  HFONT hBoldFont = CreateFontW(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

  HWND hDevLabel = CreateWindowW(L"STATIC", STR_ABOUT_DEVELOPER,
                                 WS_VISIBLE | WS_CHILD | SS_CENTER,
                                 20, 24, 320, 30, hAboutDlg, NULL, GetModuleHandleW(NULL), NULL);
  SendMessageW(hDevLabel, WM_SETFONT, (WPARAM)hBoldFont, TRUE);

  HWND hAppLabel = CreateWindowW(L"STATIC", STR_ABOUT_APP,
                                 WS_VISIBLE | WS_CHILD | SS_CENTER,
                                 20, 66, 320, 42, hAboutDlg, NULL, GetModuleHandleW(NULL), NULL);
  SendMessageW(hAppLabel, WM_SETFONT, (WPARAM)hFont, TRUE);

  HWND hGitHubBtn = CreateWindowW(L"BUTTON", STR_GITHUB_BTN,
                                  WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                                  98, 126, 160, 36, hAboutDlg, (HMENU)ID_ABOUT_GITHUB, GetModuleHandleW(NULL), NULL);
  SendMessageW(hGitHubBtn, WM_SETFONT, (WPARAM)hFont, TRUE);

  HWND hOKBtn = CreateWindowW(L"BUTTON", L"OK",
                              WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                              268, 126, 70, 36, hAboutDlg, (HMENU)ID_ABOUT_OK, GetModuleHandleW(NULL), NULL);
  SendMessageW(hOKBtn, WM_SETFONT, (WPARAM)hFont, TRUE);

  RECT rcParent, rcDlg;
  GetWindowRect(hwnd, &rcParent);
  GetWindowRect(hAboutDlg, &rcDlg);
  int x = rcParent.left + (rcParent.right - rcParent.left - (rcDlg.right - rcDlg.left)) / 2;
  int y = rcParent.top + (rcParent.bottom - rcParent.top - (rcDlg.bottom - rcDlg.top)) / 2;
  SetWindowPos(hAboutDlg, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

  EnableWindow(hwnd, FALSE);
  ShowWindow(hAboutDlg, SW_SHOWNORMAL);
  SetForegroundWindow(hAboutDlg);

  MSG msg;
  while (IsWindow(hAboutDlg) && GetMessageW(&msg, NULL, 0, 0)) {
    if (!IsDialogMessageW(hAboutDlg, &msg)) {
      TranslateMessage(&msg);
      DispatchMessageW(&msg);
    }
  }

  EnableWindow(hwnd, TRUE);
  SetForegroundWindow(hwnd);

  DeleteObject(hFont);
  DeleteObject(hBoldFont);
}

void ProcessConversion(HWND hwnd) {
  if (g_inputPath.empty()) {
    MessageBoxW(hwnd, STR_NO_IMAGE, L"Error", MB_ICONERROR);
    SetWindowTextW(g_hStatus, L"No image selected.");
    return;
  }

  std::vector<int> selectedSizes;
  for (size_t i = 0; i < g_sizeOptions.size(); ++i) {
    if (SendMessageW(g_sizeOptions[i].hwnd, BM_GETCHECK, 0, 0) == BST_CHECKED) {
      selectedSizes.push_back(g_sizeOptions[i].size);
    }
  }

  if (selectedSizes.empty()) {
    MessageBoxW(hwnd, STR_NO_SIZE, L"Error", MB_ICONERROR);
    SetWindowTextW(g_hStatus, L"No icon size selected.");
    return;
  }

  std::wstring folderName = g_inputPath;
  size_t lastDot = folderName.find_last_of(L".");
  if (lastDot != std::wstring::npos) {
    folderName = folderName.substr(0, lastDot);
  }
  folderName += L"_Converted";

  CreateDirectoryW(folderName.c_str(), NULL);

  std::wstring icoPath = folderName + L"\\icon.ico";
  bool success = ImageProcessor::CreateMultiSizeIco(g_inputPath, icoPath, selectedSizes);

  if (success) {
    SetWindowTextW(g_hStatus, L"Conversion completed successfully.");
    MessageBoxW(hwnd, STR_SUCCESS, L"Success", MB_OK | MB_ICONINFORMATION);
    ShellExecuteW(NULL, L"explore", folderName.c_str(), NULL, NULL, SW_SHOWNORMAL);
  } else {
    SetWindowTextW(g_hStatus, L"Conversion failed. Try another image.");
    MessageBoxW(hwnd, STR_ERROR, L"Error", MB_ICONERROR);
  }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
  (void)lp;
  switch (msg) {
  case WM_CREATE: {
    g_hUiFont = CreateFontW(17, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                            ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
    g_hTitleFont = CreateFontW(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                               ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                               CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

    HWND lblHeader = CreateWindowW(L"STATIC", STR_TITLE, WS_VISIBLE | WS_CHILD,
                                   20, 14, 520, 28, hwnd, NULL, NULL, NULL);
    SendMessageW(lblHeader, WM_SETFONT, (WPARAM)g_hTitleFont, TRUE);

    HWND lbl = CreateWindowW(L"STATIC", STR_SELECT_IMAGE, WS_VISIBLE | WS_CHILD,
                             20, 52, 300, 22, hwnd, NULL, NULL, NULL);
    SendMessageW(lbl, WM_SETFONT, (WPARAM)g_hUiFont, TRUE);

    g_hInPath = CreateWindowW(L"EDIT", L"",
                              WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL | ES_READONLY,
                              20, 80, 385, 30, hwnd, NULL, NULL, NULL);
    SendMessageW(g_hInPath, WM_SETFONT, (WPARAM)g_hUiFont, TRUE);

    HWND btnBrowse = CreateWindowW(L"BUTTON", STR_BROWSE_BTN,
                                   WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                                   414, 80, 110, 30, hwnd, (HMENU)ID_BROWSE, NULL, NULL);
    SendMessageW(btnBrowse, WM_SETFONT, (WPARAM)g_hUiFont, TRUE);

    HWND grpSizes = CreateWindowW(L"BUTTON", L"",
                                  WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
                                  20, 125, 504, 178, hwnd, NULL, NULL, NULL);
    SendMessageW(grpSizes, WM_SETFONT, (WPARAM)g_hUiFont, TRUE);

    HWND lblSizes = CreateWindowW(L"STATIC", STR_CHOOSE_SIZES, WS_VISIBLE | WS_CHILD,
                                  30, 143, 260, 22, hwnd, NULL, NULL, NULL);
    SendMessageW(lblSizes, WM_SETFONT, (WPARAM)g_hUiFont, TRUE);

    HWND btnSelectAll = CreateWindowW(L"BUTTON", STR_SELECT_ALL,
                                      WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                      338, 138, 90, 28, hwnd, (HMENU)ID_SELECT_ALL, NULL, NULL);
    SendMessageW(btnSelectAll, WM_SETFONT, (WPARAM)g_hUiFont, TRUE);

    HWND btnClearAll = CreateWindowW(L"BUTTON", STR_CLEAR_ALL,
                                     WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                     434, 138, 80, 28, hwnd, (HMENU)ID_CLEAR_ALL, NULL, NULL);
    SendMessageW(btnClearAll, WM_SETFONT, (WPARAM)g_hUiFont, TRUE);

    int x = 30;
    int y = 176;
    for (size_t i = 0; i < g_sizeOptions.size(); ++i) {
      std::wstring label = std::to_wstring(g_sizeOptions[i].size) + L" x " +
                           std::to_wstring(g_sizeOptions[i].size);
      g_sizeOptions[i].hwnd = CreateWindowW(L"BUTTON", label.c_str(),
                                            WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
                                            x, y, 116, 28, hwnd, NULL, NULL, NULL);
      SendMessageW(g_sizeOptions[i].hwnd, WM_SETFONT, (WPARAM)g_hUiFont, TRUE);
      if (g_sizeOptions[i].size <= 256) {
        SendMessageW(g_sizeOptions[i].hwnd, BM_SETCHECK, BST_CHECKED, 0);
      }

      x += 122;
      if ((i + 1) % 4 == 0) {
        x = 30;
        y += 34;
      }
    }

    HWND lblInfo = CreateWindowW(L"STATIC", STR_OUTPUT_INFO,
                                 WS_VISIBLE | WS_CHILD,
                                 20, 312, 504, 22, hwnd, NULL, NULL, NULL);
    SendMessageW(lblInfo, WM_SETFONT, (WPARAM)g_hUiFont, TRUE);

    g_hStatus = CreateWindowW(L"STATIC", STR_STATUS_READY,
                              WS_VISIBLE | WS_CHILD,
                              20, 338, 504, 22, hwnd, NULL, NULL, NULL);
    SendMessageW(g_hStatus, WM_SETFONT, (WPARAM)g_hUiFont, TRUE);

    HWND btnConvert = CreateWindowW(L"BUTTON", STR_CONVERT_BTN,
                                    WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                                    20, 368, 504, 46, hwnd, (HMENU)ID_CONVERT, NULL, NULL);
    SendMessageW(btnConvert, WM_SETFONT, (WPARAM)g_hUiFont, TRUE);

    HWND btnAbout = CreateWindowW(L"BUTTON", STR_ABOUT_BTN,
                                  WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                  214, 422, 110, 30, hwnd, (HMENU)ID_ABOUT, NULL, NULL);
    SendMessageW(btnAbout, WM_SETFONT, (WPARAM)g_hUiFont, TRUE);

    return 0;
  }
  case WM_COMMAND:
    if (LOWORD(wp) == ID_BROWSE) {
      BrowseFile(hwnd);
    }
    if (LOWORD(wp) == ID_CONVERT) {
      ProcessConversion(hwnd);
    }
    if (LOWORD(wp) == ID_ABOUT) {
      ShowAboutDialog(hwnd);
    }
    if (LOWORD(wp) == ID_SELECT_ALL) {
      for (size_t i = 0; i < g_sizeOptions.size(); ++i) {
        SendMessageW(g_sizeOptions[i].hwnd, BM_SETCHECK, BST_CHECKED, 0);
      }
      SetWindowTextW(g_hStatus, L"All icon sizes selected.");
    }
    if (LOWORD(wp) == ID_CLEAR_ALL) {
      for (size_t i = 0; i < g_sizeOptions.size(); ++i) {
        SendMessageW(g_sizeOptions[i].hwnd, BM_SETCHECK, BST_UNCHECKED, 0);
      }
      SetWindowTextW(g_hStatus, L"All icon sizes cleared.");
    }
    return 0;
  case WM_DESTROY:
    if (g_hUiFont) {
      DeleteObject(g_hUiFont);
      g_hUiFont = NULL;
    }
    if (g_hTitleFont) {
      DeleteObject(g_hTitleFont);
      g_hTitleFont = NULL;
    }
    PostQuitMessage(0);
    return 0;
  }

  return DefWindowProcW(hwnd, msg, wp, lp);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmd, int nShow) {
  (void)hPrev;
  (void)lpCmd;

  using namespace Gdiplus;

  GdiplusStartupInput gdiplusStartupInput;
  ULONG_PTR gdiplusToken;
  GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

  WNDCLASSEXW wc = {0};
  wc.cbSize = sizeof(wc);
  wc.lpfnWndProc = WndProc;
  wc.hInstance = hInst;
  wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
  wc.lpszClassName = L"IconMakerClass";
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hIcon = LoadAppIcon(32);
  wc.hIconSm = LoadAppIcon(16);
  RegisterClassExW(&wc);

  HWND hwnd = CreateWindowW(wc.lpszClassName, STR_TITLE,
                            WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
                            CW_USEDEFAULT, CW_USEDEFAULT, 560, 520,
                            NULL, NULL, hInst, NULL);
  SendMessageW(hwnd, WM_SETICON, ICON_BIG, (LPARAM)wc.hIcon);
  SendMessageW(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)wc.hIconSm);
  ShowWindow(hwnd, nShow);

  MSG msg;
  while (GetMessageW(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessageW(&msg);
  }

  GdiplusShutdown(gdiplusToken);
  return (int)msg.wParam;
}

