#include "GameApp.h"
#include "Game.h"
#include  "Global.h"

Game* halgame;
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
                   _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
    // これらのパラメータは使用しません。
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nShowCmd);
    // Debug ビルドでの実行時メモリ割り当てとリーク検出を許可する
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
    // ゲームインスタンスを作成します
    halgame = new Game(hInstance, L"HalGame", 1280, 720);
    // ゲームインスタンスを初期化します
    if( !halgame->Init() ) {
        return 0;
    }
    // ゲームを実行します
    return halgame->Run();
}
