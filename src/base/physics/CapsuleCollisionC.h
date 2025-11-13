//
// Created by eva00 on 25-7-2.
//

#pragma once
#include "CollisionC.h"

class CapsuleCollisionC : public CollisionC {
private:
    float m_radius = 1.0f; // 半径
    float m_halfHeight = 1.0f; // 半分の高さ
protected:
    JPH::BodyCreationSettings GetBodyCreationSettings() override;

public:
    explicit CapsuleCollisionC(
        JPH::EMotionType motionType = JPH::EMotionType::Dynamic,
        float radius = 1.0f,
        float halfHeight = 1.0f);
};
