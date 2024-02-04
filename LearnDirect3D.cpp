#include <Windows.h>
#include <d3d11.h>

#include <d3dcompiler.h>

#include <array>


#pragma comment(lib,"d3dcompiler.lib")

class Device {

public:

    Device(HWND hWnd) {

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

        const D3D_FEATURE_LEVEL featureLevelArray[] = {
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_0,
        };

        HRESULT res = D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            0, // D3D11_CREATE_DEVICE_DEBUG
            featureLevelArray,
            2,
            D3D11_SDK_VERSION,
            &sd,
            &pSwapChain,
            &pDevice,
            nullptr,
            &pDeviceContext);

        if (res != S_OK) {
            throw "fuck";
        }

        ID3D11Texture2D* pBackBuffer = nullptr;
        pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        if (!pBackBuffer) {
            throw "fuck";
        }
        pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &pRenderTargetView);
        pBackBuffer->Release();

    }

    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;

    void DrawTestTriangle() {

        HRESULT hr;

        struct Vertex {
            float x;
            float y;
        };

        const Vertex vertices[] = {
            {0.0f, 0.5f},
            {0.5f, -0.5f},
            {-0.5f, -0.5f},
        };


        ID3D11Buffer* pVertexBuffer = nullptr;
        D3D11_BUFFER_DESC bd = { 0 };
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.CPUAccessFlags = 0u;
        bd.MiscFlags = 0u;
        bd.ByteWidth = sizeof(vertices);
        bd.StructureByteStride = sizeof(Vertex);


        D3D11_SUBRESOURCE_DATA sd = { 0 };
        sd.pSysMem = vertices;

        

        hr = pDevice->CreateBuffer(&bd, &sd, &pVertexBuffer);
        if (hr != S_OK) {
            throw "asd";
        }
        const UINT stride = sizeof(Vertex);
        const UINT offset = 0u;
        pDeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &stride, &offset);







        ID3D11VertexShader *pVertexShader;

        ID3DBlob *pBlob;
        D3DReadFileToBlob(L"VertexShader.cso", &pBlob);

        pDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pVertexShader);
        pDeviceContext->VSSetShader(pVertexShader, nullptr, 0u);


        ID3D11InputLayout* pInputLayout = nullptr;
        const D3D11_INPUT_ELEMENT_DESC ied[] = {
            {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        };
        pDevice->CreateInputLayout(
            ied,
            (UINT)std::size(ied),
            pBlob->GetBufferPointer(),
            pBlob->GetBufferSize(),
            &pInputLayout);

        pDeviceContext->IASetInputLayout(pInputLayout);



        pBlob->Release();

        ID3D11PixelShader* pPixelShader;
        D3DReadFileToBlob(L"PixelShader.cso", &pBlob);
        pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pPixelShader);

        pDeviceContext->PSSetShader(pPixelShader, nullptr, 0u);





        pDeviceContext->OMSetRenderTargets(1u, &pRenderTargetView, nullptr);

        pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);





        D3D11_VIEWPORT vp = { 0 };
        vp.Width = 800;
        vp.Height = 600;
        vp.MinDepth = 0;
        vp.MaxDepth = 1;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        pDeviceContext->RSSetViewports(1u, &vp);


        pDeviceContext->Draw(std::size(vertices), 0u);

    }

    ~Device() {
        if (pRenderTargetView) {
            pRenderTargetView->Release();
            pRenderTargetView = nullptr;
        }
        if (pSwapChain) {
            pSwapChain->Release();
            pSwapChain = nullptr;
        }
        if (pDeviceContext) {
            pDeviceContext->Release();
            pDeviceContext = nullptr;
        }
        if (pDevice) {
            pDevice->Release();
            pDevice = nullptr;
        }
    }

public:
    ID3D11Device*           pDevice           = nullptr;
    ID3D11DeviceContext*    pDeviceContext    = nullptr;
    IDXGISwapChain*         pSwapChain        = nullptr;
    ID3D11RenderTargetView* pRenderTargetView = nullptr;
};

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

    switch (msg) {

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

    auto d = Device(hWnd);

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

        const float clearColor[4] = { 0.5f, 0.f, 0.f, 1.f };

        d.pDeviceContext->ClearRenderTargetView(d.pRenderTargetView, clearColor);

        d.DrawTestTriangle();
        //g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        //g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clearColor);

        d.pSwapChain->Present(1, 0); // Present with vsync
        // g_pSwapChain->Present(0, 0); // Present without vsync
    }

    DestroyWindow(hWnd);
    UnregisterClass(wc.lpszClassName, wc.hInstance);
	return 0;
}
