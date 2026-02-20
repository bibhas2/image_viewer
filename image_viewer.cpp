// image_viewer.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "image_viewer.h"
#include <wincodec.h>
#include <string>

#include <shobjidl.h>
#include "ImageUtil.h"
#include <commdlg.h>
#include <mgui.h>

#pragma comment(lib, "D3D11.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "windowscodecs.lib")
//We need dxguid.lib for some of the CLSID and IID definitions
#pragma comment(lib, "dxguid.lib")

class MainWindow : public CFrame {
public:
	ImageUtil imUtil;

    void create() {
        CFrame::create(L"Image Viewer", 800, 600, IDC_IMAGEVIEWER);
		imUtil.init(m_wnd);
	}

    void onCommand(int id, int type, CWindow* source) override {
        if (id == ID_FILE_CHOOSEFOLDER) {
            openImage();
        }
        else if (id == IDM_ABOUT) {
            //DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), m_wnd, About);
        }
        else if (id == ID_VIEW_ZOOMIN) {
            imUtil.scale(imUtil.scale() * 1.2f);
            InvalidateRect(m_wnd, NULL, FALSE);
        }
        else if (id == ID_VIEW_ZOOMOUT) {
            imUtil.scale(imUtil.scale() / 1.2f);
            InvalidateRect(m_wnd, NULL, FALSE);
        }
        else if (id == ID_VIEW_ACTUALSIZE) {
            imUtil.scale(1.0f);
            InvalidateRect(m_wnd, NULL, FALSE);
        }
        else if (id == ID_VIEW_FITWINDOW) {
			imUtil.scaleToFit();
            InvalidateRect(m_wnd, NULL, FALSE);
        }
        else if (id == ID_FILE_EXPORTIMAGE) {
			saveImage();
		}
        else if (id == IDM_EXIT) {
            CWindow::stop();
        }
        else {
            CFrame::onCommand(id, type, source);
        }
	}

    void saveImage() {
        std::wstring selected_name;
        std::vector<COMDLG_FILTERSPEC> filters =
        {
            { L"Image Files", L"*.jpg;*.jpeg;*.png;*.tiff;*.tif" },
            { L"JPEG Image", L"*.jpg;*.jpeg" },
            { L"PNG Image", L"*.png" },
            { L"TIFF Image", L"*.tiff;*.tif" },
            { L"All Files", L"*.*" },
        };

        if (!saveFileName(L"Export Image", filters, selected_name)) {
            return;
        }

        if (selected_name.empty()) {
            return;
        }

        GUID formatId;

        if (!imUtil.formatForFileExtension(selected_name, formatId)) {
            messageBox(L"Unsupported file extension.\n\nOnly JPEG and PNG files are supported.");

            return;
		}

        imUtil.saveImageToFile(selected_name);
	}

    void openImage() {
        std::wstring selected_name;
        std::vector<COMDLG_FILTERSPEC> filters =
        {
            { L"Image Files", L"*.jpg;*.jpeg;*.png;*.tiff;*.tif" },
            { L"JPEG Image", L"*.jpg;*.jpeg" },
            { L"PNG Image", L"*.png" },
            { L"TIFF Image", L"*.tiff;*.tif" },
            { L"All Files", L"*.*" },
        };

        if (!openFileName(L"Open Image", filters, selected_name)) {
            return;
        }

        if (!selected_name.empty()) {
            imUtil.loadImageFromFile(selected_name.c_str());

            InvalidateRect(m_wnd, NULL, FALSE);
        }
    }

    bool handleEvent(UINT message, WPARAM wParam, LPARAM lParam) {
        switch (message) {
        case WM_PAINT:
            PAINTSTRUCT ps;

			//We must call BeginPaint and EndPaint to validate the
			//invalidated region, or else we will get continuous
			//WM_PAINT messages.
			BeginPaint(m_wnd, &ps);
            imUtil.render();
			EndPaint(m_wnd, &ps);
            break;
        case WM_SIZE:
            imUtil.resize();
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        case WM_ERASEBKGND:
            break;
        default:
            return CWindow::handleEvent(message, wParam, lParam);
        }

        return true;
    }

    void onClose() {
        CWindow::stop();
    }
};

struct ToolsWindow : public CFrame {
    //Define a constant for width as 30
    static const int 
        WIDTH = 200,
        GAP = 4,
        LABEL_H = 14,
        TRACKBAR_H = 25;

    static const int
        ID_GRAYSCALE = 2001,
        ID_WHITEPOINTX = 2002,
        ID_WHITEPOINTY = 2003,
        ID_BLACKPOINTX = 2004,
        ID_BLACKPOINTY = 2005,
        ID_EXPOSURE = 2006,
	    ID_INVERT = 2007;

