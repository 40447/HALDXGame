//
// Created by eva00 on 25-6-11.
//

#pragma once
#include "SoAStorage.h"

class SceneHierarchy {
private:
    bool inited = false;
     UID m_selectedid = -1;
    void DrawSceneNode(UID rootid, UID &selectedid);
public:
    SceneHierarchy();
    ~SceneHierarchy() = default;

     void DrawInspector(UID selectedid);

    void Draw();
};
