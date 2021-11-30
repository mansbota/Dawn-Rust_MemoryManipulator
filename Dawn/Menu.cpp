#include "Menu.h"
#include "Crypter.h"
#include "Graphics.h"

extern bool day, esp, admin, toggleAdmin, spiderman, aimbot, animals, food, collectables, scientists, playersB, barrels, ores, hemp, stashes, traps, crates, droppedItems;

extern float aimbotFov;

Menu::Menu(HWND hwnd, Graphics* gfx) :
	gfx{ gfx },
	activeTab{ 0 },
	hwnd{ hwnd },
	tabs{ {0, std::string(String("Visuals"))}, {1, std::string(String("Aim"))}, {2, std::string(String("Misc"))} },
	run{ true }
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX9_Init(gfx->getDevice());

	ImGuiStyle& style = ImGui::GetStyle();

	style.WindowPadding = ImVec2(15, 15);
	style.WindowRounding = 10.0f;
	style.FramePadding = ImVec2(5, 5);
	style.FrameRounding = 4.0f;
	style.ItemSpacing = ImVec2(12, 8);
	style.ItemInnerSpacing = ImVec2(8, 6);
	style.IndentSpacing = 25.0f;
	style.ScrollbarSize = 15.0f;
	style.ScrollbarRounding = 9.0f;
	style.GrabMinSize = 5.0f;
	style.GrabRounding = 3.0f;

	ImGui::StyleColorsDark();
	style.Colors[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.05f, 0.07f, .88f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.50f, 0.50f, 0.53f, 0.88f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.94f, 0.73f, 0.19f, 1.00f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 0.98f, 0.95f, 0.75f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
	style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(1.00f, 0.98f, 0.95f, 0.73f);

	options.resize(3);

	initMenu();
}

void Menu::addOption(int i, const std::string& name, OptionType type, void* address)
{
	options[i].push_back(Option{ name, type, address });
}

void Menu::initMenu()
{
	addOption(0, std::string(String("Wallhack")), OptionType::Checkbox, &esp);
	addOption(0, std::string(String("Players")), OptionType::Checkbox, &playersB);
	addOption(0, std::string(), OptionType::SameLine, nullptr);
	addOption(0, std::string(String("Scientists")), OptionType::Checkbox, &scientists);
	addOption(0, std::string(String("Animals")), OptionType::Checkbox, &animals);
	addOption(0, std::string(), OptionType::SameLine, nullptr);
	addOption(0, std::string(String("Crates")), OptionType::Checkbox, &crates);
	addOption(0, std::string(String("Traps")), OptionType::Checkbox, &traps);
	addOption(0, std::string(), OptionType::SameLine, nullptr);
	addOption(0, std::string(String("Barrels")), OptionType::Checkbox, &barrels);
	addOption(0, std::string(String("Dropped Items")), OptionType::Checkbox, &droppedItems);
	addOption(0, std::string(), OptionType::SameLine, nullptr);
	addOption(0, std::string(String("Food")), OptionType::Checkbox, &food);
	addOption(0, std::string(String("Ores")), OptionType::Checkbox, &ores);
	addOption(0, std::string(), OptionType::SameLine, nullptr);
	addOption(0, std::string(String("Hemp")), OptionType::Checkbox, &hemp);
	addOption(0, std::string(String("Stash")), OptionType::Checkbox, &stashes);
	addOption(0, std::string(), OptionType::SameLine, nullptr);
	addOption(0, std::string(String("Resource Collectables")), OptionType::Checkbox, &collectables);
	addOption(2, std::string(String("Perma Day")), OptionType::Checkbox, &day);
	addOption(2, std::string(String("Free Camera")), OptionType::Checkbox, &admin);
	addOption(2, std::string(String("Spiderman")), OptionType::Checkbox, &spiderman);
	addOption(1, std::string(String("Aimbot")), OptionType::Checkbox, &aimbot);
	addOption(1, std::string(String("aimbot FOV")), OptionType::Slider, &aimbotFov);
}

Menu::~Menu()
{
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void Menu::drawMenu()
{
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin(String("Zora"), &run);

	for (const auto& tab : tabs)
	{
		ImGui::SameLine();

		if (ImGui::Button(tab.second.c_str(), ImVec2(100, 25)))
			activeTab = tab.first;
	}

	for (const auto& option : options[activeTab])
	{
		switch (option.type)
		{
		case OptionType::Checkbox:
			ImGui::Checkbox(option.label.c_str(), (bool*)option.address);
			break;
		case OptionType::ColorPicker3:
			ImGui::ColorEdit3(option.label.c_str(), (float*)option.address);
			break;
		case OptionType::ColorPicker4:
			ImGui::ColorEdit4(option.label.c_str(), (float*)option.address);
			break;
		case OptionType::Slider:
			ImGui::SliderFloat(option.label.c_str(), (float*)option.address, 10.f, 1000.f);
			break;
		case OptionType::Label:
			ImGui::Text(option.label.c_str());
			break;
		case OptionType::SameLine:
			ImGui::SameLine(160.f);
			break;
		}
	}

	ImGui::End();

	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

	if (!run)
		SendMessage(hwnd, WM_CLOSE, NULL, NULL);
}
