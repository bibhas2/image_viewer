#pragma once

#include "com_ptr.h"
#include <d2d1_2.h>
#include <wincodec.h>

struct ImageUtil
{
	float scale = 1.0;
	float whitePointX = 1.0f;
	float whitePointY = 1.0f;
	float blackPointX = 0.0f;
	float blackPointY = 0.0f;
	float saturation = 1.0f;
	float contrast = 0.0f;
	float exposure = 0.0f; // exposure value in range [-2.0, 2.0]
	boolean applyGrayscale = false;
	HWND wnd;
	SmartPtr<ID2D1Factory> pFactory;
	SmartPtr<ID2D1HwndRenderTarget> pRenderTarget;
	SmartPtr<ID2D1DeviceContext> pDeviceContext;
	SmartPtr<ID2D1Bitmap> pBitmap;
	SmartPtr <IWICImagingFactory> pWICFactory;
	SmartPtr<ID2D1Effect> scaleEffect;
	SmartPtr<ID2D1Effect> grayScaleEffect;
	SmartPtr<ID2D1Effect> brightnessEffect;
	SmartPtr<ID2D1Effect> saturationEffect;
	SmartPtr<ID2D1Effect> contrastEffect;
	SmartPtr<ID2D1Effect> exposureEffect; // exposure effect

	bool init(HWND wnd);
	bool loadImageFromFile(const wchar_t* filename);
	void resize();
	void render();
	void redraw();
};

