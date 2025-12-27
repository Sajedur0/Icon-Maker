#include "ImageProcessor.h"
#include <windows.h>
#include <commctrl.h>
#include <gdiplus.h>
#include <shlobj.h>
#include <string>
#include <vector>

// Windows version definitions for proper API support
#ifndef WINVER
#define WINVER 0x0601  // Windows 7
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601  // Windows 7
#endif
#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x0601  // Windows 7
#endif
#ifndef _WIN32_IE
#define _WIN32_IE 0x0700  // IE 7.0
#endif

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "gdiplus.lib")
#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// English Translations
const wchar_t *STR_TITLE = L"Icon Maker - Professional Icon Converter";
const wchar_t *STR_SELECT_IMAGE = L"Select Image File";
const wchar_t *STR_CHOOSE_SIZES = L"Choose Icon Sizes:";
const wchar_t *STR_EXPORT_FORMAT = L"Export Format:";
const wchar_t *STR_CONVERT_BTN = L"Convert Now";
const wchar_t *STR_SUCCESS = L"Conversion completed successfully!\nOutput folder created.";
const wchar_t *STR_ERROR = L"Sorry, an error occurred during conversion.";
const wchar_t *STR_BROWSE_BTN = L"Browse...";
const wchar_t *STR_NO_IMAGE = L"Please select an image file first.";
const wchar_t *STR_NO_SIZE = L"Please select at least one size.";
const wchar_t *STR_NO_FORMAT = L"Please select at least one output format.";
const wchar_t *STR_ABOUT_BTN = L"About";
const wchar_t *STR_ABOUT_TITLE = L"About Icon Maker";
const wchar_t *STR_ABOUT_DEVELOPER = L"Developer: Sajedur R. Roni";
const wchar_t *STR_ABOUT_APP = L"Icon Maker - Professional Icon Converter\nVersion 1.0";
const wchar_t *STR_GITHUB_BTN = L"Visit GitHub";
const wchar_t *STR_GITHUB_URL = L"https://github.com/Sajedur0";

struct SizeOption {
  int size;
  HWND hwnd;
};

std::vector<SizeOption> g_sizeOptions = {{16, NULL},  {24, NULL}, {32, NULL},
                                         {48, NULL},  {64, NULL}, {128, NULL},
                                         {256, NULL}, {512, NULL}};

HWND g_hInPath, g_hIcoCheck, g_hPngCheck;
std::wstring g_inputPath;
HWND g_hAboutDlg = NULL;

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
  }
}

void OpenGitHubLink(HWND hwnd) {
  ShellExecuteW(hwnd, L"open", STR_GITHUB_URL, NULL, NULL, SW_SHOWNORMAL);
}

LRESULT CALLBACK AboutDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
  switch (message) {
  case WM_COMMAND:
    switch (LOWORD(wParam)) {
    case 101: // GitHub button
      OpenGitHubLink(hDlg);
      break;
    case 102: // OK button
      DestroyWindow(hDlg);
      break;
    }
    break;
    
  case WM_CLOSE:
    DestroyWindow(hDlg);
    break;
    
  case WM_DESTROY:
    PostQuitMessage(0);
    break;
  }
  return DefWindowProcW(hDlg, message, wParam, lParam);
}

