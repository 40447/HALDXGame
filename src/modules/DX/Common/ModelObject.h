//***************************************************************************************
// 简易游戏对象
// Simple game object.
//***************************************************************************************

#pragma once

#include "Geometry.h"
#include "Material.h"
#include "MeshData.h"
#include "Transform.h"
#include "IEffect.h"

struct Model;

class ModelObject
{
public:
    template <class T>
    using ComPtr = Microsoft::WRL::ComPtr<T>;


    ModelObject() = default;
    ~ModelObject() = default;

    ModelObject(const ModelObject&) = default;
    ModelObject& operator=(const ModelObject&) = default;

    ModelObject(ModelObject&&) = default;
    ModelObject& operator=(ModelObject&&) = default;

    // 获取物体变换
    Transform& GetTransform();
    // 获取物体变换
    const Transform& GetTransform() const;

    //
    // 相交检测
    //
    void FrustumCulling(const DirectX::BoundingFrustum& frustumInWorld);
    void CubeCulling(const DirectX::BoundingOrientedBox& obbInWorld);
    void CubeCulling(const DirectX::BoundingBox& aabbInWorld);
    bool InFrustum() const { return m_InFrustum; }

    //
    // 模型
    //
    void SetModel(const Model* pModel);
    const Model* GetModel() const;

    DirectX::BoundingBox GetLocalBoundingBox() const;
    DirectX::BoundingBox GetLocalBoundingBox(size_t idx) const;
    DirectX::BoundingBox GetBoundingBox() const;
    DirectX::BoundingBox GetBoundingBox(size_t idx) const;
    DirectX::BoundingOrientedBox GetBoundingOrientedBox() const;
    DirectX::BoundingOrientedBox GetBoundingOrientedBox(size_t idx) const;
    //
    // 绘制
    //

    void SetVisible(bool visible) {
        m_InFrustum = visible;
        m_SubModelInFrustum.assign(m_SubModelInFrustum.size(), true);
    }

    // 绘制对象
    void Draw(ID3D11DeviceContext* deviceContext, IEffect& effect);

    void SetBoneMatrices(const std::vector<DirectX::XMFLOAT4X4>* bones);
    const std::vector<DirectX::XMFLOAT4X4>* GetBoneMatrices() const { return m_pBoneMatrices; }

protected:
    const Model* m_pModel = nullptr;
    std::vector<bool> m_SubModelInFrustum;
    Transform m_Transform = {};
    bool m_InFrustum = true;
    const std::vector<DirectX::XMFLOAT4X4>* m_pBoneMatrices = nullptr;
};
