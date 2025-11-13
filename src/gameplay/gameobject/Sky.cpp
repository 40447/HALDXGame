//
// Created by eva00 on 25-8-20.
//

#include "Sky.h"
#include "TransformC.h"
#include "ModelC.h"

Sky::Sky() = default;

Sky::~Sky() = default;

void Sky::Init() {
    GameObject::Init();
    AddComponent<ModelC>("assets/models/player/sky.obj");
    GetComponentRef<TransformC>().SetLocalScale(Vector3(100.0f, 100.0f, 100.0f));


}
