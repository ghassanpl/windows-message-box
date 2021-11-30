#pragma once

#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

#include "windows_message_box.h"

#include <string>
#include <algorithm>
#include <iterator>
#include <locale>
#include <codecvt>

#define WIN32_LEAN_AND_MEAN
#include <shlwapi.h>
#include <shellapi.h>
#pragma comment(lib, "Shlwapi.lib")
#include <Commctrl.h>
#pragma comment(lib, "Comctl32.lib")

#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

namespace ghassanpl
{
	std::wstring utf8_to_utf16(std::string_view str)
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		return converter.from_bytes(str.data(), str.data() + str.size());
	}

	windows_message_box_result windows_message_box(windows_message_box_params const& param)
	{
		/// Set the common parameters
		TASKDIALOGCONFIG task_config = {};
		task_config.cbSize = sizeof(TASKDIALOGCONFIG);
		task_config.hwndParent = param.window_handle ? (HWND)param.window_handle : GetActiveWindow();
		task_config.hInstance = GetModuleHandle(nullptr);
		task_config.dwFlags =
			TDF_ENABLE_HYPERLINKS
			| TDF_USE_COMMAND_LINKS
			//| TDF_EXPANDED_BY_DEFAULT
			| TDF_SIZE_TO_CONTENT;
		task_config.dwCommonButtons = 0;
		task_config.hMainIcon = nullptr;

		/// No radiobuttons, no other platforms use them
		task_config.cRadioButtons = 0;
		task_config.pRadioButtons = nullptr;
		task_config.nDefaultRadioButton = 0;

		task_config.pszCollapsedControlText = nullptr;
		task_config.hFooterIcon = nullptr;
		task_config.pszFooter = nullptr;
		task_config.pszFooterIcon = nullptr;
		task_config.cxWidth = 0;

		/// Titles and descriptions
		auto title_u16 = utf8_to_utf16(param.title);
		auto description_u16 = utf8_to_utf16(param.description);
		auto long_description_u16 = utf8_to_utf16(param.long_description);
		auto checkbox_text_u16 = utf8_to_utf16(param.checkbox_text);
		auto additional_info_u16 = utf8_to_utf16(param.additional_info);

		task_config.pszWindowTitle = title_u16.c_str();
		//AssumeBetween(int(icon), int(MessageBoxIcon::Warning), int(MessageBoxIcon::Security));
		task_config.pszMainIcon = MAKEINTRESOURCEW(int(param.icon));
		task_config.pszMainInstruction = description_u16.c_str();
		if (!param.long_description.empty())
			task_config.pszContent = long_description_u16.c_str();

		if (!param.checkbox_text.empty())
			task_config.pszVerificationText = checkbox_text_u16.c_str();
		if (!param.additional_info.empty())
		{
			task_config.pszExpandedControlText = L"Additional Information";
			task_config.pszExpandedInformation = additional_info_u16.c_str();
		}

		/// Callback
		if (param.callback)
		{
			task_config.pfCallback = [](HWND hwnd, UINT uNotification, WPARAM wParam, LPARAM lParam, LONG_PTR dwRefData) {
				auto ptr = (decltype(param.callback)*)dwRefData;
				return ptr->operator()((windows_message_box_event)uNotification, wParam, lParam) ? S_OK : S_FALSE;
			};
			task_config.lpCallbackData = (LONG_PTR)&param.callback;
		}

		/// Buttons
		std::vector<TASKDIALOG_BUTTON> button_vector;
		std::vector<std::wstring> button_texts;
		std::transform(param.buttons.begin(), param.buttons.end(), std::back_inserter(button_texts), utf8_to_utf16);
		std::transform(button_texts.begin(), button_texts.end(), std::back_inserter(button_vector), [id = 0](auto& wstr) mutable{ return TASKDIALOG_BUTTON{ id++, wstr.c_str() }; });
		/*int id = 0;
		for (auto& button : button_texts)
			button_vector.push_back({ id++, button.c_str() });
			*/

		task_config.cButtons = (UINT)button_vector.size();
		task_config.pButtons = button_vector.data();
		task_config.nDefaultButton = (int)param.default_button;

		int clicked_id = 0;
		BOOL checkbox_value = false;
		InitCommonControls();
		auto result = TaskDialogIndirect(&task_config, &clicked_id, nullptr, &checkbox_value);
		if (result != S_OK)
		{
			return { true };
		}

		return{ false, size_t(clicked_id), checkbox_value != 0 };
	}

}