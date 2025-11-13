#include "GameApp.h"
#include <SpriteBatch.h>

#include "DXTrace.h"
#include "RenderStates.h"

using namespace DirectX;

GameApp::GameApp(HINSTANCE hInstance, const std::wstring &windowName, int initWidth, int initHeight)
    : D3DApp(hInstance, windowName, initWidth, initHeight) {
}

GameApp::~GameApp() = default;

ID3D11Device * GameApp::GetDevice() const {
    return  m_pd3dDevice1 ? m_pd3dDevice1.Get() : m_pd3dDevice.Get();
}

ID3D11DeviceContext * GameApp::GetDeviceContext() const {
    return  m_pd3dImmediateContext1 ? m_pd3dImmediateContext1.Get() : m_pd3dImmediateContext.Get();
}

bool GameApp::Init() {
    if (!D3DApp::Init()) {
        return false;
    }

    m_TextureManager.Init(m_pd3dDevice.Get());
    m_ModelManager.Init(m_pd3dDevice.Get());

    // 必ず最初にすべてのレンダリングステートを初期化し、以下のエフェクトで使用できるようにする
    RenderStates::InitAll(m_pd3dDevice.Get());

    if (!m_BaseEffect.InitAll(m_pd3dDevice.Get()))
        return false;

    // Primitive batch for simple geometry
    m_PrimitiveBatch = std::make_unique<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>>(m_pd3dImmediateContext.Get());
    m_PrimitiveEffect = std::make_unique<DirectX::BasicEffect>(m_pd3dDevice.Get());
    m_PrimitiveEffect->SetVertexColorEnabled(true);
    const void* shaderCode = nullptr;
    size_t codeLength = 0;
    m_PrimitiveEffect->GetVertexShaderBytecode(&shaderCode, &codeLength);
    HR(m_pd3dDevice->CreateInputLayout(
            DirectX::VertexPositionColor::InputElements,
            DirectX::VertexPositionColor::InputElementCount,
            shaderCode, codeLength,
            m_PrimitiveInputLayout.ReleaseAndGetAddressOf()));

    m_SpriteBatch = std::make_unique<DirectX::SpriteBatch>(m_pd3dImmediateContext.Get());

    // ******************
    // カメラの初期化
    //

    auto camera = std::make_shared<ThirdPersonCamera>();
    m_pCamera = camera;

    camera->SetViewPort(0.0f, 0.0f, (float) m_ClientWidth, (float) m_ClientHeight);
    camera->SetTarget(XMFLOAT3(0.0f, 0.5f, 0.0f));
    camera->SetDistance(15.0f);
    camera->SetDistanceMinMax(6.0f, 100.0f);
    camera->SetRotationX(XM_PIDIV4);
    camera->SetFrustum(XM_PI / 3, AspectRatio(), 1.0f, 1000.0f);

    m_BaseEffect.SetWorldMatrix(XMMatrixIdentity());
    m_BaseEffect.SetViewMatrix(camera->GetViewMatrixXM());
    m_BaseEffect.SetProjMatrix(camera->GetProjMatrixXM());
    m_BaseEffect.SetEyePos(camera->GetPosition());

    // ******************
    // 変更されない値の初期化
    //

    // 環境光（ディレクショナルライト）
    DirectionalLight dirLight{};
    dirLight.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    dirLight.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    dirLight.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    dirLight.direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
    m_BaseEffect.SetDirLight(0, dirLight);

    // ポイントライト（点光源）
    PointLight pointLight{};
    pointLight.position = XMFLOAT3(0.0f, 20.0f, 0.0f);
    pointLight.ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
    pointLight.diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
    pointLight.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    pointLight.att = XMFLOAT3(0.0f, 0.1f, 0.0f);
    pointLight.range = 30.0f;
    m_BaseEffect.SetPointLight(0, pointLight);

    // ******************
    // ゲームオブジェクトの初期化（サブクラスに委ねる）
    //

    return true;
}

