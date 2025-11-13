//***************************************************************************************
// 定义一些实用的特效助手类
// Define utility effect helper classes.
//***************************************************************************************

#pragma once
#include "WinMin.h"
#include <string_view>
#include <memory>
#include <wrl/client.h>
#include <d3dcompiler.h>
#include "Property.h"

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11GeometryShader;
struct ID3D11ShaderResourceView;
struct ID3D11UnorderedAccessView;
struct ID3D11SamplerState;
struct ID3D11RasterizerState;
struct ID3D11DepthStencilState;
struct ID3D11BlendState;
//
// EffectHelper
//

// 渲染通道描述
// 通过指定添加着色器时提供的名字来设置着色器
//レンダリングパスの概要
// 指定された名前で追加されたシェーダーを使って、シェーダーを設定する
struct EffectPassDesc
{
    std::string_view nameVS;
    std::string_view nameDS;
    std::string_view nameHS;
    std::string_view nameGS;
    std::string_view namePS;
    std::string_view nameCS;
};

// 常量缓冲区的变量
// 非COM组件
// 定数バッファ内の変数
// COM コンポーネントではない
struct IEffectConstantBufferVariable
{
    // 设置无符号整数，也可以为bool设置\
    // 符号なし整数（unsigned int）を設定する。bool 値としても使用可能。
    virtual void SetUInt(uint32_t val) = 0;
    // 设置有符号整数
    // 符号付き整数（signed int）を設定する。
    virtual void SetSInt(int val) = 0;
    // 设置浮点数
    // 浮動小数点数（float）を設定する。
    virtual void SetFloat(float val) = 0;

    // 设置无符号整数向量，允许设置1个到4个分量
    // 着色器变量类型为bool也可以使用
    // 根据要设置的分量数来读取data的前几个分量
    // 设置无符号整数ベクトル。1から4のコンポーネントを設定可能。
    // シェーダー変数の型がboolの場合も使用可能。
    // dataの最初の数個のコンポーネントを読み取る。
    virtual void SetUIntVector(uint32_t numComponents, const uint32_t data[4]) = 0;

    // 设置有符号整数向量，允许设置1个到4个分量
    // 根据要设置的分量数来读取data的前几个分量
    // 设置符号付き整数ベクトル。1から4のコンポーネントを設定可能。
    // dataの最初の数個のコンポーネントを読み取る。
    virtual void SetSIntVector(uint32_t numComponents, const int data[4]) = 0;

    // 设置浮点数向量，允许设置1个到4个分量
    // 根据要设置的分量数来读取data的前几个分量
    // 浮動小数点数ベクトル。1から4のコンポーネントを設定可能。
    // dataの最初の数個のコンポーネントを読み取る。
    virtual void SetFloatVector(uint32_t numComponents, const float data[4]) = 0;

    // 设置无符号整数矩阵，允许行列数在1-4
    // 要求传入数据没有填充，例如3x3矩阵可以直接传入UINT[3][3]类型
    // 设置符号なし整数マトリックス。行数と列数は1から4まで許可。
    // データはパディングなしで渡す必要があります。例えば、3x3マトリックスは直接UINT[3][3]型で渡すことができます。
    virtual void SetUIntMatrix(uint32_t rows, uint32_t cols, const uint32_t* noPadData) = 0;

    // 设置有符号整数矩阵，允许行列数在1-4
    // 要求传入数据没有填充，例如3x3矩阵可以直接传入INT[3][3]类型
    // 设置符号付き整数マトリックス。行数と列数は1から4まで許可。
    // データはパディングなしで渡す必要があります。例えば、3x3マトリックスは直接INT[3][3]型で渡すことができます。
    virtual void SetSIntMatrix(uint32_t rows, uint32_t cols, const int* noPadData) = 0;

    // 设置浮点数矩阵，允许行列数在1-4
    // 要求传入数据没有填充，例如3x3矩阵可以直接传入FLOAT[3][3]类型
    // 浮動小数点数マトリックス。行数と列数は1から4まで許可。
    // データはパディングなしで渡す必要があります。例えば、3x3マトリックスは直接FLOAT[3][3]型で渡すことができます。
    virtual void SetFloatMatrix(uint32_t rows, uint32_t cols, const float* noPadData) = 0;

    // 设置其余类型，允许指定设置范围
    // その他の型を設定する。設定範囲を指定可能。
    virtual void SetRaw(const void* data, uint32_t byteOffset = 0, uint32_t byteCount = 0xFFFFFFFF) = 0;

