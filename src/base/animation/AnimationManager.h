//
// Created by eva00 on 25-6-5.
//

#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <DirectXMath.h>
#include <stdexcept>

// 单个骨骼的一组动画关键帧
struct Keyframe
{
    double time; // 动画时间点（秒）
    DirectX::XMFLOAT3 translation;
    DirectX::XMFLOAT4 rotation;
    DirectX::XMFLOAT3 scaling;
};

struct BoneAnimation
{
    std::string boneName;              // 对应骨骼名
    std::vector<Keyframe> keyframes;   // 按时间排序
};

// 一条动画（动作Clip），可以覆盖部分或全部骨骼
struct AnimationClip
{
    std::string name; // 动画名（如 "Walk"、"Run"、"Idle"）
    double duration;  // 持续时间（秒）
    double ticksPerSecond;
    // 每个骨骼各自的关键帧序列
    std::unordered_map<std::string, BoneAnimation> boneAnimations;
};

class AnimationManager {
private:
    // 动画ID到动画的映射
    std::unordered_map<std::string, AnimationClip> m_animations;
public:
    std::vector<std::string> animNames = {};
    void LoadAnimationClips(std::string path, const std::vector<std::string>& names);
    AnimationClip &GetAnimation(const std::string &name);

    bool HasAnimation(const std::string &name) const;
};
