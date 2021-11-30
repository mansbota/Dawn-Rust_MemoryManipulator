#pragma once

#include "aksInterface.h"
#include "Drawable.h"
#include "Offsets.h"
#include "Vec3.h"
#include "ViewData.h"

class GameObject : public Drawable
{
	uintptr_t baseAddress{};

	struct dataStruct {

		char data[256];

	}data{};

	std::string objectName{};
	std::uintptr_t objectClass{};
	std::uintptr_t transform{};
	std::uintptr_t visualState{};

public:
	
	GameObject(uintptr_t entity, aksInterface* mem) : baseAddress{ mem->readPtrs(entity, { 0x10, 0x30 }) }, data{ mem->read<dataStruct>(baseAddress) } {

		struct nameStr {

			char buffer[256]{};

		}name;

		name = mem->read<nameStr>(*reinterpret_cast<uintptr_t*>(data.data + offset.goName));

		objectName = std::string(name.buffer);

		objectClass = *reinterpret_cast<uintptr_t*>(data.data + 0x30);

		transform = mem->read<uintptr_t>(objectClass + offset.ocTransform);

		visualState = mem->read<uintptr_t>(transform + offset.trVisualState);
	}

	int getLayer() noexcept { return *reinterpret_cast<int*>(data.data + offset.goLayer); }

	short getTag() noexcept { return *reinterpret_cast<short*>(data.data + offset.goTag); }

	std::string getName() const noexcept { return objectName; }

	uintptr_t getAddress() const noexcept { return baseAddress; }

	Vec3 getPositionTransform(aksInterface* mem) { return mem->read<Vec3>(visualState + offset.vsPosition); }

	void draw(Window* transparentWindow, Vec3& localPos, ViewData& vData, aksInterface* iface) override final {

		D3DXVECTOR2 screenPos{};

		Vec3 position = getPositionTransform(iface);

		if (localPos.distanceFrom(position) > 250.f)
			return;

		if (!WorldToScreen(reinterpret_cast<D3DXVECTOR3*>(&position), screenPos, vData.getViewMatrix(), transparentWindow->Gfx()))
			return;

		std::string name = objectName;
		
		size_t start = name.find_last_of('/'), end = name.find_last_of('.'), len = end - start;

		name = name.substr(start + 1, len - 1);

		transparentWindow->Gfx()->drawCircle(screenPos.x, screenPos.y, 6, 30, D3DCOLOR_XRGB(0, 0, 0), true);
		
		transparentWindow->Gfx()->drawCircle(screenPos.x, screenPos.y, 4, 30, color, true);

		transparentWindow->Gfx()->drawFilledRect(screenPos.x, screenPos.y, 2, 2, color);

		transparentWindow->Gfx()->drawString(name, transparentWindow->Gfx()->centerText(screenPos.x, 2.f, name.length()), screenPos.y + 10, D3DCOLOR_XRGB(255, 255, 255));
	}
};

