#include <Windows.h>
#include <d3d11.h>

static ID3D11Device*            g_pd3dDevice           = nullptr;
static ID3D11DeviceContext*     g_pd3dDeviceContext    = nullptr;
static IDXGISwapChain*          g_pSwapChain           = nullptr;
static UINT                     g_ResizeWidth          = 0;
static UINT                     g_ResizeHeight         = 0;
static ID3D11RenderTargetView*  g_mainRenderTargetView = nullptr;

bool CreateRenderTarget() {
    ID3D11Texture2D* pBackBuffer = nullptr;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));

    if (pBackBuffer) {
        g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
        pBackBuffer->Release();
        return true;
    }

    return false;
}

void CleanupRenderTarget() {
    if (g_mainRenderTargetView) { 
        g_mainRenderTargetView->Release(); 
        g_mainRenderTargetView = nullptr; 
    }
}

bool CreateDeviceD3D(HWND hWnd) {

    DXGI_SWAP_CHAIN_DESC sd = { 0 };

    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
#ifndef NDEBUG
    // For Windows 10 or 11, to create a device that supports the debug layer, enable the "Graphics Tools" optional feature. 
    // Go to the Settings panel, Optional features, View features, and then look for "Graphics Tools".
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif // DEBUG

    D3D_FEATURE_LEVEL featureLevel;

    const D3D_FEATURE_LEVEL featureLevelArray[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_0,
    };

    HRESULT res = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        createDeviceFlags,
        featureLevelArray,
        2,
        D3D11_SDK_VERSION,
        &sd,
        &g_pSwapChain,
        &g_pd3dDevice,
        &featureLevel,
        &g_pd3dDeviceContext);

    if (res != S_OK) {
        return false;
    }

    return CreateRenderTarget();
}

void CleanupDeviceD3D() {

    CleanupRenderTarget();

    if (g_pSwapChain) { 
        g_pSwapChain->Release(); 
        g_pSwapChain = nullptr; 
    }
    if (g_pd3dDeviceContext) { 
        g_pd3dDeviceContext->Release(); 
        g_pd3dDeviceContext = nullptr; 
    }
    if (g_pd3dDevice) { 
        g_pd3dDevice->Release(); 
        g_pd3dDevice = nullptr; 
    }
}

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

    switch (msg) {

    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

int main(void) {

    const TCHAR szTitle[]       = TEXT("LearnDirect3D");
    const TCHAR szWindowClass[] = TEXT("LearnDirect3D");

    WNDCLASSEX wc = {
        .cbSize        = sizeof(wc),
        .style         = CS_CLASSDC,
        .lpfnWndProc   = WndProc,
        .hInstance     = GetModuleHandle(nullptr),
        .lpszClassName = szWindowClass,
    };

    if (RegisterClassEx(&wc) == 0) {
        return 1;
    }

    HWND hWnd = CreateWindowEx(
        0,
        szWindowClass,
        szTitle,
        WS_MINIMIZEBOX,
        0,
        0,
        1920,
        1080,
        nullptr,
        nullptr,
        wc.hInstance,
        nullptr);

    if (!hWnd) {
        return 1;
    }

    if (!CreateDeviceD3D(hWnd)) {
        goto cleanup;
    }

    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);

    while (true) {

        MSG msg;
        while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT) {
                break;
            }
        }

        // Handle window resize (we don't resize directly in the WM_SIZE handler)
        if (g_ResizeWidth != 0 && g_ResizeHeight != 0) {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            CreateRenderTarget();
        }

        const float clearColor[4] = { 0.f, 0.f, 0.f, 1.f };

        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clearColor);

        g_pSwapChain->Present(1, 0); // Present with vsync
        // g_pSwapChain->Present(0, 0); // Present without vsync
    }

cleanup:
    CleanupDeviceD3D();
    DestroyWindow(hWnd);
    UnregisterClass(wc.lpszClassName, wc.hInstance);
	return 0;
}
