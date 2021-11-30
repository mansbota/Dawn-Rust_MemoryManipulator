#pragma once
#include <Windows.h>
#include <string>
#include "Utils.h"
#include "WinException.h"
#include "Crypter.h"
#include "Offsets.h"

class Client
{
	HANDLE pHandle;

public:

	Client(std::wstring_view processName) {

		ULONG processId = getProcessID(processName);

		if (!processId)
			throw std::runtime_error(String("Launch via client."));

		pHandle = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processId);

		if (!pHandle)
			throw WINLASTEXCEPT;

		Sleep(1000);

		HANDLE pipeHandle = CreateFileA(String("\\\\.\\pipe\\mojapipa"), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if (pipeHandle == INVALID_HANDLE_VALUE)
			throw WINLASTEXCEPT;

		DWORD bytesRead{};

		if (!ReadFile(pipeHandle, &offset, sizeof(offset), &bytesRead, nullptr))
			throw WINLASTEXCEPT;

		CloseHandle(pipeHandle);

		if (offset.gom == 0)
			throw std::runtime_error("Error");
	}

	~Client() {

		if (pHandle)
			CloseHandle(pHandle);
	}

	bool processActive() const {

		DWORD exitStatus{};

		return GetExitCodeProcess(pHandle, &exitStatus) && exitStatus == STILL_ACTIVE;
	}
};

