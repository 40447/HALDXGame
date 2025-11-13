#include "XUtil.h"
#include "ModelManager.h"
#include "TextureManager.h"
#include "ImGuiLog.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <array>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

using namespace DirectX;

void ExportEmbeddedTextures(const aiScene* scene, const std::filesystem::path& modelFilePath)
{
    namespace fs = std::filesystem;
    if (!scene || !scene->HasTextures()) return;

    fs::path fbmDir = modelFilePath.parent_path() / (modelFilePath.stem().string() + ".fbm");
    fs::create_directories(fbmDir);

    for (unsigned int i = 0; i < scene->mNumTextures; ++i)
    {
        const aiTexture* tex = scene->mTextures[i];
        fs::path fileName = fs::path(tex->mFilename.C_Str()).filename();

        // fallback: 如果没有文件名就构造默认名
        if (fileName.empty())
        {
            std::string ext = tex->achFormatHint[0] ? "." + std::string(tex->achFormatHint, 3) : ".png";
            fileName = "embedded_" + std::to_string(i) + ext;
        }

        fs::path outputPath = fbmDir / fileName;

        if (tex->mHeight == 0)
        {
            // PNG/JPG 等压缩格式，直接写二进制数据
            if (!fs::exists(outputPath)) {
                std::ofstream fout(outputPath, std::ios::binary);
                fout.write(reinterpret_cast<const char*>(tex->pcData), tex->mWidth);
                fout.close();
                std::cout << "[Exported] " << outputPath << std::endl;
            }
        }
        else
        {
            std::cerr << "[Warning] Cannot export uncompressed embedded texture to: " << outputPath << std::endl;
        }
    }
}

