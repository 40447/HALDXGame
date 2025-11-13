#include "d3dApp.h"
#include "DXTrace.h"
#include "XUtil.h"
#include <sstream>

#pragma warning(disable : 6031)

extern "C" {
// 複数GPU環境ではNVIDIAやAMDの高性能GPUを優先して使用
__declspec(dllexport) uint32_t NvOptimusEnablement = 0x00000001;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 0x00000001;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace {
    // Windowsメッセージをグローバルコールバックからメンバー関数に転送するためのポインタ
    D3DApp *g_pd3dApp = nullptr;
}

LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    // CreateWindow が返事される前にWM_CREATEが発生する場合がありうるため
    return g_pd3dApp->MsgProc(hwnd, msg, wParam, lParam);
}

D3DApp::D3DApp(HINSTANCE hInstance, const std::wstring &windowName, int initWidth, int initHeight)
    : m_hAppInst(hInstance)
      , m_MainWndCaption(windowName)
      , m_ClientWidth(initWidth)
      , m_ClientHeight(initHeight)
      , m_hMainWnd(nullptr)
      , m_AppPaused(false)
      , m_Minimized(false)
      , m_Maximized(false)
      , m_Resizing(false) {
    // このクラスのインスタンスをグローバル変数に保持
    g_pd3dApp = this;
}

D3DApp::~D3DApp() {
    // D3Dのステートをクリアし、ImGuiを解放
    if (m_pd3dImmediateContext)
        m_pd3dImmediateContext->ClearState();

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

HINSTANCE D3DApp::AppInst() const { return m_hAppInst; }
HWND D3DApp::MainWnd() const { return m_hMainWnd; }
float D3DApp::AspectRatio() const { return static_cast<float>(m_ClientWidth) / m_ClientHeight; }

int D3DApp::Run() {
    MSG msg = {0};
    m_Timer.Reset();

    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            m_Timer.Tick();

            if (!m_AppPaused) {
                CalculateFrameStats();
                ImGui_ImplDX11_NewFrame();
                ImGui_ImplWin32_NewFrame();
                ImGui::NewFrame();
                PreUpdate(m_Timer.DeltaTime());
                Update(m_Timer.DeltaTime());
                PostUpdate(m_Timer.DeltaTime());
                ImGui::Render();
                Draw();
                m_FrameCount++;
            } else {
                Sleep(100);
            }
        }
    }

    return (int) msg.wParam;
}

bool D3DApp::Init() {
    if (!InitMainWindow()) return false;
    if (!InitDirect3D()) return false;
    if (!InitImGui()) return false;
    return true;
}

// ウィンドウサイズ変更時の再設定
void D3DApp::OnResize() {
    assert(m_pd3dImmediateContext);
    assert(m_pd3dDevice);
    assert(m_pSwapChain);

    if (m_pd3dDevice1 != nullptr) {
        assert(m_pd3dImmediateContext1);
        assert(m_pd3dDevice1);
        assert(m_pSwapChain1);
    }

    // バックバッファをリセットし、レンダーターゲットビューを再生成
    for (UINT i = 0; i < m_BackBufferCount; ++i)
        m_pRenderTargetViews[i].Reset();
    HR(m_pSwapChain->ResizeBuffers(m_BackBufferCount, m_ClientWidth, m_ClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM,
        m_IsDxgiFlipModel ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0));
    m_FrameCount = 0;
}

