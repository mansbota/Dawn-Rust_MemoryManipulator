
#include "Window.h"
#include "WinException.h"
#include <dwmapi.h>

#pragma comment(lib,"dwmapi.lib")

Window::WindowClass Window::m_wc;

Window::WindowClass::WindowClass()
{
	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(wc);
	wc.hInstance = GetModuleHandle(nullptr);
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = nullptr;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = nullptr;
	wc.hIconSm = nullptr;
	wc.style = 0u;
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = szWindowClass.data();
	wc.lpszMenuName = nullptr;

	RegisterClassEx(&wc);
}

Window::WindowClass::~WindowClass()
{
	UnregisterClass(szWindowClass.data(), GetModuleHandle(nullptr));
}

Window::Window(std::wstring_view name, std::wstring_view gameWindowName) :
	m_width{ GetSystemMetrics(SM_CXSCREEN) }, m_height{ GetSystemMetrics(SM_CYSCREEN) }
{
	m_hWnd = CreateWindowEx(
		NULL,                          //extended styles (it only works if we set the extended styles after creating window)
		m_wc.getClass().data(),		   //window class name
		name.data(),				   //window name
		WS_POPUP,                      //normal styles (popup so it doesn't flash at start)
		0, 0,						   //window start po
		m_width, m_height,			   //width, height of the window
		nullptr, nullptr,			   //no parent, no menu
		GetModuleHandle(nullptr),      //this process creates the window
		nullptr);					   //this is used to pass ptrs to window message loop in lparam parameter

	if (m_hWnd == nullptr)
		throw WINLASTEXCEPT;

	gfx = std::make_unique<Graphics>(m_hWnd, m_width, m_height, gameWindowName);

	//make the window visible and transparent and layered (click through)
	SetWindowLongPtr(m_hWnd, GWL_STYLE, WS_VISIBLE);
	SetWindowLongPtr(m_hWnd, GWL_EXSTYLE, WS_EX_TRANSPARENT | WS_EX_LAYERED);

	//set userdata to address of window object so that OOP can be used with message loop
	SetWindowLongPtr(m_hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

	// This is used to make sure the screen is transparent and not black
	MARGINS margin{ -1 };
	DwmExtendFrameIntoClientArea(m_hWnd, &margin);
}

Window::~Window()
{
	DestroyWindow(m_hWnd);
}

void Window::setTitle(std::wstring_view title) const noexcept
{
	SetWindowText(m_hWnd, title.data());
}

std::optional<int> Window::processMessages(Window& window)
{
	MSG msg;

	// trick for setting window as topmost while not using the WS_EX_TOPMOST tag
	HWND hwnd2 = GetForegroundWindow();
	HWND hwnd3 = GetWindow(hwnd2, GW_HWNDPREV);
	SetWindowPos(window.getHandle(), hwnd3, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
			return msg.wParam;

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return {};
}

void Window::toggleImGui() noexcept
{
	gfx.get()->toggleMenu();

	LONG_PTR style = GetWindowLongPtr(m_hWnd, GWL_EXSTYLE);

	if (gfx.get()->isMenuActive())
	{
		style ^= WS_EX_LAYERED;

		SetWindowLongPtr(m_hWnd, GWL_EXSTYLE, style);
	}
	else
	{
		style |= WS_EX_LAYERED;

		SetWindowLongPtr(m_hWnd, GWL_EXSTYLE, style);
	}
}

bool Window::isForeground() const noexcept
{
	return m_hWnd == GetForegroundWindow();
}

LRESULT WINAPI Window::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
	//get window ptr to be able to use the message handling in a OOP way
	auto ptrWnd = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

	//call the message handling from the window object
	return ptrWnd->handleMsg(hWnd, msg, wParam, lParam);
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT Window::handleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}
