//
// Created by eva00 on 25-8-5.
//

#include "NetBullet.h"
#include "CapsuleCollisionC.h"
#include "ModelC.h"
#include "Net.h"
#include <DirectXMath.h>
#include <iostream>

#include "NetPlayer.h"
#include "PlayerAttribute.h"
#include "TransformC.h"

namespace {
    struct CbulletFactoryRegistrar {
        CbulletFactoryRegistrar() {
            net::RegisterNetObjectFactory("Cbullet", [](uint32_t /*netId*/, uint32_t /*ownerId*/) -> NetGameObject & {
                return halgame->GetScene()->AddGameObject<NetBullet>();
            });
        }
    };

    static CbulletFactoryRegistrar gCbulletFactoryRegistrar;
}

NetBullet::NetBullet() {
}

NetBullet::~NetBullet() {
}

void NetBullet::Init() {
    NetGameObject::Init();
    AddComponent<ModelC>("assets/models/Bullet.fbx");
    TransformC &transform = GetComponentRef<TransformC>();
    transform.SetLocalScale(Vector3(1.0f, 1.0f, 1.0f)); // 弾丸のサイズを調整
    CapsuleCollisionC &boxC = AddComponent<CapsuleCollisionC>(JPH::EMotionType::Dynamic, 0.3f, 1.0f);
    // 重力を無効にして、弾丸が重力の影響を受けないようにします
    boxC.SetGravityFactor(0);
    boxC.SetOffsetRotation(Vector3(DirectX::XM_PI / 2, 0.0f, 0.0f));
}


void NetBullet::Update(float dt) {
    NetGameObject::Update(dt);
    m_destroyDelay -= dt;
    if (m_destroyDelay <= 0.0f) {
        Destroy();
        return;
    }
}

void NetBullet::OnContactAdded(CollisionC &my, CollisionC &other, const JPH::ContactManifold &inManifold, JPH::ContactSettings &ioSettings) {
    GameObject::OnContactAdded(my, other, inManifold, ioSettings);
    GameObject *otherObj = other.GetGameObject();
    if (otherObj == nullptr) {
        return;
    }
    const NetPlayer *otherPlayer = dynamic_cast<NetPlayer *>(otherObj);
    if (otherPlayer == nullptr) {
        return;
    }
    m_destroyDelay = 0.1f;
    if (otherPlayer->IsLocalPlayer()) {
        std::cout << "Bullet hit player with ID: " << otherPlayer->GetPlayerId() << std::endl;
        PlayerAttribute &attr = otherPlayer->GetComponentRef<PlayerAttribute>();
        attr.Hit(10); // 10のダメージを与える
    }
}

void NetBullet::Uninit() {
    if (IsLocalOwner()) {
        net::RequestNetObjectDestroy(*this);
    }
    NetGameObject::Uninit();
}

void NetBullet::Fire(DirectX::SimpleMath::Vector3 direction, float speed) {
    CapsuleCollisionC &boxC = GetComponentRef<CapsuleCollisionC>();

    // 归一化方向
    direction.Normalize();

    // 构造一个让 Y+ 朝向 direction 的旋转
    Vector3 forward = direction;
    Vector3 reference = Vector3::UnitZ; // 用 Z 轴作为参考构建正交坐标系
    Vector3 right = reference.Cross(forward);
    if (right.LengthSquared() < 1e-6f) {
        // 如果 direction 与 Z 平行，就换个参考轴
        reference = Vector3::UnitX;
        right = reference.Cross(forward);
    }
    right.Normalize();
    Vector3 up = forward.Cross(right);
    up.Normalize();

    DirectX::SimpleMath::Matrix m;
    m.Right(right);
    m.Up(forward); // Y+ 朝向 direction
    m.Forward(up);
    m.Translation(Vector3::Zero);

    Quaternion targetRotation = Quaternion::CreateFromRotationMatrix(m);

    // 设置旋转和速度
    boxC.SetRotation(targetRotation);
    boxC.SetLinearVelocity(direction * speed);
}
