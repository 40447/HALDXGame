#include "Effects.h"
#include <XUtil.h>
#include <RenderStates.h>
#include <EffectHelper.h>
#include <DXTrace.h>
#include <filesystem>
#include <algorithm>
#include <Vertex.h>
#include <TextureManager.h>
#include "LightHelper.h"

using namespace DirectX;

static constexpr size_t MAX_BONES = 256;

# pragma warning(disable: 26812)

//
// BasicEffect::Impl は BasicEffect の定義よりも先に必要
//
class BaseEffect::Impl
{
public:
    // 明示的に指定が必要
    Impl() = default;
    ~Impl() = default;

public:
    template<class T>
    using ComPtr = Microsoft::WRL::ComPtr<T>;

    std::unique_ptr<EffectHelper> m_pEffectHelper;

    ComPtr<ID3D11InputLayout> m_pVertexPosNormalTexLayout;
    ComPtr<ID3D11InputLayout> m_pVertexPosNormalTexSkinnedLayout;

    std::shared_ptr<IEffectPass> m_pCurrEffectPass;
    ComPtr<ID3D11InputLayout> m_pCurrInputLayout;
    D3D11_PRIMITIVE_TOPOLOGY m_CurrTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    XMFLOAT4X4 m_World{}, m_View{}, m_Proj{};
};

//
// BasicEffect
//
namespace
{
    // BasicEffect シングルトンインスタンス
    BaseEffect* g_pInstance = nullptr;
}

BaseEffect::BaseEffect()
{
    if (g_pInstance)
        throw std::exception("BasicEffect is a singleton!");
    g_pInstance = this;
    pImpl = std::make_unique<BaseEffect::Impl>();
}

BaseEffect::~BaseEffect() = default;

BaseEffect::BaseEffect(BaseEffect&& moveFrom) noexcept
{
    pImpl.swap(moveFrom.pImpl);
}

BaseEffect& BaseEffect::operator=(BaseEffect&& moveFrom) noexcept
{
    pImpl.swap(moveFrom.pImpl);
    return *this;
}

BaseEffect& BaseEffect::Get()
{
    if (!g_pInstance)
        throw std::exception("BasicEffect needs an instance!");
    return *g_pInstance;
}

bool BaseEffect::InitAll(ID3D11Device* device)
{
    if (!device)
        return false;

    if (!RenderStates::IsInit())
        throw std::exception("RenderStates need to be initialized first!");

    pImpl->m_pEffectHelper = std::make_unique<EffectHelper>();

    Microsoft::WRL::ComPtr<ID3DBlob> blob;

    // 頂点シェーダーの作成
    std::filesystem::path exePath = std::filesystem::current_path();
    pImpl->m_pEffectHelper->CreateShaderFromFile("BasicVS", L"shaders/Basic_VS.cso", device,
        nullptr, nullptr, nullptr, blob.GetAddressOf());
    
    // 頂点レイアウトの作成
    HR(device->CreateInputLayout(VertexPosNormalTex::GetInputLayout(), ARRAYSIZE(VertexPosNormalTex::GetInputLayout()),
        blob->GetBufferPointer(), blob->GetBufferSize(), pImpl->m_pVertexPosNormalTexLayout.GetAddressOf()));

    // スキニング用頂点シェーダー
    pImpl->m_pEffectHelper->CreateShaderFromFile("SkinnedVS", L"shaders/Skinned_VS.cso", device,
        nullptr, nullptr, nullptr, blob.ReleaseAndGetAddressOf());
    HR(device->CreateInputLayout(VertexPosNormalTexSkinned::GetInputLayout(), ARRAYSIZE(VertexPosNormalTexSkinned::GetInputLayout()),
        blob->GetBufferPointer(), blob->GetBufferSize(), pImpl->m_pVertexPosNormalTexSkinnedLayout.GetAddressOf()));

    // ピクセルシェーダーの作成
    pImpl->m_pEffectHelper->CreateShaderFromFile("BasicPS", L"shaders/Basic_PS.cso", device);

    // エフェクトパスの作成
    EffectPassDesc passDesc;
    passDesc.nameVS = "BasicVS";
    passDesc.namePS = "BasicPS";
    HR(pImpl->m_pEffectHelper->AddEffectPass("Basic", device, &passDesc));

    passDesc.nameVS = "SkinnedVS";
    passDesc.namePS = "BasicPS";
    HR(pImpl->m_pEffectHelper->AddEffectPass("Skinned", device, &passDesc));

    // サンプラーステートの設定
    pImpl->m_pEffectHelper->SetSamplerStateByName("g_Sam", RenderStates::SSLinearWrap.Get());

    // デバッグ用オブジェクト名の設定
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
    SetDebugObjectName(pImpl->m_pVertexPosNormalTexLayout.Get(), "BasicEffect.VertexPosNormalTexLayout");
    SetDebugObjectName(pImpl->m_pVertexPosNormalTexSkinnedLayout.Get(), "BasicEffect.VertexPosNormalTexSkinnedLayout");
#endif
    pImpl->m_pEffectHelper->SetDebugObjectName("BasicEffect");

    return true;
}