void Model::CreateFromFile(Model& model, ID3D11Device* device, std::string_view filename)
{
    using namespace Assimp;
    namespace fs = std::filesystem;

    // Clear previous data to avoid duplicated hierarchy when reloading
    model.bones.clear();
    model.boneNameToIndex.clear();

    model.materials.clear();
    model.meshdatas.clear();
    model.boundingbox = BoundingBox();

    Importer importer;
    // 去掉里面的点、线图元
    importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);
    auto pAssimpScene = importer.ReadFile(filename.data(),
        aiProcess_ConvertToLeftHanded |     // 转为左手系
        aiProcess_GenBoundingBoxes |        // 获取碰撞盒
        aiProcess_Triangulate |             // 将多边形拆分
        aiProcess_ImproveCacheLocality |    // 改善缓存局部性
        aiProcess_SortByPType);             // 按图元顶点数排序用于移除非三角形图元

    if (pAssimpScene && !(pAssimpScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) && pAssimpScene->HasMeshes())
    {
        ExportEmbeddedTextures(pAssimpScene, std::filesystem::path(filename));
        model.meshdatas.resize(pAssimpScene->mNumMeshes);
        model.materials.resize(pAssimpScene->mNumMaterials);
        for (uint32_t i = 0; i < pAssimpScene->mNumMeshes; ++i)
        {
            auto& mesh = model.meshdatas[i];

            auto pAiMesh = pAssimpScene->mMeshes[i];
            uint32_t numVertices = pAiMesh->mNumVertices;

            CD3D11_BUFFER_DESC bufferDesc(0, D3D11_BIND_VERTEX_BUFFER);
            D3D11_SUBRESOURCE_DATA initData{ nullptr, 0, 0 };
            // 位置
            if (pAiMesh->mNumVertices > 0)
            {
                initData.pSysMem = pAiMesh->mVertices;
                bufferDesc.ByteWidth = numVertices * sizeof(XMFLOAT3);
                device->CreateBuffer(&bufferDesc, &initData, mesh.m_pVertices.GetAddressOf());

                BoundingBox::CreateFromPoints(mesh.m_BoundingBox, numVertices,
                    (const XMFLOAT3*)pAiMesh->mVertices, sizeof(XMFLOAT3));
                if (i == 0)
                    model.boundingbox = mesh.m_BoundingBox;
                else
                    model.boundingbox.CreateMerged(model.boundingbox, model.boundingbox, mesh.m_BoundingBox);
            }

            // 法线
            if (pAiMesh->HasNormals())
            {
                initData.pSysMem = pAiMesh->mNormals;
                bufferDesc.ByteWidth = numVertices * sizeof(XMFLOAT3);
                device->CreateBuffer(&bufferDesc, &initData, mesh.m_pNormals.GetAddressOf());
            }

            // 切线和副切线
            if (pAiMesh->HasTangentsAndBitangents())
            {
                std::vector<XMFLOAT4> tangents(numVertices, XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
                for (uint32_t i = 0; i < pAiMesh->mNumVertices; ++i)
                {
                    memcpy_s(&tangents[i], sizeof(XMFLOAT3),
                        pAiMesh->mTangents + i, sizeof(XMFLOAT3));
                }

                initData.pSysMem = tangents.data();
                bufferDesc.ByteWidth = pAiMesh->mNumVertices * sizeof(XMFLOAT4);
                device->CreateBuffer(&bufferDesc, &initData, mesh.m_pTangents.GetAddressOf());

                for (uint32_t i = 0; i < pAiMesh->mNumVertices; ++i)
                {
                    memcpy_s(&tangents[i], sizeof(XMFLOAT3),
                        pAiMesh->mBitangents + i, sizeof(XMFLOAT3));
                }
                device->CreateBuffer(&bufferDesc, &initData, mesh.m_pBitangents.GetAddressOf());
            }

            // 纹理坐标
            uint32_t numUVs = 8;
            while (numUVs && !pAiMesh->HasTextureCoords(numUVs - 1))
                numUVs--;

            if (numUVs > 0)
            {
                mesh.m_pTexcoordArrays.resize(numUVs);
                for (uint32_t i = 0; i < numUVs; ++i)
                {
                    std::vector<XMFLOAT2> uvs(numVertices);
                    for (uint32_t j = 0; j < numVertices; ++j)
                    {
                        memcpy_s(&uvs[j], sizeof(XMFLOAT2),
                            pAiMesh->mTextureCoords[i] + j, sizeof(XMFLOAT2));
                    }
                    initData.pSysMem = uvs.data();
                    bufferDesc.ByteWidth = numVertices * sizeof(XMFLOAT2);
                    device->CreateBuffer(&bufferDesc, &initData, mesh.m_pTexcoordArrays[i].GetAddressOf());
                }
            }

            // 索引
            uint32_t numFaces = pAiMesh->mNumFaces;
            uint32_t numIndices = numFaces * 3;
            if (numFaces > 0)
            {
                mesh.m_IndexCount = numIndices;
                if (numIndices < 65535)
                {
                    std::vector<uint16_t> indices(numIndices);
                    for (size_t i = 0; i < numFaces; ++i)
                    {
                        indices[i * 3] = static_cast<uint16_t>(pAiMesh->mFaces[i].mIndices[0]);
                        indices[i * 3 + 1] = static_cast<uint16_t>(pAiMesh->mFaces[i].mIndices[1]);
                        indices[i * 3 + 2] = static_cast<uint16_t>(pAiMesh->mFaces[i].mIndices[2]);
                    }
                    bufferDesc = CD3D11_BUFFER_DESC(numIndices * sizeof(uint16_t), D3D11_BIND_INDEX_BUFFER);
                    initData.pSysMem = indices.data();
                    device->CreateBuffer(&bufferDesc, &initData, mesh.m_pIndices.GetAddressOf());
                }
                else
                {
                    std::vector<uint32_t> indices(numIndices);
                    for (size_t i = 0; i < numFaces; ++i)
                    {
                        memcpy_s(indices.data() + i * 3, sizeof(uint32_t) * 3,
                            pAiMesh->mFaces[i].mIndices, sizeof(uint32_t) * 3);
                    }
                    bufferDesc = CD3D11_BUFFER_DESC(numIndices * sizeof(uint32_t), D3D11_BIND_INDEX_BUFFER);
                    initData.pSysMem = indices.data();
                    device->CreateBuffer(&bufferDesc, &initData, mesh.m_pIndices.GetAddressOf());
                }
            }

            // 骨骼权重
            if (pAiMesh->HasBones())
            {
                struct BlendTemp { std::array<uint32_t,4> indices; std::array<float,4> weights; };
                std::vector<BlendTemp> weightInfo(numVertices);
                for (auto &tmp : weightInfo)
                {
                    tmp.indices = {0,0,0,0};
                    tmp.weights = {0,0,0,0};
                }
                for (uint32_t b = 0; b < pAiMesh->mNumBones; ++b)
                {
                    aiBone* ai_bone = pAiMesh->mBones[b];
                    std::string boneName = ai_bone->mName.C_Str();

                    // Ensure bone index exists before using it
                    int boneIndex;
                    auto it = model.boneNameToIndex.find(boneName);
                    if (it == model.boneNameToIndex.end())
                    {
                        boneIndex = static_cast<int>(model.bones.size());
                        model.boneNameToIndex[boneName] = boneIndex;

                        BoneInfo info;
                        info.name = boneName;
                        DirectX::XMFLOAT4X4 temp;
                        memcpy(&temp, &ai_bone->mOffsetMatrix, sizeof(ai_bone->mOffsetMatrix));
                        DirectX::XMMATRIX offset = DirectX::XMLoadFloat4x4(&temp);
                        offset = DirectX::XMMatrixTranspose(offset); // Convert from Assimp column-major
                        DirectX::XMStoreFloat4x4(&info.offsetMatrix, offset);
                        info.nodeTransform = DirectX::XMFLOAT4X4(
                            1,0,0,0,
                            0,1,0,0,
                            0,0,1,0,
                            0,0,0,1
                        );
                        model.bones.push_back(info);
                    }
                    else
                    {
                        boneIndex = it->second;
                    }

                    for (uint32_t w = 0; w < ai_bone->mNumWeights; ++w)
                    {
                        const aiVertexWeight& aw = ai_bone->mWeights[w];
                        auto &vert = weightInfo[aw.mVertexId];
                        float weight = aw.mWeight;
                        for (int slot = 0; slot < 4; ++slot)
                        {
                            if (weight > vert.weights[slot])
                            {
                                for (int s = 3; s > slot; --s)
                                {
                                    vert.weights[s] = vert.weights[s-1];
                                    vert.indices[s] = vert.indices[s-1];
                                }
                                vert.weights[slot] = weight;
                                vert.indices[slot] = boneIndex;
                                break;
                            }
                        }
                    }
                }

                std::vector<DirectX::XMUINT4> blendIndices(numVertices);
                std::vector<DirectX::XMFLOAT4> blendWeights(numVertices);
                for (uint32_t v = 0; v < numVertices; ++v)
                {
                    auto &tmp = weightInfo[v];
                    for (int k = 0; k < 4; ++k)
                    {
                        (&blendIndices[v].x)[k] = tmp.indices[k];
                        (&blendWeights[v].x)[k] = tmp.weights[k];
                    }
                    float sum = tmp.weights[0] + tmp.weights[1] + tmp.weights[2] + tmp.weights[3];
                    if (sum > 0)
                    {
                        (&blendWeights[v].x)[0] /= sum;
                        (&blendWeights[v].x)[1] /= sum;
                        (&blendWeights[v].x)[2] /= sum;
                        (&blendWeights[v].x)[3] /= sum;
                    }
                }

                if (!blendIndices.empty())
                {
                    bufferDesc = CD3D11_BUFFER_DESC(numVertices * sizeof(DirectX::XMUINT4), D3D11_BIND_VERTEX_BUFFER);
                    initData.pSysMem = blendIndices.data();
                    device->CreateBuffer(&bufferDesc, &initData, mesh.m_pBlendIndices.GetAddressOf());

                    bufferDesc = CD3D11_BUFFER_DESC(numVertices * sizeof(DirectX::XMFLOAT4), D3D11_BIND_VERTEX_BUFFER);
                    initData.pSysMem = blendWeights.data();
                    device->CreateBuffer(&bufferDesc, &initData, mesh.m_pBlendWeights.GetAddressOf());
                }
            }

            // 材质索引
            mesh.m_MaterialIndex = pAiMesh->mMaterialIndex;
        }

        // 骨骼收集及层级构建
        std::function<void(aiNode*, int)> processNode = [&](aiNode* node, int parent)
        {
            std::string name = node->mName.C_Str();
            int index;
            auto it = model.boneNameToIndex.find(name);
            if (it == model.boneNameToIndex.end())
            {
                index = static_cast<int>(model.bones.size());
                model.boneNameToIndex[name] = index;
                BoneInfo info;
                info.name = name;
                DirectX::XMStoreFloat4x4(&info.offsetMatrix, DirectX::XMMatrixIdentity());
                model.bones.push_back(info);
            }
            else
            {
                index = it->second;
            }

            model.bones[index].parentIndex = parent;
            if (parent >= 0)
                model.bones[parent].children.push_back(index);

            DirectX::XMFLOAT4X4 temp;
            memcpy(&temp, &node->mTransformation, sizeof(node->mTransformation));
            DirectX::XMMATRIX m = DirectX::XMLoadFloat4x4(&temp);
            m = DirectX::XMMatrixTranspose(m);
            DirectX::XMStoreFloat4x4(&model.bones[index].nodeTransform, m);

            for (uint32_t c = 0; c < node->mNumChildren; ++c) {
                processNode(node->mChildren[c], index);
            }
        };
        processNode(pAssimpScene->mRootNode, -1);

        // 输出骨骼信息
        // 打印骨骼结构调试输出
        for (int i = 0; i < (int)model.bones.size(); ++i) {
            const auto& bone = model.bones[i];
            std::cout << "Bone[" << i << "]: " << bone.name
                      << ", parent=" << bone.parentIndex
                      << ", children=" << bone.children.size() << std::endl;
            for (int c : bone.children) {
                if (c == i)
                    std::cerr << "骨骼自身包含自己为子骨骼: " << bone.name << std::endl;
            }
        }


        // 处理材质
        for (uint32_t i = 0; i < pAssimpScene->mNumMaterials; ++i)
        {
            auto& material = model.materials[i];

            auto pAiMaterial = pAssimpScene->mMaterials[i];
            XMFLOAT4 vec{};
            float value{};
            uint32_t boolean{};
            uint32_t num = 3;

            if (aiReturn_SUCCESS == pAiMaterial->Get(AI_MATKEY_COLOR_AMBIENT, (float*)&vec, &num))
                material.Set("$AmbientColor", vec);
            if (aiReturn_SUCCESS == pAiMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, (float*)&vec, &num))
                material.Set("$DiffuseColor", vec);
            if (aiReturn_SUCCESS == pAiMaterial->Get(AI_MATKEY_COLOR_SPECULAR, (float*)&vec, &num))
                material.Set("$SpecularColor", vec);
            if (aiReturn_SUCCESS == pAiMaterial->Get(AI_MATKEY_SPECULAR_FACTOR, value))
                material.Set("$SpecularFactor", value);
            if (aiReturn_SUCCESS == pAiMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, (float*)&vec, &num))
                material.Set("$EmissiveColor", vec);
            if (aiReturn_SUCCESS == pAiMaterial->Get(AI_MATKEY_OPACITY, value))
                material.Set("$Opacity", value);
            if (aiReturn_SUCCESS == pAiMaterial->Get(AI_MATKEY_COLOR_TRANSPARENT, (float*)&vec, &num))
                material.Set("$TransparentColor", vec);
            if (aiReturn_SUCCESS == pAiMaterial->Get(AI_MATKEY_COLOR_REFLECTIVE, (float*)&vec, &num))
                material.Set("$ReflectiveColor", vec);

            aiString aiPath;
            fs::path texFilename;
            std::string texName;

            auto TryCreateTexture = [&](aiTextureType type, std::string_view propertyName, bool genMips = false, bool forceSRGB = false) {
                if (!pAiMaterial->GetTextureCount(type))
                    return;

                pAiMaterial->GetTexture(type, 0, &aiPath);

                if (aiPath.data[0] == '*')
                {
                    texName = filename;
                    texName += aiPath.C_Str();
                    char* pEndStr = nullptr;
                    aiTexture* pTex = pAssimpScene->mTextures[strtol(aiPath.data + 1, &pEndStr, 10)];
                    TextureManager::Get().CreateFromMemory(texName, pTex->pcData, pTex->mHeight ? pTex->mWidth * pTex->mHeight : pTex->mWidth, genMips, forceSRGB);
                    material.Set(propertyName, std::string(texName));
                }
                else
                {
                    fs::path modelPath = filename;
                    fs::path modelDir = modelPath.parent_path();
                    fs::path fbmDir = modelDir / (modelPath.stem().string() + ".fbm");
                    fs::path originalTexPath = aiPath.C_Str();

                    fs::path candidatePaths[] = {
                        // 1. FBX原始路径（绝对或相对）
                        originalTexPath,
                        // 2. 模型目录下的原始路径
                        modelDir / originalTexPath,
                        // 3. .fbm 文件夹下的文件名
                        fbmDir / originalTexPath.filename(),
                        // 4. 模型目录下的文件名
                        modelDir / originalTexPath.filename()
                    };

                    bool found = false;
                    for (auto& tryPath : candidatePaths) {
                        if (fs::exists(tryPath)) {
                            texFilename = tryPath;
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        texFilename = modelDir / originalTexPath.filename(); // 兜底
                    }

                    TextureManager::Get().CreateFromFile(texFilename.string(), genMips, forceSRGB);
                    material.Set(propertyName, texFilename.string());
                }
            };

            TryCreateTexture(aiTextureType_DIFFUSE, "$Diffuse", true, true);
            TryCreateTexture(aiTextureType_NORMALS, "$Normal");
            TryCreateTexture(aiTextureType_BASE_COLOR, "$Albedo", true, true);
            TryCreateTexture(aiTextureType_NORMAL_CAMERA, "$NormalCamera");
            TryCreateTexture(aiTextureType_METALNESS, "$Metalness");
            TryCreateTexture(aiTextureType_DIFFUSE_ROUGHNESS, "$Roughness");
            TryCreateTexture(aiTextureType_AMBIENT_OCCLUSION, "$AmbientOcclusion");
        }
    }
    else
    {
        std::string warning = "[Warning]: ModelManager::CreateFromFile, failed to load \"";
        warning += filename;
        warning += "\"\n";

        if (ImGuiLog::HasInstance())
        {
            ImGuiLog::Get().AddLog(warning.c_str());
        }
        else
        {
            OutputDebugStringA(warning.c_str());
        }
    }
}

