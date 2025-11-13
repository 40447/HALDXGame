//
// Created by eva00 on 25-7-2.
//

#include "SphereCollisionC.h"
#include "GameObject.h"
#include "Global.h"
#include "TransformC.h"
#include "Jolt/Physics/Collision/Shape/SphereShape.h"

SphereCollisionC::SphereCollisionC(JPH::EMotionType motionType, float radius)
    : CollisionC(motionType),
      m_radius(radius) {
}

JPH::BodyCreationSettings SphereCollisionC::GetBodyCreationSettings() {
    auto &myTransform = m_gameObject->GetComponentRef<TransformC>();
    Vector3 myPos = myTransform.GetWorldPosition();

    RefConst<JPH::Shape> shape = new JPH::SphereShape(m_radius);
    JPH::BodyCreationSettings settings(
        shape,
        JPH::RVec3(myPos.x + m_offsetTransform.positionOffset.x,
                   myPos.y + m_offsetTransform.positionOffset.y,
                   myPos.z + m_offsetTransform.positionOffset.z),
        JPH::Quat::sIdentity(),
        m_MotionType,
        Layers::MOVING
    );
    return settings;
}