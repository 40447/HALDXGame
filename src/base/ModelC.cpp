//
// Created by eva00 on 25-5-22.
//

#include "ModelC.h"

#include "AnimatorC.h"
#include "Global.h"
#include "MixamorigBoneC.h"
#include "SimpleMath.h"
#include "TransformC.h"

ModelC::ModelC(std::string_view filename) {
    handle = halgame->AddModel(filename);
    ModelObject *modelObject = halgame->GetModelObject(handle);
    modelObject->SetVisible(false);
}

void ModelC::Init() {
    Component::Init();
}

void ModelC::Update(float dt) {
    Component::Update(dt);
    // 毎フレームモデルのTransformをチェックして更新します
    CheckTransform();
}

void ModelC::Uninit() {
    Component::Uninit();
    // ハンドルに基づいてモデルを削除します
    halgame->RemoveModel(handle);
}

ModelObject *ModelC::GetModelObject() const {
    return halgame->GetModelObject(handle);
}

void ModelC::OnInspectorGUI() {
    Component::OnInspectorGUI();
    if (ImGui::Checkbox("Visible", &visible)) {
        ModelObject *modelObject = halgame->GetModelObject(handle);
        modelObject->SetVisible(visible);
    }
}

void ModelC::CheckTransform() {
    if (handle == -1) {
        return;
    }
    ModelObject *modelObject = halgame->GetModelObject(handle);
    Transform &modelTransform = modelObject->GetTransform();
    TransformC *transformComponent = GetComponent<TransformC>();
    if (transformComponent == nullptr) {
        return;
    }
    if (HasComponent<MixamorigBoneC>()) {
        auto &ragdollC = GetComponentRef<MixamorigBoneC>();
        modelObject->SetBoneMatrices(&ragdollC.GetFinalBoneMatrices());
    }
    modelTransform.SetWorldMatrix(transformComponent->GetWorldMatrix());
    modelObject->SetVisible(visible);
}
