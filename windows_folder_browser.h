#pragma once

#include <filesystem>

namespace ghassanpl
{
	struct windows_browse_for_folder_parameters
	{
		void* window_handle = nullptr;
		std::string_view title;
	};

	std::filesystem::path windows_browse_for_folder(windows_browse_for_folder_parameters const& params);

}