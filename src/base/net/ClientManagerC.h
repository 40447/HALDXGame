//
// Created by eva00 on 25-9-30.
//

#pragma once
#include "Net.h"
#include "Component.h"

struct PacketWriter;

struct PendingSend {
    std::vector<uint8_t> bytes;
    uint8_t channel = 0;
    NetReliability r = NetReliability::Reliable;
};

class ClientManagerC : public Component {

private:
    ENetHost* Client = nullptr;
    ENetPeer* Peer   = nullptr;
    bool      Connected = false;

    void HandleEvent(const ENetEvent& ev);
    // 发送队列，避免在 Update 之外调用 ENet 函数
    std::vector<PendingSend> SendQueue;
public:
    std::string Hostname = "127.0.0.1";
    uint16_t    Port     = 12345;
    size_t      Channels = 2;
    NetEvents   Events;

    ClientManagerC() = default;
    ~ClientManagerC() override = default;

    void Init() override;           // 创建 ENet 客户端并连接
    void Uninit() override;         // 断开并销毁
    void Update(float dt) override; // 轮询事件

    void Start();
    void Stop();

    ENetHost *GetClient() const { return Client; }
    ENetPeer *GetPeer() const { return Peer; }
    bool IsConnected() const { return Connected; }

    void Send(PacketWriter &pw, uint8_t channel = 0, NetReliability r = NetReliability::Reliable);
    void Disconnect(uint32_t data = 0);

    virtual void OnInspectorGUI() override;
};
