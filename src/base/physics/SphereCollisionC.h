//
// Created by eva00 on 25-7-2.
//

#pragma once
#include "CollisionC.h"

class SphereCollisionC : public CollisionC {
private:
    float m_radius;
protected:
    JPH::BodyCreationSettings GetBodyCreationSettings() override;
public:
    explicit SphereCollisionC(
        JPH::EMotionType motionType = JPH::EMotionType::Dynamic,
        float radius = 1.0f);
};