    // 设置属性
    // 属性を設定する。
    virtual void Set(const Property& prop) = 0;

    // 获取最近一次设置的值，允许指定读取范围
    // 最近の設定値を取得する。読み取り範囲を指定可能。
    virtual HRESULT GetRaw(void* pOutput, uint32_t byteOffset = 0, uint32_t byteCount = 0xFFFFFFFF) = 0;

    virtual ~IEffectConstantBufferVariable() {}
};

// 渲染通道
// 非COM组件
// レンダリングパス
// COM コンポーネントではない
class EffectHelper;
struct IEffectPass
{
    // 设置光栅化状态
    // 设置ラスタライザーステート
    virtual void SetRasterizerState(ID3D11RasterizerState* pRS) = 0;
    // 设置混合状态
    // 设置ブレンドステート
    virtual void SetBlendState(ID3D11BlendState* pBS, const float blendFactor[4], uint32_t sampleMask) = 0;
    // 设置深度混合状态
    // 设置深度ステンシルステート
    virtual void SetDepthStencilState(ID3D11DepthStencilState* pDSS, uint32_t stencilValue) = 0;

    // 获取顶点着色器的uniform形参用于设置值
    // 頂点シェーダーのuniform変数を取得して値を設定する
    virtual std::shared_ptr<IEffectConstantBufferVariable> VSGetParamByName(std::string_view paramName) = 0;
    // 获取域着色器的uniform形参用于设置值
    // ドメインシェーダーのuniform変数を取得して値を設定する
    virtual std::shared_ptr<IEffectConstantBufferVariable> DSGetParamByName(std::string_view paramName) = 0;
    // 获取外壳着色器的uniform形参用于设置值
    // ハルシェーダーのuniform変数を取得して値を設定する
    virtual std::shared_ptr<IEffectConstantBufferVariable> HSGetParamByName(std::string_view paramName) = 0;
    // 获取几何着色器的uniform形参用于设置值
    // ジオメトリシェーダーのuniform変数を取得して値を設定する
    virtual std::shared_ptr<IEffectConstantBufferVariable> GSGetParamByName(std::string_view paramName) = 0;
    // 获取像素着色器的uniform形参用于设置值
    // ピクセルシェーダーのuniform変数を取得して値を設定する
    virtual std::shared_ptr<IEffectConstantBufferVariable> PSGetParamByName(std::string_view paramName) = 0;
    // 获取计算着色器的uniform形参用于设置值
    // コンピュートシェーダーのuniform変数を取得して値を設定する
    virtual std::shared_ptr<IEffectConstantBufferVariable> CSGetParamByName(std::string_view paramName) = 0;
    // 获取所属特效助理
    // 特効ヘルパーを取得する
    virtual EffectHelper* GetEffectHelper() = 0;
    // 获取特效名
    // 特効名を取得する
    virtual const std::string& GetPassName() = 0;
    
    // 应用着色器、常量缓冲区(包括函数形参)、采样器、着色器资源和可读写资源到渲染管线
    // シェーダー、定数バッファ（関数形参を含む）、サンプラー、シェーダーリソース、および読み書き可能なリソースをレンダーパイプラインに適用する
    virtual void Apply(ID3D11DeviceContext* deviceContext) = 0;

    // 调度计算着色器
    // 传入线程数目，内部会根据计算着色器的线程组维度调用合适的线程组数目
    // コンピュートシェーダーをディスパッチする。
    // スレッド数を渡すと、内部でコンピュートシェーダーのスレッドグループの次元に基づいて適切なスレッドグループ数を呼び出します。
    virtual void Dispatch(ID3D11DeviceContext* deviceContext, uint32_t threadX = 1, uint32_t threadY = 1, uint32_t threadZ = 1) = 0;

    virtual ~IEffectPass() {};
};

// 特效助理
// 负责管理着色器、采样器、着色器资源、常量缓冲区、着色器形参、可读写资源、渲染状态
// 特効ヘルパー
// シェーダー、サンプラー、シェーダーリソース、定数バッファ、シェーダー形参、読み書き可能なリソース、およびレンダーステートを管理する
class EffectHelper
{
public:

    EffectHelper();
    ~EffectHelper();
    // 不允许拷贝，允许移动
    // コッピーを禁止し、ムーブを許可
    EffectHelper(const EffectHelper&) = delete;
    EffectHelper& operator=(const EffectHelper&) = delete;
    EffectHelper(EffectHelper&&) = default;
    EffectHelper& operator=(EffectHelper&&) = default;

