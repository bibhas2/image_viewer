#pragma once

#include "com_ptr.h"
#include <d2d1_2.h>
#include <wincodec.h>

struct ImageUtil
{
	float scale = 1.0;
	HWND wnd;
	SmartPtr<ID2D1Factory> pFactory;
	SmartPtr<ID2D1HwndRenderTarget> pRenderTarget;
	SmartPtr<ID2D1DeviceContext> pDeviceContext;
	SmartPtr<ID2D1Bitmap> pBitmap;
	SmartPtr <IWICImagingFactory> pWICFactory;

	bool init(HWND wnd);
	bool loadImageFromFile(const wchar_t* filename);
	void resize();
	void render();
};

