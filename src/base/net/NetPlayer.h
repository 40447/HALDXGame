//
// Created by eva00 on 25-9-30.
//

#pragma once

#include "NetGameObject.h"

class Gun;
class HPUI;

class NetPlayer : public NetGameObject {
private:
    uint32_t PlayerId = 0;
    bool isAlive = true;
private:
    GameObject *Pmodel = nullptr;
    Gun *leftGun = nullptr;
    Gun *rightGun = nullptr;
    bool fireLeftNext = true;
    HPUI *hpUI = nullptr;
public:
    static NetPlayer *LocalPlayer;

    NetPlayer();

    ~NetPlayer() override;

    void Init() override;
    void Update(float dt) override;
    void Uninit() override;
    void SetPlayerId(uint32_t id);
    [[nodiscard]] uint32_t GetPlayerId() const { return PlayerId; }
    void SetIsLocalPlayer(bool isLocal);
    [[nodiscard]] bool IsLocalPlayer() const { return IsLocalOwner(); }
    bool IsAlive() const { return isAlive; }
    // 衝突イベントのハンドラ
    void OnContactAdded(CollisionC &my, CollisionC &other, const JPH::ContactManifold &inManifold,
                        JPH::ContactSettings &ioSettings) override;

    void GunFire();

    void Move(float dt);
    // 复活処理
    void Respawn();
    // プレイヤーの死亡処理
    void Die();
};
