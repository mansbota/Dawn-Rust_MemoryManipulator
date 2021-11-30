#pragma once

#include "BaseEntity.h"
#include "PlayerModel.h"
#include <vector>
#include <xmmintrin.h>
#include <intrin.h>
#include "Crypter.h"
#include "Drawable.h"
#include "ViewData.h"

#pragma warning( disable : 4996 )

class BasePlayer : public BaseEntity, public Drawable
{
	PlayerModel playerModel{};
	std::string displayName{};
	std::string activeItem{};
	uintptr_t boneTransformAddys[64]{};

public:

	enum Bone_List : int {
		// assets / prefabs / player / player_mod = 0,
		pelvis = 1,
		l_knee = 2,
		l_foot = 3,
		//l_foot = 4,
		l_toe = 5,
		l_ankle_scale = 6,
		penis = 7,
		GenitalCensor = 8,
		GenitalCensor_LOD0 = 9,
		Inner_LOD0 = 10,
		GenitalCensor_LOD1 = 11,
		GenitalCensor_LOD2 = 12,
		r_hip = 13,
		r_knee = 14,
		r_foot = 15,
		r_toe = 16,
		r_ankle_scale = 17,
		spine1 = 18,
		spine1_scale = 19,
		spine2 = 20,
		spine3 = 21,
		spine4 = 22,
		l_clavicle = 23,
		l_upperarm = 24,
		l_forearm = 25,
		l_hand = 26,
		l_index1 = 27,
		l_index2 = 28,
		l_index3 = 29,
		l_little1 = 30,
		l_little2 = 31,
		l_little3 = 32,
		l_middle1 = 33,
		l_middle2 = 34,
		l_middle3 = 35,
		l_prop = 36,
		l_ring1 = 37,
		l_ring2 = 38,
		l_ring3 = 39,
		l_thumb1 = 40,
		l_thumb2 = 41,
		l_thumb3 = 42,
		IKtarget_righthand_min = 43,
		IKtarget_righthand_max = 44,
		l_ulna = 45,
		neck = 46,
		head = 47,
		jaw = 48,
		eyeTranform = 49,
		l_eye = 50,
		l_Eyelid = 51,
		r_eye = 52,
		r_Eyelid = 53,
		r_clavicle = 54,
		r_upperarm = 55,
		r_forearm = 56,
		r_hand = 57,
		r_index1 = 58,
		r_index2 = 59,
		r_index3 = 60,
		r_little1 = 61,
		r_little2 = 62,
		r_little3 = 63,
		r_middle1 = 64,
		r_middle2 = 65,
		r_middle3 = 66,
		r_prop = 67,
		r_ring1 = 68,
		r_ring2 = 69,
		r_ring3 = 70,
		r_thumb1 = 71,
		r_thumb2 = 72,
		r_thumb3 = 73,
		IKtarget_lefthand_min = 74,
		IKtarget_lefthand_max = 75,
		r_ulna = 76,
		l_breast = 77,
		r_breast = 78,
		BoobCensor = 79,
		BreastCensor_LOD0 = 80,
		BreastCensor_LOD1 = 81,
		BreastCensor_LOD2 = 82,
		collision = 83,
		displacement = 84
	};

	BasePlayer() {}

	BasePlayer(uintptr_t gameObject, aksInterface* mem) : BaseEntity(gameObject, mem), playerModel{ *reinterpret_cast<uintptr_t*>(data.data + offset.bpPlayerModel), mem } {

		struct nameStruct {
			wchar_t name[256];
		}name{};

		name = mem->read<nameStruct>(*reinterpret_cast<uintptr_t*>(data.data + offset.bpDisplayName) + 0x14);

		size_t size = wcslen(name.name);

		if (size > 30) {

			displayName = String("Player");
		}
		else {

			char buffer[256]{};

			std::wcstombs(buffer, name.name, size);

			displayName = std::string(buffer);
		}

		uintptr_t model = *reinterpret_cast<uintptr_t*>(data.data + offset.bpModel);

		uintptr_t boneTransforms = mem->read<uintptr_t>(model + offset.mBoneTransforms);

		mem->read((uintptr_t)boneTransformAddys, (uintptr_t)(boneTransforms + 0x20), sizeof(uintptr_t) * 64);
	}

