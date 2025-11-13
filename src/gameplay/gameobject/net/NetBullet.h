//
// Created by eva00 on 25-8-5.
//

#pragma once
#include "NetGameObject.h"

class NetBullet : public NetGameObject {
private:
    bool  m_hitProcessed = false;
    float m_destroyDelay = 10.0f;

public:
    NetBullet();

    ~NetBullet() override;

    void Init() override;

    void Update(float dt) override;

    void OnContactAdded(CollisionC &my, CollisionC &other, const JPH::ContactManifold &inManifold,
                        JPH::ContactSettings &ioSettings) override;

    void Uninit() override;

    void Fire(DirectX::SimpleMath::Vector3 direction, float speed = 10.0f);
};
