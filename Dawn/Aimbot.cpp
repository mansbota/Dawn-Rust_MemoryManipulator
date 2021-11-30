#include "Aimbot.h"

struct AimbotData {

	Vec3 headPos;
	float distanceFromCenter;
};

std::vector<AimbotData> aimbotPlayers;

float aimbotFov{ 100.f };

void addToAimbot(Window& transparentWindow, BasePlayer* player, aksInterface* iface, ViewData& mainCameraPtr) {

	Vec3 headPos = player->getBoneByIndex(BasePlayer::head, iface);

	D3DXVECTOR2 screenHeadPos;

	if (Drawable::WorldToScreen(&D3DXVECTOR3(headPos.x, headPos.y, headPos.z), screenHeadPos, mainCameraPtr.getViewMatrix(), transparentWindow.Gfx())) {

		float distanceFromCenter = Vec2(screenHeadPos.x, screenHeadPos.y).distanceFrom(Vec2(transparentWindow.Gfx()->getWidth() / 2.f, transparentWindow.Gfx()->getHeight() / 2.f));

		if (distanceFromCenter < aimbotFov) {

			AimbotData aimbotData;

			aimbotData.headPos = headPos;
			aimbotData.distanceFromCenter = distanceFromCenter;

			aimbotPlayers.push_back(aimbotData);
		}
	}
}

void doAimbot(Window& transparentWindow, aksInterface* iface, BasePlayer* localPlayer, Vec3 localPlayerPos) {

	static const float middleX = transparentWindow.Gfx()->getWidth() / 2.f;
	static const float middleY = transparentWindow.Gfx()->getHeight() / 2.f;

	transparentWindow.Gfx()->drawCircle(middleX, middleY, aimbotFov, 60, D3DCOLOR_XRGB(0, 140, 140), false);

	transparentWindow.Gfx()->drawFilledRect(middleX + aimbotFov - 1, middleY, 1, 1, D3DCOLOR_XRGB(0, 140, 140));

	Vec3 targetPlayer;

	float shortestDistance{ aimbotFov };

	bool aim{ false };

	for (auto& player : aimbotPlayers) {

		float dist = player.distanceFromCenter;

		if (dist < shortestDistance) {

			shortestDistance = dist;

			targetPlayer = player.headPos;

			aim = true;
		}
	}

	if (aim && GetAsyncKeyState(0x43))
		localPlayer->aimAt(localPlayerPos, targetPlayer, iface);

	aimbotPlayers.clear();
}