	Vec3 getPosition(aksInterface* mem)		{ return playerModel.getPosition(mem); }

	float getHealth(aksInterface* mem)		{ return mem->read<float>(baseAddress + offset.bceHealth); }

	std::string getName() const noexcept	{ return displayName; }

	int getFlags(aksInterface* mem)			{ return mem->read<int>(baseAddress + offset.bpFlags); }

	bool isAdmin(aksInterface* mem)			{ return getFlags(mem) & 4; }

	bool isSleeping()						{ return *reinterpret_cast<int*>(data.data + offset.bpFlags) & 16; }

	bool isWounded(aksInterface* mem)		{ return getFlags(mem) & 64; }

	bool isLocalPlayer()					{ return playerModel.isLocalPlayer(); }

	bool wasDead(aksInterface* mem)			{ return mem->read<bool>(baseAddress + offset.bpWasDead); }

	void adminOn(aksInterface* mem)			{ mem->write<int>(baseAddress + offset.bpFlags, getFlags(mem) | 4); }

	void adminOff(aksInterface* mem)		{ mem->write<int>(baseAddress + offset.bpFlags, getFlags(mem) ^ 4); }

	void setSpiderman(aksInterface* mem) {

		uintptr_t playerWalkMovement = *reinterpret_cast<uintptr_t*>(data.data + offset.bpPlayerWalkMovement);

		mem->write(playerWalkMovement + offset.pwmGroundAngle, 0.f);

		mem->write(playerWalkMovement + offset.pwmNewGroundAngle, 0.f);
	}

	std::string getActiveItem(aksInterface* mem) noexcept {

		uintptr_t playerInventory	= *reinterpret_cast<uintptr_t*>(data.data + offset.bpPlayerInventory);

		uintptr_t containerBelt		= mem->read<uintptr_t>(playerInventory + offset.piContainerBelt);
		uintptr_t itemList			= mem->read<uintptr_t>(containerBelt + offset.cbItemList);
		uintptr_t items				= mem->read<uintptr_t>(itemList + 0x10);

		unsigned int activeItemID = *reinterpret_cast<unsigned int*>(data.data + offset.bpActiveItemId);

		struct nameStruct {

			wchar_t name[256];

		}name;

		uintptr_t itemAddresses[6];

		mem->read((uintptr_t)itemAddresses, (uintptr_t)(items + 0x20), sizeof(uintptr_t) * 6);

		for (int i = 0; i < 6; ++i)
		{
			uintptr_t item = itemAddresses[i];

			unsigned int ItemID		= mem->read<unsigned int>(item + offset.iUid);

			if (ItemID != activeItemID || !ItemID || !activeItemID) { continue; }

			uintptr_t itemDef		= mem->read<uintptr_t>(item + offset.iItemDef);
			uintptr_t shortName		= mem->read<uintptr_t>(itemDef + offset.idShortName);

			name = mem->read<nameStruct>(shortName + 0x14);

			char buffer[256];

			wcstombs(buffer, name.name, 256);

			return std::string(buffer);
		}

		return "";
	}

	Vec3 getBoneByIndex(int BoneIndex, aksInterface* mem)
	{
		Vec3 pos;

		if (getBonePosition(boneTransformAddys[BoneIndex], pos, mem)) { return pos; }

		return Vec3();
	}

	void aimAt(Vec3& myPos, Vec3& otherPos, aksInterface* mem) {

		Vec2 angles = calcAngle(myPos - otherPos);

		uintptr_t playerInput = *reinterpret_cast<uintptr_t*>(data.data + offset.bpPlayerInput);

		Vec2 curAngles = mem->read<Vec2>(playerInput + offset.piBodyAngles);

		Vec2 angleDif = angles - curAngles;

		if (angleDif.y < -300.f)
			angleDif.y += 360.f;
		else if (angleDif.y > 300.f)
			angleDif.y -= 360.f;

		clampAngle(angleDif);

		Vec2 recoilAngles = mem->read<Vec2>(playerInput + offset.piRecoilAngles);

		mem->write<Vec2>(playerInput + offset.piBodyAngles, curAngles + (angleDif / 3.f - recoilAngles / 3.f));
	}

