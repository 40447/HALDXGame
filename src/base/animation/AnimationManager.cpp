//
// Created by eva00 on 25-6-5.
//

#include "AnimationManager.h"

#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

void AnimationManager::LoadAnimationClips(std::string path, const std::vector<std::string>& names)  {
    using namespace Assimp;
    Importer importer;
    const aiScene *pAssimpScene = importer.ReadFile(
        path,
        aiProcess_Triangulate | aiProcess_ConvertToLeftHanded | aiProcess_GenBoundingBoxes
    );

    if (!pAssimpScene || !pAssimpScene->mRootNode) {
        std::cout << "アニメーションの読み込みに失敗: " << importer.GetErrorString() << std::endl;
        return;
    }

    for (unsigned int i = 0; i < pAssimpScene->mNumAnimations; ++i)
    {
        const aiAnimation* aiAnim = pAssimpScene->mAnimations[i];
        AnimationClip clip;
        //clip.name = aiAnim->mName.C_Str();
        clip.name = names[i];
        clip.duration = aiAnim->mDuration / (aiAnim->mTicksPerSecond ? aiAnim->mTicksPerSecond : 1.0);
        clip.ticksPerSecond = aiAnim->mTicksPerSecond ? aiAnim->mTicksPerSecond : 1.0;

        // 遍历每个 aiNodeAnim（每个骨骼的动画通道）
        for (unsigned int j = 0; j < aiAnim->mNumChannels; ++j)
        {
            const aiNodeAnim* channel = aiAnim->mChannels[j];
            BoneAnimation boneAnim;
            boneAnim.boneName = channel->mNodeName.C_Str();

            // 把关键帧插入
            unsigned keyCount = std::max({channel->mNumPositionKeys, channel->mNumRotationKeys, channel->mNumScalingKeys});
            for (unsigned k = 0; k < keyCount; ++k)
            {
                Keyframe frame{};
                frame.time = 0.0;
                frame.translation = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
                frame.rotation = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
                frame.scaling = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
                // 位置
                if (k < channel->mNumPositionKeys) {
                    frame.time = channel->mPositionKeys[k].mTime / clip.ticksPerSecond;
                    auto& v = channel->mPositionKeys[k].mValue;
                    frame.translation = DirectX::XMFLOAT3(v.x, v.y, v.z);
                }
                // 旋转
                if (k < channel->mNumRotationKeys) {
                    frame.time = channel->mRotationKeys[k].mTime / clip.ticksPerSecond;
                    auto& q = channel->mRotationKeys[k].mValue;
                    frame.rotation = DirectX::XMFLOAT4(q.x, q.y, q.z, q.w);
                }
                // 缩放
                if (k < channel->mNumScalingKeys) {
                    frame.time = channel->mScalingKeys[k].mTime / clip.ticksPerSecond;
                    auto& s = channel->mScalingKeys[k].mValue;
                    frame.scaling = DirectX::XMFLOAT3(s.x, s.y, s.z);
                }
                boneAnim.keyframes.push_back(frame);
            }

            // 按时间排序
            std::sort(boneAnim.keyframes.begin(), boneAnim.keyframes.end(),
                      [](const Keyframe& a, const Keyframe& b) { return a.time < b.time; });

            clip.boneAnimations[boneAnim.boneName] = std::move(boneAnim);
        }
        animNames.push_back(clip.name);
        m_animations.emplace(clip.name, std::move(clip));
    }
}

AnimationClip &AnimationManager::GetAnimation(const std::string &name) {
    auto it = m_animations.find(name);
    if (it == m_animations.end()) {
        throw std::runtime_error("アニメーションが見つかりません: " + name);
    }
    return it->second;
}

bool AnimationManager::HasAnimation(const std::string &name) const {
    return m_animations.find(name) != m_animations.end();
}
