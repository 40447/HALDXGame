//
// Created by eva00 on 25-9-30.
//

#include "NetPlayer.h"

#include "ModelC.h"
#include "Net.h"
#include "TransformC.h"
#include "BoxCollisionC.h"
#include "Gun.h"
#include "PlayerAttribute.h"
#include "ImageC.h"
#include "HPUI.h"

NetPlayer *NetPlayer::LocalPlayer = nullptr;

NetPlayer::NetPlayer() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
}

NetPlayer::~NetPlayer() = default;

void NetPlayer::Init() {
    NetGameObject::Init();
    // 添加物理
    AddComponent<PlayerAttribute>();
    auto &boxCollision = AddComponent<BoxCollisionC>(JPH::EMotionType::Dynamic, Vector3(3, 4, 3));
    const float minX = -60.0f;
    const float maxX = 60.0f;
    const float minY = -60.0f;
    const float maxY = 60.0f;
    float x = minX + static_cast<float>(std::rand()) / RAND_MAX * (maxX - minX);
    float y = minY + static_cast<float>(std::rand()) / RAND_MAX * (maxY - minY);
    boxCollision.SetPosition(Vector3(x, 5.0f, y));
    // 驾驶仓
    Pmodel = &halgame->GetScene()->AddGameObject<GameObject>();
    Pmodel->AddComponent<ModelC>("assets/models/Player/player.fbx");
    TransformC *modelTransform = Pmodel->GetComponent<TransformC>();
    GetComponentRef<TransformC>().AddChild(modelTransform);
    modelTransform->SetLocalRotationEuler(Vector3(DirectX::XM_PI / 2, 0.0f, 0.0f));
    modelTransform->SetLocalPosition(Vector3(0.0f, 1.0f, 0.0f));

    // 左手枪
    leftGun = &halgame->GetScene()->AddGameObject<Gun>();
    TransformC *leftGunTrans = leftGun->GetComponent<TransformC>();
    GetComponentRef<TransformC>().AddChild(leftGunTrans);
    leftGunTrans->SetLocalPosition(Vector3(-3.0f, 1.0f, 1.5f));
    // 右手枪
    rightGun = &halgame->GetScene()->AddGameObject<Gun>();
    TransformC *rightGunTrans = rightGun->GetComponent<TransformC>();
    GetComponentRef<TransformC>().AddChild(rightGunTrans);
    rightGunTrans->SetLocalPosition(Vector3(3.0f, 1.0f, 1.5f));

    isAlive = true;
}

void NetPlayer::Update(float dt) {
    NetGameObject::Update(dt);
    if (!isAlive) {
        return;
    }
    Move(dt);
    GunFire();
    PlayerAttribute *attr = GetComponent<PlayerAttribute>();
    if (hpUI != nullptr && attr != nullptr) {
        hpUI->SetHPPercent(static_cast<float>(attr->GetHP()) / static_cast<float>(attr->GetMaxHP()));
    }
}

void NetPlayer::Uninit() {
    if (PlayerId != 0) {
        net::OnPlayerDestroyed(*this);
    }
    NetGameObject::Uninit();
    PlayerId = 0;
    hpUI = nullptr;
}

void NetPlayer::SetPlayerId(uint32_t id) {
    PlayerId = id;
    SetNetId(id);
    net::OnPlayerSpawned(*this);
}

void NetPlayer::SetIsLocalPlayer(bool isLocal) {
    SetLocalOwner(isLocal);
    if (isLocal) {
        // 设置全局本地玩家指针
        NetPlayer::LocalPlayer = this;
        hpUI = &halgame->GetScene()->AddGameObject<HPUI>();
        GetComponent<TransformC>()->AddChild(hpUI->GetComponent<TransformC>());
    }
}

void NetPlayer::OnContactAdded(CollisionC &my, CollisionC &other, const JPH::ContactManifold &inManifold,
                               JPH::ContactSettings &ioSettings) {
    GameObject::OnContactAdded(my, other, inManifold, ioSettings);
}

void NetPlayer::GunFire() {
    if (!IsLocalPlayer()) {
        return;
    }
    static bool lastAiming = false;
    bool aiming = ImGui::IsMouseDown(ImGuiMouseButton_Right);

    if (aiming != lastAiming) {
        leftGun->Aim(aiming);
        rightGun->Aim(aiming);
        lastAiming = aiming;
    }
    if (!aiming) return;

    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left, false)) {
        if (fireLeftNext) {
            leftGun->Shoot();
        } else {
            rightGun->Shoot();
        }
        fireLeftNext = !fireLeftNext;
    }
}

void NetPlayer::Move(float dt) {
    if (!IsLocalPlayer()) {
        return;
    }
    auto &transform = GetComponentRef<TransformC>();
    Vector3 forward = transform.GetForward();
    Vector3 right = transform.GetRight();

    Vector3 moveDir = Vector3::Zero;
    if (ImGui::IsKeyDown(ImGuiKey_W)) moveDir -= forward;
    if (ImGui::IsKeyDown(ImGuiKey_S)) moveDir += forward;
    if (ImGui::IsKeyDown(ImGuiKey_A)) moveDir += right;
    if (ImGui::IsKeyDown(ImGuiKey_D)) moveDir -= right;

    const float turnSpeed = DirectX::XMConvertToRadians(90.0f); // degrees per second
    float dYaw = 0.0f;
    if (ImGui::IsKeyDown(ImGuiKey_Q)) dYaw -= turnSpeed * dt;
    if (ImGui::IsKeyDown(ImGuiKey_E)) dYaw += turnSpeed * dt;

    if (std::abs(dYaw) > 1e-6f) {
        auto &body = GetComponentRef<BoxCollisionC>();
        auto &transform = GetComponentRef<TransformC>();
        Quaternion current = transform.GetWorldRotation();
        Quaternion delta = Quaternion::CreateFromAxisAngle(Vector3::UnitY, dYaw);
        Quaternion next = delta * current;
        next.Normalize();

        body.SetRotation(next);
    }

    if (moveDir.LengthSquared() <= 1e-6f) return;
    moveDir.Normalize();
    const float moveSpeed = 10.0f;
    auto &body = GetComponentRef<BoxCollisionC>();
    Vector3 targetPos = body.GetPosition() + moveDir * moveSpeed * dt;
    body.SetPosition(targetPos);
}

void NetPlayer::Respawn() {
    Pmodel->GetComponent<ModelC>()->SetVisible(true);
    rightGun->GetComponent<ModelC>()->SetVisible(true);
    leftGun->GetComponent<ModelC>()->SetVisible(true);
    auto &boxCollision = GetComponentRef<BoxCollisionC>();
    boxCollision.SetActive(true);
    isAlive = true;
}

void NetPlayer::Die() {
    Pmodel->GetComponent<ModelC>()->SetVisible(false);
    rightGun->GetComponent<ModelC>()->SetVisible(false);
    leftGun->GetComponent<ModelC>()->SetVisible(false);
    auto &boxCollision = GetComponentRef<BoxCollisionC>();
    boxCollision.SetActive(false);
    isAlive = false;
}
