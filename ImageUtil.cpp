#include <windows.h>
#include "ImageUtil.h"

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

	return true;
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

	hr = pRenderTarget->CreateBitmapFromWicBitmap(
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
	pRenderTarget->BeginDraw();
	pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

	if (pBitmap) {
		D2D1_SIZE_F rtSize = pRenderTarget->GetSize();
		D2D1_SIZE_F bmSize = pBitmap->GetSize();

		float scaleX = rtSize.width / bmSize.width;
		float scaleY = rtSize.height / bmSize.height;
		float scale = min(scaleX, scaleY);

		float drawWidth = bmSize.width * scale;
		float drawHeight = bmSize.height * scale;

		float offsetX = (rtSize.width - drawWidth) / 2.0f;
		float offsetY = (rtSize.height - drawHeight) / 2.0f;

		D2D1_RECT_F destRect = D2D1::RectF(
			offsetX,
			offsetY,
			offsetX + drawWidth,
			offsetY + drawHeight
		);

		pRenderTarget->DrawBitmap(
			pBitmap.get(),
			destRect,
			1.0f,
			D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
			D2D1::RectF(0.0f, 0.0f, bmSize.width, bmSize.height)
		);
	}

	pRenderTarget->EndDraw();
}