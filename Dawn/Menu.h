#pragma once

#include <string>
#include <Windows.h>
#include <map>
#include <vector>
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx9.h"

class Graphics;

class Menu
{
	enum class OptionType {
		Checkbox,
		Slider,
		Label,
		ColorPicker3,
		ColorPicker4,
		Button,
		SameLine
	};

	struct Option {
		std::string label;
		OptionType type;
		void* address;
	};

	std::map<int, std::string> tabs;
	std::vector<std::vector<Option>> options;
	int activeTab;
	Graphics* gfx;
	HWND hwnd;
	bool run;

public:
	Menu(HWND hwnd, Graphics* gfx);
	void addOption(int i, const std::string& name, OptionType type, void* address);
	void initMenu();
	~Menu();

	void drawMenu();
};

