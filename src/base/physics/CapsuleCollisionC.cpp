//
// Created by eva00 on 25-7-2.
//

#include "CapsuleCollisionC.h"
#include "GameObject.h"
#include "Global.h"
#include "TransformC.h"
#include "Jolt/Physics/Collision/Shape/CapsuleShape.h"
#include "Jolt/Physics/Body/BodyCreationSettings.h"


CapsuleCollisionC::CapsuleCollisionC(JPH::EMotionType motionType, float radius,
                                     float halfHeight): CollisionC(motionType),
                                                        m_radius(radius),
                                                        m_halfHeight(halfHeight) {
}

JPH::BodyCreationSettings CapsuleCollisionC::GetBodyCreationSettings() {
    auto &myTransform = m_gameObject->GetComponentRef<TransformC>();
    Vector3 myPos = myTransform.GetWorldPosition();

    RefConst<JPH::Shape> shape = new JPH::CapsuleShape(m_halfHeight, m_radius);
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
