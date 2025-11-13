//
// Created by eva00 on 25-6-7.
//

#include "MixamoRagdollC.h"

#include <iostream>
#include <map>
#include <Jolt/Jolt.h>
#include "Global.h"
#include "MixamorigBoneC.h"
#include "TransformC.h"
#include "Jolt/Physics/Collision/Shape/CapsuleShape.h"
#include "Jolt/Physics/Constraints/SwingTwistConstraint.h"
#include "Jolt/Physics/Ragdoll/Ragdoll.h"

MixamoRagdollC::MixamoRagdollC() {
}

MixamoRagdollC::~MixamoRagdollC() {
    mRagdoll->RemoveFromPhysicsSystem();
}

std::map<std::string, bool> mixamorig = {
    {"mixamorig:Hips", true},
    {"mixamorig:Spine", true},
    {"mixamorig:Spine1", true},
    {"mixamorig:Spine2", true},
    {"mixamorig:Head", true},
    {"mixamorig:LeftArm", true},
    {"mixamorig:LeftForeArm", true},
    {"mixamorig:RightArm", true},
    {"mixamorig:RightForeArm", true},
    {"mixamorig:LeftUpLeg", true},
    {"mixamorig:LeftLeg", true},
    {"mixamorig:RightUpLeg", true},
    {"mixamorig:RightLeg", true}
};

void PrintBoneWorldPositions(const std::vector<BoneInfo> &bones, int boneIdx, DirectX::XMMATRIX parentWorld) {
    const BoneInfo &bone = bones[boneIdx];
    // 把 nodeTransform 转成 XMMATRIX
    DirectX::XMMATRIX local = XMLoadFloat4x4(&bone.nodeTransform);
    DirectX::XMMATRIX world = local * parentWorld;
    // 提取位置
    DirectX::XMFLOAT3 pos;
    XMStoreFloat3(&pos, world.r[3]); // 行主序直接取 r[3]
    if (mixamorig.find(bone.name) != mixamorig.end()) {
        printf("%s: (%.3f, %.3f, %.3f)\n", bone.name.c_str(), pos.x, pos.y, pos.z);
        std::cout << bone.name.c_str() << ": (" << pos.x << ", " << pos.y << ", " << pos.z << ")" << std::endl;
    }

    // 递归处理所有子骨骼
    for (int childIdx: bone.children) {
        PrintBoneWorldPositions(bones, childIdx, world);
    }
}

