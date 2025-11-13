//
// Created by eva00 on 25-6-24.
//

#pragma once
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/Body/MotionType.h>
#include "Component.h"
#include "SimpleMath.h"
#include "Jolt/Physics/Body/BodyCreationSettings.h"
#include "Jolt/Physics/Body/BodyInterface.h"


struct OffsetTransform {
    DirectX::SimpleMath::Vector3 positionOffset;
    DirectX::SimpleMath::Vector3 rotationOffset;

    OffsetTransform()
        : positionOffset(DirectX::SimpleMath::Vector3::Zero),
          rotationOffset(DirectX::SimpleMath::Vector3::Zero) {}

    OffsetTransform(const DirectX::SimpleMath::Vector3& pos, const DirectX::SimpleMath::Vector3& rot)
        : positionOffset(pos), rotationOffset(rot) {}
};


class CollisionC : public Component {
protected:
    bool isActive = true;
    JPH::BodyID m_bodyID;
    JPH::BodyInterface& m_bodyInterface; // ボディロックインターフェース
    JPH::EMotionType m_MotionType; // デフォルトは動的
    bool m_IsSensor = false;
    OffsetTransform m_offsetTransform = OffsetTransform(); // 位置と回転のオフセット
    virtual JPH::BodyCreationSettings GetBodyCreationSettings() = 0; // 子クラスで実装する必要があります
public:
    explicit CollisionC(JPH::EMotionType motionType = JPH::EMotionType::Dynamic, bool IsSensor = false);
    void Init() override;
    void Update(float dt) override;
    void Uninit() override;
    // 更新Tramsformのオフセット
    void RefeshTramsform();
    void SetOffsetTransform(const OffsetTransform& offset) { m_offsetTransform = offset; }
    OffsetTransform GetOffsetTransform() const { return m_offsetTransform; }
    void SetOffsetPosition(const DirectX::SimpleMath::Vector3& position);
    void SetOffsetRotation(const DirectX::SimpleMath::Vector3& rotation);
    // 重力係数を設定
    void SetGravityFactor(float inGravityFactor) const;
    // 物理ボディの線形速度を設定
    void SetLinearVelocity(const DirectX::SimpleMath::Vector3& inLinearVelocity) const;
    void SetActive(bool active);
    bool IsActive() const;
    // 物理ボディの線形速度を取得
    DirectX::SimpleMath::Vector3 GetLinearVelocity() const;
    virtual void SetPosition(DirectX::SimpleMath::Vector3 position);
    virtual void SetRotation(DirectX::SimpleMath::Quaternion rotation);
    virtual void SetRotationEuler(DirectX::SimpleMath::Vector3 eulerRadians);
    virtual DirectX::SimpleMath::Vector3 GetPosition() const;
    virtual DirectX::SimpleMath::Quaternion GetRotation() const;
    virtual DirectX::SimpleMath::Vector3 GetRotationEuler() const;

    bool NetPack(PacketWriter &writer) override;
    void NetUnpack(PacketReader &reader) override;

    void OnInspectorGUI() override;
};