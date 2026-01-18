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
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "windowscodecs.lib")
//We need dxguid.lib for some of the CLSID and IID definitions
#pragma comment(lib, "dxguid.lib")

bool select_file(HWND hWnd, bool is_folder, std::wstring& selected_name) {
    SmartPtr<IFileOpenDialog> pFileOpen;

    // 2. Create the FileOpenDialog object
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
        IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

    if (SUCCEEDED(hr)) {
        // 3. Set the dialog to "Pick Folders" mode
        DWORD dwOptions;
        if (is_folder && SUCCEEDED(pFileOpen->GetOptions(&dwOptions))) {
            pFileOpen->SetOptions(dwOptions | FOS_PICKFOLDERS);
        }

        // 4. Show the dialog box
        hr = pFileOpen->Show(hWnd);

        // 5. Get the result (if the user didn't cancel)
        if (SUCCEEDED(hr)) {
            SmartPtr<IShellItem> pItem;
            hr = pFileOpen->GetResult(&pItem);
            if (SUCCEEDED(hr)) {
                PWSTR pszFilePath;
                // Get the file system path from the Shell item
                hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                // Display the chosen path
                if (SUCCEEDED(hr)) {
                    selected_name = pszFilePath;

                    CoTaskMemFree(pszFilePath); // Free the string allocated by the Shell

                    return true;
                }
            }
        }
    }

	return false;
}

class MainWindow : public CFrame {
public:
	ImageUtil imUtil;

    void create() {
        CFrame::create("Image Viewer", 800, 600, IDC_IMAGEVIEWER);
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
            imUtil.scale *= 1.2f;
            InvalidateRect(m_wnd, NULL, FALSE);
        }
        else if (id == ID_VIEW_ZOOMOUT) {
            imUtil.scale /= 1.2f;
            InvalidateRect(m_wnd, NULL, FALSE);
        }
        else if (id == ID_VIEW_ACTUALSIZE) {
            imUtil.scale = 1.0f;
            InvalidateRect(m_wnd, NULL, FALSE);
        }
        else {
            CFrame::onCommand(id, type, source);
        }
	}

    void openImage() {
        std::wstring selected_name;

        if (!select_file(m_wnd, false, selected_name)) {
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
            imUtil.render();
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
        ID_BLACKPOINTY = 2005;

    MainWindow& mainWindow;
    CCheckBox grayscale;
	CTrackBar whitePointX;
	CTrackBar whitePointY;
	CTrackBar blackPointX;
	CTrackBar blackPointY;

    ToolsWindow(MainWindow& w) : mainWindow(w) {

    }

    void create() {
        CFrame::create("Tools", WIDTH + 4 * GAP, 200);

        int y = GAP;

        grayscale.create("Grayscale", GAP, y, WIDTH, LABEL_H, this, (HMENU) ID_GRAYSCALE);
		//Set initial state
		grayscale.setCheck(mainWindow.imUtil.applyGrayscale);

		y += LABEL_H + GAP;

		CLabel().create("White Point X", GAP, y, WIDTH, LABEL_H, this);
		y += LABEL_H + GAP;
		whitePointX.create("", GAP, y, WIDTH, TRACKBAR_H, this, (HMENU) ID_WHITEPOINTX);
		whitePointX.setMin(0);
		whitePointX.setMax(100);
		whitePointX.setPos((int)(mainWindow.imUtil.whitePointX * 100));

		y += TRACKBAR_H + GAP;

		CLabel().create("White Point Y", GAP, y, WIDTH, LABEL_H, this);
		y += LABEL_H + GAP;
		whitePointY.create("", GAP, y, WIDTH, TRACKBAR_H, this, (HMENU)ID_WHITEPOINTY);
		whitePointY.setMin(0);
		whitePointY.setMax(100);
		whitePointY.setPos((int)(mainWindow.imUtil.whitePointY * 100));
    }

	//Implement onCommand
    void onCommand(int id, int type, CWindow* source) override {
        if (id == ID_GRAYSCALE) {
			//Toggle checkbox state
			grayscale.setCheck(!grayscale.getCheck());
			//Apply grayscale effect
            mainWindow.imUtil.applyGrayscale = grayscale.getCheck();
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
                mainWindow.imUtil.whitePointX = whitePointX.getPos() / 100.0f;
            }
            else if (hwndCtl == whitePointY.getWindow()) {
                mainWindow.imUtil.whitePointY = whitePointY.getPos() / 100.0f;
            }
            else if (hwndCtl == blackPointX.getWindow()) {
                mainWindow.imUtil.blackPointX = blackPointX.getPos() / 100.0f;
            }
            else if (hwndCtl == blackPointY.getWindow()) {
                mainWindow.imUtil.blackPointY = blackPointY.getPos() / 100.0f;
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
	CWindow::init(hInstance);

    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

    if (!SUCCEEDED(hr)) {
        return FALSE;
    }

	MainWindow mainWin;

    mainWin.create();
    mainWin.show();

    ToolsWindow toolsWindow(mainWin);

    toolsWindow.create();
    toolsWindow.show();

	CWindow::loop();

    CoUninitialize();

	return 0;
}