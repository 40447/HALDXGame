//
// Created by eva00 on 25-6-10.
//

#pragma once
#include "GameObject.h"

class BoneObj :public GameObject{
    int m_boneIndex = -2; // 骨骼索引
public:
    bool isShowDebug = false; // デバッグ表示フラグ
    BoneObj();
    ~BoneObj() override;

    void SetBoneIndex(int index) { m_boneIndex = index; }
    int GetBoneIndex() const { return m_boneIndex; }

    void Init() override;
    void Update(float dt) override;
    void Uninit() override;
};
