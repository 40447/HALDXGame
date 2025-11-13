//
// Created by eva00 on 2025/11/11.
//

#include "HPUI.h"
#include "TransformC.h"
#include "ImageC.h"

HPUI::HPUI() = default;

HPUI::~HPUI() = default;

void HPUI::Init() {
    GameObject::Init();
    GameObject &HP = halgame->GetScene()->AddGameObject<GameObject>();
    GameObject &HPBG = halgame->GetScene()->AddGameObject<GameObject>();
    HPImageC = &HP.AddComponent<ImageC>("assets/Image/HP.png");
    HPBGImageC = &HPBG.AddComponent<ImageC>("assets/Image/HPBG.png");

    TransformC *HPTransform = HP.GetComponent<TransformC>();
    TransformC *HPBGTransform = HPBG.GetComponent<TransformC>();
    GetComponentRef<TransformC>().AddChild(HPTransform);
    GetComponentRef<TransformC>().AddChild(HPBGTransform);
    SetSize(Size);
    SetPosition(Position);
}

void HPUI::Update(float dt) {
    GameObject::Update(dt);
}

void HPUI::Uninit() {
    GameObject::Uninit();
}

void HPUI::SetSize(const DirectX::SimpleMath::Vector2 &inSize) {
    Size = inSize;
    HPBGImageC->SetSize(Size);
    HPMaxWidth = Size.x - Padding * 2.0f;
    HPImageC->SetSize({HPMaxWidth, Size.y - Padding * 2.0f});
}

void HPUI::SetPosition(const DirectX::SimpleMath::Vector2 &inPosition) {
    Position = inPosition;
    HPBGImageC->SetPosition(Position);
    HPImageC->SetPosition({Position.x + Padding, Position.y + Padding});
}

void HPUI::SetHPPercent(float percent) {
    percent = std::clamp(percent, 0.0f, 1.0f);
    HPImageC->SetSize({HPMaxWidth * percent, Size.y - Padding * 2.0f});
}

