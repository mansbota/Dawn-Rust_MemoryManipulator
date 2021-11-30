#include "Graphics.h"
#include "Crypter.h"

#pragma comment (lib,"d3d9.lib")
#pragma comment (lib,"d3dx9.lib")

Graphics::Graphics(HWND hWnd, int width, int height, std::wstring_view gameWindowName) :
	renderList{}, fontWidth{ 13 }, isMenuOn{ false }, width{ width }, height{ height }
{
	HRESULT hr;

	if (FAILED(hr = Direct3DCreate9Ex(D3D_SDK_VERSION, &pObject)))
		throw WINEXCEPT(hr);

	D3DPRESENT_PARAMETERS pParams = {};
	pParams.Windowed = TRUE;
	pParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
	pParams.hDeviceWindow = hWnd;
	pParams.MultiSampleQuality = D3DMULTISAMPLE_NONE;
	pParams.BackBufferFormat = D3DFMT_A8R8G8B8;
	pParams.BackBufferWidth = width;
	pParams.BackBufferHeight = height;
	pParams.EnableAutoDepthStencil = TRUE;
	pParams.AutoDepthStencilFormat = D3DFMT_D16;

	if (FAILED(hr = pObject->CreateDeviceEx(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL, hWnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING,
		&pParams, 0, &pDevice)))
		throw WINEXCEPT(hr);

	if (FAILED(hr = pDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE)))
		throw WINEXCEPT(hr);

	if (FAILED(hr = pDevice->CreateVertexBuffer(
		sizeof(Vertex) * 1024 * 10,
		D3DUSAGE_DYNAMIC,
		D3DFVF_XYZRHW | D3DFVF_DIFFUSE,
		D3DPOOL_DEFAULT, &pVBuffer, nullptr)))
		throw WINEXCEPT(hr);

	if (FAILED(hr = D3DXCreateFont(
		pDevice.Get(), fontWidth, 0,
		FW_NORMAL, 1, true,
		DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS,
		DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE,
		L"Verdana", &pFont)))
		throw WINEXCEPT(hr);

	if (FAILED(hr = pDevice->SetStreamSource(0, pVBuffer.Get(), 0, sizeof(Vertex))))
		throw WINEXCEPT(hr);

	if (FAILED(hr = pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE)))
		throw WINEXCEPT(hr);

	menu = std::make_unique<Menu>(hWnd, this);
}

void Graphics::startFrame()
{
	HRESULT hr;

	if (FAILED(hr = pDevice->Clear(0, 0, D3DCLEAR_TARGET, 0, 1.0f, 0)))
		throw WINEXCEPT(hr);

	if (FAILED(hr = pDevice->BeginScene()))
		throw WINEXCEPT(hr);
}

void Graphics::endFrame()
{
	HRESULT hr;

	if (renderList.vertices.size() > 0)
	{
		void* pVoid;

		if (FAILED(hr = pVBuffer->Lock(0, 0, &pVoid, D3DLOCK_DISCARD)))
			throw WINEXCEPT(hr);

		std::memcpy(pVoid, renderList.vertices.data(), sizeof(Vertex) * std::size(renderList.vertices));

		if (FAILED(hr = pVBuffer->Unlock()))
			throw WINEXCEPT(hr);

		int pos{ 0 };

		for (const auto& batch : renderList.batches)
		{
			for (auto i = 0u; i < batch.shapesCount; i++) {

				if (FAILED(hr = pDevice->DrawPrimitive(batch.type, pos, batch.primitiveCount / batch.shapesCount)))
					throw WINEXCEPT(hr);

				pos += batch.vertexCount / batch.shapesCount;
			}	
		}
	
		renderList.clear();
	}

	if (isMenuOn)
		menu->drawMenu();

	if (FAILED(hr = pDevice->EndScene()))
		throw WINEXCEPT(hr);

	if (FAILED(hr = pDevice->PresentEx(0, 0, 0, 0, 0)))
		throw WINEXCEPT(hr);
}

void Graphics::emptyFrame() {

	startFrame();

	endFrame();
}

int Graphics::getWidth() const
{
	return width;
}

int Graphics::getHeight() const
{
	return height;
}

