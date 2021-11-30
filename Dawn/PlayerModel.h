#pragma once

#include "aksInterface.h"
#include "Vec3.h"
#include "Offsets.h"

class PlayerModel
{
private:
	uintptr_t baseAddress{};

	struct dataStruct {
		char data[1024];
	}data{};

public:

	PlayerModel() {}

	PlayerModel(uintptr_t playerModel, aksInterface* mem) : baseAddress{ playerModel } {
		data = mem->read<dataStruct>(baseAddress);
	}

	Vec3 getPosition(aksInterface* mem) {
		return mem->read<Vec3>(baseAddress + offset.pmPosition);
	}

	bool isLocalPlayer() {
		return *reinterpret_cast<bool*>(data.data + offset.pmIsLocalPlayer);
	}
};