void GameApp::PreUpdate(float dt) {
    // フレーム更新前の処理（必要に応じて実装）
}

void GameApp::Update(float dt) {
    // 毎フレームの更新処理（必要に応じて実装）
}

void GameApp::PostUpdate(float dt) {
    // フレーム更新後にビュー行列とカメラ位置を更新
    m_BaseEffect.SetViewMatrix(m_pCamera->GetViewMatrixXM());
    m_BaseEffect.SetEyePos(m_pCamera->GetPosition());
}

void GameApp::Draw() {
    // バックバッファのレンダーターゲットビューを作成
    if (m_FrameCount < m_BackBufferCount) {
        ComPtr<ID3D11Texture2D> pBackBuffer;
        m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(pBackBuffer.GetAddressOf()));
        CD3D11_RENDER_TARGET_VIEW_DESC rtvDesc(D3D11_RTV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
        m_pd3dDevice->CreateRenderTargetView(pBackBuffer.Get(), &rtvDesc,
                                             m_pRenderTargetViews[m_FrameCount].ReleaseAndGetAddressOf());
    }

    // 画面のクリア
    float black[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    m_pd3dImmediateContext->ClearRenderTargetView(GetBackBufferRTV(), black);
    m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthTexture->GetDepthStencil(),
                                                  D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    ID3D11RenderTargetView *pRTVs[1] = {GetBackBufferRTV()};
    m_pd3dImmediateContext->OMSetRenderTargets(1, pRTVs, m_pDepthTexture->GetDepthStencil());
    D3D11_VIEWPORT viewport = m_pCamera->GetViewPort();
    m_pd3dImmediateContext->RSSetViewports(1, &viewport);

    // 描画設定の適用
    m_BaseEffect.SetRenderDefault();

    // 使用刷新队列管理游戏所有的模型对象
    // リフレッシュキューを使用してゲーム内のすべてのモデルオブジェクトを管理します
    m_ModelObjectStorage.ForEachActive([this](UID id, ModelObject *modelObject) {
        modelObject->Draw(m_pd3dImmediateContext.Get(), m_BaseEffect);
    });

    // プリミティブ描画
    if (m_PrimitiveBatch && (!m_LineQueue.empty() || !m_TriangleQueue.empty())) {
        m_PrimitiveEffect->SetWorld(DirectX::XMMatrixIdentity());
        m_PrimitiveEffect->SetView(m_pCamera->GetViewMatrixXM());
        m_PrimitiveEffect->SetProjection(m_pCamera->GetProjMatrixXM());
        m_PrimitiveEffect->Apply(m_pd3dImmediateContext.Get());
        m_pd3dImmediateContext->IASetInputLayout(m_PrimitiveInputLayout.Get());

        m_PrimitiveBatch->Begin();
        for (const auto& line : m_LineQueue)
            m_PrimitiveBatch->DrawLine(
                DirectX::VertexPositionColor(line.start, line.color),
                DirectX::VertexPositionColor(line.end, line.color));
        for (const auto& tri : m_TriangleQueue)
            m_PrimitiveBatch->DrawTriangle(
                DirectX::VertexPositionColor(tri.v1, tri.color),
                DirectX::VertexPositionColor(tri.v2, tri.color),
                DirectX::VertexPositionColor(tri.v3, tri.color));
        m_PrimitiveBatch->End();

        m_LineQueue.clear();
        m_TriangleQueue.clear();
    }

    if (m_SpriteBatch) {
        m_SpriteBatch->Begin();
        m_ImageObjectStorage.ForEachActive([this](UID, ImageObject* img){
            img->Draw(m_SpriteBatch.get());
        });
        m_SpriteBatch->End();
    }

    // ImGuiの描画
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    // フレームの表示
    HR(m_pSwapChain->Present(0, m_IsDxgiFlipModel ? DXGI_PRESENT_ALLOW_TEARING : 0));
}

// ウィンドウサイズが変更された際に呼び出される関数
void GameApp::OnResize() {
    D3DApp::OnResize();

    m_pDepthTexture = std::make_unique<Depth2D>(m_pd3dDevice.Get(), m_ClientWidth, m_ClientHeight);
    m_pDepthTexture->SetDebugObjectName("DepthTexture");

    // カメラの視野更新
    if (m_pCamera != nullptr) {
        m_pCamera->SetFrustum(XM_PI / 3, AspectRatio(), 1.0f, 1000.0f);
        m_pCamera->SetViewPort(0.0f, 0.0f, (float) m_ClientWidth, (float) m_ClientHeight);
        m_BaseEffect.SetProjMatrix(m_pCamera->GetProjMatrixXM());
    }
}

////////////////////////////新しいコード/////////////////////////////////////
// モデルを追加
UID GameApp::AddModel(std::string_view filename) {
    Model *model = m_ModelManager.GetModel(filename);
    if (model == nullptr) {
        model = m_ModelManager.CreateFromFile(filename);
    }
    std::unique_ptr<ModelObject> pModelObject = std::make_unique<ModelObject>();
    pModelObject->SetModel(model);
    return m_ModelObjectStorage.Add(std::move(pModelObject));
}

// モデルオブジェクトの取得
ModelObject *GameApp::GetModelObject(UID handle) {
    return m_ModelObjectStorage.Get(handle);
}

// モデルの削除
bool GameApp::RemoveModel(UID handle) {
    m_ModelObjectStorage.Remove(handle);
    return true;
}

UID GameApp::AddImage(std::string_view filename) {
    ID3D11ShaderResourceView* tex = m_TextureManager.GetTexture(filename);
    if (tex == nullptr) {
        tex = m_TextureManager.CreateFromFile(filename);
    }
    auto img = std::make_unique<ImageObject>();
    img->SetTexture(tex);
    return m_ImageObjectStorage.Add(std::move(img));
}

ImageObject* GameApp::GetImageObject(UID handle) {
    return m_ImageObjectStorage.Get(handle);
}

bool GameApp::RemoveImage(UID handle) {
    m_ImageObjectStorage.Remove(handle);
    return true;
}

void GameApp::DrawLine(const DirectX::XMFLOAT3 &start, const DirectX::XMFLOAT3 &end, const DirectX::XMFLOAT4 &color) {
    m_LineQueue.push_back({start, end, color});
}

void GameApp::DrawTriangle(const DirectX::XMFLOAT3 &v1, const DirectX::XMFLOAT3 &v2, const DirectX::XMFLOAT3 &v3,
    const DirectX::XMFLOAT4 &color) {
    m_TriangleQueue.push_back({v1, v2, v3, color});
}

ImVec2 GameApp::ProjectToScreen(const DirectX::XMFLOAT3& p) {
    XMMATRIX vp = m_pCamera->GetViewProjMatrixXM();
    D3D11_VIEWPORT view = m_pCamera->GetViewPort();

    XMVECTOR pos = XMVectorSet(p.x, p.y, p.z, 1.0f);
    XMVECTOR clip = XMVector4Transform(pos, vp);

    float cx = XMVectorGetX(clip);
    float cy = XMVectorGetY(clip);
    float cz = XMVectorGetZ(clip);
    float cw = XMVectorGetW(clip);

    // 检查是否在可见范围
    if (cw <= 0.0f) return { false, {} };
    if (cx < -cw || cx > cw || cy < -cw || cy > cw || cz < 0.0f || cz > cw)
        return { false, {} };

    // NDC
    float x = cx / cw * 0.5f + 0.5f;
    float y = cy / cw * 0.5f + 0.5f;

    ImVec2 result;
    result.x = view.TopLeftX + x * view.Width;
    result.y = view.TopLeftY + (1.0f - y) * view.Height;
    return  result;
}