#pragma once
#include <windows.h>
#include <gdiplus.h>
#include <vector>
#include <string>

using namespace Gdiplus;

struct IconSize {
    int width;
    int height;
    bool selected;
};

class ImageProcessor {
public:
    static bool ResizeAndSaveAsPng(const std::wstring& inputPath, const std::wstring& outputPath, int width, int height);
    static bool CreateSingleSizeIco(const std::wstring& inputPath, const std::wstring& outputPath, int width, int height);
private:
    static int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
};