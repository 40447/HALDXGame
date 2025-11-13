//
// Created by eva00 on 25-5-20.
//

#include "Game.h"

#include "TitleScene.h"
#include "enet/enet.h"
#include "Jolt/RegisterTypes.h"

// 游戏实例
// ゲームインスタンス
Game::Game(HINSTANCE hInstance, const std::wstring &windowName, int initWidth, int initHeight) : GameApp(
        hInstance, windowName, initWidth, initHeight) {
}

Game::~Game() = default;

bool Game::Init() {
    if (!GameApp::Init()) {
        return false;
    }

    // ImGuiを初期化
    ImGuiIO &io = ImGui::GetIO();
    ImGuiStyle &style = ImGui::GetStyle();
    style.IndentSpacing = 5.0f;
    ImFontConfig config;
    config.OversampleH = 1;
    config.OversampleV = 1;
    config.PixelSnapH = true;
    //日本語文字体を追加
    io.Fonts->AddFontFromFileTTF("assets/fonts/fusion-pixel-10px-monospaced-ja.ttf", 16.0f, &config,
                                 io.Fonts->GetGlyphRangesJapanese());
    config.MergeMode = true;
    // 中国語簡体字、繁体字、韓国語、ラテン文字のフォントを追加
    io.Fonts->AddFontFromFileTTF("assets/fonts/fusion-pixel-10px-monospaced-zh_hans.ttf", 16.0f, &config,
                                 io.Fonts->GetGlyphRangesChineseFull());
    io.Fonts->AddFontFromFileTTF("assets/fonts/fusion-pixel-10px-monospaced-zh_hant.ttf", 16.0f, &config,
                                 io.Fonts->GetGlyphRangesChineseFull());
    io.Fonts->AddFontFromFileTTF("assets/fonts/fusion-pixel-10px-monospaced-ko.ttf", 16.0f, &config,
                                 io.Fonts->GetGlyphRangesKorean());
    io.Fonts->AddFontFromFileTTF("assets/fonts/fusion-pixel-10px-monospaced-latin.ttf", 16.0f, &config,
                                 io.Fonts->GetGlyphRangesDefault());

    // Physicsシステムを初期化
    JPH::RegisterDefaultAllocator();
    JPH::Factory::sInstance = new JPH::Factory();
    JPH::RegisterTypes();
    mTempAllocator = new JPH::TempAllocatorImpl(32 * 1024 * 1024);
    mJobSystem = new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers,
                                              std::thread::hardware_concurrency() - 1);
    mPhysicsSystem = new PhysicsSystem();
    mPhysicsSystem->Init(10240, 0, 65536, 20480, mBroadPhaseLayerInterface, mObjectVsBroadPhaseLayerFilter,
                         mObjectVsObjectLayerFilter);
    mPhysicsSystem->SetPhysicsSettings(mPhysicsSettings);
    // 物理系统的重力设置为 -9.81f
    mPhysicsSystem->SetGravity(Vec3(0, -9.81f, 0));
    // 优化broadphase，使首次更新速度更快
    mPhysicsSystem->OptimizeBroadPhase();
    mPhysicsSystem->SetBodyActivationListener(&mBodyActivationListener);
    mPhysicsSystem->SetContactListener(&mContactListener);
    // 物理系统的绘制设置
    gHalDebugRenderer = new HalDebugRenderer();
    mBodyDrawSettings.mDrawShape = true;
    mBodyDrawSettings.mDrawShapeWireframe = true;

    // アニメを追加
    animationManager.LoadAnimationClips("assets/animation/Idle.fbx", {"Idle"});
    animationManager.LoadAnimationClips("assets/animation/Walking.fbx", {"Walking"});
    animationManager.LoadAnimationClips("assets/animation/Dance.fbx", {"Dance"});

    // ネットワークを初期化
    if (enet_initialize() != 0) {
        std::fprintf(stderr, "[Net] An error occurred while initializing ENet.\n");
        return false;
    } else {
        atexit(enet_deinitialize);
    }

    // テストシーンを読み込む
    SetScene<TitleScene>();

    return true;
}

// ゲームの実行
void Game::Update(float dt) {
    GameApp::Update(dt);
    if (mIsFirstFrame) {
        mIsFirstFrame = false;
        dt = 0;
    }
    // シーンの更新
    m_Scene->UpdateBase(dt);
    // 物理システムの更新
    mPhysicsSystem->Update(dt, 1, mTempAllocator, mJobSystem);
    // 物理ボディのデバッグ描画
    mPhysicsSystem->DrawBodies(
        mBodyDrawSettings,
        gHalDebugRenderer
    );
}

LRESULT Game::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_DESTROY:
            m_Scene->UninitBase();
            PostQuitMessage(0);
            return 0;
    }
    return GameApp::MsgProc(hwnd, msg, wParam, lParam);
}
