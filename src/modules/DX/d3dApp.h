#pragma once

#include <wrl/client.h>
#include <string>
#include <string_view>
#include "WinMin.h"
#include <d3d11_1.h>
#include <DirectXMath.h>
#include "CpuTimer.h"
#include "GpuTimer.h"

#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

class D3DApp
{
public:
    D3DApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight);
    virtual ~D3DApp();

    HINSTANCE AppInst()const;       // アプリケーションインスタンスのハンドルを取得
    HWND      MainWnd()const;       // メインウィンドウのハンドルを取得
    float     AspectRatio()const;   // 画面のアスペクト比を取得

    int Run();                      // アプリケーションを実行し、メッセージループを処理する

    // フレームワークの基本メソッド。派生クラスでオーバーライドする必要がある
    virtual bool Init();                      // ウィンドウ、Direct2D、Direct3D の初期化を行う
    virtual void OnResize();                  // ウィンドウサイズ変更時に呼び出される
    virtual void PreUpdate(float dt) = 0;     // フレームごとの更新前処理（純粋仮想）
    virtual void Update(float dt) = 0;        // フレームごとの更新処理（純粋仮想）
    virtual void PostUpdate(float dt) = 0;    // フレームごとの更新後処理（純粋仮想）
    virtual void Draw() = 0;                  // フレームごとの描画処理（純粋仮想）
    virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam); // ウィンドウメッセージのコールバック関数

protected:
    bool InitMainWindow();      // ウィンドウの初期化
    bool InitDirect3D();        // Direct3D の初期化
    bool InitImGui();           // ImGui の初期化

    void CalculateFrameStats(); // フレームレートを計算し、ウィンドウに表示
    ID3D11RenderTargetView* GetBackBufferRTV() { return m_pRenderTargetViews[m_FrameCount % m_BackBufferCount].Get(); }

protected:
    // テンプレートエイリアスで型名を簡略化（C++11）
    template <class T>
    using ComPtr = Microsoft::WRL::ComPtr<T>;

    HINSTANCE m_hAppInst;        // アプリケーションインスタンスハンドル
    HWND      m_hMainWnd;        // メインウィンドウハンドル
    bool      m_AppPaused;       // アプリケーションが一時停止中か
    bool      m_Minimized;       // 最小化されているか
    bool      m_Maximized;       // 最大化されているか
    bool      m_Resizing;        // サイズ変更中か

    bool m_IsDxgiFlipModel = false; // DXGI フリップモデルを使用するかどうか
    UINT m_BackBufferCount = 0;     // バックバッファの数
    UINT m_FrameCount = 0;          // 現在のフレーム数
    ComPtr<ID3D11RenderTargetView> m_pRenderTargetViews[2]; // 各バックバッファ用のレンダーターゲットビュー

    CpuTimer m_Timer;            // CPU用タイマー

    // Direct3D 11
    ComPtr<ID3D11Device> m_pd3dDevice;                          // D3D11 デバイス
    ComPtr<ID3D11DeviceContext> m_pd3dImmediateContext;         // D3D11 デバイスコンテキスト
    ComPtr<IDXGISwapChain> m_pSwapChain;                        // D3D11 スワップチェーン
    // Direct3D 11.1
    ComPtr<ID3D11Device1> m_pd3dDevice1;                        // D3D11.1 デバイス
    ComPtr<ID3D11DeviceContext1> m_pd3dImmediateContext1;       // D3D11.1 デバイスコンテキスト
    ComPtr<IDXGISwapChain1> m_pSwapChain1;                      // D3D11.1 スワップチェーン

    // 派生クラスがコンストラクタで設定するパラメータ
    std::wstring m_MainWndCaption;                             // メインウィンドウのタイトル
    int m_ClientWidth;                                         // クライアント領域の幅
    int m_ClientHeight;                                        // クライアント領域の高さ
};
