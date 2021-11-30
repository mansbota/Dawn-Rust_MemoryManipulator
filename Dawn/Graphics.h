#pragma once
#include <Windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include "WinException.h"
#include <wrl.h>
#include <vector>
#include "Menu.h"
#include <memory>

using namespace Microsoft::WRL;

struct Vertex
{
	float x, y, z, ht;
	DWORD Color;
};

class Graphics
{
	class RenderList
	{
	public:

		struct Batch {

			int vertexCount;
			int primitiveCount;
			int shapesCount;
			D3DPRIMITIVETYPE type;
		};

		std::vector<Vertex> vertices;
		std::vector<Batch> batches;

		template<std::size_t SIZE>
		void addVertices(Vertex (&v)[SIZE])
		{
			for (const auto& el : v)
				vertices.push_back(el);
		}

		void clear()
		{
			vertices.clear();
			batches.clear();
		}
	};

	ComPtr<IDirect3D9Ex> pObject;
	ComPtr<IDirect3DDevice9Ex> pDevice;
	ComPtr<IDirect3DVertexBuffer9> pVBuffer;
	ComPtr<ID3DXFont> pFont;

	RenderList renderList;
	std::unique_ptr<Menu> menu;
	bool isMenuOn;
	int width, height;

public:
	Graphics(HWND hWnd, int width, int height, std::wstring_view gameWindowName);
	Graphics(const Graphics&) = delete;
	Graphics& operator=(const Graphics&) = delete;

	void startFrame();
	void endFrame();
	void emptyFrame();
	int getWidth() const;
	int getHeight() const;

	void drawFilledRect(float x, float y, float width, float height, D3DCOLOR color);
	void drawBorderBox(float x, float y, float width, float height, float thickness, D3DCOLOR color);
	void drawCircle(float x, float y, float radius, int segments, D3DCOLOR color, bool filled);
	void drawString(const std::string& str, float x, float y, D3DCOLOR color);
	float centerText(float x, float width, size_t textWidth);
	void drawLine(float x, float y, float x2, float y2, D3DCOLOR color);
	IDirect3DDevice9Ex* getDevice() noexcept;
	bool isMenuActive() const noexcept { return isMenuOn; }
	void toggleMenu() { isMenuOn = !isMenuOn; }
	int fontWidth;
};
