//
// Created by eva00 on 2025/9/29.
//

#include "Net.h"
#include "ClientManagerC.h"
#include "Global.h"
#include "Game.h"
#include "PacketIO.h"
#include "Scene.h"
#include "ServerManagerC.h"
#include "NetGameObject.h"
#include "NetPlayer.h"

#include <exception>
#include <iostream>
#include <limits>
#include <string>
#include <unordered_map>
#include <vector>

namespace {
    struct NetState {
        ClientManagerC *Client = nullptr;
        std::unordered_map<uint32_t, NetGameObject *> Objects;
        std::unordered_map<uint32_t, NetPlayer *> Players;
        std::unordered_map<ENetPeer *, uint32_t> PeerToPlayer;
        std::unordered_map<uint32_t, uint32_t> ObjectOwners;
        std::unordered_map<std::string, net::NetObjectFactory> Factories;
        uint32_t LocalPlayerId = 0;
        uint32_t NextPlayerId = 1;
        uint16_t NextLocalObjectCounter = 1;
    };

    NetState &GetNetState() {
        static NetState state;
        return state;
    }

    NetPlayer *SpawnPlayerOnClient(uint32_t playerId, bool isLocal) {
        if (playerId == 0) {
            return nullptr;
        }
        NetState &gNetState = GetNetState();
        auto it = gNetState.Players.find(playerId);
        if (it != gNetState.Players.end() && it->second != nullptr) {
            return it->second;
        }
        if (halgame == nullptr || halgame->GetScene() == nullptr) {
            return nullptr;
        }

        NetPlayer &player = halgame->GetScene()->AddGameObject<NetPlayer>();
        player.SetPlayerId(playerId);
        player.SetIsLocalPlayer(isLocal);
        return &player;
    }

    void DespawnPlayerOnClient(uint32_t playerId) {
        NetState &gNetState = GetNetState();
        auto it = gNetState.Players.find(playerId);
        if (it == gNetState.Players.end()) {
            return;
        }
        NetPlayer *player = it->second;
        if (player != nullptr) {
            player->Destroy();
        }
        gNetState.Players.erase(playerId);
    }

    void ApplyComponentSyncOnClient(uint32_t netId, uint16_t componentCount, PacketReader &reader) {
        NetState &gNetState = GetNetState();
        auto objIt = gNetState.Objects.find(netId);
        NetGameObject *object = nullptr;
        if (objIt != gNetState.Objects.end()) {
            object = objIt->second;
        }

        for (uint16_t i = 0; i < componentCount; ++i) {
            std::string componentName = reader.readString();
            uint16_t payloadSize = reader.readU16();
            std::vector<uint8_t> data = reader.readBytes(payloadSize);

            if (object == nullptr || object->IsLocalOwner()) {
                continue;
            }

            PacketReader componentReader(data.data(), data.size());
            object->ApplyComponentData(componentName, componentReader);
        }
    }

    std::vector<net::ComponentPayload> ReadComponentPayloads(uint16_t componentCount, PacketReader &reader) {
        std::vector<net::ComponentPayload> payloads;
        payloads.reserve(componentCount);
        for (uint16_t i = 0; i < componentCount; ++i) {
            net::ComponentPayload payload;
            payload.Name = reader.readString();
            uint16_t payloadSize = reader.readU16();
            payload.Data = reader.readBytes(payloadSize);
            payloads.emplace_back(std::move(payload));
        }
        return payloads;
    }

    void ApplyPayloadsToObject(NetGameObject &object, const std::vector<net::ComponentPayload> &payloads) {
        for (const auto &payload : payloads) {
            if (payload.Data.empty()) {
                continue;
            }
            PacketReader componentReader(payload.Data.data(), payload.Data.size());
            object.ApplyComponentData(payload.Name, componentReader);
        }
    }

    NetGameObject *SpawnNetObjectOnClient(const std::string &className, uint32_t netId, uint32_t ownerId,
                                          const std::vector<net::ComponentPayload> &payloads) {
        NetState &gNetState = GetNetState();
        auto factoryIt = gNetState.Factories.find(className);
        if (factoryIt == gNetState.Factories.end()) {
            return nullptr;
        }
        NetGameObject &object = factoryIt->second(netId, ownerId);
        object.SetLocalOwner(ownerId != 0 && ownerId == gNetState.LocalPlayerId);
        object.SetNetId(netId);
        gNetState.ObjectOwners[netId] = ownerId;
        if (!object.IsLocalOwner()) {
            ApplyPayloadsToObject(object, payloads);
        }
        return &object;
    }

