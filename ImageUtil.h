#pragma once

#include "com_ptr.h"
#include <d2d1_2.h>
#include <wincodec.h>
#include <vector>
#include <string>

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

	float _temperature = 0.0f; // temperature value in range [-1.0, 1.0]
	float temperature() const { return _temperature; }
	void temperature(float v);

	float _tint = 0.0f; // tint value in range [-1.0, 1.0]
	float tint() const { return _tint; }
	void tint(float v);

	float _highlights = 0.0f; // highlights adjustment in range [-1.0, 1.0]
	float highlights() const { return _highlights; }
	void highlights(float v);

	float _shadows = 0.0f; // shadows adjustment in range [-1.0, 1.0]
	float shadows() const { return _shadows; }
	void shadows(float v);

	boolean _applyGrayscale = false;
	boolean applyGrayscale() const { return _applyGrayscale; }
	void applyGrayscale(boolean v);

	bool _applyInvert = false;
	boolean applyInvert() const { return _applyInvert; }
	void applyInvert(boolean v);

	HWND wnd = nullptr;

	SmartPtr<ID2D1Factory> pFactory;
	SmartPtr<ID2D1HwndRenderTarget> pRenderTarget;
	SmartPtr<ID2D1DeviceContext> pDeviceContext;
	SmartPtr<ID2D1Bitmap> pBitmap;
	SmartPtr <IWICImagingFactory> pWICFactory;
	SmartPtr<ID2D1Effect> scaleEffect;
	SmartPtr<ID2D1Effect> grayScaleEffect;
	SmartPtr<ID2D1Effect> invertEffect;
	SmartPtr<ID2D1Effect> brightnessEffect;
	SmartPtr<ID2D1Effect> saturationEffect;
	SmartPtr<ID2D1Effect> contrastEffect;
	SmartPtr<ID2D1Effect> exposureEffect;
	SmartPtr<ID2D1Effect> temperatureTintEffect;
	SmartPtr<ID2D1Effect> highlightsAndShadowsEffect;
	std::vector<SmartPtr<ID2D1Effect>> effectChain;

	bool init(HWND wnd);
	bool loadImageFromFile(const wchar_t* filename);
	bool saveImageToFile(const std::wstring& filename);
	void resize();
	void render();
	void redraw();
	void buildEffectChain();
	bool formatForFileExtension(std::wstring& file_name, GUID& wicFormatId);
};

void check_throw(HRESULT hr);