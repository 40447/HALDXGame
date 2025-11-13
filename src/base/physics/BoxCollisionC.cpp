#include "BoxCollisionC.h"
#include "GameObject.h"
#include "Global.h"
#include "TransformC.h"
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include "Jolt/Physics/Body/BodyCreationSettings.h"

BoxCollisionC::BoxCollisionC(JPH::EMotionType motionType, const DirectX::SimpleMath::Vector3 &size, bool IsSensor)
    : CollisionC(motionType, IsSensor),
      m_size(size) {
}

JPH::BodyCreationSettings BoxCollisionC::GetBodyCreationSettings() {
    auto &myTransform = m_gameObject->GetComponentRef<TransformC>();
    Vector3 myPos = myTransform.GetWorldPosition();

    RefConst<JPH::Shape> shape = new JPH::BoxShape(JPH::Vec3(m_size.x, m_size.y, m_size.z));
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
