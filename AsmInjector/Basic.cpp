#include "pch.h"
#include "Basic.h"
#include <bcrypt.h>

#include "libs\keystone\include\keystone.h"
#ifdef WIN32
#ifdef DEBUG
#pragma comment (lib,"libs/keystone/x64/keystoned.lib")
#else
#pragma comment (lib,"libs/keystone/x64/keystone.lib")
#endif
#else
#ifdef DEBUG
#pragma comment (lib,"libs/keystone/x86/keystoned.lib")
#else
#pragma comment (lib,"libs/keystone/x86/keystone.lib")
#endif
#endif
#pragma comment (lib,"ntdll.lib")

#ifdef __cplusplus
extern "C" 
{
#endif

	NTSTATUS NtSuspendProcess(HANDLE ProcessHandle);
	NTSTATUS NtResumeProcess(HANDLE ProcessHandle);

#ifdef __cplusplus
}
#endif

namespace Basic
{
	auto wString2string(const std::wstring& wstr, unsigned int codepage) -> std::string
	{
		const auto nwstrlen = ::WideCharToMultiByte(codepage, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
		if (nwstrlen > 0)
		{
			std::string str(nwstrlen, '\0');
			::WideCharToMultiByte(codepage, 0, wstr.c_str(), -1, (LPSTR)str.c_str(), nwstrlen, nullptr, nullptr);
			return std::move(str);
		}
		return std::string();
	}
	auto string2wString(const std::string& str, unsigned int codepage) -> std::wstring
	{
		const auto nstrlen = ::MultiByteToWideChar(codepage, 0, str.c_str(), -1, nullptr, 0);
		if (nstrlen > 0)
		{
			std::wstring wstr(nstrlen, L'\0');
			::MultiByteToWideChar(codepage, 0, str.c_str(), -1, (LPWSTR)wstr.c_str(), nstrlen);
			return std::move(wstr);
		}
		return std::wstring();
	}
	auto getProcessTable() -> std::map<std::wstring, uint32_t>
	{
		PROCESSENTRY32 procEntry32 = { 0 };
		std::map<std::wstring, uint32_t> processTable;
		procEntry32.dwSize = sizeof(procEntry32);

		const auto processSnapHandle = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (processSnapHandle == INVALID_HANDLE_VALUE) {
			return processTable;
		}
		auto result = Process32First(processSnapHandle, &procEntry32);
		while (result)
		{
			processTable[procEntry32.szExeFile] = procEntry32.th32ProcessID;
			result = Process32Next(processSnapHandle, &procEntry32);
		}
		::CloseHandle(processSnapHandle);
		return processTable;
	}
	auto SuspendProcess(HANDLE processHandle) -> bool
	{
		const auto asd = NtSuspendProcess(processHandle);
		return NtSuspendProcess(processHandle) == STATUS_SUCCESS;
	}
	auto ResumeProcess(HANDLE processHandle) -> bool
	{
		return NtResumeProcess(processHandle) == STATUS_SUCCESS;
	}
	auto AsmToHex(bool isWow64, uint64_t addr,std::vector<std::wstring> asmCode) -> std::vector<unsigned char>
	{
		HANDLE processHandle = nullptr;
		std::vector<unsigned char> hexCode;
		if (asmCode.size() == 0)
		{
			return hexCode;
		}
		do
		{
			for (const auto lineCode : asmCode)
			{
				ks_engine* ksEngine = nullptr;
				unsigned char* ksOpCode = nullptr;
				size_t ksOpCodeSize = 0;
				size_t ksStatCount = 0;
				
				const auto ksMode = isWow64 ? KS_MODE_32 : KS_MODE_64;
				const auto ksStatus = ks_open(KS_ARCH_X86, ksMode, &ksEngine);
				const auto lineCodeNarrow = wString2string(lineCode.c_str());
				if (KS_ERR_OK != ksStatus)
				{
					hexCode.clear();
					return hexCode;
				}
				if (-1 == ks_asm(ksEngine, lineCodeNarrow.c_str(), addr, &ksOpCode, &ksOpCodeSize, &ksStatCount))
				{
					const auto error = ks_errno(ksEngine);
					hexCode.clear();
					return hexCode;
				}
				for (unsigned int i = 0; i < ksOpCodeSize; i++)
				{
					hexCode.push_back(ksOpCode[i]);
				}
				if (nullptr != ksEngine)
				{
					ks_close(ksEngine);
				}
				if (nullptr != ksOpCode)
				{
					ks_free(ksOpCode);
				}
			}
		} while (false);
		if (nullptr != processHandle)
		{
			::CloseHandle(processHandle);
		}
		return hexCode;
	}
	auto InjectAsmToProcess(uint32_t processId, std::vector<std::wstring> asmCode) -> std::pair<Basic::injectStatus, std::tuple<uint32_t, void*, std::vector<unsigned char>>>
	{
		Basic::injectStatus status = injectStatus::success;
		BOOL isWow64;
		HANDLE processHandle = nullptr;
		void * remoteExecuteAddr = nullptr;
		std::tuple<uint32_t, void*, std::vector<unsigned char>> memBlock;
		do
		{
			processHandle = ::OpenProcess(PROCESS_QUERY_INFORMATION| PROCESS_CREATE_THREAD| PROCESS_QUERY_INFORMATION| PROCESS_VM_OPERATION| PROCESS_VM_WRITE| PROCESS_VM_READ, false, processId);
			if (nullptr == processHandle)
			{
				status = injectStatus::open_process_no_access;
				break;
			}
			if (false == IsWow64Process(processHandle, &isWow64))
			{
				status = injectStatus::open_process_no_access;
				break;
			}
			auto asmHex = AsmToHex(isWow64,0 ,asmCode);
			if (0 == asmHex.size())
			{
				status = injectStatus::asm_parse_fail;
				break;
			}
			remoteExecuteAddr = ::VirtualAllocEx(processHandle, nullptr, asmHex.size() +1, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
			if (nullptr == remoteExecuteAddr)
			{
				status = injectStatus::open_process_no_access;
				break;
			}
			asmHex = AsmToHex(isWow64,(uint64_t)remoteExecuteAddr, asmCode);
			asmHex.push_back(0xC3);
			if (FALSE == ::WriteProcessMemory(processHandle, remoteExecuteAddr, asmHex.data(), asmHex.size(), nullptr))
			{
				status = injectStatus::open_process_no_access;
				break;
			}
			const auto threadHandle = ::CreateRemoteThread(processHandle,nullptr,0,(LPTHREAD_START_ROUTINE)remoteExecuteAddr, nullptr, 0, 0);
			if (nullptr == threadHandle)
			{
				status = injectStatus::open_process_no_access;
				break;
			}
			::CloseHandle(threadHandle);
			std::get<0>(memBlock) = processId;
			std::get<1>(memBlock) = remoteExecuteAddr;
			std::get<2>(memBlock) = asmHex;
		} while (false);
		if (injectStatus::success != status && nullptr != processHandle && nullptr != remoteExecuteAddr)
		{
			::VirtualFreeEx(processHandle, remoteExecuteAddr, 0, MEM_RELEASE);
		}
		if (nullptr != processHandle)
		{
			::CloseHandle(processHandle);
		}
		return { status ,memBlock};
	}
	auto SmallestWindowFromPoint(POINT point) -> HWND
	{
		RECT rect, tempRect;
		HWND tempHwnd;
		auto hwnd = ::WindowFromPoint(point);
		if (hwnd != nullptr) {
			::GetWindowRect(hwnd, &rect);
			const auto hParent = ::GetParent(hwnd);
			if (hParent != nullptr) {
				tempHwnd = hwnd;
				do {
					tempHwnd = ::GetWindow(tempHwnd, GW_HWNDNEXT);
					::GetWindowRect(tempHwnd, &tempRect);
					if (::PtInRect(&tempRect, point) && ::GetParent(tempHwnd) == hParent && ::IsWindowVisible(tempHwnd)) {
						if (((tempRect.right - tempRect.left) * (tempRect.bottom - tempRect.top)) < ((rect.right - rect.left) * (rect.bottom - rect.top))) {
							hwnd = tempHwnd;
							::GetWindowRect(hwnd, &rect);
						}
					}
				} while (tempHwnd != nullptr);
			}
		}
		return hwnd;
	}
}