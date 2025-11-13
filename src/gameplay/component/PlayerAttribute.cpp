
#include "PlayerAttribute.h"
#include "NetPlayer.h"

void PlayerAttribute::Init() {
    Component::Init();
    HP = MaxHP;
}

void PlayerAttribute::Update(float dt) {
    Component::Update(dt);
}

void PlayerAttribute::Uninit() {
    Component::Uninit();
}

void PlayerAttribute::OnInspectorGUI() {
    Component::OnInspectorGUI();
    if (ImGui::DragInt("HP", &HP, 1, 0, MaxHP)) {
        SetHP(HP);
    }
}

void PlayerAttribute::SetHP(int hp) {
    HP = hp;
    if (HP > MaxHP) {
        HP = MaxHP;
    }
    if (HP < 0) {
        HP = 0;
    }
    if (HP <= 0) {
        GameObject *player = GetGameObject();
        NetPlayer *netPlayer = dynamic_cast<NetPlayer *>(player);
        if (netPlayer != nullptr) {
            if (netPlayer->IsAlive()) {
                netPlayer->Die();
            }
        }
    } else {
        GameObject *player = GetGameObject();
        NetPlayer *netPlayer = dynamic_cast<NetPlayer *>(player);
        if (netPlayer != nullptr) {
            if (!netPlayer->IsAlive()) {
                netPlayer->Respawn();
            }
        }
    }
}

int PlayerAttribute::GetHP() const {
    return HP;
}

void PlayerAttribute::SetMaxHP(int maxHp) {
    MaxHP = maxHp;
    if (HP > MaxHP) {
        SetHP(MaxHP);
    }
}

int PlayerAttribute::GetMaxHP() const {
    return MaxHP;
}

void PlayerAttribute::Hit(int damage) {
    SetHP(HP - damage);
}

bool PlayerAttribute::NetPack(PacketWriter &writer) {
    if (!Component::NetPack(writer)) {
        return false;
    }
    writer.writeU32(HP);
    return true;
}

void PlayerAttribute::NetUnpack(PacketReader &reader) {
    Component::NetUnpack(reader);
    SetHP(reader.readU32());
}
