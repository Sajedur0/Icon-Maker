#include "ImageProcessor.h"
#include <fstream>
#include <gdiplus.h>
#include <shlwapi.h>

using namespace Gdiplus;

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "ole32.lib")

int ImageProcessor::GetEncoderClsid(const WCHAR *format, CLSID *pClsid) {
  UINT num = 0;
  UINT size = 0;
  GetImageEncodersSize(&num, &size);
  if (size == 0)
    return -1;

  ImageCodecInfo *pImageCodecInfo = (ImageCodecInfo *)(malloc(size));
  if (pImageCodecInfo == NULL)
    return -1;

  GetImageEncoders(num, size, pImageCodecInfo);
  for (UINT j = 0; j < num; ++j) {
    if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0) {
      *pClsid = pImageCodecInfo[j].Clsid;
      free(pImageCodecInfo);
      return j;
    }
  }
  free(pImageCodecInfo);
  return -1;
}


#pragma pack(push, 1)
struct ICONDIR {
  WORD idReserved;
  WORD idType;
  WORD idCount;
};

struct ICONDIRENTRY {
  BYTE bWidth;
  BYTE bHeight;
  BYTE bColorCount;
  BYTE bReserved;
  WORD wPlanes;
  WORD wBitCount;
  DWORD dwBytesInRes;
  DWORD dwImageOffset;
};
#pragma pack(pop)

bool ImageProcessor::CreateMultiSizeIco(const std::wstring &inputPath,
                                        const std::wstring &outputPath,
                                        const std::vector<int> &sizes) {
  // Create PNG data for all sizes
  std::vector<std::vector<BYTE>> pngData;
  CLSID pngClsid;
  GetEncoderClsid(L"image/png", &pngClsid);

  for (int size : sizes) {
    Bitmap input(inputPath.c_str());
    if (input.GetLastStatus() != Ok)
      return false;

    Bitmap scaled(size, size, PixelFormat32bppARGB);
    Graphics graphics(&scaled);
    graphics.SetInterpolationMode(InterpolationModeHighQualityBicubic);
    graphics.DrawImage(&input, 0, 0, size, size);

    IStream *pStream = NULL;
    if (CreateStreamOnHGlobal(NULL, TRUE, &pStream) != S_OK) {
      return false;
    }

    std::vector<BYTE> sizeData;
    if (scaled.Save(pStream, &pngClsid, NULL) == Ok) {
      HGLOBAL hGlobal = NULL;
      if (GetHGlobalFromStream(pStream, &hGlobal) == S_OK) {
        size_t dataSize = GlobalSize(hGlobal);
        BYTE *pData = (BYTE *)GlobalLock(hGlobal);
        if (pData) {
          sizeData.assign(pData, pData + dataSize);
          GlobalUnlock(hGlobal);
        }
      }
    }

    pStream->Release();

    if (sizeData.empty())
      return false;

    pngData.push_back(sizeData);
  }

  // Write ICO file with multiple images
  std::string outputPathStr(outputPath.begin(), outputPath.end());
  std::ofstream ofs(outputPathStr, std::ios::binary);
  if (!ofs.is_open())
    return false;

  // ICO directory header (number of images)
  ICONDIR dir = {0, 1, (WORD)sizes.size()};
  ofs.write((char *)&dir, sizeof(dir));

  // Calculate total size of directory entries
  DWORD dirEntriesSize = sizeof(ICONDIRENTRY) * sizes.size();
  DWORD currentOffset = sizeof(ICONDIR) + dirEntriesSize;

  // Write directory entries
  for (size_t i = 0; i < sizes.size(); ++i) {
    ICONDIRENTRY entry;
    entry.bWidth = (sizes[i] >= 256) ? 0 : (BYTE)sizes[i];
    entry.bHeight = (sizes[i] >= 256) ? 0 : (BYTE)sizes[i];
    entry.bColorCount = 0;
    entry.bReserved = 0;
    entry.wPlanes = 1;
    entry.wBitCount = 32;
    entry.dwBytesInRes = (DWORD)pngData[i].size();
    entry.dwImageOffset = currentOffset;

    ofs.write((char *)&entry, sizeof(entry));
    currentOffset += pngData[i].size();
  }

  // Write PNG image data for all sizes
  for (const auto &data : pngData) {
    ofs.write((char *)data.data(), data.size());
  }

  ofs.close();
  return true;
}

bool ImageProcessor::CreateSingleSizeIco(const std::wstring &inputPath,
                                         const std::wstring &outputPath,
                                         int width, int height) {
  Bitmap input(inputPath.c_str());
  if (input.GetLastStatus() != Ok)
    return false;

  Bitmap scaled(width, height, PixelFormat32bppARGB);
  Graphics graphics(&scaled);
  graphics.SetInterpolationMode(InterpolationModeHighQualityBicubic);
  graphics.DrawImage(&input, 0, 0, width, height);

  // Create PNG data in memory
  CLSID pngClsid;
  GetEncoderClsid(L"image/png", &pngClsid);

  IStream *pStream = NULL;
  if (CreateStreamOnHGlobal(NULL, TRUE, &pStream) != S_OK) {
    return false;
  }

  std::vector<BYTE> pngData;
  if (scaled.Save(pStream, &pngClsid, NULL) == Ok) {
    HGLOBAL hGlobal = NULL;
    if (GetHGlobalFromStream(pStream, &hGlobal) == S_OK) {
      size_t dataSize = GlobalSize(hGlobal);
      BYTE *pData = (BYTE *)GlobalLock(hGlobal);
      if (pData) {
        pngData.assign(pData, pData + dataSize);
        GlobalUnlock(hGlobal);
      }
    }
  }

  pStream->Release();

  if (pngData.empty())
    return false;

  // Write ICO file
  std::string outputPathStr(outputPath.begin(), outputPath.end());
  std::ofstream ofs(outputPathStr, std::ios::binary);
  if (!ofs.is_open())
    return false;

  // ICO directory header (1 image)
  ICONDIR dir = {0, 1, 1};
  ofs.write((char *)&dir, sizeof(dir));

  // Directory entry for the single image
  ICONDIRENTRY entry;
  entry.bWidth = (width >= 256) ? 0 : (BYTE)width;
  entry.bHeight = (height >= 256) ? 0 : (BYTE)height;
  entry.bColorCount = 0;
  entry.bReserved = 0;
  entry.wPlanes = 1;
  entry.wBitCount = 32;
  entry.dwBytesInRes = (DWORD)pngData.size();
  entry.dwImageOffset = sizeof(ICONDIR) + sizeof(ICONDIRENTRY);

  ofs.write((char *)&entry, sizeof(entry));

  // Write PNG image data
  ofs.write((char *)pngData.data(), pngData.size());

  ofs.close();
  return true;
}
