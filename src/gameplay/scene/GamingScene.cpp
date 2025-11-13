#include "GamingScene.h"

#include "ClientManagerC.h"
#include "GameObject.h"
#include "ModelC.h"
#include "ServerManagerC.h"
#include "TransformC.h"
#include "BoxCollisionC.h"
#include "Sky.h"
#include "NetPlayer.h"
#include "PlayerAttribute.h"

void GamingScene::Init() {
    Sky &sky = AddGameObject<Sky>();
    int size = 13;
    const float spacing = 20.0f;
    const float halfExtent = (size - 1) * spacing * 0.5f;
    GameObject &map = AddGameObject<GameObject>();
    map.SetName("Map");
    for (int x = 0; x < size; x++) {
        for (int y = 0; y < size; y++) {
            float xpos = x * spacing - halfExtent;
            float ypos = y * spacing - halfExtent;

            auto &ground = AddGameObject<GameObject>();
            ground.SetName("ground");
            map.GetComponentRef<TransformC>().AddChild(ground.GetComponent<TransformC>());
            // 地面のモデルを追加します
            ground.AddComponent<ModelC>("assets\\models\\ground_19.obj"); //("assets/models/player/GMAP2.fbx");
            ground.AddComponent<BoxCollisionC>(
                    JPH::EMotionType::Static, // 静的な物体
                    Vector3(10, 1, 10) // サイズを設定します
            );
            ground.GetComponentRef<BoxCollisionC>().SetOffsetTransform(
                    OffsetTransform{
                            Vector3(0, -2, 0), // 地面の位置オフセット
                            Vector3(0, 0, 0) // 回転オフセットはなし
                    }
            );
            ground.GetComponentRef<BoxCollisionC>().SetPosition(Vector3(xpos, -5, ypos));
        }
    }
}

void GamingScene::Update(float dt) {
    // 打开一个模态对话框
    ClientManagerC *client = root->GetComponent<ClientManagerC>();

    if (client == nullptr || client->GetClient() == nullptr) {
        ShowModeSelectionDialog();
    }
    // 进入到联机状态后，设置摄像机跟随本地玩家
    if (NetPlayer::LocalPlayer != nullptr) {
        halgame->m_pCamera->SetTarget(NetPlayer::LocalPlayer->GetComponent<TransformC>()->GetWorldPosition());
        auto *pt = NetPlayer::LocalPlayer->GetComponent<TransformC>();
        Vector3 playerPos = pt->GetWorldPosition();
        Vector3 f = pt->GetForward();
        f.Normalize();
        halgame->m_pCamera->SetTarget(playerPos + Vector3(0.0f, 3.0f, 0.0f));
        float playerYaw = std::atan2(f.x, f.z);
        halgame->m_pCamera->SetRotationY(playerYaw + DirectX::XM_PI);
        halgame->m_pCamera->SetRotationX(DirectX::XMConvertToRadians(20.0f));
        halgame->m_pCamera->SetDistance(1.0f);
        // 如果玩家死亡，弹出复活对话框
        if (!NetPlayer::LocalPlayer->IsAlive()) {
            ImGui::OpenPopup("ゲームモード");
            if (ImGui::BeginPopupModal("ゲームモード", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::TextUnformatted("あなたは死にました。復活しますか？");
                ImGui::Separator();
                if (ImGui::Button("復活", ImVec2(140, 0))) {
                    PlayerAttribute *attr = NetPlayer::LocalPlayer->GetComponent<PlayerAttribute>();
                    if (attr != nullptr) {
                        attr->SetHP(attr->GetMaxHP());
                    }
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
        }
    }
}

void GamingScene::Uninit() {
}

void GamingScene::ShowModeSelectionDialog() {
    static char ip[64] = "127.0.0.1";
    static int port = 8123;

    ImGui::OpenPopup("ゲームモード");
    if (ImGui::BeginPopupModal("ゲームモード", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextUnformatted("ゲームモードを選んでください");
        ImGui::Separator();

        ImGui::InputText("IPアドレス", ip, IM_ARRAYSIZE(ip));
        ImGui::InputInt("ポート", &port);
        if (port < 1) port = 1;
        if (port > 65535) port = 65535;

        // 一行放两颗按钮
        if (ImGui::Button("開始(ホスト)", ImVec2(140, 0))) {
            // 读取同一组 ip/port。若你的 Server 只用 Port，就忽略 ip。
            ServerManagerC &server_c = root->AddComponent<ServerManagerC>();
            server_c.Port = port;
            // 如果你的 Server 有 Bind/Addr 字段，可在这里使用：
            // server_c.BindAddress = ip;
            server_c.Start();
            ClientManagerC &client_c = root->AddComponent<ClientManagerC>();
            client_c.Hostname = ip;
            client_c.Port = port;
            client_c.Start();

            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("接続(クライアント)", ImVec2(160, 0))) {
            ClientManagerC &client_c = root->AddComponent<ClientManagerC>();
            client_c.Hostname = ip;
            client_c.Port = port;
            client_c.Start();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}
