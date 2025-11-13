//
// Created by eva00 on 25-9-30.
//
#pragma once

#include <enet/enet.h>      // ENet 头
#include <SimpleMath.h>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

// 发送可靠性选项
enum class NetReliability : uint8_t {
    Unreliable = 0,
    Reliable   = 1,
};

// 简单的事件回调接口
struct NetEvents {
    std::function<void(ENetPeer*)>              OnConnect;
    std::function<void(ENetPeer*, ENetPacket*)> OnReceive;
    std::function<void(ENetPeer*)>              OnDisconnect;
};

// 工具：创建 ENetPacket
inline ENetPacket* MakePacket(const void* data, size_t size, NetReliability r) {
    const enet_uint32 flags = (r == NetReliability::Reliable) ? ENET_PACKET_FLAG_RELIABLE : 0;
    return enet_packet_create(data, size, flags);
}

// 最大每帧处理事件数，防止卡死
constexpr int MAX_EVENTS_PER_FRAME = 64;

class ClientManagerC;
class ServerManagerC;
class NetGameObject;
class NetPlayer;

namespace net {

enum class MessageType : uint8_t {
    Hello = 1,
    Welcome,
    SpawnPlayer,
    DespawnPlayer,
    SpawnNetObject,
    DespawnNetObject,
    ComponentSync,
};

void ClientOnConnect(ClientManagerC &client, ENetPeer *peer);
void ClientOnReceive(ClientManagerC &client, ENetPeer *peer, ENetPacket *packet);
void ClientOnDisconnect(ClientManagerC &client, ENetPeer *peer);
void ClientShutdown(ClientManagerC &client);

void ServerOnConnect(ServerManagerC &server, ENetPeer *peer);
void ServerOnReceive(ServerManagerC &server, ENetPeer *peer, ENetPacket *packet);
void ServerOnDisconnect(ServerManagerC &server, ENetPeer *peer);
void ServerShutdown(ServerManagerC &server);

void RegisterNetObject(NetGameObject &object);
void UnregisterNetObject(NetGameObject &object);

using NetObjectFactory = std::function<NetGameObject &(uint32_t netId, uint32_t ownerId)>;
void RegisterNetObjectFactory(const std::string &className, NetObjectFactory factory);

struct ComponentPayload {
    std::string              Name;
    std::vector<uint8_t>     Data;
};

void SendComponentStates(const NetGameObject &object, const std::vector<ComponentPayload> &payloads);

bool SpawnLocalNetObject(NetGameObject &object, const std::string &className, const std::vector<ComponentPayload> &payloads);
void RequestNetObjectDestroy(const NetGameObject &object);

void OnPlayerSpawned(NetPlayer &player);
void OnPlayerDestroyed(NetPlayer &player);

bool IsLocalPlayer(uint32_t playerId);
uint32_t GetLocalPlayerId();

} // namespace net