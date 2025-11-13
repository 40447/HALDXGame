//
// Created by eva00 on 25-6-24.
//

#include "CollisionC.h"

#include "GameObject.h"
#include "Global.h"
#include "TransformC.h"

using namespace DirectX::SimpleMath;

CollisionC::CollisionC(JPH::EMotionType motionType, bool IsSensor) : m_bodyInterface(
        halgame->GetPhysicsSystem()->
                GetBodyInterface()),
                                                                     m_MotionType(motionType),
                                                                     m_IsSensor(IsSensor) {
}

void CollisionC::Init() {
    Component::Init();
    JPH::BodyCreationSettings settings = GetBodyCreationSettings();
    settings.mIsSensor = m_IsSensor;
    m_bodyID = m_bodyInterface.CreateBody(settings)->GetID();
    m_bodyInterface.AddBody(m_bodyID, JPH::EActivation::Activate);
    halgame->GetContactListener().AddBodyToUIDMap(m_bodyID, GetUID());
}

void CollisionC::Uninit() {
    Component::Uninit();
    m_bodyInterface.RemoveBody(m_bodyID);
    halgame->GetContactListener().RemoveBodyFromUIDMap(m_bodyID);
}

void CollisionC::RefeshTramsform() {
    JPH::RVec3 pos = m_bodyInterface.GetCenterOfMassPosition(m_bodyID);
    JPH::Quat rot = m_bodyInterface.GetRotation(m_bodyID);
    Quaternion dxRot(rot.GetX(), rot.GetY(), rot.GetZ(), rot.GetW());

    // 局部旋转偏移
    Quaternion offsetRot = Quaternion::CreateFromYawPitchRoll(
            m_offsetTransform.rotationOffset.y,
            m_offsetTransform.rotationOffset.x,
            m_offsetTransform.rotationOffset.z
    );

    // 注意这里换乘顺序
    Quaternion finalRot = offsetRot * dxRot;

    // 位移也基于偏移旋转后的方向
    Vector3 worldOffset = Vector3::Transform(m_offsetTransform.positionOffset, finalRot);

    auto &myTransform = m_gameObject->GetComponentRef<TransformC>();
    myTransform.SetWorldPosition(Vector3(pos.GetX(), pos.GetY(), pos.GetZ()) - worldOffset);
    myTransform.SetWorldRotation(finalRot);
}

void CollisionC::Update(float dt) {
    Component::Update(dt);
    if (isActive != m_bodyInterface.IsAdded(m_bodyID)) {
        if (isActive) {
            m_bodyInterface.AddBody(m_bodyID, JPH::EActivation::Activate);
        } else {
            m_bodyInterface.RemoveBody(m_bodyID);
        }
    }

    if (isActive) {
        RefeshTramsform();
    }
}


void CollisionC::SetOffsetPosition(const DirectX::SimpleMath::Vector3 &position) {
    m_offsetTransform.positionOffset = position;
}

void CollisionC::SetOffsetRotation(const DirectX::SimpleMath::Vector3 &rotation) {
    m_offsetTransform.rotationOffset = rotation;
}

void CollisionC::SetGravityFactor(float inGravityFactor) const {
    m_bodyInterface.SetGravityFactor(m_bodyID, inGravityFactor);
}

void CollisionC::SetLinearVelocity(const DirectX::SimpleMath::Vector3 &inLinearVelocity) const {
    JPH::RVec3 velocity(inLinearVelocity.x, inLinearVelocity.y, inLinearVelocity.z);
    m_bodyInterface.SetLinearVelocity(m_bodyID, velocity);
}

DirectX::SimpleMath::Vector3 CollisionC::GetLinearVelocity() const {
    JPH::Vec3 velocity = m_bodyInterface.GetLinearVelocity(m_bodyID);
    return {velocity.GetX(), velocity.GetY(), velocity.GetZ()};
}

void CollisionC::SetPosition(Vector3 position) {
    m_bodyInterface.SetPosition(
            m_bodyID, JPH::RVec3(position.x, position.y, position.z), JPH::EActivation::Activate);
    RefeshTramsform();
}

void CollisionC::SetRotation(Quaternion rotation) {
    m_bodyInterface.SetRotation(
            m_bodyID, JPH::Quat(rotation.x, rotation.y, rotation.z, rotation.w), JPH::EActivation::Activate);
}

void CollisionC::SetRotationEuler(Vector3 eulerRadians) {
    Quaternion q = Quaternion::CreateFromYawPitchRoll(eulerRadians.y, eulerRadians.x, eulerRadians.z);
    SetRotation(q);
}

