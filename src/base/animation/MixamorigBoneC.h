//
// Created by eva00 on 25-6-10.
//

#pragma once
#include <DirectXMath.h>

#include "BoneObj.h"
#include "Component.h"

class MixamorigBoneC: public Component {
private:
    std::vector<DirectX::XMFLOAT4X4> m_finalBoneMatrices;
    BoneObj *m_hipsBone = nullptr;
    std::unordered_map<std::string, UID> m_boneNameToUID; // 骨骼名称到UID的映射
    std::vector<std::string> m_boneNames; // 骨骼名称列表
    bool isShowDebug = false; // デバッグ表示フラグ
public:
    MixamorigBoneC();
    ~MixamorigBoneC() override;

    void Init() override;
    void Uninit() override;
    void Update(float dt) override;

    std::vector<DirectX::XMFLOAT4X4>& GetFinalBoneMatrices();
    const BoneObj* GetBone(const std::string &boneName) const;

    const std::vector<std::string> *GetBoneNames() const;

    virtual void OnInspectorGUI() override;
};