    MainWindow& mainWindow;
    CCheckBox grayscale;
	CCheckBox invert;
	CTrackBar highlights;
	CTrackBar shadows;
	CTrackBar whitePointX;
	CTrackBar whitePointY;
	CTrackBar blackPointX;
	CTrackBar blackPointY;
	CTrackBar saturation;
	CTrackBar contrast;
	CTrackBar exposure;
	CTrackBar temperature;
	CTrackBar tint;

    ToolsWindow(MainWindow& w) : mainWindow(w) {

    }

    void create() {
        CFrame::create(L"Tools", WIDTH + 8 * GAP, 640);

		//Hide the minimize, maximize and resize options
        LONG_PTR style = GetWindowLongPtr(getWindow(), GWL_STYLE);
        // Remove maximize, minimize, and thickframe (resizing border)
        style &= ~(WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_THICKFRAME);
        SetWindowLongPtr(getWindow(), GWL_STYLE, style);

        int y = GAP;

		//Create invert checkbox
		invert.create(L"Invert Colors", GAP, y, WIDTH, LABEL_H, this, (HMENU)ID_INVERT);
		//Set initial state
		invert.setCheck(mainWindow.imUtil.applyInvert());
		y += LABEL_H + GAP;

        grayscale.create(L"Grayscale", GAP, y, WIDTH, LABEL_H, this, (HMENU) ID_GRAYSCALE);
		//Set initial state
		grayscale.setCheck(mainWindow.imUtil.applyGrayscale());
		y += LABEL_H + GAP;

		// Highlights
		CLabel().create(L"Highlights", GAP, y, WIDTH, LABEL_H, this);
		y += LABEL_H + GAP;
		highlights.create(GAP, y, WIDTH, TRACKBAR_H, this, (HMENU)0);
		highlights.setMin(-100);
		highlights.setMax(100);
        highlights.setPos((int)(mainWindow.imUtil.highlights() * 100));
		y += TRACKBAR_H + GAP;

		// Shadows
		CLabel().create(L"Shadows", GAP, y, WIDTH, LABEL_H, this);
		y += LABEL_H + GAP;
		shadows.create(GAP, y, WIDTH, TRACKBAR_H, this, (HMENU)0);
		shadows.setMin(-100);
		shadows.setMax(100);
		shadows.setPos((int)(mainWindow.imUtil.shadows() * 100));
		y += TRACKBAR_H + GAP;

		CLabel().create(L"White Point X", GAP, y, WIDTH, LABEL_H, this);
		y += LABEL_H + GAP;
		whitePointX.create(GAP, y, WIDTH, TRACKBAR_H, this, (HMENU) ID_WHITEPOINTX);
		whitePointX.setMin(0);
		whitePointX.setMax(100);
		whitePointX.setPos((int)(mainWindow.imUtil.whitePointX() * 100));

		y += TRACKBAR_H + GAP;

		CLabel().create(L"White Point Y", GAP, y, WIDTH, LABEL_H, this);
		y += LABEL_H + GAP;
		whitePointY.create(GAP, y, WIDTH, TRACKBAR_H, this, (HMENU)ID_WHITEPOINTY);
		whitePointY.setMin(0);
		whitePointY.setMax(100);
		whitePointY.setPos((int)(mainWindow.imUtil.whitePointY() * 100));

		y += TRACKBAR_H + GAP;
		CLabel().create(L"Black Point X", GAP, y, WIDTH, LABEL_H, this);
		y += LABEL_H + GAP;
		blackPointX.create(GAP, y, WIDTH, TRACKBAR_H, this, (HMENU)ID_BLACKPOINTX);
		blackPointX.setMin(0);
		blackPointX.setMax(100);
		blackPointX.setPos((int)(mainWindow.imUtil.blackPointX() * 100));

		y += TRACKBAR_H + GAP;
		CLabel().create(L"Black Point Y", GAP, y, WIDTH, LABEL_H, this);
		y += LABEL_H + GAP;
		blackPointY.create(GAP, y, WIDTH, TRACKBAR_H, this, (HMENU)ID_BLACKPOINTY);
		blackPointY.setMin(0);
		blackPointY.setMax(100);
		blackPointY.setPos((int)(mainWindow.imUtil.blackPointY() * 100));

        y += TRACKBAR_H + GAP;

		CLabel().create(L"Saturation", GAP, y, WIDTH, LABEL_H, this);
		y += LABEL_H + GAP;
		saturation.create(GAP, y, WIDTH, TRACKBAR_H, this, (HMENU)0);
		saturation.setMin(0);
		saturation.setMax(100);
		saturation.setPos((int)(mainWindow.imUtil.saturation() * 100 / 2.0));

        y += TRACKBAR_H + GAP;

		CLabel().create(L"Contrast", GAP, y, WIDTH, LABEL_H, this);
		y += LABEL_H + GAP;
		contrast.create(GAP, y, WIDTH, TRACKBAR_H, this, (HMENU)0);
		contrast.setMin(-100);
		contrast.setMax(100);
		contrast.setPos((int)(mainWindow.imUtil.contrast() * 100));

        y += TRACKBAR_H + GAP;

		// Exposure
		CLabel().create(L"Exposure", GAP, y, WIDTH, LABEL_H, this);
		y += LABEL_H + GAP;
		exposure.create(GAP, y, WIDTH, TRACKBAR_H, this, (HMENU)ID_EXPOSURE);
		// Map exposure [-2.0,2.0] to trackbar [-200,200]
		exposure.setMin(-200);
		exposure.setMax(200);
		exposure.setPos((int)(mainWindow.imUtil.exposure() * 100));

        y += TRACKBAR_H + GAP;

		// Temperature
		CLabel().create(L"Temperature", GAP, y, WIDTH, LABEL_H, this);
		y += LABEL_H + GAP;
		temperature.create(GAP, y, WIDTH, TRACKBAR_H, this, (HMENU)0);
		temperature.setMin(-100);
		temperature.setMax(100);
		temperature.setPos((int)(mainWindow.imUtil.temperature() * 100));
		y += TRACKBAR_H + GAP;

		// Tint
		CLabel().create(L"Tint", GAP, y, WIDTH, LABEL_H, this);
		y += LABEL_H + GAP;
		tint.create(GAP, y, WIDTH, TRACKBAR_H, this, (HMENU)0);
		tint.setMin(-100);
		tint.setMax(100);
		tint.setPos((int)(mainWindow.imUtil.tint() * 100));
        y += TRACKBAR_H + GAP;
    }

