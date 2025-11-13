#pragma once

#include <d3d11_1.h>
#include <DirectXMath.h>

#include "SpriteBatch.h"

class ImageObject {
private:
    ID3D11ShaderResourceView* m_pTexture = nullptr;
    DirectX::XMFLOAT2 m_Position = {0.0f, 0.0f};
    DirectX::XMFLOAT2 m_Size = {0.0f, 0.0f};
    bool visible = true;
public:
    void SetTexture(ID3D11ShaderResourceView* texture);
    void SetPosition(const DirectX::XMFLOAT2& pos) { m_Position = pos; }
    void SetSize(const DirectX::XMFLOAT2& size) { m_Size = size; }
    const DirectX::XMFLOAT2& GetPosition() const { return m_Position; }
    const DirectX::XMFLOAT2& GetSize() const { return m_Size; }
    void Draw(DirectX::DX11::SpriteBatch* spriteBatch) const;
    void SetVisible(bool v);
};

