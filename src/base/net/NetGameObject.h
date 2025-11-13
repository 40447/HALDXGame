//
// Created by eva00 on 25-9-30.
//

#pragma once
#include <string>
#include <vector>

#include "GameObject.h"
#include "Net.h"
#include "TransformC.h"

class NetGameObject : public GameObject {
protected:
    TransformC *Transform = nullptr;
    uint32_t    NetId      = 0;
    bool        bIsLocalOwner = false;
    bool        bIsRegistered = false;

    void RegisterSelf();
    void UnregisterSelf();

public:
    NetGameObject();

    ~NetGameObject() override;

    void Init() override;

    void Update(float dt) override;

    void Uninit() override;

    void SetNetId(uint32_t id);
    [[nodiscard]] uint32_t GetNetId() const { return NetId; }

    void SetLocalOwner(bool isLocal);
    [[nodiscard]] bool IsLocalOwner() const { return bIsLocalOwner; }

    TransformC *GetTransform() const { return Transform; }

    void ApplyComponentData(const std::string &componentName, PacketReader &reader);

    [[nodiscard]] std::vector<net::ComponentPayload> CollectComponentPayloads() const;
};
