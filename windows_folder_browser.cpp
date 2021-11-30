#include "windows_folder_browser.h"

#define WIN32_LEAN_AND_MEAN
#include <shlwapi.h>
#include <ShlObj.h>
#include <shellapi.h>
#pragma comment(lib, "Shlwapi.lib")
#include <Commctrl.h>
#pragma comment(lib, "Comctl32.lib")

namespace ghassanpl
{
  /// TODO:
  /// For Windows Vista or later, it is recommended that you use IFileDialog with the FOS_PICKFOLDERS option rather than the SHBrowseForFolder function.
  /// This uses the Open Files dialog in pick folders modeand is the preferred implementation.
  
  std::wstring utf8_to_utf16(std::string_view str);

  std::filesystem::path windows_browse_for_folder(windows_browse_for_folder_parameters const& params)
  {
    std::filesystem::path result;

    std::ignore = CoInitialize(nullptr);

    BROWSEINFOW folderinfo{};
    folderinfo.hwndOwner = params.window_handle ? (HWND)params.window_handle : GetActiveWindow();
    folderinfo.pidlRoot = NULL;
    TCHAR dbuf[MAX_PATH] = {};
    folderinfo.pszDisplayName = dbuf;
    auto title_u16 = utf8_to_utf16(params.title);
    folderinfo.lpszTitle = title_u16.c_str();
    folderinfo.ulFlags = BIF_USENEWUI | BIF_VALIDATE | BIF_SHAREABLE;
    folderinfo.lpfn = [](HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData) -> int {
      if (uMsg == BFFM_VALIDATEFAILED)
        return 1;
      return 0;
    };

    auto pidl = SHBrowseForFolderW(&folderinfo);

    if (pidl)
    {
      TCHAR buf[MAX_PATH] = {};
      SHGetPathFromIDList(pidl, buf);
      result = buf;
      CoTaskMemFree(pidl);
    }
    
    CoUninitialize();

    return result;
	}
}