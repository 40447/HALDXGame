//
// Created by eva00 on 25-5-29.
//
#pragma once

#include <SimpleMath.h>
#include <vector>
#include "Component.h"

class TransformC : public Component {
private:
    // ローカル座標
    DirectX::SimpleMath::Vector3 m_localPosition;
    // ローカル回転をクォータニオンで格納
    DirectX::SimpleMath::Quaternion m_localRotation;
    // オイラー角を格納するための変数
    DirectX::SimpleMath::Vector3 m_localEuler;
    // ローカルスケール
    DirectX::SimpleMath::Vector3 m_localScale;

    // ローカル変換行列
    DirectX::SimpleMath::Matrix m_localMatrix;
    // ワールド変換行列
    DirectX::SimpleMath::Matrix m_worldMatrix;
    // 変換行列が更新されたかどうかのフラグ
    bool m_dirty;

    // 親TransformC
    TransformC *m_parent;
    // 子TransformCのリスト
    std::vector<TransformC *> m_Childs;
    // 変換行列を更新する
    void UpdateTransform();

    // ローカル変換行列を再計算する
    void RecalculateLocalMatrix();

public:
    TransformC();

    ~TransformC() override = default;

    void Uninit() override;

    //ローカル座標の設定
    void SetLocalPosition(const DirectX::SimpleMath::Vector3 &position);

    // ローカル回転の設定
    void SetLocalRotation(const DirectX::SimpleMath::Quaternion &rotation);

    // ローカル回転をオイラー角で設定（ラジアン単位）
    void SetLocalRotationEuler(const DirectX::SimpleMath::Vector3 &eulerRadians);

    // ローカルスケールの設定
    void SetLocalScale(const DirectX::SimpleMath::Vector3 &scale);

    // ワールド座標の設定
    void SetWorldPosition(const DirectX::SimpleMath::Vector3 &position);

    // ワールド回転の設定（クォータニオン）
    void SetWorldRotation(const DirectX::SimpleMath::Quaternion &rotation);

    // ローカル座標の取得
    DirectX::SimpleMath::Vector3 GetLocalPosition() const;

    // ローカル回転の取得
    DirectX::SimpleMath::Quaternion GetLocalRotation() const;

    // ローカル回転をオイラー角で取得（ラジアン単位）
    DirectX::SimpleMath::Vector3 GetLocalRotationEuler() const;

    // ローカルスケールの取得
    DirectX::SimpleMath::Vector3 GetLocalScale() const;

    //ローカル変換行列の取得
    DirectX::SimpleMath::Matrix GetLocalMatrix();

    // ワールドの変換行列を取得
    DirectX::SimpleMath::Matrix GetWorldMatrix();

    // ワールド座標の取得
    DirectX::SimpleMath::Vector3 GetWorldPosition();

    // ワールド回転の取得（クォータニオン）
    DirectX::SimpleMath::Quaternion GetWorldRotation();

    // ワールド回転をオイラー角で取得（ラジアン単位）
    DirectX::SimpleMath::Vector3 GetWorldRotationEuler();

    // ワールドスケールの取得
    DirectX::SimpleMath::Vector3 GetWorldScale();

    DirectX::SimpleMath::Vector3 GetForward();
     DirectX::SimpleMath::Vector3 GetRight();
    DirectX::SimpleMath::Vector3 GetLeft();
    DirectX::SimpleMath::Vector3 GetUp();
    DirectX::SimpleMath::Vector3 GetDown();

    void OnInspectorGUI() override;

    bool NetPack(PacketWriter &writer) override;
    void NetUnpack(PacketReader &reader) override;
private:
    // 親TransformCを設定
    void SetParent(TransformC *newParent, bool keepWorld = true);

    // 子TransformCを削除
    void RemoveChild(TransformC *child);

public:
    // 親TransformCを取得
    [[nodiscard]] TransformC *GetParent() const { return m_parent; }

    // 子TransformCを追加 keepWorldはワールド座標を保持するかどうか
    void AddChild(TransformC *child, bool keepWorld = true);
    void ForChilds(const std::function<void(TransformC *)> &func) const;
    [[nodiscard]] std::vector<TransformC *> GetChilds() const { return m_Childs; }
};