    uint32_t GenerateLocalNetId(uint32_t ownerId) {
        NetState &gNetState = GetNetState();
        if (ownerId == 0) {
            return 0;
        }
        for (uint32_t attempt = 0; attempt < std::numeric_limits<uint16_t>::max(); ++attempt) {
            uint16_t counter = gNetState.NextLocalObjectCounter++;
            if (gNetState.NextLocalObjectCounter == 0) {
                gNetState.NextLocalObjectCounter = 1;
            }
            uint32_t netId = (ownerId << 16) | counter;
            if (netId == ownerId) {
                continue; // reserve player ids
            }
            if (gNetState.Objects.find(netId) != gNetState.Objects.end()) {
                continue;
            }
            if (gNetState.ObjectOwners.find(netId) != gNetState.ObjectOwners.end()) {
                continue;
            }
            return netId;
        }
        return 0;
    }
} // namespace

namespace net {
    void ClientOnConnect(ClientManagerC &client, ENetPeer * /*peer*/) {
        NetState &gNetState = GetNetState();
        gNetState.Client = &client;
        PacketWriter pw;
        pw.writeU8(static_cast<uint8_t>(MessageType::Hello));
        client.Send(pw, 0, NetReliability::Reliable);
    }

    void ClientOnReceive(ClientManagerC & /*client*/, ENetPeer * /*peer*/, ENetPacket *packet) {
        NetState &gNetState = GetNetState();
        try {
            PacketReader reader(packet->data, packet->dataLength);
            MessageType type = static_cast<MessageType>(reader.readU8());
            switch (type) {
                case MessageType::Welcome: {
                    gNetState.LocalPlayerId = reader.readU32();
                    gNetState.NextLocalObjectCounter = 1;
                    break;
                }
                case MessageType::SpawnPlayer: {
                    uint32_t playerId = reader.readU32();
                    bool isLocal = (playerId == gNetState.LocalPlayerId);
                    SpawnPlayerOnClient(playerId, isLocal);
                    break;
                }
                case MessageType::DespawnPlayer: {
                    uint32_t playerId = reader.readU32();
                    if (playerId == gNetState.LocalPlayerId) {
                        gNetState.LocalPlayerId = 0;
                    }
                    DespawnPlayerOnClient(playerId);
                    break;
                }
                case MessageType::SpawnNetObject: {
                    uint32_t netId = reader.readU32();
                    uint32_t ownerId = reader.readU32();
                    std::string className = reader.readString();
                    uint16_t componentCount = reader.readU16();
                    std::vector<net::ComponentPayload> payloads = ReadComponentPayloads(componentCount, reader);

                    gNetState.ObjectOwners[netId] = ownerId;
                    auto objIt = gNetState.Objects.find(netId);
                    if (objIt != gNetState.Objects.end() && objIt->second != nullptr) {
                        NetGameObject *object = objIt->second;
                        object->SetLocalOwner(ownerId != 0 && ownerId == gNetState.LocalPlayerId);
                        if (!object->IsLocalOwner()) {
                            ApplyPayloadsToObject(*object, payloads);
                        }
                        break;
                    }
                    SpawnNetObjectOnClient(className, netId, ownerId, payloads);
                    break;
                }
                case MessageType::DespawnNetObject: {
                    uint32_t netId = reader.readU32();
                    reader.readU32(); // ownerId, currently unused on client
                    gNetState.ObjectOwners.erase(netId);
                    auto objIt = gNetState.Objects.find(netId);
                    if (objIt != gNetState.Objects.end()) {
                        if (NetGameObject *object = objIt->second) {
                            object->Destroy();
                        }
                    }
                    break;
                }
                case MessageType::ComponentSync: {
                    uint32_t netId = reader.readU32();
                    uint16_t componentCount = reader.readU16();
                    ApplyComponentSyncOnClient(netId, componentCount, reader);
                    break;
                }
                default:
                    break;
            }
        } catch (const std::exception &e) {
            std::cout << "[Net] Failed to process client packet: " << e.what() << std::endl;
        }
    }

    void ClientOnDisconnect(ClientManagerC &client, ENetPeer * /*peer*/) {
        NetState &gNetState = GetNetState();
        if (gNetState.Client == &client) {
            gNetState.Client = nullptr;
        }
        gNetState.LocalPlayerId = 0;

        std::vector<uint32_t> playerIds;
        playerIds.reserve(gNetState.Players.size());
        for (const auto &kv: gNetState.Players) {
            playerIds.push_back(kv.first);
        }
        for (uint32_t id: playerIds) {
            DespawnPlayerOnClient(id);
        }
        gNetState.Objects.clear();
        gNetState.ObjectOwners.clear();
        gNetState.NextLocalObjectCounter = 1;
    }

