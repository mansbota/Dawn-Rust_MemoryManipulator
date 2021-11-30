#include "Utils.h"
#include <TlHelp32.h>
#include "WinException.h"
#include "Offsets.h"
#include "aksInterface.h"
#include "Window.h"

ULONG getProcessID(std::wstring_view processName) {

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (snapshot == INVALID_HANDLE_VALUE)
		throw WINLASTEXCEPT;

	PROCESSENTRY32 pe32{};
	pe32.dwSize = sizeof(PROCESSENTRY32);

	if (Process32First(snapshot, &pe32)) {

		do {

			if (processName == reinterpret_cast<wchar_t*>(pe32.szExeFile))
			{
				CloseHandle(snapshot);
				return pe32.th32ProcessID;
			}

		} while (Process32Next(snapshot, &pe32));
	}

	CloseHandle(snapshot);

	return 0;
}

void toggleMenu(Window& transparentWindow) {

	transparentWindow.toggleImGui();

	mouseClick(MOUSEEVENTF_MIDDLEDOWN, MOUSEEVENTF_MIDDLEUP);
}

void mouseClick(DWORD keyCodeFirst, DWORD keyCodeSecond) {

	INPUT ip{};

	ip.type = INPUT_MOUSE;
	ip.mi.dwFlags = keyCodeFirst | MOUSEEVENTF_ABSOLUTE;

	SendInput(1, &ip, sizeof(INPUT));

	ip.mi.dwFlags = keyCodeSecond | MOUSEEVENTF_ABSOLUTE;

	SendInput(1, &ip, sizeof(INPUT));
}

void keyClick(DWORD keyCode) {

	INPUT ip2{};

	ip2.type = INPUT_KEYBOARD;
	ip2.ki.wScan = 0;					// hardware scan code for key
	ip2.ki.time = 0;
	ip2.ki.dwExtraInfo = 0;

	ip2.ki.wVk = keyCode;
	ip2.ki.dwFlags = 0;					// 0 for key press

	SendInput(1, &ip2, sizeof(INPUT));

	ip2.ki.dwFlags = KEYEVENTF_KEYUP;	// KEYEVENTF_KEYUP for key release

	SendInput(1, &ip2, sizeof(INPUT));
}

uint64_t classScan(const char* name, aksInterface& iface) {

	auto base = iface.getModuleBase64(wString(L"GameAssembly.dll"));

	auto dos_header = iface.read<IMAGE_DOS_HEADER>(base);

	auto data_header = iface.read<IMAGE_SECTION_HEADER>(base + dos_header.e_lfanew + sizeof(IMAGE_NT_HEADERS64) + (3 * 40));

	auto next_section = iface.read<IMAGE_SECTION_HEADER>(base + dos_header.e_lfanew + sizeof(IMAGE_NT_HEADERS64) + (4 * 40));

	auto data_size = next_section.VirtualAddress - data_header.VirtualAddress;

	if (strcmp((char*)data_header.Name, String(".data")))
		throw std::runtime_error(String("[!] Section order changed"));

	for (uint64_t offset = data_size; offset > 0; offset -= 8) {

		struct kName {
			char klass_name[256] = { 0 };
		}kNameIns;

		auto klass = iface.read<uint64_t>(base + data_header.VirtualAddress + offset);

		if (klass == 0) { continue; }

		auto name_pointer = iface.read<uint64_t>(klass + 0x10);

		if (name_pointer == 0) { continue; }

		kNameIns = iface.read<kName>(name_pointer);

		if (!strcmp(kNameIns.klass_name, name))
			return klass;
	}

	throw std::runtime_error((char*)String("[!] Unable to find ") + std::string(name) + (char*)String(" in scan"));
}

bool contains(const std::string& name, const std::initializer_list<const char*>& names) {
	for (const auto& el : names) {
		if (name.find(el) != std::string::npos)
			return true;
	}
	return false;
}

std::uintptr_t getObject(std::string_view objName, aksInterface& iface) {

	static const uintptr_t unityPlayerAddress = iface.getModuleBase64(wString(L"UnityPlayer.dll"));

	static const uintptr_t gameObjectManager = iface.read<uintptr_t>(offset.gom + unityPlayerAddress);

	uintptr_t taggedObj = iface.read<uintptr_t>(gameObjectManager + 0x8);

	for (auto i = 0u; i < 10000; i++) {

		uintptr_t gameObject = iface.read<uintptr_t>(taggedObj + 0x10);

		struct nameStr {
			char buffer[256]{};
		}name;

		uintptr_t namePtr = iface.read<uintptr_t>(gameObject + offset.goName);

		name = iface.read<nameStr>(namePtr);

		if (objName == name.buffer) 
			return gameObject;

		taggedObj = iface.read<uintptr_t>(taggedObj + 0x8);
	}

	return 0;
}