void Model::CreateFromGeometry(Model& model, ID3D11Device* device, const GeometryData& data, bool isDynamic)
{
    // 默认材质
    model.materials = { Material{} };
    model.materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
    model.materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f));
    model.materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
    model.materials[0].Set<float>("$SpecularFactor", 10.0f);
    model.materials[0].Set<float>("$Opacity", 1.0f);

    model.meshdatas = { MeshData{} };
    model.meshdatas[0].m_pTexcoordArrays.resize(1);
    model.meshdatas[0].m_VertexCount = (uint32_t)data.vertices.size();
    model.meshdatas[0].m_IndexCount = (uint32_t)(!data.indices16.empty() ? data.indices16.size() : data.indices32.size());
    model.meshdatas[0].m_MaterialIndex = 0;

    CD3D11_BUFFER_DESC bufferDesc(0,
        D3D11_BIND_VERTEX_BUFFER,
        isDynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT,
        isDynamic ? D3D11_CPU_ACCESS_WRITE : 0);
    D3D11_SUBRESOURCE_DATA initData{ nullptr, 0, 0 };

    initData.pSysMem = data.vertices.data();
    bufferDesc.ByteWidth = (uint32_t)data.vertices.size() * sizeof(XMFLOAT3);
    device->CreateBuffer(&bufferDesc, &initData, model.meshdatas[0].m_pVertices.GetAddressOf());

    if (!data.normals.empty())
    {
        initData.pSysMem = data.normals.data();
        bufferDesc.ByteWidth = (uint32_t)data.normals.size() * sizeof(XMFLOAT3);
        device->CreateBuffer(&bufferDesc, &initData, model.meshdatas[0].m_pNormals.GetAddressOf());
    }

    if (!data.texcoords.empty())
    {
        initData.pSysMem = data.texcoords.data();
        bufferDesc.ByteWidth = (uint32_t)data.texcoords.size() * sizeof(XMFLOAT2);
        device->CreateBuffer(&bufferDesc, &initData, model.meshdatas[0].m_pTexcoordArrays[0].GetAddressOf());
    }

    if (!data.tangents.empty())
    {
        initData.pSysMem = data.tangents.data();
        bufferDesc.ByteWidth = (uint32_t)data.tangents.size() * sizeof(XMFLOAT4);
        device->CreateBuffer(&bufferDesc, &initData, model.meshdatas[0].m_pTangents.GetAddressOf());
    }

    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.CPUAccessFlags = 0;
    if (!data.indices16.empty())
    {
        initData.pSysMem = data.indices16.data();
        bufferDesc = CD3D11_BUFFER_DESC((uint16_t)data.indices16.size() * sizeof(uint16_t), D3D11_BIND_INDEX_BUFFER);
        device->CreateBuffer(&bufferDesc, &initData, model.meshdatas[0].m_pIndices.GetAddressOf());
    }
    else
    {
        initData.pSysMem = data.indices32.data();
        bufferDesc = CD3D11_BUFFER_DESC((uint32_t)data.indices32.size() * sizeof(uint32_t), D3D11_BIND_INDEX_BUFFER);
        device->CreateBuffer(&bufferDesc, &initData, model.meshdatas[0].m_pIndices.GetAddressOf());
    }
}

