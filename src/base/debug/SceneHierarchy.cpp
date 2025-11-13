//
// Created by eva00 on 25-6-11.
//

#include "SceneHierarchy.h"

#include "GameObject.h"
#include "Global.h"
#include "imgui.h"
#include "TransformC.h"

SceneHierarchy::SceneHierarchy() {
    m_selectedid = -1;
}

void SceneHierarchy::Draw() {
    if (inited == false) {
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(300, ImGui::GetIO().DisplaySize.y));
    }
    ImGui::Begin("Hierarchy", nullptr, ImGuiWindowFlags_NoMove);
    GameObject *root = halgame->GetScene()->GetRoot();
    if (root != nullptr) {
        UID rootid = root->GetUID();
        DrawSceneNode(rootid, m_selectedid);
    } else {
        ImGui::Text("No Scene Loaded");
    }
    ImGui::End();
    if (inited == false) {
        ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 300, 0));
        ImGui::SetNextWindowSize(ImVec2(300, ImGui::GetIO().DisplaySize.y));
    }
    // 选中节点的详细信息
    ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_NoMove);
    DrawInspector(m_selectedid);
    ImGui::End();
    inited = true;
}

void SceneHierarchy::DrawSceneNode(const UID rootid, UID &selectedid) {
    TransformC *root = halgame->GetScene()->GetComponent<TransformC>(rootid);
    if (root == nullptr) {
        return;
    }
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
    if (rootid == selectedid) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }
    std::vector<TransformC *> childrens = root->GetChilds();
    if (childrens.empty()) {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }
    GameObject *go = root->GetGameObject();
    std::string m_Name = go->GetName() + " (" + std::to_string(go->GetUID()) + ")";
    bool nodeOpen = ImGui::TreeNodeEx(reinterpret_cast<void *>(static_cast<uintptr_t>(rootid)), flags, m_Name.c_str());
    if (ImGui::IsItemClicked()) {
        selectedid = rootid;
    }
    if (nodeOpen) {
        for (TransformC *child: childrens) {
            DrawSceneNode(child->GetUID(), selectedid);
        }
        ImGui::TreePop();
    }
}

void SceneHierarchy::DrawInspector(UID selectedid) {
    if (selectedid == -1) {
        ImGui::Text("No Object Selected");
        return;
    }

    TransformC* transform_c = halgame->GetScene()->GetComponent<TransformC>(selectedid);
    if (!transform_c) {
        ImGui::Text("GameObject not found.");
        return;
    }

    GameObject *go = transform_c->GetGameObject();
    ImGui::Text("名前 : %s (%d)", go->GetName().c_str(), go->GetUID());
    if (ImGui::CollapsingHeader(go->GetName().c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
        go->OnInspectorGUI();
    }
    const std::unordered_map<std::string, UID>& mymap = go->GetComponentMap();
    for (const auto& [name, uid] : mymap) {
        if (ImGui::CollapsingHeader(name.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
            go->GetComponent<Component>(name)->OnInspectorGUI();
        }
    }
}
