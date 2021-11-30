#pragma once

#include <string_view>
#include <string>
#include <functional>
#include <span>

namespace ghassanpl
{
	struct windows_message_box_result;
	struct windows_message_box_params;

	enum class windows_message_box_event;

	/// Basic API

	/// Enum: windows_message_box_icon
	/// Type of icon in the message box. If not given, `Information` will be used by default.
	/// 
	/// Warning - An exclamation-point icon
	/// Error - A stop-sign icon
	/// Information - An icon consisting of a lowercase letter i in a circle
	/// Security - A shield icon
	enum class windows_message_box_icon
	{
		Warning = -1,
		Error = -2,
		Information = -3,
		Security = -4
	};

	/// Struct: windows_message_box_result
	/// Holds the results of the windows_message_box invocation
	struct windows_message_box_result
	{
		/// Field: Failed
		/// Will be true if the message box was closed via the "X" button or failed to appear (perhaps due to argument errors)
		bool Failed = false;

		/// Field: ClickedButton
		/// The number of the button that was clicked
		size_t ClickedButton = 0;

		/// Field: CheckboxValue
		/// Whether or not the checkbox was checked
		bool CheckboxValue = false;

		explicit operator bool() const noexcept { return !Failed; }
		operator size_t() const noexcept { return Failed ? -1 : ClickedButton; }
	};

	namespace msg
	{
		/// Title of the message box window. This is "Message" by default.
		struct title { std::string_view str; };

		/// The primary instruction to the user (the main header of the message box)
		struct description { std::string_view str; };

		/// The longer description of the message (the smaller text of the message box).
		/// This text can contain hyperlinks (`<A HREF="">asd</A>`) which will trigger the callback function, if given.
		struct long_description { std::string_view str; };

		/// If a non-empty checkbox_text is given, a checkbox will be present in the message box. The message box result will specify whether or not it was checked.
		/// The callback, if present, will also be called when the checkbox state changes.
		struct checkbox_text { std::string_view str; };

		/// If a non-empty additional_info is given, a collapsible sub-section will be present in the message box, containg the text given in the additional info.
		/// This text can contain hyperlinks (`<A HREF="">asd</A>`) which will trigger the callback function, if given.
		struct additional_info { std::string_view str; };

		/// If given, the button with this name will be selected by default
		struct default_button { std::string_view str; };

		/// If given, the message box will be modal to this window. Must be intitalized with a value of type <c>static_cast&lt;void*&gt;(HWND)</c>.
		struct window_handle { void* handle; };

		static inline constexpr std::string_view ok_button[] = { "OK" };
		static inline constexpr std::string_view yes_no_buttons[] = { "Yes", "No" };
		static inline constexpr std::string_view yes_no_cancel_buttons[] = { "Yes", "No", "Cancel" };
		static inline constexpr std::string_view abort_retry_ignore_buttons[] = { "Abort", "Retry", "Ignore" };
	}

	/// Function: windows_message_box
	/// The primary internal workhorse function. Using the variadic version instead of this is preferable.
	/// 
	/// Parameters:
	///		param - Structure holding the parameters of the message box
	/// Returns:
	///		The result of the user interacting with the message box
	windows_message_box_result windows_message_box(windows_message_box_params const& param);

	/// Function: windows_message_box
	/// Primary function to display the message box
	/// 
	/// Parameters:
	///		title - Title of the message box window
	///		description - The primary instruction to the user (the main header of the message box)
	///		args - Additional parameters to tweak the message box behavior. They can be:
	/// 
	/// - one of the types in namespace msg
	/// - a <windows_message_box_icon> (::Warning, ::Error, ::Information or ::Security) specifying the type of icon to display. This is ::Information by default
	/// - a range convertible to a range of string_views - these will be used as the names of the buttons to display in the message box. The default is a single "OK" button
	/// - a size_t 0-based number of the button that should be highlighted by default. If not given, the 0th button is highlighted
	/// - a function of signature `bool(<windows_message_box_event>, uintptr_t, uintptr_t)` that will be used as a callback, and called when different events occur during the user interaction. See `windows_message_box_event` for more info
	/// Returns:
	///		The result of the user interacting with the message box. See <windows_message_box_result> for more info.
	template <typename... ARGS>
	windows_message_box_result windows_message_box(std::string_view title, std::string_view description, ARGS&&... args)
	{
		return ::ghassanpl::windows_message_box({ ::ghassanpl::msg::title{title}, ::ghassanpl::msg::description{description}, std::forward<ARGS>(args)... });
	}

	namespace msg
	{
		template <typename... ARGS>
		bool confirm(std::string_view description, ARGS&&... args)
		{
			auto result = ::ghassanpl::windows_message_box({ ::ghassanpl::msg::title{"Are you sure?"}, ::ghassanpl::msg::description{description}, ::ghassanpl::msg::yes_no_buttons, 1, std::forward<ARGS>(args)... });
			return result && result == 0;
		}
	}

	/// Class: windows_message_box_event
	enum class windows_message_box_event
	{
		DialogCreated = 0,
		ButtonClicked = 2, /// param1 = (int)button id
		LinkClicked = 3, /// param1 = (const wchar_t*)href
		DialogDestroyed = 5,
		CheckboxClicked = 8, /// param2 = (bool)checked
		HelpRequested = 9,
	};

	/// Struct: windows_message_box_params
	/// Holds all the parameters for the message box. Prefer to use the variadic version of <windows_message_box>
	struct windows_message_box_params
	{
		std::string_view title = "Message";
		windows_message_box_icon icon = windows_message_box_icon::Information;
		std::string_view description{};
		std::span<std::string_view const> buttons = msg::ok_button;
		std::vector<std::string_view> buttons_storage;
		size_t default_button = 0;
		std::string_view default_button_str{};
		std::string_view long_description{};
		std::string_view checkbox_text{};
		std::string_view additional_info{};
		std::function<bool(windows_message_box_event, uintptr_t, uintptr_t)> callback{};
		void* window_handle = nullptr;

		template <typename... ARGS>
		windows_message_box_params(ARGS&&... args)
		{
			(Set(std::forward<ARGS>(args)), ...);
		}

		void Set(msg::title param) { title = param.str; }

		void Set(windows_message_box_icon param) { icon = param; }

		void Set(msg::description param) { description = param.str; }
		void Set(std::string_view desc) { description = desc; }

		void Set(std::span<std::string_view const> param) { buttons = param; }

		template <typename T>
		requires (std::constructible_from<decltype(buttons_storage), decltype(std::begin(std::declval<T>())), decltype(std::end(std::declval<T>()))> && !std::constructible_from<decltype(buttons), T>)
		void Set(T param) { buttons_storage = std::vector<std::string_view>{ std::begin(param), std::end(param) }; buttons = buttons_storage; }

		void Set(msg::default_button param) { default_button_str = param.str; }
		void Set(size_t param) { default_button = param; }

		void Set(msg::long_description param) { long_description = param.str; }

		void Set(msg::checkbox_text param) { checkbox_text = param.str; }

		void Set(msg::additional_info param) { additional_info = param.str; }

		void Set(std::function<bool(windows_message_box_event, uintptr_t, uintptr_t)> param) { callback = std::move(param); }
		// TODO: void Set(std::function<bool(windows_message_box_event, int, std::string_view)> param) { callback = std::move(param); }

		void Set(msg::window_handle param) { window_handle = param.handle; }
	};

}