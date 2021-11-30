#pragma once

#include <Windows.h>
#include "WinException.h"
#include <mutex>
#include "Crypter.h"
#include "Utils.h"

#define OPERATION_READ 0x1
#define OPERATION_WRITE 0x2
#define OPERATION_GET_X64_MODULE 0x3
#define OPERATION_GET_X32_MODULE 0x4
#define OPERATION_CONNECT 0x5
#define OPERATION_DISCONNECT 0x6

typedef struct _INPUT_INFO {

	CHAR		operationType;
	DWORD		sourcePID;        //game pid
	ULONGLONG	sourceAddress;
	DWORD		targetPID;		  //this process pid
	ULONGLONG	targetAddress;
	SIZE_T		size;

} INPUT_INFO, * PINPUT_INFO;

class aksInterface
{
	HANDLE hFileMapping		= nullptr;
	HANDLE eventUMFinished	= nullptr;
	HANDLE eventKMFinished	= nullptr;
	void* baseAddress		= nullptr;
	static std::mutex memMutex;

	INPUT_INFO inputInfo{};

public:

	aksInterface(std::wstring_view processName) {

		inputInfo.targetPID = GetCurrentProcessId();

		inputInfo.sourcePID = getProcessID(processName);

		if (!inputInfo.sourcePID)
			throw std::runtime_error(String("Error. Start the program."));

		hFileMapping = OpenFileMappingA(FILE_MAP_WRITE, FALSE, String("Global\\Objekt"));

		if (!hFileMapping)
			throw WINLASTEXCEPT;

		baseAddress = MapViewOfFile(hFileMapping, FILE_MAP_WRITE, 0, 0, sizeof(INPUT_INFO));

		if (!baseAddress) {

			CloseHandle(hFileMapping);

			throw WINLASTEXCEPT;
		}

		eventUMFinished = OpenEventA(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, String("Global\\BaseUMEvent"));

		if (!eventUMFinished) {

			UnmapViewOfFile(baseAddress);

			CloseHandle(hFileMapping);

			throw WINLASTEXCEPT;
		}

		eventKMFinished = OpenEventA(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, String("Global\\BaseKMEvent"));

		if (!eventKMFinished) {

			UnmapViewOfFile(baseAddress);

			CloseHandle(hFileMapping);

			CloseHandle(eventUMFinished);

			throw WINLASTEXCEPT;
		}

		if (!connect())
			throw std::runtime_error(String("Can't connect!\n"));
	}

	aksInterface (const aksInterface& other)			= delete;

	aksInterface& operator=(const aksInterface& other)  = delete;

	~aksInterface() {

		disconnect();

		if (baseAddress)
			UnmapViewOfFile(baseAddress);

		if (hFileMapping)
			CloseHandle(hFileMapping);

		if (eventUMFinished)
			CloseHandle(eventUMFinished);

		if (eventKMFinished)
			CloseHandle(eventKMFinished);
	}

	DWORD addToMemory() {

		std::unique_lock<std::mutex> lock(memMutex);

		std::memcpy(baseAddress, &inputInfo, sizeof(INPUT_INFO));

		if (!ResetEvent(eventKMFinished))
			throw WINLASTEXCEPT;

		if (!SetEvent(eventUMFinished))
			throw WINLASTEXCEPT;

		DWORD status = WaitForSingleObject(eventKMFinished, INFINITE);

		return status;
	}

	bool connect() {

		DWORD success{};

		inputInfo.operationType = OPERATION_CONNECT;
		inputInfo.targetAddress = reinterpret_cast<ULONGLONG>(&success);

		DWORD status = addToMemory();

		if (status == WAIT_FAILED)
			throw WINLASTEXCEPT;

		else if (status == WAIT_OBJECT_0) {

			if (success == 1)
				return true;

			return false;
		}

		return false;
	}

	void disconnect() {

		inputInfo.operationType = OPERATION_DISCONNECT;

		DWORD status = addToMemory();

		if (status == WAIT_FAILED)
			throw WINLASTEXCEPT;
	}

	template <class T>
	T read(std::uintptr_t address) {

		T object{};

		if (!address)
			return object;

		inputInfo.operationType = OPERATION_READ;
		inputInfo.size = sizeof(T);
		inputInfo.sourceAddress = address;
		inputInfo.targetAddress = reinterpret_cast<ULONGLONG>(&object);

		DWORD status = addToMemory();

		if (status == WAIT_FAILED)
			throw WINLASTEXCEPT;

		else if (status == WAIT_OBJECT_0)
			return object;

		return T{};
	}

	template <class T>
	void write(std::uintptr_t address, const T& value) {

		inputInfo.operationType = OPERATION_WRITE;
		inputInfo.size = sizeof(T);
		inputInfo.sourceAddress = address;
		inputInfo.targetAddress = reinterpret_cast<ULONGLONG>(&value);

		DWORD status = addToMemory();

		if (status == WAIT_FAILED)
			throw WINLASTEXCEPT;
	}

	void read(std::uintptr_t target, std::uintptr_t source, SIZE_T size) {

		inputInfo.operationType = OPERATION_READ;
		inputInfo.size = size;
		inputInfo.sourceAddress = source;
		inputInfo.targetAddress = target;

		DWORD status = addToMemory();

		if (status == WAIT_FAILED)
			throw WINLASTEXCEPT;
	}

	std::uintptr_t getModuleBase64(std::wstring_view moduleName) {

		std::uintptr_t moduleAddress{};

		inputInfo.operationType = OPERATION_GET_X64_MODULE;
		inputInfo.size = 256 * sizeof(wchar_t);
		inputInfo.sourceAddress = reinterpret_cast<ULONGLONG>(&moduleAddress);
		inputInfo.targetAddress = reinterpret_cast<ULONGLONG>(moduleName.data());

		DWORD status = addToMemory();

		if (status == WAIT_FAILED)
			throw WINLASTEXCEPT;

		else if (status == WAIT_OBJECT_0)
			return moduleAddress;

		return 0;
	}

	std::uintptr_t getModuleBase32(std::wstring_view moduleName) {

		std::uintptr_t moduleAddress{};

		inputInfo.operationType = OPERATION_GET_X32_MODULE;
		inputInfo.size = 256 * sizeof(wchar_t);
		inputInfo.sourceAddress = reinterpret_cast<ULONGLONG>(&moduleAddress);
		inputInfo.targetAddress = reinterpret_cast<ULONGLONG>(moduleName.data());

		DWORD status = addToMemory();

		if (status == WAIT_FAILED)
			throw WINLASTEXCEPT;

		else if (status == WAIT_OBJECT_0)
			return moduleAddress;

		return 0;
	}

	std::uintptr_t readPtrs(std::uintptr_t address, const std::initializer_list<std::uintptr_t>& offsets) {

		for (const auto offset : offsets) {
			address = read<uintptr_t>(address + offset);
		}

		return address;
	}
};