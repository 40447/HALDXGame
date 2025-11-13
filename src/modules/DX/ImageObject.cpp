#include "ImageObject.h"
#include <wrl/client.h>

using namespace DirectX;

void ImageObject::SetTexture(ID3D11ShaderResourceView *texture) {
    m_pTexture = texture;
    if (m_pTexture) {
        Microsoft::WRL::ComPtr<ID3D11Resource> res;
        m_pTexture->GetResource(res.GetAddressOf());
        Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;
        res.As(&tex);
        if (tex) {
            D3D11_TEXTURE2D_DESC desc{};
            tex->GetDesc(&desc);
            m_Size = XMFLOAT2(static_cast<float>(desc.Width), static_cast<float>(desc.Height));
        }
    }
}

void ImageObject::Draw(DX11::SpriteBatch *spriteBatch) const {
    if (!spriteBatch || !m_pTexture) {
        return;
    }
    if (visible == false) {
        return;
    }
    RECT dest;
    dest.left = static_cast<LONG>(m_Position.x);
    dest.top = static_cast<LONG>(m_Position.y);
    dest.right = static_cast<LONG>(m_Position.x + m_Size.x);
    dest.bottom = static_cast<LONG>(m_Position.y + m_Size.y);
    spriteBatch->Draw(m_pTexture, dest);
}

void ImageObject::SetVisible(bool v) {
    visible = v;
}
