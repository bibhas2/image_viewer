#include <windows.h>
#include "ImageUtil.h"
//Some of the effects CLSIDs are defined in d2d1effects_2.h
#include <d2d1effects_2.h>

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

	return true;
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
		// Get the size of the bitmap
		D2D1_SIZE_F bmpSize = pBitmap->GetSize();

		// Get the size of the render target
		D2D1_SIZE_F rtSize = pRenderTarget->GetSize();

		SmartPtr<ID2D1Effect> lastEffect;

		brightnessEffect->SetValue(D2D1_BRIGHTNESS_PROP_WHITE_POINT, D2D1::Vector2F(whitePointX, whitePointY));
		brightnessEffect->SetValue(D2D1_BRIGHTNESS_PROP_BLACK_POINT, D2D1::Vector2F(blackPointX, blackPointY));

		brightnessEffect->SetInput(0, pBitmap.get());
		lastEffect = brightnessEffect;
		
		if (applyGrayscale) {
			grayScaleEffect->SetInputEffect(0, lastEffect.get());
			
			lastEffect = grayScaleEffect;
		}

		scaleEffect->SetValue(D2D1_SCALE_PROP_SCALE, D2D1::Vector2F(scale, scale));
		scaleEffect->SetInputEffect(0, lastEffect.get());

		pDeviceContext->DrawImage(
			scaleEffect.get()
		);
	}

	pDeviceContext->EndDraw();
}