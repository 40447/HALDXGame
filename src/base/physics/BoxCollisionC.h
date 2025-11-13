//
// Created by eva00 on 25-6-4.
//

#pragma once
#include "CollisionC.h"

class BoxCollisionC : public CollisionC {
private:
    DirectX::SimpleMath::Vector3 m_size;

protected:
    JPH::BodyCreationSettings GetBodyCreationSettings() override;

public:
    explicit BoxCollisionC(
        JPH::EMotionType motionType = JPH::EMotionType::Dynamic,
        const DirectX::SimpleMath::Vector3 &size = DirectX::SimpleMath::Vector3(1, 1, 1), bool IsSensor = false);
};