void MixamoRagdollC::Init() {
    Component::Init();

    // 构建 Skeleton
    JPH::Ref<JPH::Skeleton> skeleton = new JPH::Skeleton();
    JPH::uint hips = skeleton->AddJoint("mixamorig:Hips"); // 0
    JPH::uint lower_body = skeleton->AddJoint("mixamorig:Spine", hips); // 1
    JPH::uint mid_body = skeleton->AddJoint("mixamorig:Spine1", lower_body); // 2
    JPH::uint upper_body = skeleton->AddJoint("mixamorig:Spine2", mid_body); // 3
    JPH::uint head = skeleton->AddJoint("mixamorig:Head", upper_body); // 4

    JPH::uint upper_arm_l = skeleton->AddJoint("mixamorig:LeftArm", upper_body); // 5
    JPH::uint lower_arm_l = skeleton->AddJoint("mixamorig:LeftForeArm", upper_arm_l); // 6

    JPH::uint upper_arm_r = skeleton->AddJoint("mixamorig:RightArm", upper_body); // 7
    JPH::uint lower_arm_r = skeleton->AddJoint("mixamorig:RightForeArm", upper_arm_r); // 8

    JPH::uint upper_leg_l = skeleton->AddJoint("mixamorig:LeftUpLeg", hips); // 9
    JPH::uint lower_leg_l = skeleton->AddJoint("mixamorig:LeftLeg", upper_leg_l); // 10

    JPH::uint upper_leg_r = skeleton->AddJoint("mixamorig:RightUpLeg", hips); // 11
    JPH::uint lower_leg_r = skeleton->AddJoint("mixamorig:RightLeg", upper_leg_r); // 12

    // Ragdoll Settings
    JPH::RagdollSettings *mRagdollSettings = new JPH::RagdollSettings;
    mRagdollSettings->mSkeleton = skeleton;
    mRagdollSettings->mParts.resize(skeleton->GetJointCount());

    float scale = 2.0f; // 模型缩放比例

    // Capsule Shapes，参数可再调优
    JPH::Ref<JPH::Shape> shapes[] = {
        new JPH::CapsuleShape(0.13f * scale, 0.11f * scale), // Hips
        new JPH::CapsuleShape(0.12f * scale, 0.09f * scale), // Spine
        new JPH::CapsuleShape(0.12f * scale, 0.09f * scale), // Spine1
        new JPH::CapsuleShape(0.13f * scale, 0.10f * scale), // Spine2
        new JPH::CapsuleShape(0.08f * scale, 0.10f * scale), // Head

        new JPH::CapsuleShape(0.13f * scale, 0.06f * scale), // LeftArm
        new JPH::CapsuleShape(0.13f * scale, 0.05f * scale), // LeftForeArm

        new JPH::CapsuleShape(0.13f * scale, 0.06f * scale), // RightArm
        new JPH::CapsuleShape(0.13f * scale, 0.05f * scale), // RightForeArm

        new JPH::CapsuleShape(0.16f * scale, 0.08f * scale), // LeftUpLeg
        new JPH::CapsuleShape(0.16f * scale, 0.06f * scale), // LeftLeg

        new JPH::CapsuleShape(0.16f * scale, 0.08f * scale), // RightUpLeg
        new JPH::CapsuleShape(0.16f * scale, 0.06f * scale), // RightLeg
    };

    // World空间的初始位置
    JPH::RVec3 positions[] = {
        JPH::RVec3(0, 1.15f, 0) * scale,
        JPH::RVec3(0, 1.30f, 0) * scale,
        JPH::RVec3(0, 1.40f, 0) * scale,
        JPH::RVec3(0, 1.55f, 0) * scale,
        JPH::RVec3(0, 1.75f, 0) * scale,

        JPH::RVec3(-0.41f, 1.55f, 0) * scale,
        JPH::RVec3(-0.75f, 1.55f, 0) * scale,

        JPH::RVec3(0.41f, 1.55f, 0) * scale,
        JPH::RVec3(0.75f, 1.55f, 0) * scale,

        JPH::RVec3(-0.16f, 0.80f, 0) * scale,
        JPH::RVec3(-0.16f, 0.30f, 0) * scale,

        JPH::RVec3(0.16f, 0.80f, 0) * scale,
        JPH::RVec3(0.16f, 0.30f, 0) * scale,
    };

    // 旋转
    JPH::Quat rotations[] = {
        JPH::Quat::sRotation(JPH::Vec3::sAxisZ(), 0.5f * JPH::JPH_PI), // Hips
        JPH::Quat::sRotation(JPH::Vec3::sAxisZ(), 0.5f * JPH::JPH_PI), // Spine
        JPH::Quat::sRotation(JPH::Vec3::sAxisZ(), 0.5f * JPH::JPH_PI), // Spine1
        JPH::Quat::sRotation(JPH::Vec3::sAxisZ(), 0.5f * JPH::JPH_PI), // Spine2
        JPH::Quat::sIdentity(), // Head

        JPH::Quat::sRotation(JPH::Vec3::sAxisZ(), 0.5f * JPH::JPH_PI), // LeftArm
        JPH::Quat::sRotation(JPH::Vec3::sAxisZ(), 0.5f * JPH::JPH_PI), // LeftForeArm

        JPH::Quat::sRotation(JPH::Vec3::sAxisZ(), 0.5f * JPH::JPH_PI), // RightArm
        JPH::Quat::sRotation(JPH::Vec3::sAxisZ(), 0.5f * JPH::JPH_PI), // RightForeArm

        JPH::Quat::sIdentity(), // LeftUpLeg
        JPH::Quat::sIdentity(), // LeftLeg

        JPH::Quat::sIdentity(), // RightUpLeg
        JPH::Quat::sIdentity(), // RightLeg
    };

    // Constraint positions
    JPH::RVec3 constraint_positions[] = {
        JPH::RVec3::sZero(),
        JPH::RVec3(0, 1.23f, 0) * scale,
        JPH::RVec3(0, 1.35f, 0) * scale,
        JPH::RVec3(0, 1.50f, 0) * scale,
        JPH::RVec3(0, 1.65f, 0) * scale,

        JPH::RVec3(-0.22f, 1.55f, 0) * scale,
        JPH::RVec3(-0.60f, 1.55f, 0) * scale,

        JPH::RVec3(0.22f, 1.55f, 0) * scale,
        JPH::RVec3(0.60f, 1.55f, 0) * scale,

        JPH::RVec3(-0.16f, 1.02f, 0) * scale,
        JPH::RVec3(-0.16f, 0.55f, 0) * scale,

        JPH::RVec3(0.16f, 1.02f, 0) * scale,
        JPH::RVec3(0.16f, 0.55f, 0) * scale,
    };

    // Twist Axis
    JPH::Vec3 twist_axis[] = {
        JPH::Vec3::sZero(), // Hips
        JPH::Vec3::sAxisY(), // Spine
        JPH::Vec3::sAxisY(), // Spine1
        JPH::Vec3::sAxisY(), // Spine2
        JPH::Vec3::sAxisY(), // Head

        -JPH::Vec3::sAxisX(), // LeftArm
        -JPH::Vec3::sAxisX(), // LeftForeArm

        JPH::Vec3::sAxisX(), // RightArm
        JPH::Vec3::sAxisX(), // RightForeArm

        -JPH::Vec3::sAxisY(), // LeftUpLeg
        -JPH::Vec3::sAxisY(), // LeftLeg

        -JPH::Vec3::sAxisY(), // RightUpLeg
        -JPH::Vec3::sAxisY(), // RightLeg
    };

    // 角度限制 (Twist, Normal, Plane)
    float twist_angle[] = {
        0.0f, // Hips
        5.0f, // Spine
        5.0f, // Spine1
        5.0f, // Spine2
        90.0f, // Head

        45.0f, // LeftArm
        45.0f, // LeftForeArm

        45.0f, // RightArm
        45.0f, // RightForeArm

        45.0f, // LeftUpLeg
        45.0f, // LeftLeg

        45.0f, // RightUpLeg
        45.0f, // RightLeg
    };

    float normal_angle[] = {
        0.0f, // Hips
        10.0f, // Spine
        10.0f, // Spine1
        10.0f, // Spine2
        45.0f, // Head

        90.0f, // LeftArm
        0.0f, // LeftForeArm

        90.0f, // RightArm
        0.0f, // RightForeArm

        45.0f, // LeftUpLeg
        0.0f, // LeftLeg

        45.0f, // RightUpLeg
        0.0f, // RightLeg
    };

    float plane_angle[] = {
        0.0f, // Hips
        10.0f, // Spine
        10.0f, // Spine1
        10.0f, // Spine2
        45.0f, // Head

        45.0f, // LeftArm
        90.0f, // LeftForeArm

        45.0f, // RightArm
        90.0f, // RightForeArm

        45.0f, // LeftUpLeg
        60.0f, // LeftLeg

        45.0f, // RightUpLeg
        60.0f, // RightLeg
    };


    // 配置各部分
    for (int p = 0; p < skeleton->GetJointCount(); ++p) {
        JPH::RagdollSettings::Part &part = mRagdollSettings->mParts[p];
        part.SetShape(shapes[p]);
        part.mPosition = positions[p];
        part.mRotation = rotations[p];
        part.mMotionType = JPH::EMotionType::Dynamic;
        part.mObjectLayer = Layers::MOVING;

        if (p > 0) {
            auto *constraint = new JPH::SwingTwistConstraintSettings;
            constraint->mDrawConstraintSize = 0.1f;
            constraint->mPosition1 = constraint->mPosition2 = constraint_positions[p];
            constraint->mTwistAxis1 = constraint->mTwistAxis2 = twist_axis[p];
            constraint->mPlaneAxis1 = constraint->mPlaneAxis2 = JPH::Vec3::sAxisZ();
            constraint->mTwistMinAngle = -JPH::DegreesToRadians(twist_angle[p]);
            constraint->mTwistMaxAngle = JPH::DegreesToRadians(twist_angle[p]);
            constraint->mNormalHalfConeAngle = JPH::DegreesToRadians(normal_angle[p]);
            constraint->mPlaneHalfConeAngle = JPH::DegreesToRadians(plane_angle[p]);
            part.mToParent = constraint;
        }
    }

    // 推荐加的初始化
    mRagdollSettings->Stabilize();
    mRagdollSettings->DisableParentChildCollisions();
    mRagdollSettings->CalculateBodyIndexToConstraintIndex();

    mRagdoll = mRagdollSettings->CreateRagdoll(0, 0, halgame->GetPhysicsSystem());
    mRagdoll->AddToPhysicsSystem(JPH::EActivation::Activate);

    mRagdollPose.SetSkeleton(mRagdollSettings->GetSkeleton());
    mRagdollPose.CalculateJointMatrices();
    mRagdoll->SetPose(mRagdollPose);
}

