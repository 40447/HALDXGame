//***************************************************************************************
// 一人称（自由視点）および三人称カメラを提供するクラス
//***************************************************************************************

#pragma once

#include "WinMin.h"
#include <d3d11_1.h>
#include <DirectXMath.h>
#include "Transform.h"


class Camera
{
public:
    Camera() = default;
    virtual ~Camera() = 0;

    //
    // カメラの位置を取得する
    //

    DirectX::XMVECTOR GetPositionXM() const;
    DirectX::XMFLOAT3 GetPosition() const;

    //
    // カメラの回転を取得する
    //

    // X軸回りのオイラー角（ラジアン）を取得
    float GetRotationX() const;
    // Y軸回りのオイラー角（ラジアン）を取得
    float GetRotationY() const;

    //
    // カメラの軸ベクトルを取得する
    //

    DirectX::XMVECTOR GetRightAxisXM() const;
    DirectX::XMFLOAT3 GetRightAxis() const;
    DirectX::XMVECTOR GetUpAxisXM() const;
    DirectX::XMFLOAT3 GetUpAxis() const;
    DirectX::XMVECTOR GetLookAxisXM() const;
    DirectX::XMFLOAT3 GetLookAxis() const;

    //
    // 各種行列を取得する
    //

    DirectX::XMMATRIX GetLocalToWorldMatrixXM() const;
    DirectX::XMMATRIX GetViewMatrixXM() const;
    DirectX::XMMATRIX GetProjMatrixXM(bool reversedZ = false) const;
    DirectX::XMMATRIX GetViewProjMatrixXM(bool reversedZ = false) const;

    // ビューポートを取得
    D3D11_VIEWPORT GetViewPort() const;

    float GetNearZ() const;
    float GetFarZ() const;
    float GetFovY() const;
    float GetAspectRatio() const;

    // 視錐台を設定する
    void SetFrustum(float fovY, float aspect, float nearZ, float farZ);

    // ビューポートを設定する
    void SetViewPort(const D3D11_VIEWPORT& viewPort);
    void SetViewPort(float topLeftX, float topLeftY, float width, float height, float minDepth = 0.0f, float maxDepth = 1.0f);

protected:

    // カメラの変換（位置、回転）
    Transform m_Transform = {};

    // 視錐台のパラメータ
    float m_NearZ = 0.0f;
    float m_FarZ = 0.0f;
    float m_Aspect = 0.0f;
    float m_FovY = 0.0f;

    // 現在のビューポート
    D3D11_VIEWPORT m_ViewPort = {};

};

class FirstPersonCamera : public Camera
{
public:
    FirstPersonCamera() = default;
    ~FirstPersonCamera() override;

    // カメラの位置を設定する
    void SetPosition(float x, float y, float z);
    void SetPosition(const DirectX::XMFLOAT3& pos);
    // カメラの方向を設定する
    void LookAt(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& target,const DirectX::XMFLOAT3& up);
    void LookTo(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& to, const DirectX::XMFLOAT3& up);
    // 横移動（平行移動）
    void Strafe(float d);
    // 前進（XZ平面での移動）
    void Walk(float d);
    // 前方移動（Z軸方向への移動）
    void MoveForward(float d);
    // 任意方向への移動
    void Translate(const DirectX::XMFLOAT3& dir, float magnitude);
    // 上下方向の視線移動
    // 正のrad値で上向き、負のrad値で下向き
    void Pitch(float rad);
    // 左右方向の視線移動
    // 正のrad値で右回転、負のrad値で左回転
    void RotateY(float rad);
};

class ThirdPersonCamera : public Camera
{
public:
    ThirdPersonCamera() = default;
    ~ThirdPersonCamera() override;

    // 現在追跡中の対象の位置を取得する
    DirectX::XMFLOAT3 GetTargetPosition() const;
    // 対象との距離を取得する
    float GetDistance() const;
    // 対象を中心に垂直方向に回転する（X軸回転のオイラー角は[0, π/3]に制限）
    void RotateX(float rad);
    // 対象を中心に水平方向に回転する
    void RotateY(float rad);
    // 対象に近づく
    void Approach(float dist);
    // 初期のX軸回転角度を設定（[0, π/3]に制限）
    void SetRotationX(float rad);
    // 初期のY軸回転角度を設定
    void SetRotationY(float rad);
    // 追跡対象の位置を設定し、バインドする
    void SetTarget(const DirectX::XMFLOAT3& target);
    // 初期の距離を設定
    void SetDistance(float dist);
    // 最小・最大の許容距離を設定
    void SetDistanceMinMax(float minDist, float maxDist);

private:
    // 対象の位置
    DirectX::XMFLOAT3 m_Target = {};
    // 現在の距離
    float m_Distance = 0.0f;
    // 最小・最大の許容距離
    float m_MinDist = 0.0f, m_MaxDist = 0.0f;
};
