//
// Created by eva00 on 2025/11/11.
//

#pragma once

#include "GameObject.h"

class ImageC;

class HPUI : public GameObject {
private:
    ImageC *HPImageC = nullptr;
    ImageC *HPBGImageC = nullptr;
    DirectX::SimpleMath::Vector2 Position = {380.0f, 5.0f};
    DirectX::SimpleMath::Vector2 Size = {520.0f, 30.0f};
    float Padding = 3.0f;
    float HPMaxWidth = Size.x - Padding * 2.0f;

public:
    HPUI();

    ~HPUI() override;

    void Init() override;

    void Update(float dt) override;

    void Uninit() override;
    void SetSize(const DirectX::SimpleMath::Vector2 &inSize);
    void SetPosition(const DirectX::SimpleMath::Vector2 &inPosition);

    void SetHPPercent(float percent);
};
