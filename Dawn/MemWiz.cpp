#include "MemWiz.h"

namespace Mem
{
	MemWiz::MemWiz(std::wstring_view processName, std::wstring_view moduleName, Access_Flags flags) :
		m_mainModuleName{ moduleName }
	{
		DWORD procId{};

		if ((procId = getProcessId(processName)) == 0)
			throw WINLASTEXCEPT;

		if (!parseModules(procId))
			throw WINLASTEXCEPT;

		if ((m_handle = OpenProcess(static_cast<DWORD>(flags), 0, procId)) == NULL)
			throw WINLASTEXCEPT;
	}

	MemWiz::MemWiz(const MemWiz& other)
	{
		if (!DuplicateHandle(GetCurrentProcess(), other.m_handle, GetCurrentProcess(), &m_handle, 0, FALSE, DUPLICATE_SAME_ACCESS))
			throw WINLASTEXCEPT;

		m_modules = other.m_modules;
		m_mainModuleName = other.m_mainModuleName;
		m_mainModuleAddress = other.m_mainModuleAddress;
	}

	MemWiz& MemWiz::operator=(const MemWiz& other)
	{
		if (this != &other)
		{
			safeCloseHandle();

			if (!DuplicateHandle(GetCurrentProcess(), other.m_handle, GetCurrentProcess(), &m_handle, 0, FALSE, DUPLICATE_SAME_ACCESS))
				throw WINLASTEXCEPT;

			m_modules = other.m_modules;
			m_mainModuleName = other.m_mainModuleName;
			m_mainModuleAddress = other.m_mainModuleAddress;
		}

		return *this;
	}

	MemWiz::MemWiz(MemWiz&& other) noexcept
	{
		m_handle = other.m_handle;
		other.m_handle = NULL;
		m_mainModuleAddress = other.m_mainModuleAddress;
		m_modules = std::move(other.m_modules);
		m_mainModuleName = std::move(other.m_mainModuleName);
	}

	MemWiz& MemWiz::operator=(MemWiz&& other) noexcept
	{
		if (this != &other)
		{
			safeCloseHandle();

			m_handle = other.m_handle;
			other.m_handle = NULL;
			m_mainModuleAddress = other.m_mainModuleAddress;
			m_modules = std::move(other.m_modules);
			m_mainModuleName = std::move(other.m_mainModuleName);
		}

		return *this;
	}

	MemWiz::~MemWiz()
	{
		safeCloseHandle();
	}

	DWORD MemWiz::getProcessId(std::wstring_view processName) const
	{
		DWORD procID{ 0 };
		HANDLE hSnap{ CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0) };

		if (hSnap != INVALID_HANDLE_VALUE)
		{
			PROCESSENTRY32 pe32;
			pe32.dwSize = sizeof(PROCESSENTRY32);

			if (Process32First(hSnap, &pe32))
			{
				do
				{
					if (processName == reinterpret_cast<wchar_t*>(pe32.szExeFile))
					{
						procID = pe32.th32ProcessID;
						break;
					}

				} while (Process32Next(hSnap, &pe32));
			}
		}
		else
			return procID;

		CloseHandle(hSnap);

		return procID;
	}

	bool MemWiz::parseModules(const DWORD procId)
	{
		HANDLE hSnap{ CreateToolhelp32Snapshot(TH32CS_SNAPMODULE32 | TH32CS_SNAPMODULE, procId) };

		if (hSnap != INVALID_HANDLE_VALUE)
		{
			MODULEENTRY32 me32;
			me32.dwSize = sizeof(MODULEENTRY32);

			if (Module32First(hSnap, &me32))
			{
				do
				{
					m_modules[me32.szModule] = me32;

					if (me32.szModule == m_mainModuleName)
						m_mainModuleAddress = reinterpret_cast<std::uintptr_t>(me32.modBaseAddr);

				} while (Module32Next(hSnap, &me32));
			}
		}
		else
			return false;

		CloseHandle(hSnap);

		return true;
	}

	bool MemWiz::patch(const std::uintptr_t address, LPCVOID bytes, const int length) const
	{
		DWORD old;

		if (!VirtualProtectEx(m_handle, (LPVOID)address, length, PAGE_EXECUTE_READWRITE, &old))
			return false;

		auto success = WriteProcessMemory(m_handle, (LPVOID)address, bytes, length, 0);

		if (!VirtualProtectEx(m_handle, (LPVOID)address, length, old, &old))
			return false;

		return success;
	}

	bool MemWiz::nop(const std::uintptr_t address, const int length) const
	{
		BYTE* nopArray = new BYTE[length];
		memset(nopArray, 0x90, length);

		auto success = patch(address, nopArray, length);

		delete[] nopArray;

		return success;
	}

	std::uintptr_t MemWiz::readPtrs(
		std::uintptr_t address,
		const std::initializer_list<std::uintptr_t>& offsets) const noexcept
	{
		for (const auto element : offsets)
			address = MemWiz::read<std::uintptr_t>(address) + element;

		return address;
	}

	bool MemWiz::read(const std::uintptr_t srcAddress, const std::uintptr_t trgAddress, SIZE_T size) const noexcept
	{
		SIZE_T bytesRead{};

		return ReadProcessMemory(m_handle, (LPCVOID)srcAddress, (LPVOID)trgAddress, size, &bytesRead)
			&& bytesRead == size;
	}

	std::uintptr_t MemWiz::scan(char* base, const int size, const char* pattern, std::string_view mask) const
	{
		int patternLength = mask.size();

		for (int i{ 0 }; i < size - patternLength; i++)
		{
			bool found{ true };

			for (int j{ 0 }; j < patternLength; j++)
			{
				if (mask[j] != '?' && pattern[j] != *(base + i + j))
				{
					found = false;
					break;
				}
			}

			if (found)
				return reinterpret_cast<std::uintptr_t>(base + i);
		}

		return 0;
	}

	std::uintptr_t MemWiz::sigScan(const std::wstring& module, const char* pattern, std::string_view mask, int offsetStart)
	{
		MODULEENTRY32 me32 = m_modules.at(module);
		
		char* moduleBytes = new char[me32.modBaseSize];

		if (!ReadProcessMemory(m_handle, (LPCVOID)me32.modBaseAddr, moduleBytes, me32.modBaseSize, 0))
			return 0;

		std::uintptr_t data = scan(moduleBytes, me32.modBaseSize, pattern, mask);
		
		std::uintptr_t offset = data - reinterpret_cast<std::uintptr_t>(moduleBytes);
		
		delete[] moduleBytes;

		return reinterpret_cast<std::uintptr_t>(me32.modBaseAddr + offset + offsetStart);
	}
}