void Model::SetDebugObjectName(std::string_view name)
{
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
    std::string baseStr = name.data();
    size_t sz = meshdatas.size();
    std::string str;
    str.reserve(100);
    for (size_t i = 0; i < sz; ++i)
    {
        baseStr = name.data();
        baseStr += "[" + std::to_string(i) + "].";
        if (meshdatas[i].m_pVertices)
            ::SetDebugObjectName(meshdatas[i].m_pVertices.Get(), baseStr + "vertices");
        if (meshdatas[i].m_pNormals)
            ::SetDebugObjectName(meshdatas[i].m_pNormals.Get(), baseStr + "normals");
        if (meshdatas[i].m_pTangents)
            ::SetDebugObjectName(meshdatas[i].m_pTangents.Get(), baseStr + "tangents");
        if (meshdatas[i].m_pBitangents)
            ::SetDebugObjectName(meshdatas[i].m_pBitangents.Get(), baseStr + "bitangents");
        if (meshdatas[i].m_pColors)
            ::SetDebugObjectName(meshdatas[i].m_pColors.Get(), baseStr + "colors");
        if (meshdatas[i].m_pBlendIndices)
            ::SetDebugObjectName(meshdatas[i].m_pBlendIndices.Get(), baseStr + "blendIndices");
        if (meshdatas[i].m_pBlendWeights)
            ::SetDebugObjectName(meshdatas[i].m_pBlendWeights.Get(), baseStr + "blendWeights");
        if (!meshdatas[i].m_pTexcoordArrays.empty())
        {
            size_t texSz = meshdatas[i].m_pTexcoordArrays.size();
            for (size_t j = 0; j < texSz; ++j)
                ::SetDebugObjectName(meshdatas[i].m_pTexcoordArrays[j].Get(), baseStr + "uv" + std::to_string(j));
        }
        if (meshdatas[i].m_pIndices)
            ::SetDebugObjectName(meshdatas[i].m_pIndices.Get(), baseStr + "indices");
    }
#else
    UNREFERENCED_PARAMETER(name);
#endif
}

