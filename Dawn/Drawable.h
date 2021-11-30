#pragma once
#include "Window.h"

class Vec3;
class ViewData;
class aksInterface;

class Drawable
{
protected:

	D3DCOLOR color;

public:

	static bool WorldToScreen(D3DXVECTOR3* origin, D3DXVECTOR2& out, D3DXMATRIX viewMatrix, Graphics* gfx)
	{
		D3DXMATRIX temp;

		D3DXMatrixTranspose(&temp, &viewMatrix);

		D3DXVECTOR3 translationVector = D3DXVECTOR3(temp._41, temp._42, temp._43);
		D3DXVECTOR3 up = D3DXVECTOR3(temp._21, temp._22, temp._23);
		D3DXVECTOR3 right = D3DXVECTOR3(temp._11, temp._12, temp._13);

		float w = D3DXVec3Dot(&translationVector, origin) + temp._44;

		if (w < 0.098f)
			return false;

		float y = D3DXVec3Dot(&up, origin) + temp._24;
		float x = D3DXVec3Dot(&right, origin) + temp._14;

		out.x = (gfx->getWidth() / 2) * (1.f + x / w);
		out.y = (gfx->getHeight() / 2) * (1.f - y / w);

		return true;
	}

	virtual void draw(Window* transparentWindow, Vec3& localPos, ViewData& vData, aksInterface* iface) = 0;

	void setColor(D3DCOLOR color) { this->color = color; }
};


