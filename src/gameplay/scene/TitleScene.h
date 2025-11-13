//
// Created by eva00 on 25-9-10.
//
#pragma once
#include "Scene.h"
#include "ImageC.h"
class TitleScene : public Scene {

public:

    void Init() override;
    void Update(float dt) override;
    void Uninit() override;
private:
    ImageC* enterImage = nullptr;
    float animTime = 0.0f;
    float yPos = 0.0f;
    float speed = 50.0f;
    int direction = 1;
};