namespace
{
    // ModelManager单例
    ModelManager* s_pInstance = nullptr;
}


ModelManager::ModelManager()
{
    if (s_pInstance)
        throw std::exception("ModelManager is a singleton!");
    s_pInstance = this;
}

ModelManager::~ModelManager()
{
}

ModelManager& ModelManager::Get()
{
    if (!s_pInstance)
        throw std::exception("ModelManager needs an instance!");
    return *s_pInstance;
}

void ModelManager::Init(ID3D11Device* device)
{
    m_pDevice = device;
    m_pDevice->GetImmediateContext(m_pDeviceContext.ReleaseAndGetAddressOf());
}

Model* ModelManager::CreateFromFile(std::string_view filename)
{
    return CreateFromFile(filename, filename);
}

// 多用图创建模型 比如路径下的模型 创建多个子模型等等
Model* ModelManager::CreateFromFile(std::string_view name, std::string_view filename)
{
    XID modelID = StringToID(name);
    auto& model = m_Models[modelID];
    Model::CreateFromFile(model, m_pDevice.Get(), filename);
    return &model;
}

Model* ModelManager::CreateFromGeometry(std::string_view name, const GeometryData& data, bool isDynamic)
{
    XID modelID = StringToID(name);
    auto& model = m_Models[modelID];
    Model::CreateFromGeometry(model, m_pDevice.Get(), data, isDynamic);

    return &model;
}

const Model* ModelManager::GetModel(std::string_view name) const
{
    XID nameID = StringToID(name);
    if (auto it = m_Models.find(nameID); it != m_Models.end())
        return &it->second;
    return nullptr;
}

Model* ModelManager::GetModel(std::string_view name)
{
    XID nameID = StringToID(name);
    if (m_Models.count(nameID))
        return &m_Models[nameID];
    return nullptr;
}