// ウィンドウメッセージ処理
LRESULT D3DApp::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(m_hMainWnd, msg, wParam, lParam))
        return true;

    switch (msg) {
        case WM_ACTIVATE:
            if (LOWORD(wParam) == WA_INACTIVE) {
                //m_AppPaused = true;
                //m_Timer.Stop();
            } else {
                //m_AppPaused = false;
                //m_Timer.Start();
            }
            return 0;

        case WM_SIZE:
            m_ClientWidth = LOWORD(lParam);
            m_ClientHeight = HIWORD(lParam);
            if (m_pd3dDevice) {
                if (wParam == SIZE_MINIMIZED) {
                    //m_AppPaused = true;
                    m_Minimized = true;
                    m_Maximized = false;
                } else if (wParam == SIZE_MAXIMIZED) {
                    //m_AppPaused = false;
                    m_Minimized = false;
                    m_Maximized = true;
                    OnResize();
                } else if (wParam == SIZE_RESTORED) {
                    if (m_Minimized) {
                        //m_AppPaused = false;
                        m_Minimized = false;
                        OnResize();
                    } else if (m_Maximized) {
                        //m_AppPaused = false;
                        m_Maximized = false;
                        OnResize();
                    } else if (!m_Resizing) {
                        OnResize();
                    }
                }
            }
            return 0;

        case WM_ENTERSIZEMOVE:
            //m_AppPaused = true;
            m_Resizing = true;
            //m_Timer.Stop();
            return 0;

        case WM_EXITSIZEMOVE:
            //m_AppPaused = false;
            m_Resizing = false;
            //m_Timer.Start();
            OnResize();
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_MENUCHAR:
            return MAKELRESULT(0, MNC_CLOSE);

        case WM_GETMINMAXINFO:
            ((MINMAXINFO *) lParam)->ptMinTrackSize.x = 200;
            ((MINMAXINFO *) lParam)->ptMinTrackSize.y = 200;
            return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

bool D3DApp::InitMainWindow() {
    WNDCLASS wc;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = MainWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = m_hAppInst;
    wc.hIcon = LoadIcon(0, IDI_APPLICATION);
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = (HBRUSH) GetStockObject(NULL_BRUSH);
    wc.lpszMenuName = 0;
    wc.lpszClassName = L"D3DWndClassName";

    // ウィンドウクラスを登録する
    if (!RegisterClass(&wc)) {
        MessageBox(0, L"RegisterClass Failed.", 0, 0);
        return false;
    }

    // 画面中央にウィンドウを配置するため、スクリーンサイズを取得
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // クライアント領域のサイズからウィンドウ全体のサイズを計算
    RECT R = {0, 0, m_ClientWidth, m_ClientHeight};
    AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
    int width = R.right - R.left;
    int height = R.bottom - R.top;

    // ウィンドウを作成
    m_hMainWnd = CreateWindow(L"D3DWndClassName", m_MainWndCaption.c_str(),
                              WS_OVERLAPPEDWINDOW, (screenWidth - width) / 2, (screenHeight - height) / 2, width,
                              height, 0, 0, m_hAppInst, 0);

    if (!m_hMainWnd) {
        MessageBox(0, L"CreateWindow Failed.", 0, 0);
        return false;
    }

    ShowWindow(m_hMainWnd, SW_SHOW);
    UpdateWindow(m_hMainWnd);

    return true;
}

bool D3DApp::InitDirect3D() {
    HRESULT hr = S_OK;

    // D3Dデバイスとデバイスコンテキストを作成
    UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG; // デバッグ用フラグ
#endif

    // 使用するドライバタイプ
    D3D_DRIVER_TYPE driverTypes[] = {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT numDriverTypes = ARRAYSIZE(driverTypes);

    // 対応する機能レベル
    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };
    UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    D3D_FEATURE_LEVEL featureLevel;
    D3D_DRIVER_TYPE d3dDriverType;
    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++) {
        d3dDriverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDevice(nullptr, d3dDriverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
                               D3D11_SDK_VERSION, m_pd3dDevice.GetAddressOf(), &featureLevel,
                               m_pd3dImmediateContext.GetAddressOf());

        if (hr == E_INVALIDARG) {
            // 一部APIではD3D_FEATURE_LEVEL_11_1を認識できないため、11.0以下で再試行
            hr = D3D11CreateDevice(nullptr, d3dDriverType, nullptr, createDeviceFlags, &featureLevels[1],
                                   numFeatureLevels - 1,
                                   D3D11_SDK_VERSION, m_pd3dDevice.GetAddressOf(), &featureLevel,
                                   m_pd3dImmediateContext.GetAddressOf());
        }

        if (SUCCEEDED(hr))
            break;
    }

    if (FAILED(hr)) {
        MessageBox(0, L"D3D11CreateDevice Failed.", 0, 0);
        return false;
    }

    // 11.0または11.1機能レベルがサポートされているか確認
    if (featureLevel != D3D_FEATURE_LEVEL_11_0 && featureLevel != D3D_FEATURE_LEVEL_11_1) {
        MessageBox(0, L"Direct3D Feature Level 11_0 or 11_1 unsupported.", 0, 0);
        return false;
    }

    // MSAAの品質レベルを確認
    UINT quality;
    m_pd3dDevice->CheckMultisampleQualityLevels(
        DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, 4, &quality);
    assert(quality > 0);

    // DXGIファクトリの取得
    ComPtr<IDXGIDevice> dxgiDevice = nullptr;
    ComPtr<IDXGIAdapter> dxgiAdapter = nullptr;
    ComPtr<IDXGIFactory1> dxgiFactory1 = nullptr; // DXGI 1.1
    ComPtr<IDXGIFactory2> dxgiFactory2 = nullptr; // DXGI 1.2（Windows 8以降）

    HR(m_pd3dDevice.As(&dxgiDevice));
    HR(dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf()));
    HR(dxgiAdapter->GetParent(IID_PPV_ARGS(dxgiFactory1.GetAddressOf())));

    // IDXGIFactory2をサポートするか確認
    hr = dxgiFactory1.As(&dxgiFactory2);
    if (dxgiFactory2 != nullptr) {
        HR(m_pd3dDevice.As(&m_pd3dDevice1));
        HR(m_pd3dImmediateContext.As(&m_pd3dImmediateContext1));

        // スワップチェーンの設定
        DXGI_SWAP_CHAIN_DESC1 sd;
        ZeroMemory(&sd, sizeof(sd));
        sd.Width = m_ClientWidth;
        sd.Height = m_ClientHeight;
        sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
#if _WIN32_WINNT >= _WIN32_WINNT_WIN10
        m_BackBufferCount = 2;
        sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
        m_IsDxgiFlipModel = true;
#else
        m_BackBufferCount = 1;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        sd.Flags = 0;
#endif
        sd.BufferCount = m_BackBufferCount;

        DXGI_SWAP_CHAIN_FULLSCREEN_DESC fd;
        fd.RefreshRate.Numerator = 60;
        fd.RefreshRate.Denominator = 1;
        fd.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        fd.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        fd.Windowed = TRUE;

        // スワップチェーンを作成
        HR(dxgiFactory2->CreateSwapChainForHwnd(m_pd3dDevice.Get(), m_hMainWnd, &sd, &fd, nullptr, m_pSwapChain1.
            GetAddressOf()));
        HR(m_pSwapChain1.As(&m_pSwapChain));
    } else {
        // DXGI 1.1用のスワップチェーン設定
        DXGI_SWAP_CHAIN_DESC sd;
        ZeroMemory(&sd, sizeof(sd));
        sd.BufferDesc.Width = m_ClientWidth;
        sd.BufferDesc.Height = m_ClientHeight;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.BufferCount = 1;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        sd.OutputWindow = m_hMainWnd;
        sd.Windowed = TRUE;
        sd.Flags = 0;
        m_BackBufferCount = 1;
        HR(dxgiFactory1->CreateSwapChain(m_pd3dDevice.Get(), &sd, m_pSwapChain.GetAddressOf()));
    }

    // Alt+Enterによる全画面切替を無効化
    dxgiFactory1->MakeWindowAssociation(m_hMainWnd, DXGI_MWA_NO_ALT_ENTER | DXGI_MWA_NO_WINDOW_CHANGES);

    // デバッグ名を設定（デバッグビルド時）
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
    SetDebugObjectName(m_pd3dImmediateContext.Get(), "ImmediateContext");
    SetDebugObjectName(m_pSwapChain.Get(), "SwapChain");
