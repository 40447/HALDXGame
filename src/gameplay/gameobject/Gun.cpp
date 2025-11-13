
#include "Gun.h"

#include "ModelC.h"
#include "TransformC.h"
#include "BoxCollisionC.h"
#include "NetBullet.h"
#include "Net.h"
#include "CapsuleCollisionC.h"
Gun::Gun() {
}
Gun::~Gun() {
}


void Gun::Init() {
    GameObject::Init();


    AddComponent<ModelC>("assets/models/Player/GUN.fbx");

    GetComponentRef<TransformC>().SetLocalPosition(Vector3(0, 0, 0));

    //AddComponent<BoxCollisionC>(JPH::EMotionType::Dynamic, Vector3(1, 2, 3));W
}

void Gun::Update(float dt) {

    GameObject::Update(dt);

    float pitchOffset = 0.0f;
    if (recoilTimer > 0.0f) {
        recoilTimer -= dt;
        float t = 1.0f - (recoilTimer / recoilDuration);
        pitchOffset = recoilAngle * (1.0f - t);
    }

    GetComponentRef<TransformC>().SetLocalRotationEuler(
           baseRotation + Vector3(pitchOffset, 0.0f, 0.0f)
       );



}
void Gun::Uninit() {
    GameObject::Uninit();
}

void Gun::OnContactAdded(CollisionC &my, CollisionC &other, const JPH::ContactManifold &inManifold, JPH::ContactSettings &ioSettings) {
    GameObject::OnContactAdded(my, other, inManifold, ioSettings);

}

void Gun::Aim(bool aiming) {

    isAiming = aiming;

    if (aiming) {
        // GetComponentRef<TransformC>().SetLocalRotationEuler(Vector3(DirectX::XM_PI / 2, 0.0f, 0.0f));
        baseRotation = Vector3(DirectX::XM_PI / 2, 0.0f, 0.0f);
    } else {
        // GetComponentRef<TransformC>().SetLocalRotationEuler(Vector3(0.0f, 0.0f, 0.0f));
        baseRotation = Vector3(0.0f, 0.0f, 0.0f);
    }
}

void Gun::Shoot() {

    NetBullet &bullet = halgame->GetScene()->AddGameObject<NetBullet>();
    bullet.SetLocalOwner(true);
    TransformC &myTransformC = GetComponentRef<TransformC>();

    Vector3 muzzleOffsetLocal(0.0f, -10.0f, -0.8f);
    DirectX::SimpleMath::Matrix gunWorld = myTransformC.GetWorldMatrix();
    Vector3 muzzleWorldPos = Vector3::Transform(muzzleOffsetLocal, gunWorld);

    bullet.GetComponentRef<CapsuleCollisionC>().SetPosition(muzzleWorldPos);
    bullet.Fire(myTransformC.GetUp() * -1, 30.0f);
    net::SpawnLocalNetObject(bullet, "Cbullet", bullet.CollectComponentPayloads());
    recoilTimer = recoilDuration;
}