    void ClientShutdown(ClientManagerC &client) {
        NetState &gNetState = GetNetState();
        if (gNetState.Client == &client) {
            gNetState.Client = nullptr;
        }
        gNetState.LocalPlayerId = 0;
        std::vector<uint32_t> playerIds;
        playerIds.reserve(gNetState.Players.size());
        for (const auto &kv: gNetState.Players) {
            playerIds.push_back(kv.first);
        }
        for (uint32_t id: playerIds) {
            DespawnPlayerOnClient(id);
        }
        gNetState.Objects.clear();
        gNetState.Players.clear();
        gNetState.ObjectOwners.clear();
        gNetState.NextLocalObjectCounter = 1;
    }

    void ServerOnConnect(ServerManagerC & /*server*/, ENetPeer *peer) {
        NetState &gNetState = GetNetState();
        gNetState.PeerToPlayer.erase(peer);
    }

    void ServerOnReceive(ServerManagerC &server, ENetPeer *peer, ENetPacket *packet) {
        NetState &gNetState = GetNetState();
        try {
            PacketReader reader(packet->data, packet->dataLength);
            MessageType type = static_cast<MessageType>(reader.readU8());
            switch (type) {
                case MessageType::Hello: {
                    uint32_t playerId = gNetState.NextPlayerId++;
                    gNetState.PeerToPlayer[peer] = playerId;

                    // Welcome message to the new client
                    PacketWriter welcome;
                    welcome.writeU8(static_cast<uint8_t>(MessageType::Welcome));
                    welcome.writeU32(playerId);
                    server.SendTo(peer, welcome, 0, NetReliability::Reliable);

                    // Inform the new client about existing players
                    for (const auto &kv: gNetState.PeerToPlayer) {
                        if (kv.first == peer) {
                            continue;
                        }
                        PacketWriter spawnExisting;
                        spawnExisting.writeU8(static_cast<uint8_t>(MessageType::SpawnPlayer));
                        spawnExisting.writeU32(kv.second);
                        server.SendTo(peer, spawnExisting, 0, NetReliability::Reliable);
                    }

                    // Broadcast the new player to everyone
                    PacketWriter spawnNew;
                    spawnNew.writeU8(static_cast<uint8_t>(MessageType::SpawnPlayer));
                    spawnNew.writeU32(playerId);
                    server.Broadcast(spawnNew, 0, NetReliability::Reliable);
                    break;
                }
                case MessageType::SpawnNetObject: {
                    uint32_t netId = reader.readU32();
                    uint32_t ownerId = reader.readU32();
                    std::string className = reader.readString();
                    uint16_t componentCount = reader.readU16();

                    auto peerIt = gNetState.PeerToPlayer.find(peer);
                    if (peerIt == gNetState.PeerToPlayer.end() || peerIt->second != ownerId) {
                        ReadComponentPayloads(componentCount, reader);
                        break;
                    }

                    PacketWriter relay;
                    relay.writeU8(static_cast<uint8_t>(MessageType::SpawnNetObject));
                    relay.writeU32(netId);
                    relay.writeU32(ownerId);
                    relay.writeString(className);
                    relay.writeU16(componentCount);

                    for (uint16_t i = 0; i < componentCount; ++i) {
                        std::string componentName = reader.readString();
                        uint16_t payloadSize = reader.readU16();
                        std::vector<uint8_t> data = reader.readBytes(payloadSize);

                        relay.writeString(componentName);
                        relay.writeU16(payloadSize);
                        if (!data.empty()) {
                            relay.writeBytes(data.data(), data.size());
                        }
                    }

                    gNetState.ObjectOwners[netId] = ownerId;
                    server.Broadcast(relay, 0, NetReliability::Reliable);
                    break;
                }
                case MessageType::DespawnNetObject: {
                    uint32_t netId = reader.readU32();
                    uint32_t ownerId = reader.readU32();

                    auto peerIt = gNetState.PeerToPlayer.find(peer);
                    if (peerIt == gNetState.PeerToPlayer.end() || peerIt->second != ownerId) {
                        break;
                    }

                    gNetState.ObjectOwners.erase(netId);

                    PacketWriter relay;
                    relay.writeU8(static_cast<uint8_t>(MessageType::DespawnNetObject));
                    relay.writeU32(netId);
                    relay.writeU32(ownerId);
                    server.Broadcast(relay, 0, NetReliability::Reliable);
                    break;
                }
                case MessageType::ComponentSync: {
                    uint32_t netId = reader.readU32();
                    uint16_t componentCount = reader.readU16();

                    auto it = gNetState.PeerToPlayer.find(peer);
                    if (it == gNetState.PeerToPlayer.end()) {
                        ReadComponentPayloads(componentCount, reader);
                        break;
                    }

                    uint32_t playerId = it->second;
                    std::vector<net::ComponentPayload> payloads = ReadComponentPayloads(componentCount, reader);

                    bool allowed = (netId == playerId);
                    if (!allowed) {
                        auto ownerIt = gNetState.ObjectOwners.find(netId);
                        if (ownerIt != gNetState.ObjectOwners.end()) {
                            allowed = (ownerIt->second == playerId);
                        } else if ((netId >> 16) == playerId) {
                            allowed = true;
                        }
                    }

                    if (!allowed) {
                        break;
                    }

                    if (gNetState.ObjectOwners.find(netId) == gNetState.ObjectOwners.end() && netId != playerId) {
                        gNetState.ObjectOwners[netId] = playerId;
                    }

                    PacketWriter relay;
                    relay.writeU8(static_cast<uint8_t>(MessageType::ComponentSync));
                    relay.writeU32(netId);
                    relay.writeU16(componentCount);

                    for (const auto &payload : payloads) {
                        relay.writeString(payload.Name);
                        relay.writeU16(static_cast<uint16_t>(payload.Data.size()));
                        if (!payload.Data.empty()) {
                            relay.writeBytes(payload.Data.data(), payload.Data.size());
                        }
                    }

                    server.Broadcast(relay, 0, NetReliability::Unreliable);
                    break;
                }
                default:
                    break;
            }
        } catch (const std::exception &e) {
            std::cout << "[Net] Failed to process server packet: " << e.what() << std::endl;
        }
    }

