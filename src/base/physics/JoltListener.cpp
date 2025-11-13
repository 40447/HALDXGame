#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/ContactListener.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Math/Vec3.h>
#include "JoltListener.h"

#include "CollisionC.h"
#include "GameObject.h"
#include "Global.h"

using namespace std;

JPH::ValidateResult JoltContactListener::OnContactValidate(
    const JPH::Body &inBody1,
    const JPH::Body &inBody2,
    JPH::RVec3Arg inBaseOffset,
    const JPH::CollideShapeResult &inCollisionResult) {
    return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
}

void JoltContactListener::OnContactAdded(
    const JPH::Body &inBody1,
    const JPH::Body &inBody2,
    const JPH::ContactManifold &inManifold,
    JPH::ContactSettings &ioSettings) {
    const JPH::BodyID id1 = inBody1.GetID();
    const JPH::BodyID id2 = inBody2.GetID();
    auto it = m_bodyToUIDMap.find(id1);
    if (it == m_bodyToUIDMap.end()) {
        return;
    }
    CollisionC &collision1 = halgame->GetScene()->GetComponentRef<CollisionC>(it->second);
    it = m_bodyToUIDMap.find(id2);
    if (it == m_bodyToUIDMap.end()) {
        return;
    }
    CollisionC &collision2 = halgame->GetScene()->GetComponentRef<CollisionC>(it->second);
    collision1.GetGameObject()->OnContactAdded(collision1, collision2, inManifold, ioSettings);
    collision2.GetGameObject()->OnContactAdded(collision2, collision1, inManifold, ioSettings);
}

void JoltContactListener::OnContactPersisted(
    const JPH::Body &inBody1,
    const JPH::Body &inBody2,
    const JPH::ContactManifold &inManifold,
    JPH::ContactSettings &ioSettings) {
    const JPH::BodyID id1 = inBody1.GetID();
    const JPH::BodyID id2 = inBody2.GetID();
    auto it = m_bodyToUIDMap.find(id1);
    if (it == m_bodyToUIDMap.end()) {
        return;
    }
    CollisionC &collision1 = halgame->GetScene()->GetComponentRef<CollisionC>(it->second);
    it = m_bodyToUIDMap.find(id2);
    if (it == m_bodyToUIDMap.end()) {
        return;
    }
    CollisionC &collision2 = halgame->GetScene()->GetComponentRef<CollisionC>(it->second);
    collision1.GetGameObject()->OnContactPersisted(collision1, collision2, inManifold, ioSettings);
    collision2.GetGameObject()->OnContactPersisted(collision2, collision1, inManifold, ioSettings);
}

void JoltContactListener::OnContactRemoved(const JPH::SubShapeIDPair &inSubShapePair) {
    const JPH::BodyID id1 = inSubShapePair.GetBody1ID();
    const JPH::BodyID id2 = inSubShapePair.GetBody2ID();
    auto it = m_bodyToUIDMap.find(id1);
    if (it == m_bodyToUIDMap.end()) {
        return;
    }
    CollisionC &collision1 = halgame->GetScene()->GetComponentRef<CollisionC>(it->second);
    it = m_bodyToUIDMap.find(id2);
    if (it == m_bodyToUIDMap.end()) {
        return;
    }
    CollisionC &collision2 = halgame->GetScene()->GetComponentRef<CollisionC>(it->second);
    collision1.GetGameObject()->OnContactRemoved(collision1, collision2);
    collision2.GetGameObject()->OnContactRemoved(collision2, collision1);
}

void JoltContactListener::AddBodyToUIDMap(const JPH::BodyID &bodyID, UID uid) {
    m_bodyToUIDMap[bodyID] = uid;
}

void JoltContactListener::RemoveBodyFromUIDMap(const JPH::BodyID &bodyID) {
    auto it = m_bodyToUIDMap.find(bodyID);
    if (it != m_bodyToUIDMap.end()) {
        m_bodyToUIDMap.erase(it);
    }
}

void JoltBodyActivationListener::OnBodyActivated(
    const JPH::BodyID &inBodyID,
    JPH::uint64 inBodyUserData) {
    //cout << "A body got activated" << endl;
}

void JoltBodyActivationListener::OnBodyDeactivated(
    const JPH::BodyID &inBodyID,
    JPH::uint64 inBodyUserData) {
    //cout << "A body went to sleep" << endl;
}
