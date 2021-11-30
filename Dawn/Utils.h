#pragma once
#include <Windows.h>
#include <string>
 
class Window;
class aksInterface;

ULONG getProcessID(std::wstring_view processName);

void mouseClick(DWORD, DWORD);

void keyClick(DWORD);

void toggleMenu(Window& transparentWindow);

uint64_t classScan(const char* name, aksInterface& iface);

bool contains(const std::string& name, const std::initializer_list<const char*>& names);

std::uintptr_t getObject(std::string_view objName, aksInterface& iface);