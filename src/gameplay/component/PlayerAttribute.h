#pragma once
#include "Component.h"

class PlayerAttribute: public Component {
private:
    int HP = 100;
    int MaxHP = 100;
public:
    void Init() override;
    void Update(float dt) override;
    void Uninit() override;
    void OnInspectorGUI() override;

    void SetHP(int hp);
    int GetHP() const;
    void SetMaxHP(int maxHp);
    int GetMaxHP() const;
    void Hit(int damage);

    bool NetPack(PacketWriter &writer) override;
    void NetUnpack(PacketReader &reader) override;
};
