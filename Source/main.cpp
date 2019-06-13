#include "stdafx.h"
#include "View.h"
#include "Widget.h"
#include "Help.h"
#include "SaveLoad.h"

HWND mainWindowHandle = NULL;
ID3D11Device* pd3dDevice = nullptr;
ID3D11DeviceContext* pImmediateContext = nullptr;
static IDXGISwapChain* pSwapChain = nullptr;
static ID3D11RenderTargetView* pRenderTargetView = nullptr;
ID3D11Texture2D* pBackBuffer = NULL;

#ifdef SAFE_RELEASE
#undef SAFE_RELEASE
#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p) = nullptr; } }
#endif

void windowSizePositionChanged();

// ---------------------------------------------------------------------
void WriteToBmp(const char* inFilePath);
int hk = 0;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_EXITSIZEMOVE:
		windowSizePositionChanged();
		return 0;

	case 0x0112: // window Maximize titlebar button
		if (wParam == SC_MAXIMIZE || wParam == SC_RESTORE) 
			SetTimer(hWnd, 2, 100, NULL);
		break;
	case WM_IME_NOTIFY:
		return 0;
	case WM_CREATE:
		SetTimer(hWnd, 1, 100, NULL);
		break;
	case WM_TIMER:
		if (wParam == 2) {
			KillTimer(hWnd, 2);
			windowSizePositionChanged();
		}
		else {
			widget.timerHandler();
		}
		break;
	case WM_KEYDOWN:
		widget.keyDown(wParam);
		break;
	case WM_KEYUP:
		widget.keyUp(wParam);
		break;
	case WM_LBUTTONDOWN:
		widget.lButtonDown(lParam);
		break;
	case WM_LBUTTONUP:
		widget.lButtonUp();
		break;
	case WM_RBUTTONDOWN:
		widget.rButtonDown(lParam);
		break;
	case WM_RBUTTONUP:
		widget.rButtonUp();
		break;
	case WM_MOUSEMOVE:
		widget.mouseMove(wParam, lParam);
		break;
	case WM_MOUSEWHEEL:
		widget.mouseWheelHandler(GET_WHEEL_DELTA_WPARAM(wParam));
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

static char* CLASS_NAME = (char *)"MainWin";
static char* WINDOW_NAME = (char*)"Param";

HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow) {
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = CLASS_NAME;
	wcex.hIconSm = NULL;
	if (!RegisterClassEx(&wcex)) {
		MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return E_FAIL;
	}

	windowHeight = 800; // (int)::GetSystemMetrics(SM_CYSCREEN) - 100;
	windowWidth = windowHeight; //  (int)::GetSystemMetrics(SM_CXSCREEN);
	RECT rc = { 0, 0, windowWidth,windowHeight };

	XSIZE = windowWidth;
	YSIZE = windowHeight;

	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	mainWindowHandle = CreateWindow(
		CLASS_NAME,
		WINDOW_NAME,
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		rc.right - rc.left,
		rc.bottom - rc.top,
		NULL, NULL, hInstance, NULL);
	if (mainWindowHandle == NULL) {
		MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return E_FAIL;
	}
	ShowWindow(mainWindowHandle, nCmdShow);

	widget.create(mainWindowHandle, hInstance);
	help.create(mainWindowHandle, hInstance);
	saveLoad.create(mainWindowHandle, hInstance);
	windowSizePositionChanged();
	return S_OK;
}

HRESULT InitializeD3D11(HWND hWnd) {
	HRESULT hr = S_OK;
	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] = {
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = windowWidth;
	sd.BufferDesc.Height = windowHeight;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 1;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	SAFE_RELEASE(pSwapChain);
	SAFE_RELEASE(pd3dDevice);
	SAFE_RELEASE(pImmediateContext);
	SAFE_RELEASE(pRenderTargetView);
	SAFE_RELEASE(pBackBuffer);

	// Create Device, DeviceContext, SwapChain, FeatureLevel
	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++) {
		driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain(NULL, driverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels, D3D11_SDK_VERSION,
			&sd, &pSwapChain, &pd3dDevice, &featureLevel, &pImmediateContext);
		if (SUCCEEDED(hr)) break;
	}
	ABORT(hr);

	// Create Render Target View Object from SwapChain's Back Buffer.
	// Access one of swap chain's back buffer.[0-based buffer index, interface type which manipulates buffer, output param]
//	ID3D11Texture2D* pBackBuffer = NULL;
	hr = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)& pBackBuffer);
	ABORT(hr);

	hr = pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &pRenderTargetView);
	//	pBackBuffer->Release();
	ABORT(hr);

	return S_OK;
}

void windowSizePositionChanged() {
	RECT r;
	GetClientRect(mainWindowHandle, &r);
	int xs = r.right - r.left;
	int ys = r.bottom - r.top;

	char str[132];
	sprintf_s(str, 131, "windowSizePositionChanged Size %d %d\n", xs,ys);
	OutputDebugStringA(str);

	if (xs != windowWidth || ys != windowHeight) {
		// change scaling so image looks same as before window size change
		IMAGE_DELTA *= float(windowWidth) / float(xs);
		IMAGE_DELTA *= float(windowHeight) / float(ys);
		windowWidth = xs;
		windowHeight = ys;
		XSIZE = xs;
		YSIZE = ys;

		ABORT(InitializeD3D11(mainWindowHandle));

		view.Initialize(pd3dDevice, pImmediateContext);
		widget.refresh();
	}
}

HRESULT Render(ID3D11DeviceContext* pContext, ID3D11RenderTargetView* pTargetView) {
	float ClearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
	pContext->OMSetRenderTargets(1, &pTargetView, NULL);
	pContext->ClearRenderTargetView(pTargetView, ClearColor);
	view.Render(pContext);
	pSwapChain->Present(0, 0);
	return S_OK;
}

// ----------------------------------------------------------------------------------

int WINAPI wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE, // hPrevInstance,
	_In_ LPWSTR, // lpCmdLine,
	_In_ int nShowCmd)
{
	LRESULT ret = InitWindow(hInstance, nShowCmd);
	if (ret != S_OK) { exit(-11); }

	ret = InitializeD3D11(mainWindowHandle);
	if (ret != S_OK) { exit(-12); }

	view.Initialize(pd3dDevice, pImmediateContext);

	MSG msg = { 0 };
	while (WM_QUIT != msg.message) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			//		fractal.update();
			Render(pImmediateContext, pRenderTargetView);
		}
	}

	SAFE_RELEASE(pBackBuffer);
	SAFE_RELEASE(pRenderTargetView);
	SAFE_RELEASE(pSwapChain);
	view.Destroy();
	return (int)msg.wParam;
}

// ===============================

void abortProgram(char* name, int line) {
	char str[256];
	sprintf_s(str, 255, "Error in file % s, line % d", name, line);

	MessageBox(NULL, str, "Program Exit", MB_ICONEXCLAMATION | MB_OK);
	exit(-1);
}
