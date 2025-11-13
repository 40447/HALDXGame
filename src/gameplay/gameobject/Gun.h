//
// Created by eva00 on 25-7-27.
//
#pragma once

#include"GameObject.h"

class Gun : public GameObject {
public:
    Gun();

    ~Gun() override;


    void Init() override;

    void Update(float dt) override;

    void Uninit() override;

    void OnContactAdded(CollisionC &my, CollisionC &other, const JPH::ContactManifold &inManifold, JPH::ContactSettings &ioSettings) override;

    void Aim(bool aiming);

    void Shoot();

private:
    float recoilTimer = 0.0f;
    float recoilDuration = 0.08f;
    float recoilAngle = 0.15f;
    Vector3 baseRotation = Vector3::Zero;
    bool isAiming = false;
};
