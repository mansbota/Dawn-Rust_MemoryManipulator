#pragma once

#include "aksInterface.h"
#include <d3d9.h>
#include <d3dx9.h>
#include "Offsets.h"

class ViewData
{
	std::uintptr_t baseAddress{};

	struct DataStruct {
		char data[2048];
	}data{};

public:

	ViewData() {}

	ViewData(uintptr_t address, aksInterface* mem) : baseAddress{ mem->readPtrs(address, { 0x30, 0x18 }) }, data{} {}

	void update(aksInterface* mem) {
		data = mem->read<DataStruct>(baseAddress);
	}

	const D3DMATRIX getViewMatrix() {
		return *reinterpret_cast<D3DMATRIX*>(data.data + offset.entPtrviewMatrix);
	}

	float getFOVMultiplier() {
		return *reinterpret_cast<float*>(data.data + offset.entPtrfovMult);
	}
};

