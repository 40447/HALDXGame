//
// Created by eva00 on 25-9-30.
//
#pragma once
#include "Net.h"
#include "Component.h"

struct PacketWriter;

class ServerManagerC : public Component {
private:
    ENetHost *Server = nullptr;
    std::vector<ENetPeer *> Peers;

    void HandleEvent(const ENetEvent &ev);

public:
    uint16_t Port = 12345;
    size_t MaxClients = 32;
    size_t Channels = 2;
    NetEvents Events;

public:
    ServerManagerC() = default;

    ~ServerManagerC() override = default;

    void Init() override; // 创建 ENetHost（监听）
    void Uninit() override; // 销毁 ENetHost
    void Update(float dt) override; // 轮询 ENet 事件

    void Start();

    void Stop();

    void Broadcast(PacketWriter &pw, uint8_t channel = 0, NetReliability r = NetReliability::Reliable);

    void SendTo(ENetPeer *peer, PacketWriter &pw, uint8_t channel = 0, NetReliability r = NetReliability::Reliable);

    const std::vector<ENetPeer *> &GetPeers() const { return Peers; }

    void OnConnect(ENetPeer *peer);

    void OnReceive(ENetPeer *peer, ENetPacket *packet);

    void OnDisconnect(ENetPeer *peer);

    virtual void OnInspectorGUI() override;
};
