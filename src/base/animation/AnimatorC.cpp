//
// Created by eva00 on 25-6-5.
//

#include "AnimatorC.h"

#include "AnimationManager.h"
#include "GameObject.h"
#include "Global.h"
#include "MixamorigBoneC.h"
#include "ModelC.h"
#include "TransformC.h"

AnimatorC::AnimatorC(const std::string &defaultAnim) {
    m_currentAnimName = defaultAnim;
}

void AnimatorC::Init() {
    Component::Init();
}

void AnimatorC::Update(float dt) {
    Component::Update(dt);
    if (!HasComponent<MixamorigBoneC>()) {
        return;
    }
    // 获取当前动画clip
    if (!halgame->animationManager.HasAnimation(m_currentAnimName)) {
        return;
    }
    const AnimationClip &clip = halgame->animationManager.GetAnimation(m_currentAnimName);

    const AnimationClip *prevClip = nullptr;
    if (m_prevAnimName != m_currentAnimName && halgame->animationManager.HasAnimation(m_prevAnimName)) {
        prevClip = &halgame->animationManager.GetAnimation(m_prevAnimName);
    }

    // 更新动画时间
    if (m_isPlaying) {
        m_currentTime += dt * m_playbackSpeed;
    }
    // 循环动画
    if (m_currentTime > clip.duration) {
        m_currentTime = fmod(m_currentTime, clip.duration);
    }
    MixamorigBoneC &boneC = GetComponentRef<MixamorigBoneC>();
    auto boneNames = boneC.GetBoneNames();
    for (const auto &boneName: *boneNames) {
        auto BoneObj = boneC.GetBone(boneName);
        if (BoneObj == nullptr) {
            continue; // 骨骼不存在，跳过
        }
        TransformC &boneTransform = BoneObj->GetComponentRef<TransformC>();
        auto it = clip.boneAnimations.find(boneName);
        if (it == clip.boneAnimations.end() || it->second.keyframes.empty()) {
            continue;
        }

        // 计算关键帧之间的插值
        KeyframePrevNext kn = FindPrevNextKeyframes(it->second.keyframes, m_currentTime);
        TransformData boneTransformData = GetBoneTransformAtTime(kn.prev, kn.next, kn.t);

        // 如果有前一个动画，进行融合
        if (prevClip != nullptr) {
            auto it = prevClip->boneAnimations.find(boneName);
            if (it == prevClip->boneAnimations.end() || it->second.keyframes.empty()) {
                // 前一个动画没有这个骨骼的动画，直接使用当前动画
            } else {
                KeyframePrevNext knPrev = FindPrevNextKeyframes(it->second.keyframes, m_currentTime);

                TransformData prevBoneTransformData = GetBoneTransformAtTime(knPrev.prev, knPrev.next, knPrev.t);
                // 计算融合因子
                if (m_blendFactor < 1.0f && m_isPlaying) {
                    m_blendFactor += dt * BlendSpeed;
                    if (m_blendFactor > 1.0f) {
                        m_blendFactor = 1.0f;
                    }
                }
                // 融合两个动画的变换
                boneTransformData.translation = Vector3::Lerp(prevBoneTransformData.translation, boneTransformData.translation, m_blendFactor);
                boneTransformData.rotation = Quaternion::Slerp(prevBoneTransformData.rotation, boneTransformData.rotation, m_blendFactor);
                boneTransformData.scale = Vector3::Lerp(prevBoneTransformData.scale, boneTransformData.scale, m_blendFactor);
            }
        }

        boneTransform.SetLocalPosition(boneTransformData.translation);
        boneTransform.SetLocalRotation(boneTransformData.rotation);
        boneTransform.SetLocalScale(boneTransformData.scale);
    }
}

void AnimatorC::Uninit() {
    Component::Uninit();
}

void AnimatorC::Play(const std::string &animName) {
    if (!halgame->animationManager.HasAnimation(animName)) {
        return;
    }
    if (m_currentAnimName == animName) {
        return;
    }
    if (m_isPlaying == false) {
        return;
    }
    m_blendFactor = 0.0f;
    m_currentTime = 0.0;
    m_prevAnimName = m_currentAnimName;
    m_currentAnimName = animName;
}

bool AnimatorC::NetPack(PacketWriter &writer) {
    if (!Component::NetPack(writer)) {
        return false;
    }
    writer.writeString(m_currentAnimName);
    return true;
}

void AnimatorC::NetUnpack(PacketReader &reader) {
    Component::NetUnpack(reader);
    Play(reader.readString());
}

void AnimatorC::OnInspectorGUI() {
    Component::OnInspectorGUI();
    if (ImGui::BeginCombo("Previous Animation", m_prevAnimName.c_str())) {
        for (int i = 0; i < halgame->animationManager.animNames.size(); i++) {
            bool isSelected = (m_prevAnimName == halgame->animationManager.animNames[i]);
            if (ImGui::Selectable(halgame->animationManager.animNames[i].c_str(), isSelected)) {
                m_prevAnimName = halgame->animationManager.animNames[i];
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    if (ImGui::BeginCombo("Current Animation", m_currentAnimName.c_str())) {
        for (int i = 0; i < halgame->animationManager.animNames.size(); i++) {
            bool isSelected = (m_currentAnimName == halgame->animationManager.animNames[i]);
            if (ImGui::Selectable(halgame->animationManager.animNames[i].c_str(), isSelected)) {
                m_currentAnimName = halgame->animationManager.animNames[i];
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
    ImGui::Text("Current Time: %.2f", m_currentTime);
    ImGui::Checkbox("Is Playing", &m_isPlaying);
    ImGui::SliderFloat("Playback Speed", &m_playbackSpeed, 0.0f, 3.0f);
    ImGui::SliderFloat("BlendSpeed", &BlendSpeed, 0.0f, 2.0f);
    ImGui::SliderFloat("Blend Factor", &m_blendFactor, 0.0f, 1.0f);
}

TransformData AnimatorC::GetBoneTransformAtTime(const Keyframe *prev, const Keyframe *next, float t) {
    TransformData transform{};
    transform.translation = Vector3::Lerp(prev->translation, next->translation, t);
    transform.rotation = Quaternion::Slerp(prev->rotation, next->rotation, t);
    transform.scale = Vector3::Lerp(prev->scaling, next->scaling, t);
    return transform;
}

KeyframePrevNext AnimatorC::FindPrevNextKeyframes(const std::vector<Keyframe> &keyframes, float currentTime) {
    KeyframePrevNext result{nullptr, nullptr, 0.0f};
    if (keyframes.empty()) {
        return result;
    }
    result.prev = &keyframes.front();
    result.next = &keyframes.back();
    // 特殊情况 当前时间在第一个关键帧之前
    for (size_t i = 1; i < keyframes.size(); ++i) {
        if (currentTime < keyframes[i].time) {
            result.next = &keyframes[i];
            result.prev = &keyframes[i - 1];
            break;
        }
    }
    float deltaTime = result.next->time - result.prev->time;
    result.t = (result.next->time - result.prev->time) > 0 ? (currentTime - result.prev->time) / (result.next->time - result.prev->time) : 0.0;
    return result;
}
