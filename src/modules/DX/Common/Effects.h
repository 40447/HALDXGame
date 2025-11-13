//***************************************************************************************
// シンプルなエフェクト管理フレームワーク
// Simple effect management framework.
//***************************************************************************************

#pragma once

#include <IEffect.h>
#include <Material.h>
#include <MeshData.h>
#include <LightHelper.h>
#include <vector>

class BaseEffect : public IEffect, public IEffectTransform,
    public IEffectMaterial, public IEffectMeshData
{
public:
    BaseEffect();
    ~BaseEffect() override;

    BaseEffect(BaseEffect&& moveFrom) noexcept;
    BaseEffect& operator=(BaseEffect&& moveFrom) noexcept;

    // シングルトンの取得
    static BaseEffect& Get();

    // 必要なリソースの初期化
    bool InitAll(ID3D11Device* device);

    //
    // IEffectTransform インターフェースの実装
    //
    void XM_CALLCONV SetWorldMatrix(DirectX::FXMMATRIX W) override;
    void XM_CALLCONV SetViewMatrix(DirectX::FXMMATRIX V) override;
    void XM_CALLCONV SetProjMatrix(DirectX::FXMMATRIX P) override;

    //
    // IEffectMaterial インターフェースの実装
    //
    void SetMaterial(const Material& material) override;

    //
    // IEffectMeshData インターフェースの実装
    //
    MeshDataInput GetInputData(const MeshData& meshData) override;

    //
    // BasicEffect 独自の機能
    //

    // デフォルトの描画状態を設定
    void SetRenderDefault();
    void SetRenderSkinned();
    void SetBoneMatrices(const std::vector<DirectX::XMFLOAT4X4>& boneMatrices);

    // 各ライトタイプの最大数
    static constexpr int maxLights = 5;

    void SetDirLight(uint32_t pos, const DirectionalLight& dirLight);    // 平行光源の設定
    void SetPointLight(uint32_t pos, const PointLight& pointLight);      // 点光源の設定
    void SetSpotLight(uint32_t pos, const SpotLight& spotLight);         // スポットライトの設定

    void SetEyePos(const DirectX::XMFLOAT3& eyePos);                     // カメラ（視点）位置の設定

    // 定数バッファおよびテクスチャリソースの変更を適用
    void Apply(ID3D11DeviceContext* deviceContext) override;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl; // 実装の隠蔽（Pimplイディオム）
};
