//
// Created by eva00 on 25-5-22.
//

#pragma once
#include <string_view>
#include "Component.h"
#include "Game.h"
#include "SoAStorage.h"

//---------------------------------------------------------------//
// ModelC类用于给GameObject添加模型显示功能
// ModelCクラスはGameObjectにモデル表示機能を追加するためのものです
//---------------------------------------------------------------//

class ModelC : public Component {
private:
    // 缓存模型句柄
    // モデルハンドルをキャッシュします
    UID handle = -1;
    bool visible = true;
    // 检查并且更新模型的Transform
    // モデルのTransformをチェックして更新します
    void CheckTransform();
public:
    explicit ModelC(std::string_view filename);
    void Init() override;
    void Update(float dt) override;
    void Uninit() override;
    UID GetHandle() const { return handle; }
    // 获取模型对象
    // モデルオブジェクトを取得します
    ModelObject *GetModelObject() const;
    // 设置模型是否可见
    // モデルの可視性を設定します
    void SetVisible(bool v) { visible = v; }

    virtual void OnInspectorGUI() override;
    
};
