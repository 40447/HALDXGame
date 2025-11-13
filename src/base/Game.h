//
// Created by eva00 on 25-5-20.
//
#pragma once
#include <Jolt/Jolt.h>
// Jolt includes
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>

#include "AnimationManager.h"
#include "GameApp.h"
#include "HalDebugRenderer.h"
#include "JoltListener.h"
#include "Scene.h"
#include "Layers.h"
class Scene;


//---------------------------------------------------------------//
// 定义游戏程序, 包含了游戏的窗口, 游戏循环, 渲染, 输入处理等功能
// ゲームプログラムを定義します。ゲームのウィンドウ、ゲームループ、レンダリング、入力処理などの機能を含みます。
//---------------------------------------------------------------//

class Game : public GameApp {
private:
    // 現在のシーン
    Scene *m_Scene = nullptr;
    //临时内存分配器
    JPH::TempAllocator *mTempAllocator = nullptr;
    //作业系统
    JPH::JobSystemThreadPool *mJobSystem = nullptr;
    // 对象层到广相层的映射表
    BPLayerInterfaceImpl mBroadPhaseLayerInterface;
    // 对象层与广相层之间的过滤器
    ObjectVsBroadPhaseLayerFilterImpl mObjectVsBroadPhaseLayerFilter;
    // 对象层与对象层之间的过滤器
    ObjectLayerPairFilterImpl mObjectVsObjectLayerFilter;
    // 物理系统实例
    PhysicsSystem *mPhysicsSystem = nullptr;
    JPH::PhysicsSettings mPhysicsSettings;
    //接触监听器
    JoltContactListener mContactListener;
    // 刚体激活监听器
    JoltBodyActivationListener mBodyActivationListener;
    // デバッグレンダラー
    HalDebugRenderer *gHalDebugRenderer = nullptr;
    BodyManager::DrawSettings mBodyDrawSettings;
public:
    AnimationManager animationManager;
private:
    bool mIsFirstFrame = true;
public:
    Game(HINSTANCE hInstance, const std::wstring &windowName, int initWidth, int initHeight);
    ~Game() override;

    // ゲームの初期化
    bool Init() override;
    // ゲームの更新 dtはデルタタイム
    void Update(float dt) override;

    // シーンを設定する
    template<typename T>
    void SetScene();

    // シーンを取得する
    Scene *GetScene() const { return m_Scene; }
    // 物理システムを取得する
    PhysicsSystem *GetPhysicsSystem() const { return mPhysicsSystem; }

    HalDebugRenderer *GetDebugRenderer() const { return gHalDebugRenderer; }
    JoltContactListener &GetContactListener() { return mContactListener; }
    // ウィンドウメッセージのコールバック関数
    virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) override;
};

// シーンを設定する
template<typename T>
void Game::SetScene() {
    // TはSceneのサブクラスである必要があります
    static_assert(std::is_base_of<Scene, T>::value, "TはSceneのサブクラスでなければなりません");
    // 既存のシーンを解放
    if (m_Scene != nullptr) {
        m_Scene->UninitBase();
        delete m_Scene;
    }
    mIsFirstFrame = true;
    // 新しいシーンを作成
    m_Scene = new T();
    // シーンを初期化
    m_Scene->InitBase();
}
