#pragma once

#include "d3dApp.h"
#include "Effects.h"
#include <PrimitiveBatch.h>
#include <VertexTypes.h>
#include <Effects.h>
#include <vector>

#include "Camera.h"
#include "ImageObject.h"
#include "ModelManager.h"
#include "ModelObject.h"
#include "SoAStorage.h"
#include "Texture2D.h"
#include "TextureManager.h"


class GameApp : public D3DApp
{
private:
    TextureManager m_TextureManager;                     // テクスチャマネージャー
    ModelManager m_ModelManager;                         // モデルマネージャー
    BaseEffect m_BaseEffect;                           // オブジェクト描画用エフェクト管理
    std::unique_ptr<Depth2D> m_pDepthTexture;            // デプスバッファ
    SoAStorage<ModelObject> m_ModelObjectStorage;        // モデルオブジェクトのストレージ
    SoAStorage<ImageObject> m_ImageObjectStorage;        // 画像オブジェクトのストレージ
    std::unique_ptr<DirectX::SpriteBatch> m_SpriteBatch; // スプライト描画用
    std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>> m_PrimitiveBatch; // プリミティブ描画用
    std::unique_ptr<DirectX::BasicEffect> m_PrimitiveEffect; // ライン描画用エフェクト
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_PrimitiveInputLayout; // 入力レイアウト

    struct LinePrimitive {
        DirectX::XMFLOAT3 start;
        DirectX::XMFLOAT3 end;
        DirectX::XMFLOAT4 color;
    };

    struct TrianglePrimitive {
        DirectX::XMFLOAT3 v1;
        DirectX::XMFLOAT3 v2;
        DirectX::XMFLOAT3 v3;
        DirectX::XMFLOAT4 color;
    };

    std::vector<LinePrimitive> m_LineQueue;          // 1フレーム分のライン描画キュー
    std::vector<TrianglePrimitive> m_TriangleQueue;  // 1フレーム分の三角形描画キュー
protected:
    bool Init() override;                                // 初期化処理
    void OnResize() override;                            // リサイズ時の処理
    void PreUpdate(float dt) override;                   // 更新前処理
    void Update(float dt) override;                      // 更新処理
    void PostUpdate(float dt) override;                  // 更新後処理
    void Draw() override;                                // 描画処理

public:
    GameApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight);
    ~GameApp() override;
    ID3D11Device *GetDevice() const ;
    // デバイスコンテキストを取得
    ID3D11DeviceContext *GetDeviceContext() const;
    std::shared_ptr<ThirdPersonCamera> m_pCamera;        // サードパーソンカメラ

    UID AddModel(std::string_view filename);       // モデルの追加
    ModelObject* GetModelObject(UID handle);       // モデルオブジェクトの取得
    bool RemoveModel(UID handle);                  // モデルの削除

    UID AddImage(std::string_view filename);       // 画像の追加
    ImageObject* GetImageObject(UID handle);       // 画像オブジェクトの取得
    bool RemoveImage(UID handle);                  // 画像の削除

    // 描画用のラインを追加
    void DrawLine(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, const DirectX::XMFLOAT4& color);
    void DrawTriangle(const DirectX::XMFLOAT3& v1, const DirectX::XMFLOAT3& v2, const DirectX::XMFLOAT3& v3, const DirectX::XMFLOAT4& color) ;
    // 画面座標に変換
    ImVec2 ProjectToScreen(const DirectX::XMFLOAT3 &p);
};
