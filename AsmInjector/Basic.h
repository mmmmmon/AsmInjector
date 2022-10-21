#pragma once
#include <iostream>
#include <tlhelp32.h>
#include <algorithm>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include "def.h"

namespace Basic
{
	enum class injectStatus {
		success,
		open_process_no_access,
		asm_parse_fail
	};
	static std::map<injectStatus, std::wstring> injectStatusStringTable =
	{
		{injectStatus::success,L"success"},
		{injectStatus::open_process_no_access,L"open_process_no_access"},
		{injectStatus::asm_parse_fail,L"asm_parse_fail"}
	};
	auto wString2string(const std::wstring& wstr, unsigned int codepage = CP_ACP)->std::string;
	auto string2wString(const std::string& str, unsigned int codepage = CP_ACP)->std::wstring;
	auto InjectAsmToProcess(uint32_t processId, std::vector<std::wstring> asmCode)->std::pair<Basic::injectStatus, std::tuple<uint32_t, void*, std::vector<unsigned char>>>;
	auto SmallestWindowFromPoint(POINT point)->HWND;
	auto SuspendProcess(HANDLE processHandle) -> bool;
	auto ResumeProcess(HANDLE processHandle) -> bool;
}