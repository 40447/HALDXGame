//
// Created by eva00 on 25-6-10.
//

#include "BoneObj.h"

#include "TransformC.h"

BoneObj::BoneObj() {
}

BoneObj::~BoneObj() {
}

void BoneObj::Init() {
    GameObject::Init();
}

void BoneObj::Update(float dt) {
    GameObject::Update(dt);
    if (isShowDebug) {
        TransformC &transform = GetComponentRef<TransformC>();
        TransformC *parentTransform = transform.GetParent();
        if (parentTransform != nullptr) {
            // 親TransformCが存在する場合、親のワールド位置を取得
            DirectX::SimpleMath::Vector3 parentPosition = parentTransform->GetWorldPosition();
            auto p1 = halgame->ProjectToScreen(transform.GetWorldPosition());
            auto p2 = halgame->ProjectToScreen(parentPosition);
            ImGui::GetBackgroundDrawList()->AddLine(p1, p2, IM_COL32(255, 0, 0, 255));
        }
    }
}

void BoneObj::Uninit() {
    GameObject::Uninit();
}
