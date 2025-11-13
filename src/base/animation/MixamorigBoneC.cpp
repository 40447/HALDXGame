//
// Created by eva00 on 25-6-10.
//
#include "MixamorigBoneC.h"

#include <set>
#include "Global.h"
#include "ModelC.h"
#include "../../modules/DX/ModelManager.h"
#include "TransformC.h"

MixamorigBoneC::MixamorigBoneC() {
}

MixamorigBoneC::~MixamorigBoneC() {
}

void MixamorigBoneC::Init() {
    Component::Init();
    ModelC &modelC = GetComponentRef<ModelC>();
    ModelObject *modelObject = halgame->GetModelObject(modelC.GetHandle());
    const Model *model = modelObject->GetModel();
    auto it = model->boneNameToIndex.find("mixamorig:Hips");
    if (it == model->boneNameToIndex.end()) {
        throw std::runtime_error(" 'mixamorig:Hips' 骨骼が見つかりません。");
    }

    int hipsIndex = it->second;
    const BoneInfo &hipsBone = model->bones[hipsIndex];
    std::function<void (const BoneInfo &boneInfo, const GameObject &gameObject, std::set<int> &visited)> addBone = [&
            ](const BoneInfo &boneInfo, const GameObject &gameObject, std::set<int> &visited) {
        std::string boneName = boneInfo.name;
        TransformC &transform_c = gameObject.GetComponentRef<TransformC>();
        // 新しいBoneObjを作成
        BoneObj &boneObj = halgame->GetScene()->AddGameObject<BoneObj>();
        if (m_hipsBone == nullptr) {
            m_hipsBone = &boneObj; // 最初の骨骼を保存
        }
        boneObj.SetName(boneName);
        TransformC &boneTransform = boneObj.GetComponentRef<TransformC>();
        transform_c.AddChild(&boneTransform);
        // 骨骼のUIDを保存
        m_boneNameToUID[boneName] = boneObj.GetUID();
        m_boneNames.push_back(boneName);
        // 骨骼のnodeTransform から位置、回転、スケールを抽出
        DirectX::XMMATRIX mat = DirectX::XMLoadFloat4x4(&boneInfo.nodeTransform);
        DirectX::XMVECTOR scaleVec, rotQuat, transVec;
        DirectX::XMMatrixDecompose(&scaleVec, &rotQuat, &transVec, mat);
        DirectX::XMFLOAT3 scale, position;
        DirectX::XMFLOAT4 rotation;
        DirectX::XMStoreFloat3(&scale, scaleVec);
        DirectX::XMStoreFloat3(&position, transVec);
        DirectX::XMStoreFloat4(&rotation, rotQuat);
        // スケール、位置、回転を設定
        boneTransform.SetLocalPosition(position);
        boneTransform.SetLocalRotation(DirectX::SimpleMath::Quaternion(rotation.x, rotation.y, rotation.z, rotation.w));
        boneTransform.SetLocalScale(DirectX::SimpleMath::Vector3(scale.x, scale.y, scale.z));

        //  子骨骼を再帰的に追加
        for (int index: boneInfo.children) {
            if (visited.find(index) != visited.end()) {
                continue; // 既に訪問済みの骨骼はスキップ
            }
            visited.insert(index);
            addBone(model->bones[index], boneObj, visited);
            visited.erase(index);
        }
    };
    std::set<int> visited;
    visited.insert(hipsIndex);
    addBone(hipsBone, *m_gameObject, visited);

    DirectX::XMFLOAT4X4 identity;
    DirectX::XMStoreFloat4x4(&identity, DirectX::XMMatrixIdentity());
    m_finalBoneMatrices.assign(model->bones.size(), identity);
}

void MixamorigBoneC::Update(float dt) {
    Component::Update(dt);
    const Model *model = halgame->GetModelObject(GetComponentRef<ModelC>().GetHandle())->GetModel();
    std::function<void (BoneObj *boneObj, DirectX::XMMATRIX parentMat)> updateBone = [&](BoneObj *boneObj, DirectX::XMMATRIX parentMat) {
        boneObj->isShowDebug = isShowDebug;
        std::string boneName = boneObj->GetName();
        int boneIndex = model->boneNameToIndex.at(boneName);
        const BoneInfo &boneInfo = model->bones[boneIndex];
        TransformC &boneTransform = boneObj->GetComponentRef<TransformC>();
        // ローカル行列を取得し、親の行列と組み合わせてグローバル行列を計算
        DirectX::SimpleMath::Matrix global = boneTransform.GetLocalMatrix() * parentMat;
        DirectX::XMMATRIX finalMat = boneInfo.offsetMatrix *  global;
        DirectX::XMStoreFloat4x4(&m_finalBoneMatrices[boneIndex], finalMat);
        boneTransform.ForChilds([&](TransformC *child) {
            updateBone(dynamic_cast<BoneObj *>(child->GetGameObject()), global);
        });
    };
    updateBone(m_hipsBone, DirectX::XMMatrixIdentity());
}

void MixamorigBoneC::Uninit() {
    Component::Uninit();
}

std::vector<DirectX::XMFLOAT4X4> &MixamorigBoneC::GetFinalBoneMatrices() {
    return m_finalBoneMatrices;
}

const BoneObj *MixamorigBoneC::GetBone(const std::string &boneName) const {
    auto it = m_boneNameToUID.find(boneName);
    if (it == m_boneNameToUID.end()) {
        return nullptr; // 骨骼が見つからない場合はnullptrを返す
    }
    UID uid = it->second;
    BoneObj &boneObj = halgame->GetScene()->GetGameObjectRef<BoneObj>(uid);
    return &boneObj;
}

const std::vector<std::string> *MixamorigBoneC::GetBoneNames() const {
    return &m_boneNames;
}

void MixamorigBoneC::OnInspectorGUI() {
    Component::OnInspectorGUI();
    ImGui::Text("Bone Count: %zu", m_boneNames.size());
    ImGui::Checkbox("Show Bone Debug", &isShowDebug);
}