	//Implement onCommand
    void onCommand(int id, int type, CWindow* source) override {
        if (id == ID_GRAYSCALE) {
			//Toggle checkbox state
			grayscale.setCheck(!grayscale.getCheck());
			//Apply grayscale effect
            mainWindow.imUtil.applyGrayscale(grayscale.getCheck());
			mainWindow.imUtil.redraw();
		}
        else if (id == ID_INVERT) {
            //Toggle checkbox state
            invert.setCheck(!invert.getCheck());
            //Apply invert effect
            mainWindow.imUtil.applyInvert(invert.getCheck());
            mainWindow.imUtil.redraw();
        }
        else {
            CFrame::onCommand(id, type, source);
        }
	}

    bool handleEvent(UINT message, WPARAM wParam, LPARAM lParam) {
        HWND hwndCtl = (HWND)lParam;

        switch (message) {
        case WM_HSCROLL:
            if (hwndCtl == whitePointX.getWindow()) {
                mainWindow.imUtil.whitePointX(whitePointX.getPos() / 100.0f);
            }
            else if (hwndCtl == whitePointY.getWindow()) {
                mainWindow.imUtil.whitePointY(whitePointY.getPos() / 100.0f);
            }
            else if (hwndCtl == blackPointX.getWindow()) {
                mainWindow.imUtil.blackPointX(blackPointX.getPos() / 100.0f);
            }
            else if (hwndCtl == blackPointY.getWindow()) {
                mainWindow.imUtil.blackPointY(blackPointY.getPos() / 100.0f);
            }
            else if (hwndCtl == saturation.getWindow()) {
                mainWindow.imUtil.saturation(saturation.getPos() / 100.0f * 2.0f);
            }
            else if (hwndCtl == contrast.getWindow()) {
                mainWindow.imUtil.contrast(contrast.getPos() / 100.0f);
            }
			else if (hwndCtl == exposure.getWindow()) {
				// Trackbar gives [-200,200]; convert to [-2.0,2.0]
				mainWindow.imUtil.exposure(exposure.getPos() / 100.0f);
			}
			else if (hwndCtl == temperature.getWindow()) {
				mainWindow.imUtil.temperature(temperature.getPos() / 100.0f);
			}
			else if (hwndCtl == tint.getWindow()) {
				mainWindow.imUtil.tint(tint.getPos() / 100.0f);
			}
			else if (hwndCtl == highlights.getWindow()) {
				mainWindow.imUtil.highlights(highlights.getPos() / 100.0f);
			}
			else if (hwndCtl == shadows.getWindow()) {
				mainWindow.imUtil.shadows(shadows.getPos() / 100.0f);
			}

            mainWindow.imUtil.redraw();

            break;
        default:
            return CFrame::handleEvent(message, wParam, lParam);
        }

        return true;
    }
};

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
	CWindow::init(hInstance, IDC_IMAGEVIEWER);

	MainWindow mainWin;

    mainWin.create();
    mainWin.show();

    ToolsWindow toolsWindow(mainWin);

    toolsWindow.create();
    toolsWindow.show();

	CWindow::loop();

	return 0;
}