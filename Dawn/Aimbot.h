#pragma once
#include "Vec3.h"
#include <vector>
#include "BasePlayer.h"
#include "aksInterface.h"

void addToAimbot(Window& transparentWindow, BasePlayer* player, aksInterface* iface, ViewData& mainCameraPtr);

void doAimbot(Window& transparentWindow, aksInterface* iface, BasePlayer* localPlayer, Vec3 localPlayerPos);