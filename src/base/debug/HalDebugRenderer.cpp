#include "HalDebugRenderer.h"

#include "Global.h"

using namespace DirectX;

// 用于绘线
void HalDebugRenderer::DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor) {
    using DirectX::XMFLOAT3;
    using DirectX::XMFLOAT4;
    XMFLOAT3 start{static_cast<float>(inFrom.GetX()), static_cast<float>(inFrom.GetY()), static_cast<float>(inFrom.GetZ())};
    XMFLOAT3 end{static_cast<float>(inTo.GetX()), static_cast<float>(inTo.GetY()), static_cast<float>(inTo.GetZ())};
    XMFLOAT4 color{inColor.r / 255.0f, inColor.g / 255.0f, inColor.b / 255.0f, inColor.a / 255.0f};
    halgame->DrawLine(start, end, color);
}

void HalDebugRenderer::DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3, JPH::ColorArg inColor, ECastShadow inCastShadow)
{
    using DirectX::XMFLOAT3;
    using DirectX::XMFLOAT4;
    XMFLOAT3 v1{static_cast<float>(inV1.GetX()), static_cast<float>(inV1.GetY()), static_cast<float>(inV1.GetZ())};
    XMFLOAT3 v2{static_cast<float>(inV2.GetX()), static_cast<float>(inV2.GetY()), static_cast<float>(inV2.GetZ())};
    XMFLOAT3 v3{static_cast<float>(inV3.GetX()), static_cast<float>(inV3.GetY()), static_cast<float>(inV3.GetZ())};
    XMFLOAT4 color{inColor.r / 255.0f, inColor.g / 255.0f, inColor.b / 255.0f, inColor.a / 255.0f};
    halgame->DrawTriangle(v1, v2, v3, color);
}


void HalDebugRenderer::DrawText3D(JPH::RVec3Arg inPosition, const std::string_view &inString, JPH::ColorArg inColor, float inHeight) {

}