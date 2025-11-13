//
// Created by eva00 on 25-5-29.
//
#include "Scene.h"

#include "ClientManagerC.h"
#include "Component.h"
#include "GameObject.h"
#include "ServerManagerC.h"
#include "TransformC.h"

Scene::Scene() = default;

Scene::~Scene() = default;

void Scene::InitBase() {
    // rootを初期化
    root = new GameObject();
    UID handle = m_GameObjectStorage.Add(std::unique_ptr<GameObject>(std::move(root)));
    root->SetUID(handle);
    root->SetName("root");
    root->InitBase();
    rootTransform = root->GetComponent<TransformC>();
    rootTransform->SetLocalPosition(Vector3::Zero);
    rootTransform->SetLocalRotation(Quaternion::Identity);
    rootTransform->SetLocalScale(Vector3(1, 1, 1));
    Init();
}

void Scene::UninitBase() {
    // 释放所有的GameObject, m_Component不用在这里释放, 因为GameObject会释放它们
    // すべでのGameObjectを解放します。m_Componentはここで解放する必要はありません。なぜなら、GameObjectがそれらを解放するからです。
    m_GameObjectStorage.ForEachActive([this](UID id, GameObject *game_object) {
        game_object->UninitBase();
    });
    m_GameObjectStorage.Reset();
    Uninit();
}

void Scene::UpdateBase(float dt) {
    ClientManagerC *client = root->GetComponent<ClientManagerC>();
    if (client != nullptr) {
        // 如果进行联机的话, 则等待连接成功后再更新场景
        // オンライン接続を行う場合、接続が成功するまでシ
        if (client->GetClient() !=nullptr && client->IsConnected() == false) {
            client->Update(dt);
            ServerManagerC *server = root->GetComponent<ServerManagerC>();
            if (server != nullptr) {
                server->Update(dt);
            }
            // 等待服务器链接成功后再更新场景
            ImGui::OpenPopup("接続中...");
            if (ImGui::BeginPopupModal("接続中...", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::TextUnformatted("サーバーに接続中...");
                ImGui::Separator();
                if (ImGui::Button("切断", ImVec2(120, 0))) {
                    // 切断
                    client->Stop();
                }
                ImGui::EndPopup();
            }
            return;
        }
    }
    // 先更新所有的Component, 再更新所有GameObject
    // すべてのComponentを先に更新し、その後にすべてのGameObjectを更新します。
    m_ComponentStorage.ForEachActive([this, dt](UID id, Component *component) {
        component->Update(dt);
    });
    m_GameObjectStorage.ForEachActive([this, dt](UID id, GameObject *game_object) {
        game_object->UpdateBase(dt);
    });
    Update(dt);
    m_SceneHierarchy.Draw();
}

void Scene::RemoveGameObject(UID handle) {
    m_GameObjectStorage.Get(handle)->UninitBase();
    m_GameObjectStorage.Remove(handle);
}

void Scene::RemoveComponent(UID handle) {
    m_ComponentStorage.Get(handle)->Uninit();
    m_ComponentStorage.Remove(handle);
}