    void ServerOnDisconnect(ServerManagerC &server, ENetPeer *peer) {
        NetState &gNetState = GetNetState();
        auto it = gNetState.PeerToPlayer.find(peer);
        if (it == gNetState.PeerToPlayer.end()) {
            return;
        }
        uint32_t playerId = it->second;
        gNetState.PeerToPlayer.erase(it);

        std::vector<uint32_t> ownedNetIds;
        ownedNetIds.reserve(gNetState.ObjectOwners.size());
        for (const auto &kv : gNetState.ObjectOwners) {
            if (kv.second == playerId) {
                ownedNetIds.push_back(kv.first);
            }
        }
        for (uint32_t netId : ownedNetIds) {
            gNetState.ObjectOwners.erase(netId);
            PacketWriter despawnObject;
            despawnObject.writeU8(static_cast<uint8_t>(MessageType::DespawnNetObject));
            despawnObject.writeU32(netId);
            despawnObject.writeU32(playerId);
            server.Broadcast(despawnObject, 0, NetReliability::Reliable);
        }

        PacketWriter despawn;
        despawn.writeU8(static_cast<uint8_t>(MessageType::DespawnPlayer));
        despawn.writeU32(playerId);
        server.Broadcast(despawn, 0, NetReliability::Reliable);
    }

    void ServerShutdown(ServerManagerC & /*server*/) {
        NetState &gNetState = GetNetState();
        gNetState.PeerToPlayer.clear();
        gNetState.NextPlayerId = 1;
        gNetState.ObjectOwners.clear();
    }

    void RegisterNetObject(NetGameObject &object) {
        NetState &gNetState = GetNetState();
        if (object.GetNetId() == 0) {
            return;
        }
        gNetState.Objects[object.GetNetId()] = &object;
    }

    void UnregisterNetObject(NetGameObject &object) {
        NetState &gNetState = GetNetState();
        uint32_t netId = object.GetNetId();
        auto it = gNetState.Objects.find(netId);
        if (it != gNetState.Objects.end() && it->second == &object) {
            gNetState.Objects.erase(it);
        }
        gNetState.ObjectOwners.erase(netId);
    }

