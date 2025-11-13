
#pragma once
#include "Component.h"
#include <SimpleMath.h>

class ModelObject;

class ImageC: public Component {
private:
    UID handle = -1;
public:
    explicit ImageC(std::string_view filename);
    void Init() override;
    void Update(float dt) override;
    void Uninit() override;
    void OnInspectorGUI() override;
    UID GetHandle() const { return handle; }
    void SetVisible(bool v);
    void SetPosition(const DirectX::XMFLOAT2& pos);
    void SetSize(const DirectX::XMFLOAT2& size);
};
