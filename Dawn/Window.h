#pragma once
#include <Windows.h>
#include <string_view>
#include "Graphics.h"
#include <memory>
#include <optional>

class Window
{
	static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
	LRESULT handleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

	class WindowClass
	{
		std::wstring_view szWindowClass = L"Zora";

	public:
		WindowClass();
		~WindowClass();
		std::wstring_view getClass() const noexcept { return szWindowClass; }
	};

	HWND m_hWnd;
	int m_width, m_height;
	static WindowClass m_wc;
	std::unique_ptr<Graphics> gfx;

public:
	Window(std::wstring_view name, std::wstring_view gameWindowName);
	~Window();
	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;
	void setTitle(std::wstring_view title) const noexcept;
	Graphics* Gfx() { return gfx.get(); }
	static std::optional<int> processMessages(Window& window);
	HWND getHandle() const { return m_hWnd; }
	void toggleImGui() noexcept;
	bool isForeground() const noexcept;
};