	void draw(Window* transparentWindow, Vec3& localPos, ViewData& vData, aksInterface* iface) override final {

		drawPlayer(transparentWindow, localPos, vData, iface);

	}

private:

	Vec2 calcAngle(const Vec3& delta) {

		static constexpr float pi{ 3.14159265f };

		float pitch = asin(delta.y / Vec3::magnitude(delta)) * (180.f / pi);

		float yaw = -atan2(delta.x, -delta.z) * (180.f / pi);

		return Vec2(pitch, yaw);
	}

	void clampAngle(Vec2& angles) {

		if (angles.x > 89.f)
			angles.x = 89.f;
		else if (angles.x < -89.f)
			angles.x = -89.f;

		if (angles.y < -360.f)
			angles.y = -360.f;
		else if (angles.y > 360.f)
			angles.y = 360.f;
	}

	void drawPlayer(Window* transparentWindow, Vec3& localPos, ViewData& vData, aksInterface* iface) {

		D3DXVECTOR2 screenPos{};

		Vec3 position = getPosition(iface);

		float distance = position.distanceFrom(localPos);

		if (distance > 250.f)
			return;

		if (!WorldToScreen(reinterpret_cast<D3DXVECTOR3*>(&position), screenPos, vData.getViewMatrix(), transparentWindow->Gfx()))
			return;

		if (wasDead(iface))
			return;

		std::string udaljenost = std::to_string(static_cast<int>(distance));

		if (isSleeping() || isWounded(iface)) {

			transparentWindow->Gfx()->drawString(        //udaljenost
				udaljenost,
				transparentWindow->Gfx()->centerText(screenPos.x, 2.f, udaljenost.length()),
				screenPos.y - 14,
				D3DCOLOR_XRGB(255, 255, 255));

			std::string name = getName();

			transparentWindow->Gfx()->drawString(        //ime igraca
				name,
				transparentWindow->Gfx()->centerText(screenPos.x, 2.f, name.length()),
				screenPos.y - 30,
				D3DCOLOR_XRGB(255, 255, 255));
		}
		else {

			if (distance < 60.f)
				drawSkeleton(transparentWindow, vData.getViewMatrix(), iface);

			float width = transparentWindow->Gfx()->getWidth() / 4.f / distance * vData.getFOVMultiplier();

			float height = transparentWindow->Gfx()->getHeight() / distance * vData.getFOVMultiplier();

			transparentWindow->Gfx()->drawBorderBox(screenPos.x - (width / 2), screenPos.y - height, width, height, 1, color);

			drawHealthBar(transparentWindow, screenPos, distance, width, height, iface);

			transparentWindow->Gfx()->drawString(        //udaljenost
				udaljenost,
				transparentWindow->Gfx()->centerText(screenPos.x, 2.f, udaljenost.length()),
				screenPos.y - height - 14,
				D3DCOLOR_XRGB(255, 255, 255));

			std::string name = getName();

			transparentWindow->Gfx()->drawString(        //ime igraca
				name,
				transparentWindow->Gfx()->centerText(screenPos.x, 2.f, name.length()),
				screenPos.y - height - 30,
				D3DCOLOR_XRGB(255, 255, 255));

			std::string activeItem = getActiveItem(iface);

			transparentWindow->Gfx()->drawString(         //stvar aktivna
				activeItem,
				transparentWindow->Gfx()->centerText(screenPos.x, 2.f, activeItem.length()),
				screenPos.y + 18,
				D3DCOLOR_XRGB(255, 255, 255));
		}
	}