#endif

    // ウィンドウサイズ変更に応じてOnResizeを呼び出す
    OnResize();

    return true;
}

bool D3DApp::InitImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // キーボード操作を有効化
    io.ConfigFlags |= ImGuiConfigFlags_IsSRGB; // SRGBフラグ（独自変更が必要）
    io.ConfigWindowsMoveFromTitleBarOnly = true; // タイトルバーのみで移動可能にする

    ImGui::StyleColorsDark(); // ダークテーマのスタイルを適用

    // プラットフォーム/レンダリングの初期化
    ImGui_ImplWin32_Init(m_hMainWnd);
    ImGui_ImplDX11_Init(m_pd3dDevice.Get(), m_pd3dImmediateContext.Get());

    return true;
}

void D3DApp::CalculateFrameStats() {
    // 毎秒のフレーム数とフレーム時間を計算し、ウィンドウタイトルに表示する
    static int frameCnt = 0;
    static float timeElapsed = 0.0f;

    frameCnt++;

    if ((m_Timer.TotalTime() - timeElapsed) >= 1.0f) {
        float fps = (float) frameCnt;
        float mspf = 1000.0f / fps;

        std::wostringstream outs;
        outs.precision(6);
        outs << m_MainWndCaption << L"    "
                << L"FPS: " << fps << L"    "
                << L"Frame Time: " << mspf << L" (ms)";
        SetWindowText(m_hMainWnd, outs.str().c_str());

        frameCnt = 0;
        timeElapsed += 1.0f;
    }
}