void ShowAboutDialog(HWND hwnd) {
  // Register About dialog window class
  WNDCLASSW wc = {0};
  wc.lpfnWndProc = AboutDialogProc;
  wc.hInstance = GetModuleHandle(NULL);
  wc.hbrBackground = CreateSolidBrush(RGB(240, 240, 240));
  wc.lpszClassName = L"AboutDialogClass";
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  RegisterClassW(&wc);
  
  // Create a simple modal dialog window
  HWND hAboutDlg = CreateWindowExW(WS_EX_DLGMODALFRAME, L"AboutDialogClass", STR_ABOUT_TITLE,
                                   WS_POPUP | WS_BORDER | WS_CAPTION | WS_SYSMENU,
                                   CW_USEDEFAULT, CW_USEDEFAULT, 350, 200,
                                   hwnd, NULL, GetModuleHandle(NULL), NULL);
  
  if (hAboutDlg) {
    // Create regular font for most elements
    HFONT hFont = CreateFontW(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                              ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                              DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
    
    // Create bold font for developer name
    HFONT hBoldFont = CreateFontW(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                  ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                  DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
    
    // Developer info label (bold)
    HWND hDevLabel = CreateWindowW(L"STATIC", STR_ABOUT_DEVELOPER, 
                                   WS_VISIBLE | WS_CHILD | SS_CENTER,
                                   20, 20, 310, 30, hAboutDlg, NULL, GetModuleHandle(NULL), NULL);
    SendMessage(hDevLabel, WM_SETFONT, (WPARAM)hBoldFont, TRUE);
    
    // App info label (normal)
    HWND hAppLabel = CreateWindowW(L"STATIC", STR_ABOUT_APP,
                                   WS_VISIBLE | WS_CHILD | SS_CENTER,
                                   20, 60, 310, 40, hAboutDlg, NULL, GetModuleHandle(NULL), NULL);
    SendMessage(hAppLabel, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    // GitHub button
    HWND hGitHubBtn = CreateWindowW(L"BUTTON", STR_GITHUB_BTN,
                                    WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                                    100, 110, 150, 35, hAboutDlg, (HMENU)101, GetModuleHandle(NULL), NULL);
    SendMessage(hGitHubBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    // OK button
    HWND hOKBtn = CreateWindowW(L"BUTTON", L"OK",
                                WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                260, 110, 70, 35, hAboutDlg, (HMENU)102, GetModuleHandle(NULL), NULL);
    SendMessage(hOKBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    // Center dialog on parent
    RECT rcParent, rcDlg;
    GetWindowRect(hwnd, &rcParent);
    SetWindowPos(hAboutDlg, NULL, 0, 0, 350, 200, SWP_NOMOVE | SWP_NOZORDER);
    GetWindowRect(hAboutDlg, &rcDlg);
    int x = rcParent.left + (rcParent.right - rcParent.left - (rcDlg.right - rcDlg.left)) / 2;
    int y = rcParent.top + (rcParent.bottom - rcParent.top - (rcDlg.bottom - rcDlg.top)) / 2;
    SetWindowPos(hAboutDlg, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    
    ShowWindow(hAboutDlg, SW_SHOWNORMAL);
    SetFocus(hGitHubBtn); // Set initial focus
    
    // Message loop for dialog
    MSG msg;
    while (IsWindow(hAboutDlg)) {
      if (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
      } else {
        break;
      }
    }
    
    DeleteObject(hFont);
    DeleteObject(hBoldFont);
  }
}

void ProcessConversion(HWND hwnd) {
  if (g_inputPath.empty()) {
    MessageBoxW(hwnd, STR_NO_IMAGE, L"Error", MB_ICONERROR);
    return;
  }

  // Get selected sizes
  std::vector<int> selectedSizes;
  for (auto &opt : g_sizeOptions) {
    if (SendMessage(opt.hwnd, BM_GETCHECK, 0, 0) == BST_CHECKED) {
      selectedSizes.push_back(opt.size);
    }
  }

  if (selectedSizes.empty()) {
    MessageBoxW(hwnd, STR_NO_SIZE, L"Error", MB_ICONERROR);
    return;
  }

  bool doIco = SendMessage(g_hIcoCheck, BM_GETCHECK, 0, 0) == BST_CHECKED;
  bool doPng = SendMessage(g_hPngCheck, BM_GETCHECK, 0, 0) == BST_CHECKED;

  if (!doIco && !doPng) {
    MessageBoxW(hwnd, STR_NO_FORMAT, L"Error", MB_ICONERROR);
    return;
  }

  // Create folder
  std::wstring folderName = g_inputPath;
  size_t lastDot = folderName.find_last_of(L".");
  if (lastDot != std::wstring::npos)
    folderName = folderName.substr(0, lastDot);
  folderName += L"_Converted";

  CreateDirectoryW(folderName.c_str(), NULL);

  bool success = true;

  // Export ICO files (each size separately)
  if (doIco) {
    for (auto &size : selectedSizes) {
      std::wstring icoPath = folderName + L"\\icon_" + std::to_wstring(size) +
                             L"x" + std::to_wstring(size) + L".ico";
      if (!ImageProcessor::CreateSingleSizeIco(g_inputPath, icoPath, size, size)) {
        success = false;
      }
    }
  }

  // Export PNG files (each size separately)
  if (doPng) {
    for (auto &size : selectedSizes) {
      std::wstring pngPath = folderName + L"\\image_" + std::to_wstring(size) +
                             L"x" + std::to_wstring(size) + L".png";
      if (!ImageProcessor::ResizeAndSaveAsPng(g_inputPath, pngPath, size, size)) {
        success = false;
      }
    }
  }

  if (success) {
    MessageBoxW(hwnd, STR_SUCCESS, L"Success", MB_OK | MB_ICONINFORMATION);
    ShellExecuteW(NULL, L"explore", folderName.c_str(), NULL, NULL,
                  SW_SHOWNORMAL);
  } else {
    MessageBoxW(hwnd, STR_ERROR, L"Error", MB_ICONERROR);
  }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
  switch (msg) {
  case WM_CREATE: {
    // Create modern font
    HFONT hFont = CreateFontW(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                              ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                              DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

    // Title label
    HWND lbl = CreateWindowW(L"STATIC", STR_SELECT_IMAGE, WS_VISIBLE | WS_CHILD,
                             20, 20, 300, 20, hwnd, NULL, NULL, NULL);
    SendMessage(lbl, WM_SETFONT, (WPARAM)hFont, TRUE);

    // File path input
    g_hInPath = CreateWindowW(L"EDIT", L"",
                              WS_VISIBLE | WS_CHILD | WS_BORDER |
                                  ES_AUTOHSCROLL | ES_READONLY,
                              20, 45, 340, 25, hwnd, NULL, NULL, NULL);
    SendMessage(g_hInPath, WM_SETFONT, (WPARAM)hFont, TRUE);

    // Browse button
    HWND btnBrowse = CreateWindowW(L"BUTTON", STR_BROWSE_BTN, WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                                   365, 45, 80, 25, hwnd, (HMENU)1, NULL, NULL);
    SendMessage(btnBrowse, WM_SETFONT, (WPARAM)hFont, TRUE);

    // Sizes label
    HWND lblSizes = CreateWindowW(L"STATIC", STR_CHOOSE_SIZES, WS_VISIBLE | WS_CHILD, 20,
                      90, 300, 20, hwnd, NULL, NULL, NULL);
    SendMessage(lblSizes, WM_SETFONT, (WPARAM)hFont, TRUE);

    // Size checkboxes
    int x = 20, y = 115;
    for (int i = 0; i < g_sizeOptions.size(); ++i) {
      std::wstring label = std::to_wstring(g_sizeOptions[i].size) + L" × " +
                           std::to_wstring(g_sizeOptions[i].size);
      g_sizeOptions[i].hwnd = CreateWindowW(
          L"BUTTON", label.c_str(), WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, x,
          y, 100, 25, hwnd, NULL, NULL, NULL);
      SendMessage(g_sizeOptions[i].hwnd, WM_SETFONT, (WPARAM)hFont, TRUE);
      if (g_sizeOptions[i].size <= 256)
        SendMessage(g_sizeOptions[i].hwnd, BM_SETCHECK, BST_CHECKED, 0);

      x += 110;
      if ((i + 1) % 4 == 0) {
        x = 20;
        y += 30;
      }
    }

    // Format label
    HWND lblFmt = CreateWindowW(L"STATIC", STR_EXPORT_FORMAT, WS_VISIBLE | WS_CHILD, 20,
                      y + 10, 300, 20, hwnd, NULL, NULL, NULL);
    SendMessage(lblFmt, WM_SETFONT, (WPARAM)hFont, TRUE);

    // Format checkboxes
    g_hIcoCheck = CreateWindowW(L"BUTTON", L"ICO (Single-size per file)",
                                WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 20,
                                y + 35, 250, 25, hwnd, NULL, NULL, NULL);
    SendMessage(g_hIcoCheck, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(g_hIcoCheck, BM_SETCHECK, BST_CHECKED, 0);

    g_hPngCheck = CreateWindowW(L"BUTTON", L"PNG (Individual)",
                                WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 280,
                                y + 35, 150, 25, hwnd, NULL, NULL, NULL);
    SendMessage(g_hPngCheck, WM_SETFONT, (WPARAM)hFont, TRUE);

    // Convert button
    HWND btnConvert = CreateWindowW(
        L"BUTTON", STR_CONVERT_BTN, WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        20, y + 80, 340, 40, hwnd, (HMENU)2, NULL, NULL);
    SendMessage(btnConvert, WM_SETFONT, (WPARAM)hFont, TRUE);

    // About button
    HWND btnAbout = CreateWindowW(
        L"BUTTON", STR_ABOUT_BTN, WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        365, y + 80, 80, 40, hwnd, (HMENU)3, NULL, NULL);
    SendMessage(btnAbout, WM_SETFONT, (WPARAM)hFont, TRUE);

    break;
  }
  case WM_COMMAND:
    if (LOWORD(wp) == 1)
      BrowseFile(hwnd);
    if (LOWORD(wp) == 2)
      ProcessConversion(hwnd);
    if (LOWORD(wp) == 3)
      ShowAboutDialog(hwnd);
    break;
  case WM_DESTROY:
    PostQuitMessage(0);
    break;
  default:
    return DefWindowProcW(hwnd, msg, wp, lp);
  }
  return 0;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmd, int nShow) {
  using namespace Gdiplus;
  
  GdiplusStartupInput gdiplusStartupInput;
  ULONG_PTR gdiplusToken;
  GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

  WNDCLASSW wc = {0};
  wc.lpfnWndProc = WndProc;
  wc.hInstance = hInst;
  wc.hbrBackground = CreateSolidBrush(RGB(240, 240, 240)); // Light gray background
  wc.lpszClassName = L"IconMakerClass";
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  RegisterClassW(&wc);

  HWND hwnd = CreateWindowW(
      wc.lpszClassName, STR_TITLE,
      WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, CW_USEDEFAULT,
      CW_USEDEFAULT, 480, 420, NULL, NULL, hInst, NULL);
  ShowWindow(hwnd, nShow);

  MSG msg;
  while (GetMessageW(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessageW(&msg);
  }

  GdiplusShutdown(gdiplusToken);
  return (int)msg.wParam;
}