	void drawHealthBar(Window* transparentWindow, D3DXVECTOR2 screenPos, float distance, float w, float h, aksInterface* iface) {

		float health = getHealth(iface);

		transparentWindow->Gfx()->drawFilledRect(screenPos.x - w / 2.f - 10, screenPos.y, 5, -(h * (health / 100.f)), D3DCOLOR_XRGB(0, 200, 0));
	}

	void drawSkeleton(Window* transparentWindow, const D3DXMATRIX& viewMatrix, aksInterface* iface) {

		std::vector<Vec3> bonePositions = getBonePositions({
			BasePlayer::l_foot,
			BasePlayer::l_knee,
			BasePlayer::spine1,
			BasePlayer::r_knee,
			BasePlayer::r_foot }, iface
		);

		std::vector<D3DXVECTOR2> screenBonePositions = getScreenBonePos(bonePositions, viewMatrix, transparentWindow);

		drawLines(screenBonePositions, transparentWindow);

		bonePositions = getBonePositions({
			BasePlayer::spine1,
			BasePlayer::spine4,
			BasePlayer::r_upperarm,
			BasePlayer::r_forearm,
			BasePlayer::r_hand }, iface
		);

		screenBonePositions = getScreenBonePos(bonePositions, viewMatrix, transparentWindow);

		drawLines(screenBonePositions, transparentWindow);

		bonePositions = getBonePositions({
			BasePlayer::head,
			BasePlayer::spine4,
			BasePlayer::l_upperarm,
			BasePlayer::l_forearm,
			BasePlayer::l_hand }, iface
		);

		screenBonePositions = getScreenBonePos(bonePositions, viewMatrix, transparentWindow);

		drawLines(screenBonePositions, transparentWindow);
	}

	void drawLines(const std::vector<D3DXVECTOR2>& screenBonePositions, Window* transparentWindow) {

		for (size_t i = 0u; i < screenBonePositions.size(); i++) {

			if (i + 1 < screenBonePositions.size()) {

				transparentWindow->Gfx()->drawLine(

					screenBonePositions[i].x,

					screenBonePositions[i].y,

					screenBonePositions[i + 1].x,

					screenBonePositions[i + 1].y,

					D3DCOLOR_XRGB(255, 255, 255)
				);
			}
		}
	}

	std::vector<D3DXVECTOR2> getScreenBonePos(const std::vector<Vec3>& positions, const D3DXMATRIX& viewMatrix, Window* transparentWindow) {

		std::vector<D3DXVECTOR2> screenPositions;

		for (const auto pos : positions) {

			D3DXVECTOR2 screenPos{};

			if (WorldToScreen(&D3DXVECTOR3(pos.x, pos.y, pos.z), screenPos, viewMatrix, transparentWindow->Gfx()))
				screenPositions.push_back(screenPos);
		}

		return screenPositions;
	}

	std::vector<Vec3> getBonePositions(const std::initializer_list<short>& tags, aksInterface* iface) {

		std::vector<Vec3> pos;

		for (const auto tag : tags) {
			pos.push_back(getBoneByIndex(tag, iface));
		}

		return pos;
	}

	struct TransformAccessReadOnly
	{
		ULONGLONG pTransformData;
	};

	struct TransformData
	{
		ULONGLONG pTransformArray;
		ULONGLONG pTransformIndices;
	};

	struct Matrix34
	{
		D3DXVECTOR4 vec0;
		D3DXVECTOR4 vec1;
		D3DXVECTOR4 vec2;
	};

