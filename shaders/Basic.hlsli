#include "LightHelper.hlsli"

static const uint MAX_BONES = 256;

Texture2D g_DiffuseMap : register(t0);
SamplerState g_Sam : register(s0);


cbuffer CBChangesEveryDrawing : register(b0)
{
    matrix g_World;
    matrix g_WorldInvTranspose;
    Material g_Material;
}

cbuffer CBChangesEveryFrame : register(b1)
{
    matrix g_ViewProj;
    float3 g_EyePosW;
}

cbuffer CBChangesRarely : register(b2)
{
    DirectionalLight g_DirLight[5];
    PointLight g_PointLight[5];
    SpotLight g_SpotLight[5];
}

cbuffer CBBones : register(b3)
{
    matrix g_BoneTransforms[MAX_BONES];
};



struct VertexPosNormalTex
{
    float3 posL : POSITION;
    float3 normalL : NORMAL;
    float2 tex : TEXCOORD;
};

struct VertexSkinned
{
    float3 posL : POSITION;
    float3 normalL : NORMAL;
    float2 tex : TEXCOORD;
    uint4 boneIndices : BLENDINDICES;
    float4 boneWeights : BLENDWEIGHT;
};

struct VertexPosHWNormalTex
{
    float4 posH : SV_POSITION;
    float3 posW : POSITION;     // 在世界中的位置
    float3 normalW : NORMAL;    // 法向量在世界中的方向
    float2 tex : TEXCOORD;
};






