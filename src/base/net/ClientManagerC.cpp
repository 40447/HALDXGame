//
// Created by eva00 on 25-9-30.
//

#include "ClientManagerC.h"

#include <iostream>

#include "imgui.h"
#include "PacketIO.h"
#include "Net.h"
#include "NetPlayer.h"

void ClientManagerC::Init() {
}

void ClientManagerC::Uninit() {
    Stop();
}

void ClientManagerC::Update(float dt) {
    if (Client == nullptr) {
        return;
    }
    ENetEvent ev;
    int handled = 0;
    while (handled < MAX_EVENTS_PER_FRAME && enet_host_service(Client, &ev, 0) > 0) {
        HandleEvent(ev);
        ++handled;
        if (Client == nullptr) {
            break;
        }
    }
    if (Peer && Connected && !SendQueue.empty()) {
        for (auto& s : SendQueue) {
            ENetPacket* pkt = MakePacket(s.bytes.data(), s.bytes.size(), s.r);
            enet_peer_send(Peer, s.channel, pkt);
        }
        SendQueue.clear();
        enet_host_flush(Client);
    }
}

void ClientManagerC::Start() {
    if (Client != nullptr) {
        return;
    }
    Client = enet_host_create(nullptr, 1, Channels, 0, 0);
    if (Client == nullptr) {
        std::cout << "[ClientC] Failed to create client host::" << Hostname << ":" << Port << std::endl;
        return;
    }
    ENetAddress addr{};
    if (enet_address_set_host(&addr, Hostname.c_str()) != 0) {
        std::cout << "[ClientC] Failed to resolve hostname: " << Hostname << std::endl;
        enet_host_destroy(Client);
        Client = nullptr;
        return;
    }
    addr.port = Port;

    Peer = enet_host_connect(Client, &addr, Channels, 0);
    if (!Peer) {
        std::cout << "[ClientC] No available peers for initiating connection to " << Hostname << ":" << Port << std::endl;
        enet_host_destroy(Client);
        Client = nullptr;
        return;
    }
    std::cout << "[ClientC] Connecting to " << Hostname << ":" << Port << std::endl;
}

void ClientManagerC::Stop() {
    if (Peer) {
        enet_peer_disconnect(Peer, 0);
        ENetEvent ev;
        bool gotDisconnect = false;

        // 等待断开
        while (enet_host_service(Client, &ev, 100) > 0) {
            if (ev.type == ENET_EVENT_TYPE_DISCONNECT) {
                gotDisconnect = true;
                break;
            }
        }
        if (!gotDisconnect) {
            enet_peer_reset(Peer); // 仅在未收到DISCONNECT时强制重置
        }
        Peer = nullptr;
    }
    if (Client) {
        enet_host_destroy(Client);
        Client = nullptr;
        std::cout << "[ClientC] Stopped" << std::endl;
    }
    Connected = false;
    net::ClientShutdown(*this);
    NetPlayer::LocalPlayer = nullptr;
}


void ClientManagerC::HandleEvent(const ENetEvent &ev) {
    switch (ev.type) {
        case ENET_EVENT_TYPE_CONNECT:
            Connected = true;
            net::ClientOnConnect(*this, ev.peer);
            if (Events.OnConnect) {
                Events.OnConnect(ev.peer);
            }
            std::cout << "[ClientC] Connected to " << Hostname << ":" << Port << std::endl;
            break;
        case ENET_EVENT_TYPE_RECEIVE:
            net::ClientOnReceive(*this, ev.peer, ev.packet);
            if (Events.OnReceive) {
                Events.OnReceive(ev.peer, ev.packet);
            }
            enet_packet_destroy(ev.packet);
            break;
        case ENET_EVENT_TYPE_DISCONNECT:
            Connected = false;
            net::ClientOnDisconnect(*this, ev.peer);
            if (Events.OnDisconnect) {
                Events.OnDisconnect(ev.peer);
            }
            std::cout << "[ClientC] Disconnected from " << Hostname << ":" << Port << std::endl;
            if (Peer) {
                Peer = nullptr;
            }
            if (Client != nullptr) {
                enet_host_destroy(Client);
                Client = nullptr;
            }
            break;
        default: break;
    }
}

void ClientManagerC::Send(PacketWriter &pw, uint8_t channel, NetReliability r) {
    if (Peer == nullptr) {
        std::cout << "[ClientC] Cannot send data, not connected." << std::endl;
        return;
    }
    if (Client == nullptr) {
        std::cout << "[ClientC] Cannot send data, client not initialized." << std::endl;
        return;
    }
    PendingSend s;
    s.bytes.assign(pw.data(), pw.data() + pw.size());
    s.channel = channel;
    s.r = r;
    SendQueue.emplace_back(std::move(s));
}

void ClientManagerC::Disconnect(uint32_t data) {
    if (Peer && Connected) {
        enet_peer_disconnect(Peer, data);
    }
}

void ClientManagerC::OnInspectorGUI() {
    Component::OnInspectorGUI();
    ImGui::Text("Hostname: %s", Hostname.c_str());
    ImGui::Text("Port: %d", Port);
    ImGui::Text("Channels: %zu", Channels);
    ImGui::Text("Connected: %s", Connected ? "Yes" : "No");
}
