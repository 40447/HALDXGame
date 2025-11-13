//
// Created by eva00 on 25-6-4.
//

#include "Component.h"


Component::Component() = default;

Component::~Component() = default;

bool Component::operator==(const Component *selected) const {
    if (uid == -1 || selected == nullptr) {
        return false; // UIDが-1またはselectedがnullptrの場合はfalseを返す
    }
    if (selected->GetUID() == -1) {
        return false; // selectedのUIDが-1の場合はfalseを返す
    }
    // uidとselectedのUIDが一致するかどうかを比較
    return uid == selected->GetUID();
}

void Component::SetGameObject(GameObject *gameObject) {
    m_gameObject = gameObject;
}

GameObject *Component::GetGameObject() const {
    return m_gameObject;
}

void Component::SetUID(UID uuid) {
    uid = uuid;
}

UID Component::GetUID() const {
    return uid;
}

void Component::Init() {
}

void Component::Uninit() {
}

void Component::Update(float dt) {
}

void Component::OnInspectorGUI() {
}

bool Component::NetPack(PacketWriter &writer) {
    return true;
}

void Component::NetUnpack(PacketReader &reader) {
}
