#include "aksInterface.h"
#include "BasePlayer.h"
#include "GameObject.h"
#include <vector>
#include "Vec3.h"
#include "ViewData.h"
#include "Offsets.h"
#include <mutex>
#include <thread>
#include "Crypter.h"
#include "Client.h"
#include "Utils.h"
#include "Drawable.h"

void cachedEntities(std::atomic<bool>& endThread);

extern std::vector<std::shared_ptr<Drawable>> entities;

extern BasePlayer	localPlayer;

extern uintptr_t	mainCameraVisualState, todCycle;

extern ViewData		mainCameraPtr;

extern bool			processEntities;

std::atomic<bool>	endThread{ false };

std::mutex			mutex;

bool animals{ true }, food{ true }, collectables{ true }, scientists{ true }, playersB{ true }, barrels{ true }, ores{ true }, hemp{ true }, stashes{ true }, traps{ true }, crates{ true }, droppedItems{ true };

int WINAPI WinMain(
	_In_ HINSTANCE,          //hIsntance
	_In_opt_ HINSTANCE,      //hLastInstance
	_In_ LPSTR,              //lpCmdLine
	_In_ int)                //nCmdShow
{
	std::vector<std::shared_ptr<Drawable>> tempEntities;

	BasePlayer tempLocal;

	try
	{
		Client client(wString(L"Client.exe"));

		aksInterface iface(wString(L"RustClient.exe"));

		std::thread cachedEntitiesThread(cachedEntities, std::ref(endThread));

		uintptr_t baseNetworkable = classScan(String("BaseNetworkable"), iface);

		while (true) {

			if (!client.processActive()) {

				endThread = true;

				cachedEntitiesThread.join();

				break;
			}

			uintptr_t entityRealm = iface.read<uintptr_t>(baseNetworkable + 0xB8);

			uintptr_t clientEntities = iface.read<uintptr_t>(entityRealm);

			uintptr_t clientEntities_list = iface.read<uintptr_t>(clientEntities + 0x10);

			uintptr_t clientEntities_values = iface.read<uintptr_t>(clientEntities_list + 0x28);

			int entityCount = iface.read<int>(clientEntities_values + 0x10);

			if (entityCount == 0) {

				mutex.lock();

				processEntities = false;

				mutex.unlock();

				continue;
			}

			uintptr_t mainCamGO = getObject(String("Main Camera"), iface);

			if (mainCamGO == 0)
				continue;

			mainCameraPtr = ViewData{ mainCamGO, &iface };

			mainCameraVisualState = iface.readPtrs(mainCamGO, { 0x30, 0x8, 0x38 });

			uintptr_t skyDomeBE = iface.readPtrs(getObject(String("Sky Dome"), iface), { 0x30, 0x18, 0x28 });

			todCycle = iface.read<uintptr_t>(skyDomeBE + offset.sdTODCycle);

			////////// [^] Getting Main camera and Sky dome objects

			while (true) {

				if (!client.processActive()) {

					endThread = true;

					cachedEntitiesThread.join();

					break;
				}

				int entityCount = iface.read<int>(clientEntities_values + 0x10);

				if (entityCount == 0) {

					mutex.lock();

					processEntities = false;

					mutex.unlock();

					break;
				}

				uintptr_t entityBuffer = iface.read<uintptr_t>(clientEntities_values + 0x18);

				std::vector<uintptr_t> entityPtrs(entityCount);

				iface.read((uintptr_t)entityPtrs.data(), (uintptr_t)(entityBuffer + 0x20), sizeof(uintptr_t) * entityCount);

				for (int i = 0; i < entityCount; i++) {

					std::shared_ptr<GameObject> gameObj = std::make_shared<GameObject>( entityPtrs[i], &iface );

					std::string name = gameObj->getName();

					if (crates && contains(name, { String("crate"), String("supply_drop") })) {

						gameObj->setColor(D3DCOLOR_XRGB(204, 0, 204));

						tempEntities.push_back(gameObj);

					}
					else if (gameObj->getTag() == 6) {

						std::shared_ptr<BasePlayer> player = std::make_shared<BasePlayer>( gameObj->getAddress(), &iface );

						if (player->isLocalPlayer()) 
							tempLocal = *player;
						
						else {

							if (scientists && contains(name, { String("scientist") })) {

								player->setColor(D3DCOLOR_XRGB(0, 206, 127));

								tempEntities.push_back(player);
							}

							if (playersB && contains(name, { String("player.prefab") })) {

								player->setColor(D3DCOLOR_XRGB(244, 0, 244));

								tempEntities.push_back(player);
							}
						}
					}
					else if (gameObj->getLayer() == 26) {

						if (droppedItems && !contains(name, { String("seed"), String("rock"), String("torch"), String("hammer"), String("bone") })) {

							gameObj->setColor(D3DCOLOR_XRGB(0, 128, 255));

							tempEntities.push_back(gameObj);
						}
					}
					else if (gameObj->getLayer() == 11) {

						if (animals) {

							gameObj->setColor(D3DCOLOR_XRGB(40, 126, 109));

							tempEntities.push_back(gameObj);
						}
					}
					else if (gameObj->getLayer() == 0) {

						if (contains(name, { String("photo"), String("junkpile"), String("weapon"), String("planner"), String("explosive"), String("binoculars"), String("charge"), String("tool"), String("orebonus") }))
							continue;

						if (food && contains(name, { String("potato"), String("trash-pile"), String("berry"), String("corn"), String("mushroom"), String("pumpkin") })) {

							gameObj->setColor(D3DCOLOR_XRGB(120, 45, 6));

							tempEntities.push_back(gameObj);
						}

						if (collectables && contains(name, { String("stone-collectable"), String("wood-collectable"), String("metal-collectable"), String("sulfur-collectable") })) {

							gameObj->setColor(D3DCOLOR_XRGB(12, 45, 60));

							tempEntities.push_back(gameObj);
						}

						if (barrels && contains(name, { String("barrel") })) {

							gameObj->setColor(D3DCOLOR_XRGB(76, 0, 153));

							tempEntities.push_back(gameObj);
						}

						if (ores && contains(name, { String("stone-ore"), String("sulfur-ore"), String("metal-ore") })) {

							gameObj->setColor(D3DCOLOR_XRGB(204, 102, 0));

							tempEntities.push_back(gameObj);
						}

						if (hemp && contains(name, { String("hemp-collectable") })) {

							gameObj->setColor(D3DCOLOR_XRGB(0, 230, 6));

							tempEntities.push_back(gameObj);
						}

						if (stashes && contains(name, { String("stash") })) {

							gameObj->setColor(D3DCOLOR_XRGB(153, 255, 51));

							tempEntities.push_back(gameObj);
						}
					}
					else {

						if (traps && contains(name, { String("barricade.wood"), String("barricade.metal"), String("turret"), String("trap"), String("landmine"), String("spike") }) && !contains(name, { String("fish") })) {

							gameObj->setColor(D3DCOLOR_XRGB(230, 0, 0));

							tempEntities.push_back(gameObj);
						}
					}

					std::this_thread::sleep_for(std::chrono::milliseconds(1));
				}

				mutex.lock();

				entities = tempEntities;

				localPlayer = tempLocal;

				processEntities = true;

				mutex.unlock();

				tempEntities.clear();

				if (endThread)
					break;
			}

			if (endThread)
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

	return 0;
}