    // 设置编译好的着色器文件缓存路径并创建
    // 若设置为""，则关闭缓存
    // 若forceWrite为true，每次运行程序都会强制覆盖保存
    // 默认情况下不会缓存编译好的着色器
    // 在shader没完成修改的时候应开启forceWrite
    // コンパイル済みシェーダーファイルのキャッシュパスを設定して作成する
    // ""（空文字列）を指定すると、キャッシュは無効になる
    // forceWrite が true の場合、毎回プログラム実行時にキャッシュを強制的に上書き保存する
    // デフォルトでは、コンパイル済みのシェーダーはキャッシュされない
    // シェーダーの編集が完了していない場合は、forceWrite を有効にしておくべき
    void SetBinaryCacheDirectory(std::wstring_view cacheDir, bool forceWrite = false);

    // 编译着色器 或 读取着色器字节码，按下述顺序：
    // 1. 如果开启着色器字节码文件缓存路径 且 关闭强制覆盖，则优先尝试读取${cacheDir}/${shaderName}.cso并添加
    // 2. 否则读取filename。若为着色器字节码，直接添加
    // 3. 若filename为hlsl源码，则进行编译和添加。开启着色器字节码文件缓存会保存着色器字节码到${cacheDir}/${shaderName}.cso
    // 注意：
    // 1. 不同着色器代码，若常量缓冲区使用同一个槽，对应的定义应保持完全一致
    // 2. 不同着色器代码，若存在全局变量，定义应保持完全一致
    // 3. 不同着色器代码，若采样器、着色器资源或可读写资源使用同一个槽，对应的定义应保持完全一致，否则只能使用按槽设置
    // シェーダーをコンパイルまたはシェーダーバイトコードを読み込む。以下の順序に従う：
    // 1. シェーダーバイトコードのキャッシュパスが有効で、forceWrite が無効な場合：
    //    ${cacheDir}/${shaderName}.cso を優先的に読み込み、追加する
    // 2. それ以外の場合は filename を読み込む。filename がシェーダーバイトコードであれば、そのまま追加
    // 3. filename が HLSL ソースコードであれば、コンパイルして追加する。
    //    シェーダーバイトコードのキャッシュが有効な場合は、${cacheDir}/${shaderName}.cso に保存される
    //
    // 注意：
    // 1. 異なるシェーダーコード間で、同じスロットを使う定数バッファがある場合、定義は完全に一致させる必要がある
    // 2. 異なるシェーダーコード間で、グローバル変数が存在する場合、その定義は完全に一致させる必要がある
    // 3. 異なるシェーダーコード間で、同じスロットを使うサンプラー、シェーダーリソース、RWリソースがある場合、
    //    定義は完全に一致させる必要がある。そうでない場合は、スロット単位で設定する必要がある
    HRESULT CreateShaderFromFile(std::string_view shaderName, std::wstring_view filename, ID3D11Device* device,
        LPCSTR entryPoint = nullptr, LPCSTR shaderModel = nullptr, const D3D_SHADER_MACRO* pDefines = nullptr, ID3DBlob** ppShaderByteCode = nullptr);

    // 仅编译着色器
    //シェーダーだけをコンパイルする
    static HRESULT CompileShaderFromFile(std::wstring_view filename, LPCSTR entryPoint, LPCSTR shaderModel, ID3DBlob** ppShaderByteCode, ID3DBlob** ppErrorBlob = nullptr,
        const D3D_SHADER_MACRO* pDefines = nullptr, ID3DInclude* pInclude = D3D_COMPILE_STANDARD_FILE_INCLUDE);

    // 添加编译好的着色器二进制信息并为其设置标识名
    // 该函数不会保存着色器二进制编码到文件
    // 注意：
    // 1. 不同着色器代码，若常量缓冲区使用同一个register，对应的定义应保持完全一致
    // 2. 不同着色器代码，若存在全局变量，定义应保持完全一致
    // 3. 不同着色器代码，若采样器、着色器资源或可读写资源使用同一个槽，对应的定义应保持完全一致
    // コンパイル済みのシェーダーバイナリ情報を追加し、それに識別名を設定する
    // この関数はシェーダーバイナリをファイルに保存しない
    //
    // 注意：
    // 1. 異なるシェーダーコード間で、同じ register を使用する定数バッファがある場合、定義は完全に一致させる必要がある
    // 2. 異なるシェーダーコード間で、グローバル変数が存在する場合、その定義は完全に一致させる必要がある
    // 3. 異なるシェーダーコード間で、同じスロットを使うサンプラー、シェーダーリソース、RWリソースがある場合、
    //    定義は完全に一致させる必要がある
    HRESULT AddShader(std::string_view name, ID3D11Device* device, ID3DBlob* blob);

