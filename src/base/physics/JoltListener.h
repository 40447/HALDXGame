#pragma once
#include <Jolt/Physics/Collision/ContactListener.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Math/Real.h>
#include <Jolt/Core/Core.h>

#include "SoAStorage.h"

class JoltContactListener : public JPH::ContactListener {
public:
    // 衝突点が物理エンジンに追加される前に呼び出される検証関数。
    // ここで false（Reject）を返すと、その接触点は無効化される。
    // 例：特定のレイヤー間の当たり判定を無効にするなど。
    virtual JPH::ValidateResult OnContactValidate(
        const JPH::Body &inBody1,
        const JPH::Body &inBody2,
        JPH::RVec3Arg inBaseOffset,
        const JPH::CollideShapeResult &inCollisionResult
    ) override;

    // 新しい衝突点が検出された時に呼び出される。
    // 最初の1回のみ呼ばれ、同じ物体同士の継続的な接触では呼ばれない。
    // 効果音やエフェクトのトリガーに使える。
    // ioSettings で摩擦や反発係数を調整可能
    virtual void OnContactAdded(
        const JPH::Body &inBody1,
        const JPH::Body &inBody2,
        const JPH::ContactManifold &inManifold,
        JPH::ContactSettings &ioSettings
    ) override;

    // 前のフレームでも接触していた物体が、今フレームでも引き続き接触している場合に呼び出される。
    // 毎フレーム呼ばれるので、継続的な処理（滑り音など）に適している。
    // ioSettings を変更すれば摩擦なども動的に調整できる。
    virtual void OnContactPersisted(
        const JPH::Body &inBody1,
        const JPH::Body &inBody2,
        const JPH::ContactManifold &inManifold,
        JPH::ContactSettings &ioSettings
    ) override;

    // 以前のフレームで接触していた物体同士が、現在は接触していない場合に呼び出される。
    // この関数では Body にアクセスしてはいけない（すでに削除された可能性あり）。
    // 状態のリセットや終了処理に使う。
    // 必要な情報は OnContactAdded や OnContactPersisted で事前にキャッシュしておくこと。
    virtual void OnContactRemoved(
        const JPH::SubShapeIDPair &inSubShapePair
    ) override;

private:
    std::unordered_map<JPH::BodyID, UID> m_bodyToUIDMap; // Map to store BodyID to UID mapping
public:
    void AddBodyToUIDMap(const JPH::BodyID &bodyID, UID uid);
    void RemoveBodyFromUIDMap(const JPH::BodyID &bodyID);
};

class JoltBodyActivationListener : public JPH::BodyActivationListener {
public:
    // Called when a body becomes active (e.g. due to a collision or force)
    virtual void OnBodyActivated(
        const JPH::BodyID &inBodyID,
        JPH::uint64 inBodyUserData
    ) override;

    // Called when a body goes to sleep
    virtual void OnBodyDeactivated(
        const JPH::BodyID &inBodyID,
        JPH::uint64 inBodyUserData
    ) override;
};
