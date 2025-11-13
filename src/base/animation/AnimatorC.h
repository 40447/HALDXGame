//
// Created by eva00 on 25-6-5.
//
#pragma once
#include <string>

#include "Component.h"

struct TransformData {
    DirectX::XMFLOAT3 translation;
    DirectX::XMFLOAT4 rotation;   // quaternion
    DirectX::XMFLOAT3 scale;
};

struct KeyframePrevNext {
    const Keyframe* prev;
    const Keyframe* next;
    float t; // 插值因子
};

class AnimatorC: public Component {
private:
    // 之前动画名称
    std::string m_prevAnimName = "Idle";
    // 当前播放的动画名称
    std::string m_currentAnimName = "Idle";
    // 动画融合率
    float m_blendFactor = 0.0f;
    // 动画融合速度
    float BlendSpeed = 0.2f;
    // 动画播放时间
    double m_currentTime = 0.0;
    // 动画播放速度
    float m_playbackSpeed = 1.0f;


    // 是否播放动画
    bool m_isPlaying = true;

private:
    static TransformData GetBoneTransformAtTime(const Keyframe *prev, const Keyframe *next, float t);
    static KeyframePrevNext FindPrevNextKeyframes(const std::vector<Keyframe>& keyframes, float currentTime);
public:
    explicit AnimatorC(const std::string &defaultAnim = "Idle");
    void Init() override;
    void Update(float dt) override;
    void Uninit() override;

    void Play(const std::string& animName);

    bool NetPack(PacketWriter &writer) override;
    void NetUnpack(PacketReader &reader) override;

    virtual void OnInspectorGUI() override;
};