Vector3 CollisionC::GetPosition() const {
    JPH::RVec3 pos = m_bodyInterface.GetCenterOfMassPosition(m_bodyID);
    return Vector3(pos.GetX(), pos.GetY(), pos.GetZ());
}

Vector3 CollisionC::GetRotationEuler() const {
    JPH::Quat rot = m_bodyInterface.GetRotation(m_bodyID);
    Quaternion q(rot.GetX(), rot.GetY(), rot.GetZ(), rot.GetW());

    float pitch = std::atan2(2.0f * (q.y * q.z + q.w * q.x), 1.0f - 2.0f * (q.x * q.x + q.y * q.y));
    float yaw = std::asin(2.0f * (q.w * q.y - q.x * q.z));
    float roll = std::atan2(2.0f * (q.w * q.z + q.x * q.y), 1.0f - 2.0f * (q.y * q.y + q.z * q.z));

    return Vector3(pitch, yaw, roll);
}

void CollisionC::OnInspectorGUI() {
    Component::OnInspectorGUI();

    Vector3 pos = GetPosition();
    float posArr[3] = {pos.x, pos.y, pos.z};
    if (ImGui::DragFloat3("BoxPos", posArr, 0.1f)) {
        SetPosition(Vector3(posArr[0], posArr[1], posArr[2]));
    }

    Vector3 rot = GetRotationEuler();
    float rotArr[3] = {rot.x, rot.y, rot.z};
    if (ImGui::DragFloat3("BoxRot", rotArr, 0.1f)) {
        SetRotationEuler(Vector3(rotArr[0], rotArr[1], rotArr[2]));
    }

    float offsetArr[3] = {
            m_offsetTransform.positionOffset.x, m_offsetTransform.positionOffset.y, m_offsetTransform.positionOffset.z
    };
    if (ImGui::DragFloat3("OffsetPos", offsetArr, 0.1f)) {
        m_offsetTransform.positionOffset = Vector3(offsetArr[0], offsetArr[1], offsetArr[2]);
    }

    float rotOffsetArr[3] = {
            m_offsetTransform.rotationOffset.x, m_offsetTransform.rotationOffset.y, m_offsetTransform.rotationOffset.z
    };
    if (ImGui::DragFloat3("OffsetRot", rotOffsetArr, 0.1f)) {
        m_offsetTransform.rotationOffset = Vector3(rotOffsetArr[0], rotOffsetArr[1], rotOffsetArr[2]);
    }
}

bool CollisionC::NetPack(PacketWriter &writer) {
    if (!Component::NetPack(writer)) {
        return false;
    }
    DirectX::SimpleMath::Vector3 pos = GetPosition();
    DirectX::SimpleMath::Vector4 rot = GetRotation();
    DirectX::SimpleMath::Vector3 vel = GetLinearVelocity();
    writer.writeF32(pos.x);
    writer.writeF32(pos.y);
    writer.writeF32(pos.z);
    writer.writeF32(rot.x);
    writer.writeF32(rot.y);
    writer.writeF32(rot.z);
    writer.writeF32(rot.w);
    writer.writeF32(vel.x);
    writer.writeF32(vel.y);
    writer.writeF32(vel.z);
    return true;
}

void CollisionC::NetUnpack(PacketReader &reader) {
    Component::NetUnpack(reader);
    float px = reader.readF32();
    float py = reader.readF32();
    float pz = reader.readF32();
    float rx = reader.readF32();
    float ry = reader.readF32();
    float rz = reader.readF32();
    float rw = reader.readF32();
    float vx = reader.readF32();
    float vy = reader.readF32();
    float vz = reader.readF32();
    if ((GetPosition() - Vector3(px, py, pz)).Length() > 0.01f) {
        SetPosition(Vector3(px, py, pz));
    }
    if ((GetRotation() - Quaternion(rx, ry, rz, rw)).Length() > 0.01f) {
        SetRotation(Quaternion(rx, ry, rz, rw));
    }
    DirectX::SimpleMath::Vector3 velocity = GetLinearVelocity();
    if ((velocity - DirectX::SimpleMath::Vector3(vx, vy, vz)).Length() > 0.01f) {
        SetLinearVelocity(Vector3(vx, vy, vz));
    }
}

DirectX::SimpleMath::Quaternion CollisionC::GetRotation() const {
    Quat rot = m_bodyInterface.GetRotation(m_bodyID);
    return Quaternion(rot.GetX(), rot.GetY(), rot.GetZ(), rot.GetW());
}

void CollisionC::SetActive(bool active) {
    isActive = active;
}

bool CollisionC::IsActive() const {
    return isActive;
}