void Graphics::drawFilledRect(float x, float y, float width, float height, D3DCOLOR color)
{
	Vertex v[] =
	{
		{x, y + height, .0f, .0f, color},
		{x, y, .0f, .0f, color},
		{x + width, y + height, .0f, .0f, color},

		{x + width, y + height, .0f, .0f, color},
		{x, y, .0f, .0f, color},
		{x + width, y, .0f, .0f, color}
	};
	
	if (!renderList.batches.empty() && renderList.batches.back().type == D3DPT_TRIANGLELIST)
	{
		renderList.batches.back().primitiveCount += 2;
		renderList.batches.back().vertexCount += 6;
		renderList.batches.back().shapesCount++;
	}
	else
		renderList.batches.emplace_back(RenderList::Batch{ 6, 2, 1, D3DPT_TRIANGLELIST });

	renderList.addVertices(v);
}

void Graphics::drawBorderBox(float x, float y, float width, float height, float thickness, D3DCOLOR color)
{
	drawFilledRect(x - 1.f, y - 1.f, width, thickness + 2.f, D3DCOLOR_XRGB(0, 0, 0));
	drawFilledRect(x - 1.f, y - 1.f, thickness + 2.f, height + 3.f, D3DCOLOR_XRGB(0, 0, 0));
	drawFilledRect(x + width - 1.f, y - 1.f, thickness + 2.f, height + 3.f, D3DCOLOR_XRGB(0, 0, 0));
	drawFilledRect(x, y + height - 1.f, width + thickness, thickness + 2.f, D3DCOLOR_XRGB(0, 0, 0));

	drawFilledRect(x, y, width, thickness, color);
	drawFilledRect(x, y, thickness, height, color);
	drawFilledRect(x + width, y, thickness, height, color);
	drawFilledRect(x, y + height, width + thickness, thickness, color);
}

void Graphics::drawCircle(float x, float y, float radius, int segments, D3DCOLOR color, bool filled)
{
	static constexpr double pi = 3.14159265359;

	int numberVerts{ 0 };

	double angle{ 0.0 };

	for (int i = 0; i <= segments; i++) {

		numberVerts++;

		renderList.vertices.emplace_back(Vertex{ (float)(x + cos(angle) * radius), (float)(y + -sin(angle) * radius), .0f, .0f, color });

		angle += (2.0 * pi) / segments;
	}

	if (!renderList.batches.empty() && renderList.batches.back().type == (filled ? D3DPT_TRIANGLEFAN : D3DPT_LINESTRIP))
	{
		renderList.batches.back().vertexCount += numberVerts;
		renderList.batches.back().primitiveCount += numberVerts;
		renderList.batches.back().shapesCount++;
	}
	else 
	{
		renderList.batches.emplace_back(RenderList::Batch{ numberVerts, numberVerts, 1, filled ? D3DPT_TRIANGLEFAN : D3DPT_LINESTRIP });
	}
}

float Graphics::centerText(float x, float width, size_t textWidth)
{
	x += width / 2;

	return x - (textWidth * (fontWidth / 2)) / 2;
}

void Graphics::drawString(const std::string& str, float x, float y, D3DCOLOR color)
{
	RECT fontPos;

	fontPos.left = static_cast<int>(x);
	fontPos.top = static_cast<int>(y);

	pFont->DrawTextA(0, str.c_str(), static_cast<INT>(str.length()), &fontPos, DT_NOCLIP, color);
}

void Graphics::drawLine(float x, float y, float x2, float y2, D3DCOLOR color)
{
	Vertex v[] =
	{
		Vertex{ x, y, .0f, .0f, color},
		Vertex{ x2, y2, .0f, .0f, color}
	};

	if (!renderList.batches.empty() && renderList.batches.back().type == D3DPT_LINELIST)
	{
		renderList.batches.back().vertexCount += 2;
		renderList.batches.back().primitiveCount += 1;
		renderList.batches.back().shapesCount++;
	}
	else
		renderList.batches.push_back(RenderList::Batch{ 2, 1, 1, D3DPT_LINELIST });	

	renderList.addVertices(v);
}

IDirect3DDevice9Ex* Graphics::getDevice() noexcept
{
	return pDevice.Get();
}

