#pragma once

#include <iostream>

struct Offsets {

	std::uintptr_t goLayer;
	std::uintptr_t goTag;
	std::uintptr_t goName;
	std::uintptr_t ocTransform;
	std::uintptr_t trVisualState;
	std::uintptr_t vsPosition;
	std::uintptr_t entPtrviewMatrix;
	std::uintptr_t entPtrfovMult;
	std::uintptr_t bpPlayerModel;
	std::uintptr_t pmPosition;
	std::uintptr_t pmIsLocalPlayer;
	std::uintptr_t bpDisplayName;
	std::uintptr_t bpModel;
	std::uintptr_t mBoneTransforms;
	std::uintptr_t bceHealth;
	std::uintptr_t bpFlags;
	std::uintptr_t bpPlayerWalkMovement;
	std::uintptr_t pwmGroundAngle;
	std::uintptr_t pwmNewGroundAngle;
	std::uintptr_t bpPlayerInventory;
	std::uintptr_t piContainerBelt;
	std::uintptr_t cbItemList;
	std::uintptr_t bpActiveItemId;
	std::uintptr_t iUid;
	std::uintptr_t iItemDef;
	std::uintptr_t idShortName;
	std::uintptr_t sdTODCycle;
	std::uintptr_t tcHour;
	std::uintptr_t bpPlayerInput;
	std::uintptr_t piBodyAngles;
	std::uintptr_t piRecoilAngles;
	std::uintptr_t bpWasDead;
	std::uintptr_t gom = 0;

};

extern Offsets offset;