void MixamoRagdollC::Update(float dt) {
    Component::Update(dt);
    UpdateFinalBoneMatrices();
}

void MixamoRagdollC::Uninit() {
    Component::Uninit();
}

void MixamoRagdollC::UpdateFinalBoneMatrices() {
    MixamorigBoneC &mixamorigBoneC = m_gameObject->GetComponentRef<MixamorigBoneC>();
    mRagdoll->GetPose(mRagdollPose);
    auto ragdollskeleton = mRagdoll->GetRagdollSettings()->GetSkeleton();
    for (int i = 0; i < ragdollskeleton->GetJointCount(); ++i) {
        std::string jointName = ragdollskeleton->GetJoint(i).mName.c_str();
        const BoneObj *boneObj = mixamorigBoneC.GetBone(jointName);
        if (boneObj == nullptr) {
            continue;
        }
        const auto &jointMat = mRagdollPose.GetJointMatrix(i);
        Vec3 translation = jointMat.GetTranslation();
        Vector3 position = Vector3(translation.GetX(), translation.GetY(), translation.GetZ());
        Quat quat = jointMat.GetQuaternion();
        Quaternion rotation = Quaternion(quat.GetX(), quat.GetY(), quat.GetZ(), quat.GetW());
        TransformC &transformC = boneObj->GetComponentRef<TransformC>();
        transformC.SetLocalPosition(position);
        transformC.SetLocalRotation(rotation);
    }
}
