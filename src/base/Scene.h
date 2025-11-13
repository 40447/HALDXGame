#pragma once
#include <string>

#include "SceneHierarchy.h"
#include "SoAStorage.h"


class TransformC;
class GameObject;
class Component;
//---------------------------------------------------------------//
// Scene是游戏当前所有GameObject和Component的容器, 负责管理GameObject和Component的生命周期和更新。
// Sceneは現在のゲームのすべてのGameObjectとComponentのコンテナであり、GameObjectとComponentのライフサイクルと更新を管理します。
//----------------------------------------------------------------//

class Scene {
protected:
    GameObject *root;
    TransformC *rootTransform = nullptr;
    // GameObjectのコンテナ
    SoAStorage<GameObject> m_GameObjectStorage;
    // Componentのコンテナ
    SoAStorage<Component> m_ComponentStorage;
    SceneHierarchy m_SceneHierarchy;

public:
    Scene();

    virtual ~Scene();

    virtual void Init() = 0;

    virtual void Uninit() = 0;

    virtual void Update(float dt) = 0;

    void InitBase();

    void UninitBase();

    void UpdateBase(float dt);

    // ゲームオブジェクトを追加
    template<class T, class... Args>
    T &AddGameObject(Args &&... args);

    // ゲームオブジェクトを取得
    template<typename T>
    T &GetGameObjectRef(UID handle);

    template<class T>
    T *GetGameObject(UID handle);

    // ゲームオブジェクトを削除
    void RemoveGameObject(UID handle);

    // コンポーネントを追加
    template<class T, class... Args>
    T &AddComponent(GameObject *parent, Args &&... args);

    // コンポーネントを取得
    template<typename T>
    T &GetComponentRef(UID handle);

    template<class T>
    T *GetComponent(UID handle);
    // コンポーネントを削除
    void RemoveComponent(UID handle);
    // Rootを取得
    GameObject *GetRoot() const { return root; }
    // Rootが持つTransformCを取得
    TransformC *GetRootTransform() const {  return rootTransform;  }
};

template<typename T, typename... Args>
T &Scene::AddGameObject(Args &&... args) {
    static_assert(std::is_base_of_v<GameObject, T>, "T は GameObject を継承する必要があります。");
    std::unique_ptr<T> gameObject = std::make_unique<T>(std::forward<Args>(args)...);
    T *rawPtr = gameObject.get();
    UID handle = m_GameObjectStorage.Add(std::unique_ptr<GameObject>(std::move(gameObject)));
    rawPtr->SetUID(handle);
    rootTransform->AddChild(rawPtr->GetComponent<TransformC>());
    // typeid(T).name() の先頭の "class " を削除
    std::string name = static_cast<std::string>(typeid(T).name()).substr(6);
    rawPtr->SetName(name);
    rawPtr->InitBase();
    return *rawPtr;
}

template<typename T>
T &Scene::GetGameObjectRef(UID handle) {
    static_assert(std::is_base_of_v<GameObject, T>, "T は GameObject を継承する必要があります。");
    GameObject *base = m_GameObjectStorage.Get(handle);
    // 这里使用的强制转换, 虽然有点费效率, 但是优化费劲,调用频率不高, 所以暂时放弃优化. 懒!!
    // ここでは強制キャストを使用しています。効率は少し低下しますが、最適化は面倒で、呼び出し頻度も高くないため、今のところ最適化は行いません。
    return dynamic_cast<T &>(*base);
}

template<typename T>
T *Scene::GetGameObject(UID handle) {
    static_assert(std::is_base_of_v<GameObject, T>, "T は GameObject を継承する必要があります。");
    if (!m_GameObjectStorage.Has(handle)) {
        return nullptr;
    }
    T *base = m_GameObjectStorage.Get(handle);
    return dynamic_cast<T *>(base);
}

template<typename T, typename... Args>
T &Scene::AddComponent(GameObject *parent, Args &&... args) {
    static_assert(std::is_base_of_v<Component, T>, "T は Component を継承する必要があります。");
    std::unique_ptr<T> component = std::make_unique<T>(std::forward<Args>(args)...);
    T *rawPtr = component.get();
    UID uid = m_ComponentStorage.Add(std::unique_ptr<Component>(std::move(component)));
    rawPtr->SetGameObject(parent);
    rawPtr->SetUID(uid);
    // typeid(T).name() の先頭の "class " を削除
    std::string name = static_cast<std::string>(typeid(T).name()).substr(6);
    rawPtr->SetName(name);
    rawPtr->Init();
    return *rawPtr;
}

template<typename T>
T &Scene::GetComponentRef(UID handle) {
    static_assert(std::is_base_of_v<Component, T>, "T は Component を継承する必要があります。");
    Component *base = m_ComponentStorage.Get(handle);
    return dynamic_cast<T &>(*base);
}

template<typename T>
T *Scene::GetComponent(UID handle) {
    static_assert(std::is_base_of_v<Component, T>, "T は Component を継承する必要があります。");
    if (!m_ComponentStorage.Has(handle)) {
        return nullptr;
    }
    Component *base = m_ComponentStorage.Get(handle);
    return dynamic_cast<T *>(base);
}
