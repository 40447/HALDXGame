//
// Created by eva00 on 25-9-10.
//

#include "TitleScene.h"

#include "TransformC.h"
#include "GamingScene.h"

void TitleScene::Init() {

    auto &tttt01 = AddGameObject<GameObject>();
    tttt01.AddComponent<ImageC>("assets/models/text/tttt01.png");
    ImageC *image = tttt01.GetComponent<ImageC>();
    image->SetSize(DirectX::XMFLOAT2(1280, 720));
    image->SetPosition(DirectX::XMFLOAT2(0, 0));

    auto &tttt02 = AddGameObject<GameObject>();
    tttt02.AddComponent<ImageC>("assets/models/text/0001.png");
    ImageC *image2 = tttt02.GetComponent<ImageC>();
    image2->SetSize(DirectX::XMFLOAT2(1280, 720));
    image2->SetPosition(DirectX::XMFLOAT2(0, 0));

    auto &enter = AddGameObject<GameObject>();
    enter.AddComponent<ImageC>("assets/models/text/ENTER00.png");
    enterImage = enter.GetComponent<ImageC>();
    enterImage->SetSize(DirectX::XMFLOAT2(1280, 720));
    enterImage->SetPosition(DirectX::XMFLOAT2(0, 0));


}

void TitleScene::Update(float dt) {

    animTime += 0.00016;
    float yPos = sinf(animTime * 2.0f) * 20.0f;
    enterImage->SetPosition({0, 200 + yPos});

    if (ImGui::IsKeyDown(ImGuiKey_Enter)) {
        halgame->SetScene<GamingScene>();
    }

}

void TitleScene::Uninit() {

}