    // 添加带流输出的几何着色器并为其设置标识名
    // 该函数不会保存着色器二进制编码到文件
    // 注意：
    // 1. 不同着色器代码，若常量缓冲区使用同一个槽，对应的定义应保持完全一致
    // 2. 不同着色器代码，若存在全局变量，定义应保持完全一致
    // 3. 不同着色器代码，若采样器、着色器资源或可读写资源使用同一个槽，对应的定义应保持完全一致，否则只能使用按槽设置
    //
    // ストリーム出力付きのジオメトリシェーダーを追加し、それに識別名を設定する
    // この関数はシェーダーバイナリをファイルに保存しない
    // 注意：
    // 1. 異なるシェーダーコード間で、同じスロットを使用する定数バッファがある場合、定義は完全に一致させる必要がある
    // 2. 異なるシェーダーコード間で、グローバル変数が存在する場合、その定義は完全に一致させる必要がある
    // 3. 異なるシェーダーコード間で、同じスロットを使用するサンプラー、シェーダーリソース、RWリソースがある場合、
    //    定義は完全に一致させる必要がある。そうでない場合は、スロット単位での設定のみが可能
    HRESULT AddGeometryShaderWithStreamOutput(std::string_view name, ID3D11Device* device, ID3D11GeometryShader* gsWithSO, ID3DBlob* blob);

    // 清空所有内容
    // すべての内容をクリアする
    void Clear();

    // 创建渲染通道
    // 渲染パスを作成する
    HRESULT AddEffectPass(std::string_view effectPassName, ID3D11Device* device, const EffectPassDesc* pDesc);
    // 获取特定渲染通道
    // 特定のレンダリングパスを取得する
    std::shared_ptr<IEffectPass> GetEffectPass(std::string_view effectPassName);

    // 获取常量缓冲区的变量用于设置值
    // 定数バッファの変数を取得して値を設定する
    std::shared_ptr<IEffectConstantBufferVariable> GetConstantBufferVariable(std::string_view name);

    // 按槽设置采样器状态
    // スロットごとにサンプラーステートを設定する
    void SetSamplerStateBySlot(uint32_t slot, ID3D11SamplerState* samplerState);
    // 按名设置采样器状态(若存在同槽多名称则只能使用按槽设置)
    // 名前でサンプラーステートを設定する（同じスロットに複数の名前がある場合は、スロットごとの設定のみ使用可能）
    void SetSamplerStateByName(std::string_view name, ID3D11SamplerState* samplerState);
    // 按名映射采样器状态槽(找不到返回-1)
    // 名前でサンプラーステートのスロットをマッピングする（見つからない場合は-1を返す）
    int MapSamplerStateSlot(std::string_view name);

    // 按槽设置着色器资源
    // スロットごとにシェーダーリソースを設定する
    void SetShaderResourceBySlot(uint32_t slot, ID3D11ShaderResourceView* srv);
    // 按名设置着色器资源(若存在同槽多名称则只能使用按槽设置)
    // 名前でシェーダーリソースを設定する（同じスロットに複数の名前がある場合は、スロットごとの設定のみ使用可能）
    void SetShaderResourceByName(std::string_view name, ID3D11ShaderResourceView* srv);
    // 按名映射着色器资源槽(找不到返回-1)
    // 名前でシェーダーリソースのスロットをマッピングする（見つからない場合は-1を返す）
    int MapShaderResourceSlot(std::string_view name);

    // 按槽设置可读写资源
    // スロットごとに読み書き可能なリソースを設定する
    void SetUnorderedAccessBySlot(uint32_t slot, ID3D11UnorderedAccessView* uav, uint32_t* pInitialCount = nullptr);
    // 按名设置可读写资源(若存在同槽多名称则只能使用按槽设置)
    // 名前で読み書き可能なリソースを設定する（同じスロットに複数の名前がある場合は、スロットごとの設定のみ使用可能）
    void SetUnorderedAccessByName(std::string_view name, ID3D11UnorderedAccessView* uav, uint32_t* pInitialCount = nullptr);
    // 按名映射可读写资源槽(找不到返回-1)
    // 名前で読み書き可能なリソースのスロットをマッピングする（見つからない場合は-1を返す）
    int MapUnorderedAccessSlot(std::string_view name);


    // 设置调试对象名
    // デバッグオブジェクト名を設定する
    void SetDebugObjectName(std::string name);

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};