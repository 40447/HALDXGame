//
// Created by eva00 on 2025/9/10.
//

#include "ImageC.h"
#include "Global.h"

ImageC::ImageC(std::string_view filename) {
    handle = halgame->AddImage(filename);
    ImageObject *imageObject = halgame->GetImageObject(handle);
    imageObject->SetVisible(true);
}

void ImageC::Init() {
    Component::Init();
}

void ImageC::Update(float dt) {
    Component::Update(dt);
}

void ImageC::Uninit() {
    Component::Uninit();
    halgame->RemoveImage(handle);
}

void ImageC::OnInspectorGUI() {
    Component::OnInspectorGUI();
    ImageObject *imageObject = halgame->GetImageObject(handle);
    if (!imageObject) {
        return;
    }
    DirectX::XMFLOAT2 m_Position = imageObject->GetPosition();
    float posArr[2] = { m_Position.x, m_Position.y };
    if (ImGui::DragFloat2("Position", posArr, 0.1f)) {
        SetPosition(DirectX::XMFLOAT2(posArr[0], posArr[1]));
    }
    DirectX::XMFLOAT2 m_Size = imageObject->GetSize();
    float sizeArr[2] = { m_Size.x, m_Size.y };
    if (ImGui::DragFloat2("Size", sizeArr, 0.1f)) {
        SetSize(DirectX::XMFLOAT2(sizeArr[0], sizeArr[1]));
    }
}

void ImageC::SetVisible(bool v) {
    ImageObject *imageObject = halgame->GetImageObject(handle);
    if (imageObject) {
        imageObject->SetVisible(v);
    }
}

void ImageC::SetPosition(const DirectX::XMFLOAT2 &pos) {
    ImageObject *imageObject = halgame->GetImageObject(handle);
    if (imageObject) {
        imageObject->SetPosition(pos);
    }
}

void ImageC::SetSize(const DirectX::XMFLOAT2 &size) {
    ImageObject *imageObject = halgame->GetImageObject(handle);
    if (imageObject) {
        imageObject->SetSize(size);
    }
}