void XM_CALLCONV BaseEffect::SetWorldMatrix(DirectX::FXMMATRIX W)
{
    XMStoreFloat4x4(&pImpl->m_World, W);
}

void XM_CALLCONV BaseEffect::SetViewMatrix(DirectX::FXMMATRIX V)
{
    XMStoreFloat4x4(&pImpl->m_View, V);
}

void XM_CALLCONV BaseEffect::SetProjMatrix(DirectX::FXMMATRIX P)
{
    XMStoreFloat4x4(&pImpl->m_Proj, P);
}

void BaseEffect::SetMaterial(const Material& material)
{
    TextureManager& tm = TextureManager::Get();

    PhongMaterial phongMat{};
    phongMat.ambient = material.Get<XMFLOAT4>("$AmbientColor");
    phongMat.diffuse = material.Get<XMFLOAT4>("$DiffuseColor");
    phongMat.diffuse.w = material.Get<float>("$Opacity");
    phongMat.specular = material.Get<XMFLOAT4>("$SpecularColor");
    phongMat.specular.w = material.Has<float>("$SpecularFactor") ? material.Get<float>("$SpecularFactor") : 1.0f;
    pImpl->m_pEffectHelper->GetConstantBufferVariable("g_Material")->SetRaw(&phongMat);

    auto pStr = material.TryGet<std::string>("$Diffuse");
    pImpl->m_pEffectHelper->SetShaderResourceByName("g_DiffuseMap", pStr ? tm.GetTexture(*pStr) : tm.GetNullTexture());
}

MeshDataInput BaseEffect::GetInputData(const MeshData& meshData)
{
    MeshDataInput input;
    input.pInputLayout = pImpl->m_pCurrInputLayout.Get();
    input.topology = pImpl->m_CurrTopology;

    if (pImpl->m_pCurrInputLayout.Get() == pImpl->m_pVertexPosNormalTexSkinnedLayout.Get())
    {
        input.pVertexBuffers = {
            meshData.m_pVertices.Get(),
            meshData.m_pNormals.Get(),
            meshData.m_pTexcoordArrays.empty() ? nullptr : meshData.m_pTexcoordArrays[0].Get(),
            meshData.m_pBlendIndices.Get(),
            meshData.m_pBlendWeights.Get()
        };
        input.strides = { 12, 12, 8, 16, 16 };
        input.offsets = { 0, 0, 0, 0, 0 };
    }
    else
    {
        input.pVertexBuffers = {
            meshData.m_pVertices.Get(),
            meshData.m_pNormals.Get(),
            meshData.m_pTexcoordArrays.empty() ? nullptr : meshData.m_pTexcoordArrays[0].Get()
        };
        input.strides = { 12, 12, 8 };
        input.offsets = { 0, 0, 0 };
    }

    input.pIndexBuffer = meshData.m_pIndices.Get();
    input.indexCount = meshData.m_IndexCount;

    return input;
}

