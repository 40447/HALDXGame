#pragma once

#include <SimpleMath.h>
#include <string>

#include "Global.h"
#include "SoAStorage.h"

class Scene;
class Component;

namespace JPH {
    class ContactSettings;
    class ContactManifold;
}

class CollisionC;
using namespace DirectX::SimpleMath;


//---------------------------------------------------------------//
// GameObject类是一切游戏物体的基础：角色、敌人、道具、地板……统统都是 GameObject。
// 可以添加Component来扩展功能，比如添加ModelC的话就可以拥有模型显示功能，添加TransformC就可以拥有位置、旋转、缩放等变换功能。
// 注意：TransformC默认会被添加到每个GameObject中，所以不需要手动添加。
//---------------------------------------------------------------//
// GameObjectクラスは、すべてのゲームオブジェクトの基底クラスです：キャラクター、敵、アイテム、床など、すべてがGameObjectです。
// コンポーネントを追加することで機能を拡張できます。例えば、ModelCを追加するとモデル表示機能が、TransformCを追加すると位置、回転、スケーリングなどの変換機能が得られます。
// 注意：TransformCはデフォルトで各GameObjectに追加されるため、手動で追加する必要はありません。
//---------------------------------------------------------------//


// このファイルはゲームオブジェクトの基底クラスを定義します。
class GameObject {
private:
    // ゲームオブジェクトの一意の識別子
    UID uid = -1;
    std::unordered_map<std::string, UID> m_ComponentMap;
    GameObject(const GameObject&) = delete;
    GameObject& operator=(const GameObject&) = delete;

protected:
    // GameObjectの初期化
    virtual void    Init() {
    };
    // GameObjectの解放
    virtual void Uninit() {
    };
    // GameObjectの更新
    virtual void Update(float dt) {
    };
    // ゲームオブジェクトの名前
    std::string m_Name = "GameObject";

public:
    GameObject();

    virtual ~GameObject();

    // ゲームオブジェクトのUIDを設定
    void SetUID(UID uuid);

    // ゲームオブジェクトのUIDを取得
    UID GetUID() const;

    // ゲームオブジェクトの初期化
    void InitBase();

    // ゲームオブジェクトの解放
    void UninitBase();

    // ゲームオブジェクトの更新
    void UpdateBase(float dt);

    void Destroy();

    // ゲームオブジェクトの名前を設定・取得
    void SetName(std::string name) { m_Name = name; }
    std::string GetName() const { return m_Name; }

    // コンポーネントを追加する
    template<class T, class... Args>
    T &AddComponent(Args &&... args);

    // コンポーネントを取得する
    template<typename T>
    T &GetComponentRef() const;

    // コンポーネントが存在するか確認する
    template<class T>
    bool HasComponent() const;

    template<class T>
    T *GetComponent() const;

    template<class T>
    T *GetComponent(std::string name) const;

    std::unordered_map<std::string, UID> GetComponentMap() const;
    // 新しい衝突点が検出された時に呼び出される。
    virtual void OnContactAdded(CollisionC &my, CollisionC &other, const JPH::ContactManifold &inManifold, JPH::ContactSettings &ioSettings) {};
    // 前のフレームでも接触していた物体が、今フレームでも引き続き接触している場合に呼び出される。
    virtual void OnContactPersisted(CollisionC &my, CollisionC &other, const JPH::ContactManifold &inManifold, JPH::ContactSettings &ioSettings) {};
    // 以前のフレームで接触していた物体同士が、現在は接触していない場合に呼び出される。
    virtual void OnContactRemoved(CollisionC &my, CollisionC &other) {};

    // インスペクターGUIを描画する
    virtual void OnInspectorGUI();
};

template<typename T, typename... Args>
T &GameObject::AddComponent(Args &&... args) {
    // TはComponentの派生クラスである必要があります
    static_assert(std::is_base_of_v<Component, T>, "T は Component の派生クラスでなければなりません");
    std::string name = static_cast<std::string>(typeid(T).name()).substr(6);
    // 同じタイプのコンポーネントがすでに存在するか確認
    auto it = m_ComponentMap.find(name);
    if (it != m_ComponentMap.end()) {
        // すでに同じタイプのコンポーネントが存在する場合はエラーを出す
        //assert(false && "ゲームオブジェクトに重複コンポーネントが追加されました");
        return GetComponentRef<T>();
    }
    // 今はSceneを取得して、コンポーネントを追加する
    Scene *scene = halgame->GetScene();
    // Tのインスタンスを作成し、GameObjectに追加する
    T &component = scene->AddComponent<T>(this, std::forward<Args>(args)...);
    // コンポーネントのUIDをキャッシュする
    m_ComponentMap[name] = component.GetUID();
    return component;
}

template<typename T>
bool GameObject::HasComponent() const {
    auto it = m_ComponentMap.find(static_cast<std::string>(typeid(T).name()).substr(6));
    return it != m_ComponentMap.end();
}

template<typename T>
T &GameObject::GetComponentRef() const {
    auto it = m_ComponentMap.find(static_cast<std::string>(typeid(T).name()).substr(6));
    if (it == m_ComponentMap.end()) {
        throw std::runtime_error("ゲームオブジェクトにコンポーネントが見つかりません");
    }
    return halgame->GetScene()->GetComponentRef<T>(it->second);
}

template<typename T>
T *GameObject::GetComponent() const {
    std::string typeName = static_cast<std::string>(typeid(T).name()).substr(6);
    auto it = m_ComponentMap.find(typeName);
    if (it == m_ComponentMap.end()) {
        return nullptr; // コンポーネントが見つからない場合はnullptrを返す
    }
    return halgame->GetScene()->GetComponent<T>(it->second);
}

template<class T>
T * GameObject::GetComponent(std::string name) const {
    auto it = m_ComponentMap.find(name);
    if (it == m_ComponentMap.end()) {
        return nullptr; // コンポーネントが見つからない場合はnullptrを返す
    }
    return halgame->GetScene()->GetComponent<T>(it->second);
}
