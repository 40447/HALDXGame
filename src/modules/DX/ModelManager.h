//***************************************************************************************
// 简易游戏对象
// Simple game object.
//***************************************************************************************

#pragma once

#include "WinMin.h"
#include "Geometry.h"
#include "Material.h"
#include "MeshData.h"
#include <d3d11_1.h>
#include <wrl/client.h>

// Bone 信息
struct BoneInfo
{
    std::string name;
    DirectX::XMFLOAT4X4 offsetMatrix; // aiBone::mOffsetMatrix
    DirectX::XMFLOAT4X4 nodeTransform{ 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 }; // default transform
    int parentIndex = -1;
    std::vector<int> children; // 子骨骼索引
};

struct Model
{
    Model() = default;
    ~Model() = default;
    Model(Model&) = delete;
    Model& operator=(const Model&) = delete;
    Model(Model&&) = default;
    Model& operator=(Model&&) = default;

    std::vector<Material> materials;
    std::vector<MeshData> meshdatas;
    DirectX::BoundingBox boundingbox;
    // 骨骼信息
    std::vector<BoneInfo> bones;
    std::unordered_map<std::string, int> boneNameToIndex;
    static void CreateFromFile(Model& model, ID3D11Device* device, std::string_view filename);
    static void CreateFromGeometry(Model& model, ID3D11Device* device, const GeometryData& data, bool isDynamic = false);
    
    void SetDebugObjectName(std::string_view name);
};

class ModelManager
{
public:
    ModelManager();
    ~ModelManager();
    ModelManager(ModelManager&) = delete;
    ModelManager& operator=(const ModelManager&) = delete;
    ModelManager(ModelManager&&) = default;
    ModelManager& operator=(ModelManager&&) = default;

    static ModelManager& Get();
    void Init(ID3D11Device* device);
    Model* CreateFromFile(std::string_view filename);
    Model* CreateFromFile(std::string_view name, std::string_view filename);
    Model* CreateFromGeometry(std::string_view name, const GeometryData& data, bool isDynamic = false);

    const Model* GetModel(std::string_view name) const;
    Model* GetModel(std::string_view name);
private:
    Microsoft::WRL::ComPtr<ID3D11Device> m_pDevice;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_pDeviceContext;
    std::unordered_map<size_t, Model> m_Models;
};
