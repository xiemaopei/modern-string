﻿/* Copyright 2024 The Kingsoft's modern-string Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#pragma once

#include "ks_string.h"

namespace ks_string_util {
	//convert from ...
	MODERN_STRING_API
	ks_immutable_wstring wstring_from_u8_chars(const char8_t* p, size_t len);
	MODERN_STRING_API
	ks_immutable_wstring wstring_from_u16_chars(const char16_t* p, size_t len); //you can use ks_immutable_wstring(p, len) directly, also
	MODERN_STRING_API
	ks_immutable_wstring wstring_from_u32_chars(const char32_t* p, size_t len);

	MODERN_STRING_API
	ks_immutable_wstring wstring_from_native_chars(const char* p, size_t len);
	MODERN_STRING_API
	ks_immutable_wstring wstring_from_native_wide_chars(const wchar_t* p, size_t len);

	//convert to ...
	MODERN_STRING_API
	std::basic_string<char8_t> wstring_to_std_u8_string(const ks_wstring_view& str_view);
	MODERN_STRING_API
	std::basic_string<char16_t> wstring_to_std_u16_string(const ks_wstring_view& str_view); //you can use str.data() and str.length() directly, also
	MODERN_STRING_API
	std::basic_string<char32_t> wstring_to_std_u32_string(const ks_wstring_view& str_view);

	MODERN_STRING_API
	std::string wstring_to_std_native_string(const ks_wstring_view& str_view);
	MODERN_STRING_API
	std::wstring wstring_to_std_native_wide_string(const ks_wstring_view& str_view);

	MODERN_STRING_API
	char32_t __take_next_rune(const ks_wstring_view& str_view, size_t* pos);
}
