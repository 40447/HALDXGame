#include "Basic.hlsli"

VertexPosHWNormalTex VS(VertexSkinned vIn)
{
    float4x4 skinMat = 
        g_BoneTransforms[vIn.boneIndices.x] * vIn.boneWeights.x +
        g_BoneTransforms[vIn.boneIndices.y] * vIn.boneWeights.y +
        g_BoneTransforms[vIn.boneIndices.z] * vIn.boneWeights.z +
        g_BoneTransforms[vIn.boneIndices.w] * vIn.boneWeights.w;

    float4 posL = mul(float4(vIn.posL, 1.0f), skinMat);
    float3 normalL = mul(vIn.normalL, (float3x3)skinMat);

    VertexPosHWNormalTex vOut;
    float4 posW = mul(posL, g_World);
    vOut.posW = posW.xyz;
    vOut.posH = mul(posW, g_ViewProj);
    vOut.normalW = mul(normalL, (float3x3)g_WorldInvTranspose);
    vOut.tex = vIn.tex;
    return vOut;
}
