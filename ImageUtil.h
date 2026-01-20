#pragma once

#include "com_ptr.h"
#include <d2d1_2.h>
#include <wincodec.h>

struct ImageUtil
{
	float _scale = 1.0f;
	float scale() const { return _scale; }
	void scale(float v);

	float _whitePointX = 1.0f;
	float whitePointX() const { return _whitePointX; }
	void whitePointX(float v);

	float _whitePointY = 1.0f;
	float whitePointY() const { return _whitePointY; }
	void whitePointY(float v);

	float _blackPointX = 0.0f;
	float blackPointX() const { return _blackPointX; }
	void blackPointX(float v);

	float _blackPointY = 0.0f;
	float blackPointY() const { return _blackPointY; }
	void blackPointY(float v);

	float _saturation = 1.0f;
	float saturation() const { return _saturation; }
	void saturation(float v);

	float _contrast = 0.0f;
	float contrast() const { return _contrast; }
	void contrast(float v);

	float _exposure = 0.0f; // exposure value in range [-2.0, 2.0]
	float exposure() const { return _exposure; }
	void exposure(float v);

	boolean _applyGrayscale = false;
	boolean applyGrayscale() const { return _applyGrayscale; }
	void applyGrayscale(boolean v);

	HWND wnd = nullptr;

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