    void SendComponentStates(const NetGameObject &object, const std::vector<ComponentPayload> &payloads) {
        NetState &gNetState = GetNetState();
        if (gNetState.Client == nullptr) {
            return;
        }
        if (!gNetState.Client->IsConnected()) {
            return;
        }
        if (object.GetNetId() == 0) {
            return;
        }
        if (payloads.empty()) {
            return;
        }

        std::vector<const ComponentPayload *> validPayloads;
        validPayloads.reserve(payloads.size());
        for (const auto &payload: payloads) {
            if (payload.Data.size() > std::numeric_limits<uint16_t>::max()) {
                continue;
            }
            validPayloads.push_back(&payload);
        }

        if (validPayloads.empty()) {
            return;
        }

        PacketWriter pw;
        pw.writeU8(static_cast<uint8_t>(MessageType::ComponentSync));
        pw.writeU32(object.GetNetId());
        pw.writeU16(static_cast<uint16_t>(validPayloads.size()));

        for (const ComponentPayload *payload: validPayloads) {
            pw.writeString(payload->Name);
            pw.writeU16(static_cast<uint16_t>(payload->Data.size()));
            if (!payload->Data.empty()) {
                pw.writeBytes(payload->Data.data(), payload->Data.size());
            }
        }

        gNetState.Client->Send(pw, 0, NetReliability::Unreliable);
    }

    void RegisterNetObjectFactory(const std::string &className, NetObjectFactory factory) {
        NetState &gNetState = GetNetState();
        if (className.empty()) {
            return;
        }
        if (!factory) {
            gNetState.Factories.erase(className);
            return;
        }
        gNetState.Factories[className] = std::move(factory);
    }

    bool SpawnLocalNetObject(NetGameObject &object, const std::string &className, const std::vector<ComponentPayload> &payloads) {
        NetState &gNetState = GetNetState();
        if (className.empty()) {
            return false;
        }
        if (gNetState.Factories.find(className) == gNetState.Factories.end()) {
            return false;
        }
        uint32_t ownerId = gNetState.LocalPlayerId;
        if (ownerId == 0) {
            return false;
        }
        if (!object.IsLocalOwner()) {
            object.SetLocalOwner(true);
        }
        uint32_t netId = object.GetNetId();
        if (netId == 0) {
            netId = GenerateLocalNetId(ownerId);
            if (netId == 0) {
                return false;
            }
            object.SetNetId(netId);
        }
        gNetState.ObjectOwners[netId] = ownerId;

        if (gNetState.Client == nullptr || !gNetState.Client->IsConnected()) {
            return true;
        }

        std::vector<const ComponentPayload *> validPayloads;
        validPayloads.reserve(payloads.size());
        for (const auto &payload : payloads) {
            if (payload.Data.size() > std::numeric_limits<uint16_t>::max()) {
                continue;
            }
            validPayloads.push_back(&payload);
        }

        PacketWriter pw;
        pw.writeU8(static_cast<uint8_t>(MessageType::SpawnNetObject));
        pw.writeU32(netId);
        pw.writeU32(ownerId);
        pw.writeString(className);
        pw.writeU16(static_cast<uint16_t>(validPayloads.size()));

        for (const ComponentPayload *payload : validPayloads) {
            pw.writeString(payload->Name);
            pw.writeU16(static_cast<uint16_t>(payload->Data.size()));
            if (!payload->Data.empty()) {
                pw.writeBytes(payload->Data.data(), payload->Data.size());
            }
        }

        gNetState.Client->Send(pw, 0, NetReliability::Reliable);
        return true;
    }

    void RequestNetObjectDestroy(const NetGameObject &object) {
        NetState &gNetState = GetNetState();
        if (!object.IsLocalOwner()) {
            return;
        }
        uint32_t netId = object.GetNetId();
        if (netId == 0) {
            return;
        }
        gNetState.ObjectOwners.erase(netId);
        if (gNetState.Client == nullptr || !gNetState.Client->IsConnected()) {
            return;
        }
        uint32_t ownerId = gNetState.LocalPlayerId;
        if (ownerId == 0) {
            return;
        }

        PacketWriter pw;
        pw.writeU8(static_cast<uint8_t>(MessageType::DespawnNetObject));
        pw.writeU32(netId);
        pw.writeU32(ownerId);
        gNetState.Client->Send(pw, 0, NetReliability::Reliable);
    }

    void OnPlayerSpawned(NetPlayer &player) {
        NetState &gNetState = GetNetState();
        if (player.GetPlayerId() == 0) {
            return;
        }
        gNetState.Players[player.GetPlayerId()] = &player;
    }

    void OnPlayerDestroyed(NetPlayer &player) {
        NetState &gNetState = GetNetState();
        auto it = gNetState.Players.find(player.GetPlayerId());
        if (it != gNetState.Players.end() && it->second == &player) {
            gNetState.Players.erase(it);
        }
    }

    bool IsLocalPlayer(uint32_t playerId) {
        NetState &gNetState = GetNetState();
        return playerId != 0 && playerId == gNetState.LocalPlayerId;
    }

    uint32_t GetLocalPlayerId() {
        NetState &gNetState = GetNetState();
        return gNetState.LocalPlayerId;
    }
}
