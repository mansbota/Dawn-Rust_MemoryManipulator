#include <vector>
#include "GameObject.h"
#include "BasePlayer.h"
#include "ViewData.h"
#include "Window.h"
#include "Menu.h"
#include <mutex>
#include "Utils.h"
#include "Aimbot.h"

std::vector<std::shared_ptr<Drawable>> entities;

BasePlayer localPlayer;

extern std::mutex mutex;

bool day{ false }, esp{ true }, admin{ false }, toggleAdmin{ false }, spiderman{ false }, aimbot{ false }, processEntities{ false };

uintptr_t mainCameraVisualState, todCycle;

ViewData mainCameraPtr;

void cachedEntities(std::atomic<bool>& endThread) {

	try {

		aksInterface iface(wString(L"RustClient.exe"));

		Window transparentWindow(L"", wString(L"Rust"));

		while (true) {

			if (endThread)
				break;

			if (GetAsyncKeyState(VK_INSERT) & 1)
				toggleMenu(transparentWindow);

			mutex.lock();

			if (!processEntities) {

				mutex.unlock();

				transparentWindow.Gfx()->emptyFrame();

				if (auto code = Window::processMessages(transparentWindow))
					break;

				std::this_thread::sleep_for(std::chrono::milliseconds(1));

				continue;
			}

			Vec3 localPlayerPos = iface.read<Vec3>(mainCameraVisualState + offset.vsPosition);

			transparentWindow.Gfx()->startFrame();

			for (auto i = 0; i < entities.size(); i++) {

				mainCameraPtr.update(&iface);

				if (esp)
					entities[i]->draw(&transparentWindow, localPlayerPos, mainCameraPtr, &iface);

				if (aimbot) {

					BasePlayer* player = dynamic_cast<BasePlayer*>(entities[i].get());

					if (player)
						addToAimbot(transparentWindow, player, &iface, mainCameraPtr);
				}
			}

			transparentWindow.Gfx()->endFrame();

			if (aimbot) 
				doAimbot(transparentWindow, &iface, &localPlayer, localPlayerPos);

			mutex.unlock();

			if (spiderman) 
				localPlayer.setSpiderman(&iface);

			if (admin != toggleAdmin) {

				localPlayer.adminOn(&iface);

				toggleMenu(transparentWindow);

				std::this_thread::sleep_for(std::chrono::milliseconds(300));

				keyClick(VK_END);

				std::this_thread::sleep_for(std::chrono::milliseconds(100));

				localPlayer.adminOff(&iface);

				toggleAdmin = admin;
			}

			if (day) 
				iface.write<float>(todCycle + offset.tcHour, 11.0f);
			
			if (auto code = Window::processMessages(transparentWindow))
				break;
		}
	}
	catch (BasicException& ex)
	{
		MessageBoxA(nullptr, ex.what(), ex.getType(), MB_ICONEXCLAMATION | MB_OK);
	}
	catch (std::exception& ex)
	{
		MessageBoxA(nullptr, ex.what(), String("Standard Exception"), MB_ICONEXCLAMATION | MB_OK);
	}
	catch (...)
	{
		MessageBoxA(nullptr, String("Unidentified Exception"), String("Unknown Exception"), MB_ICONEXCLAMATION | MB_OK);
	}

	endThread = true;
}