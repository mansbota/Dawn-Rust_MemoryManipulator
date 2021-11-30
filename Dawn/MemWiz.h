#pragma once
#include <Windows.h>
#include <string_view>
#include <tlhelp32.h>
#include <cassert>
#include <unordered_map>
#include <optional>
#include "WinException.h"

namespace Mem
{
	enum class Access_Flags : DWORD
	{
		FULL = PROCESS_ALL_ACCESS,
		READ = PROCESS_VM_READ | PROCESS_VM_OPERATION,
		WRITE = PROCESS_VM_WRITE | PROCESS_VM_OPERATION,
		READ_WRITE = READ | WRITE
	};

	class MemWiz
	{
		HANDLE m_handle;
		std::unordered_map<std::wstring, MODULEENTRY32> m_modules;
		std::wstring m_mainModuleName;
		std::uintptr_t m_mainModuleAddress;

		DWORD getProcessId(std::wstring_view processName) const;
		bool parseModules(const DWORD procId);

	public:
		MemWiz(std::wstring_view processName, std::wstring_view mainModule, Access_Flags flags = Access_Flags::FULL);
		MemWiz(const MemWiz& other);
		MemWiz& operator=(const MemWiz& other);
		MemWiz(MemWiz&& other) noexcept;
		MemWiz& operator=(MemWiz&& other) noexcept;
		~MemWiz();

		void safeCloseHandle()
		{
			if (m_handle)
				if (!CloseHandle(m_handle))
					throw WINLASTEXCEPT;
		}

		std::uintptr_t mainModule() 
		{ 
			return m_mainModuleAddress; 
		}

		std::uintptr_t getModule(const std::wstring& name)
		{ 
			return reinterpret_cast<std::uintptr_t>(m_modules.at(name).modBaseAddr);
		}

		bool processActive() const 
		{ 
			DWORD exitCode;
			return GetExitCodeProcess(m_handle, &exitCode) && exitCode == STILL_ACTIVE;
		}

		// Memory management functions
		template <class T>
		T read(const std::uintptr_t address) const noexcept;

		template <class T>
		T read(
			const std::uintptr_t address,
			const std::initializer_list<std::uintptr_t>& offsets) const noexcept;

		bool read(
			const std::uintptr_t srcAddress,
			const std::uintptr_t trgAddress,
			SIZE_T size) const noexcept;

		template <class T>
		bool write(const std::uintptr_t address, const T& value) const noexcept;

		bool patch(const std::uintptr_t address, LPCVOID bytes, const int length) const;
		bool nop(const std::uintptr_t address, const int length) const;

		std::uintptr_t readPtrs(
			std::uintptr_t address,
			const std::initializer_list<std::uintptr_t>& offsets) const noexcept;

		std::uintptr_t scan(char* base, const int size, const char* pattern, std::string_view mask) const;
		std::uintptr_t sigScan(const std::wstring& module, const char* pattern, std::string_view mask, int offsetStart);
	};

	template <class T>
	T MemWiz::read(const std::uintptr_t address) const noexcept
	{
		T object{};

		ReadProcessMemory(m_handle, (LPCVOID)address, &object, sizeof(object), nullptr);
			
		return object;
	}

	template <class T>
	T MemWiz::read(const std::uintptr_t address, const std::initializer_list<std::uintptr_t>& offsets) const noexcept
	{
		auto result = MemWiz::readPtrs(address, offsets);

		return MemWiz::read<T>(result);
	}

	template <class T>
	bool MemWiz::write(const std::uintptr_t address, const T& value) const noexcept
	{
		SIZE_T bytesWritten{};

		return WriteProcessMemory(m_handle, (LPVOID)address, &value, sizeof(value), &bytesWritten)
			&& bytesWritten == sizeof(T);
	}
}

