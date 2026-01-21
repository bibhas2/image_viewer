#include <windows.h>
#include "ImageUtil.h"
//Some of the effects CLSIDs are defined in d2d1effects_2.h
#include <d2d1effects_2.h>

// Setter implementations moved from header
void ImageUtil::scale(float v) { 
	_scale = v; 
	scaleEffect->SetValue(D2D1_SCALE_PROP_SCALE, D2D1::Vector2F(_scale, _scale));
}

void ImageUtil::whitePointX(float v) { 
	_whitePointX = v; 

	brightnessEffect->SetValue(D2D1_BRIGHTNESS_PROP_WHITE_POINT, D2D1::Vector2F(_whitePointX, _whitePointY));

}
void ImageUtil::whitePointY(float v) { 
	_whitePointY = v; 
	brightnessEffect->SetValue(D2D1_BRIGHTNESS_PROP_WHITE_POINT, D2D1::Vector2F(_whitePointX, _whitePointY));
}
void ImageUtil::blackPointX(float v) { 
	_blackPointX = v; 
	brightnessEffect->SetValue(D2D1_BRIGHTNESS_PROP_BLACK_POINT, D2D1::Vector2F(_blackPointX, _blackPointY));
}
void ImageUtil::blackPointY(float v) { 
	_blackPointY = v; 
	brightnessEffect->SetValue(D2D1_BRIGHTNESS_PROP_BLACK_POINT, D2D1::Vector2F(_blackPointX, _blackPointY));
}
void ImageUtil::saturation(float v) { 
	_saturation = v; 
	saturationEffect->SetValue(D2D1_SATURATION_PROP_SATURATION, _saturation);
}
void ImageUtil::contrast(float v) { 
	_contrast = v; 
	contrastEffect->SetValue(D2D1_CONTRAST_PROP_CONTRAST, _contrast);
}
void ImageUtil::exposure(float v) { 
	_exposure = v; 
	exposureEffect->SetValue(D2D1_EXPOSURE_PROP_EXPOSURE_VALUE, _exposure);
}
void ImageUtil::applyGrayscale(boolean v) { 
	_applyGrayscale = v; 

	//Rebuild the effect chain
	buildEffectChain();
}

// Constructor to initialize the ImageUtil with a window handle
bool ImageUtil::init(HWND _wnd)
{
	wnd = _wnd;

	// Initialize Direct2D Factory
	HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory);
	
	if (!SUCCEEDED(hr)) {
		return false;
	}

	// Initialize WIC Imaging Factory
	hr = CoCreateInstance(
		CLSID_WICImagingFactory,
		nullptr,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&pWICFactory)
	);

	if (!SUCCEEDED(hr)) {
		return false;
	}

	// Create Render Target
	RECT rc;

	GetClientRect(wnd, &rc);
	
	hr = pFactory->CreateHwndRenderTarget(
		D2D1::RenderTargetProperties(),
		D2D1::HwndRenderTargetProperties(
			wnd,
			D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top)
		),
		&pRenderTarget
	);

	if (!SUCCEEDED(hr)) {
		return false;
	}

	//Get the device context from the render target
	hr = pRenderTarget->QueryInterface(IID_PPV_ARGS(&pDeviceContext));

	if (!SUCCEEDED(hr)) {
		return false;
	}

	hr = pDeviceContext->CreateEffect(CLSID_D2D1Brightness, &brightnessEffect);

	if (!SUCCEEDED(hr)) {
		return false;
	}

	hr = pDeviceContext->CreateEffect(CLSID_D2D1Grayscale, &grayScaleEffect);

	if (!SUCCEEDED(hr)) {
		return false;
	}

	hr = pDeviceContext->CreateEffect(CLSID_D2D1Scale, &scaleEffect);

	if (!SUCCEEDED(hr)) {
		return false;
	}

	hr = pDeviceContext->CreateEffect(CLSID_D2D1Saturation, &saturationEffect);

	if (!SUCCEEDED(hr)) {
		return false;
	}

	hr = pDeviceContext->CreateEffect(CLSID_D2D1Contrast, &contrastEffect);

	if (!SUCCEEDED(hr)) {
		return false;
	}

	// Create exposure effect
	hr = pDeviceContext->CreateEffect(CLSID_D2D1Exposure, &exposureEffect);

	if (!SUCCEEDED(hr)) {
		// If exposure effect is not available, continue without it
		exposureEffect.reset();
	}

	// Build the effect chain
	buildEffectChain();

	//The saturation effects is all wonky. Set its saturation to 1.0 initially.
	saturation(saturation());

	return true;
}

void ImageUtil::redraw() {
	InvalidateRect(wnd, NULL, FALSE);
}

// Load an image from a file and create a Direct2D bitmap
bool ImageUtil::loadImageFromFile(const wchar_t* filename)
{
	SmartPtr<IWICBitmapDecoder> pDecoder;

	HRESULT hr = pWICFactory->CreateDecoderFromFilename(
		filename,
		nullptr,
		GENERIC_READ,
		WICDecodeMetadataCacheOnLoad,
		&pDecoder
	);

	if (!SUCCEEDED(hr)) {
		return false;
	}

	SmartPtr<IWICBitmapFrameDecode> pFrame;

	hr = pDecoder->GetFrame(0, &pFrame);

	if (!SUCCEEDED(hr)) {
		return false;
	}

	SmartPtr<IWICFormatConverter> pConverter;

	hr = pWICFactory->CreateFormatConverter(&pConverter);

	if (!SUCCEEDED(hr)) {
		return false;
	}

	hr = pConverter->Initialize(
		pFrame.get(),
		GUID_WICPixelFormat32bppPBGRA,
		WICBitmapDitherTypeNone,
		nullptr,
		0.0,
		WICBitmapPaletteTypeCustom
	);

	if (!SUCCEEDED(hr)) {
		return false;
	}

	pBitmap.reset();

	hr = pDeviceContext->CreateBitmapFromWicBitmap(
		pConverter.get(),
		nullptr,
		&pBitmap
	);

	if (!SUCCEEDED(hr)) {
		return false;
	}

	//Set the bitmap as the input to the first effect in the chain
	effectChain.front()->SetInput(0, pBitmap.get());

	return true;
}

void ImageUtil::buildEffectChain() {
	effectChain.clear();

	effectChain.push_back(brightnessEffect);
	effectChain.push_back(saturationEffect);
	effectChain.push_back(contrastEffect);
	effectChain.push_back(exposureEffect);

	if (_applyGrayscale) {
		effectChain.push_back(grayScaleEffect);
	}

	//Build the graph
	for (size_t i = 1; i < effectChain.size(); ++i) {
		effectChain[i]->SetInputEffect(0, effectChain[i - 1].get());
	}

	//Set the last effect as input to the scale effect
	scaleEffect->SetInputEffect(0, effectChain.back().get());
}

// Resize the render target when the window size changes
void ImageUtil::resize()
{
	RECT rc;

	GetClientRect(wnd, &rc);
	pRenderTarget->Resize(D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top));
}

// Render the loaded bitmap onto the window
void ImageUtil::render()
{
	pDeviceContext->BeginDraw();
	pDeviceContext->Clear(D2D1::ColorF(D2D1::ColorF::White));

	if (pBitmap) {
		pDeviceContext->DrawImage(
			scaleEffect.get()
		);
	}

	pDeviceContext->EndDraw();
}