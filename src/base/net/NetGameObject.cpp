//
// Created by eva00 on 25-9-30.
//

#include "NetGameObject.h"

#include "Component.h"

#include <limits>
#include <utility>

NetGameObject::NetGameObject() = default;

NetGameObject::~NetGameObject() {
    UnregisterSelf();
}

void NetGameObject::Init() {
    GameObject::Init();
    Transform = GetComponent<TransformC>();
}

void NetGameObject::Update(float dt) {
    GameObject::Update(dt);

    if (!bIsLocalOwner || NetId == 0) {
        return;
    }
    std::vector<net::ComponentPayload> payloads = CollectComponentPayloads();
    if (!payloads.empty()) {
        net::SendComponentStates(*this, payloads);
    }
}

void NetGameObject::Uninit() {
    UnregisterSelf();
    GameObject::Uninit();
}

void NetGameObject::SetNetId(uint32_t id) {
    if (NetId == id) {
        return;
    }
    NetId = id;
    RegisterSelf();
}

void NetGameObject::SetLocalOwner(bool isLocal) {
    bIsLocalOwner = isLocal;
}

void NetGameObject::ApplyComponentData(const std::string &componentName, PacketReader &reader) {
    if (bIsLocalOwner) {
        return;
    }

    Component *component = GetComponent<Component>(componentName);
    if (component == nullptr) {
        return;
    }

    component->NetUnpack(reader);
}

void NetGameObject::RegisterSelf() {
    if (NetId == 0 || bIsRegistered == true) {
        return;
    }
    net::RegisterNetObject(*this);
    bIsRegistered = true;
}

void NetGameObject::UnregisterSelf() {
    if (!bIsRegistered) {
        return;
    }
    net::UnregisterNetObject(*this);
    bIsRegistered = false;
}

std::vector<net::ComponentPayload> NetGameObject::CollectComponentPayloads() const {
    std::vector<net::ComponentPayload> payloads;
    const auto componentMap = GetComponentMap();
    if (componentMap.empty()) {
        return payloads;
    }

    payloads.reserve(componentMap.size());
    for (const auto &entry : componentMap) {
        Component *component = GetComponent<Component>(entry.first);
        if (component == nullptr) {
            continue;
        }

        PacketWriter writer;
        if (!component->NetPack(writer)) {
            continue;
        }

        if (writer.buf.empty()) {
            continue;
        }

        if (writer.buf.size() > std::numeric_limits<uint16_t>::max()) {
            continue;
        }

        net::ComponentPayload payload;
        payload.Name = entry.first;
        payload.Data = std::move(writer.buf);
        payloads.emplace_back(std::move(payload));
    }
    return payloads;
}