	bool getBonePosition(uintptr_t boneTransform, Vec3& pos, aksInterface* mem)
	{
		__m128 result;

		const __m128 mulVec0 = { -2.000, 2.000, -2.000, 0.000 };
		const __m128 mulVec1 = { 2.000, -2.000, -2.000, 0.000 };
		const __m128 mulVec2 = { -2.000, -2.000, 2.000, 0.000 };

		uint64_t pTransform = mem->read<uintptr_t>(boneTransform + 0x10);

		TransformAccessReadOnly pTransformAccessReadOnly = mem->read<TransformAccessReadOnly>(pTransform + 0x38);
		unsigned int index = mem->read<unsigned int>(pTransform + 0x40);
		TransformData transformData = mem->read<TransformData>(pTransformAccessReadOnly.pTransformData + 0x18);

		if (transformData.pTransformArray <= 0x00 || transformData.pTransformIndices <= 0x00)
			return false;

		SIZE_T sizeMatriciesBuf = sizeof(Matrix34) * index + sizeof(Matrix34);
		SIZE_T sizeIndicesBuf = sizeof(int) * index + sizeof(int);

		// Allocate memory for storing large amounts of data (matricies and indicies)
		PVOID pMatriciesBuf = malloc(sizeMatriciesBuf);
		PVOID pIndicesBuf = malloc(sizeIndicesBuf);

		if (pMatriciesBuf && pIndicesBuf)
		{
			// Read Matricies array into the buffer
			mem->read((uintptr_t)pMatriciesBuf, (uintptr_t)transformData.pTransformArray, sizeMatriciesBuf);
			// Read Indices array into the buffer
			mem->read((uintptr_t)pIndicesBuf, (uintptr_t)transformData.pTransformIndices, sizeIndicesBuf);

			result = *(__m128*)((ULONGLONG)pMatriciesBuf + 0x30 * index);
			int transformIndex = *(int*)((ULONGLONG)pIndicesBuf + 0x4 * index);
			int pSafe = 0;
			while (transformIndex >= 0 && pSafe++ < 200)
			{
				Matrix34 matrix34 = *(Matrix34*)((ULONGLONG)pMatriciesBuf + 0x30 * transformIndex);

				__m128 xxxx = _mm_castsi128_ps(_mm_shuffle_epi32(*(__m128i*)(&matrix34.vec1), 0x00));	// xxxx
				__m128 yyyy = _mm_castsi128_ps(_mm_shuffle_epi32(*(__m128i*)(&matrix34.vec1), 0x55));	// yyyy
				__m128 zwxy = _mm_castsi128_ps(_mm_shuffle_epi32(*(__m128i*)(&matrix34.vec1), 0x8E));	// zwxy
				__m128 wzyw = _mm_castsi128_ps(_mm_shuffle_epi32(*(__m128i*)(&matrix34.vec1), 0xDB));	// wzyw
				__m128 zzzz = _mm_castsi128_ps(_mm_shuffle_epi32(*(__m128i*)(&matrix34.vec1), 0xAA));	// zzzz
				__m128 yxwy = _mm_castsi128_ps(_mm_shuffle_epi32(*(__m128i*)(&matrix34.vec1), 0x71));	// yxwy
				__m128 tmp7 = _mm_mul_ps(*(__m128*)(&matrix34.vec2), result);

				result = _mm_add_ps(
					_mm_add_ps(
						_mm_add_ps(
							_mm_mul_ps(
								_mm_sub_ps(
									_mm_mul_ps(_mm_mul_ps(xxxx, mulVec1), zwxy),
									_mm_mul_ps(_mm_mul_ps(yyyy, mulVec2), wzyw)),
								_mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(tmp7), 0xAA))),
							_mm_mul_ps(
								_mm_sub_ps(
									_mm_mul_ps(_mm_mul_ps(zzzz, mulVec2), wzyw),
									_mm_mul_ps(_mm_mul_ps(xxxx, mulVec0), yxwy)),
								_mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(tmp7), 0x55)))),
						_mm_add_ps(
							_mm_mul_ps(
								_mm_sub_ps(
									_mm_mul_ps(_mm_mul_ps(yyyy, mulVec0), yxwy),
									_mm_mul_ps(_mm_mul_ps(zzzz, mulVec1), zwxy)),
								_mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(tmp7), 0x00))),
							tmp7)), *(__m128*)(&matrix34.vec0));

				transformIndex = *(int*)((ULONGLONG)pIndicesBuf + 0x4 * transformIndex);
			}

			free(pMatriciesBuf);
			free(pIndicesBuf);
		}

		pos.x = result.m128_f32[0];
		pos.y = result.m128_f32[1];
		pos.z = result.m128_f32[2];

		return true;
	}
};