void BaseEffect::SetDirLight(uint32_t pos, const DirectionalLight& dirLight)
{
    // 指定位置にディレクショナルライトを設定
    pImpl->m_pEffectHelper->GetConstantBufferVariable("g_DirLight")->SetRaw(&dirLight, (sizeof dirLight) * pos, sizeof dirLight);
}

void BaseEffect::SetPointLight(uint32_t pos, const PointLight& pointLight)
{
    // 指定位置にポイントライトを設定
    pImpl->m_pEffectHelper->GetConstantBufferVariable("g_PointLight")->SetRaw(&pointLight, (sizeof pointLight) * pos, sizeof pointLight);
}

void BaseEffect::SetSpotLight(uint32_t pos, const SpotLight& spotLight)
{
    // 指定位置にスポットライトを設定
    pImpl->m_pEffectHelper->GetConstantBufferVariable("g_SpotLight")->SetRaw(&spotLight, (sizeof spotLight) * pos, sizeof spotLight);
}

void BaseEffect::SetEyePos(const DirectX::XMFLOAT3& eyePos)
{
    // 視点位置（カメラ）をシェーダーに渡す
    pImpl->m_pEffectHelper->GetConstantBufferVariable("g_EyePosW")->SetFloatVector(3, reinterpret_cast<const float*>(&eyePos));
}

void BaseEffect::SetRenderDefault()
{
    // デフォルトの描画パスを選択
    pImpl->m_pCurrEffectPass = pImpl->m_pEffectHelper->GetEffectPass("Basic");
    pImpl->m_pCurrInputLayout = pImpl->m_pVertexPosNormalTexLayout;
    pImpl->m_CurrTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
}

void BaseEffect::SetRenderSkinned()
{
    pImpl->m_pCurrEffectPass = pImpl->m_pEffectHelper->GetEffectPass("Skinned");
    pImpl->m_pCurrInputLayout = pImpl->m_pVertexPosNormalTexSkinnedLayout;
    pImpl->m_CurrTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
}

void BaseEffect::SetBoneMatrices(const std::vector<DirectX::XMFLOAT4X4>& boneMatrices)
{
    if (!boneMatrices.empty())
    {
        size_t count = std::min<size_t>(boneMatrices.size(), MAX_BONES);
        std::vector<DirectX::XMFLOAT4X4> transposed(count);
        for (size_t i = 0; i < count; ++i)
        {
            DirectX::XMMATRIX m = DirectX::XMLoadFloat4x4(&boneMatrices[i]);
            m = DirectX::XMMatrixTranspose(m);
            DirectX::XMStoreFloat4x4(&transposed[i], m);
        }
        pImpl->m_pEffectHelper->GetConstantBufferVariable("g_BoneTransforms")
            ->SetRaw(transposed.data(), 0, sizeof(DirectX::XMFLOAT4X4) * count);
    }
}

void BaseEffect::Apply(ID3D11DeviceContext* deviceContext)
{
    // 各種行列の読み込みとトランスポーズ
    XMMATRIX W = XMLoadFloat4x4(&pImpl->m_World);
    XMMATRIX V = XMLoadFloat4x4(&pImpl->m_View);
    XMMATRIX P = XMLoadFloat4x4(&pImpl->m_Proj);

    XMMATRIX VP = V * P;
    XMMATRIX WInvT = XMath::InverseTranspose(W);

    W = XMMatrixTranspose(W);
    VP = XMMatrixTranspose(VP);
    WInvT = XMMatrixTranspose(WInvT);

    // 定数バッファへの行列設定
    pImpl->m_pEffectHelper->GetConstantBufferVariable("g_WorldInvTranspose")->SetFloatMatrix(4, 4, (FLOAT*)&WInvT);
    pImpl->m_pEffectHelper->GetConstantBufferVariable("g_ViewProj")->SetFloatMatrix(4, 4, (FLOAT*)&VP);
    pImpl->m_pEffectHelper->GetConstantBufferVariable("g_World")->SetFloatMatrix(4, 4, (FLOAT*)&W);

    // パスの適用
    if (pImpl->m_pCurrEffectPass)
        pImpl->m_pCurrEffectPass->Apply(deviceContext);
}
