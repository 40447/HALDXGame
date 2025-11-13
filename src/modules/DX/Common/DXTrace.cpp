#include "DXTrace.h"
#include <cstdio>

HRESULT WINAPI DXTraceW(_In_z_ const WCHAR* strFile, _In_ DWORD dwLine, _In_ HRESULT hr,
    _In_opt_ const WCHAR* strMsg, _In_ bool bPopMsgBox)
{
    WCHAR strBufferFile[MAX_PATH];
    WCHAR strBufferLine[128];
    WCHAR strBufferError[300];
    WCHAR strBufferMsg[1024];
    WCHAR strBufferHR[40];
    WCHAR strBuffer[3000];

    swprintf_s(strBufferLine, 128, L"%lu", dwLine);
    if (strFile)
    {
        swprintf_s(strBuffer, 3000, L"%ls(%ls): ", strFile, strBufferLine);
        OutputDebugStringW(strBuffer);
    }

    size_t nMsgLen = (strMsg) ? wcsnlen_s(strMsg, 1024) : 0;
    if (nMsgLen > 0)
    {
        OutputDebugStringW(strMsg);
        OutputDebugStringW(L" ");
    }
    // Windows SDK 8.0起DirectX的错误信息已经集成进错误码中，可以通过FormatMessageW获取错误信息字符串
    // 不需要分配字符串内存
    //Windows SDK 8.0から DirectXのエラーメッセージはエラーコードに統合されており、FormatMessageWを使用してエラーメッセージ文字列を取得できます。
    // 文字列のメモリを割り当てる必要はありません
    FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        strBufferError, 256, nullptr);

    WCHAR* errorStr = wcsrchr(strBufferError, L'\r');
    if (errorStr)
    {
        // 擦除FormatMessageW带来的换行符(把\r\n的\r置换为\0即可)
        // FormatMessageWによって追加された改行文字を削除します（\r\nの\rを\0に置き換えるだけです）
        errorStr[0] = L'\0';
    }

    swprintf_s(strBufferHR, 40, L" (0x%0.8x)", hr);
    wcscat_s(strBufferError, strBufferHR);
    swprintf_s(strBuffer, 3000, L"错误码含义：%ls", strBufferError);
    OutputDebugStringW(strBuffer);

    OutputDebugStringW(L"\n");

    if (bPopMsgBox)
    {
        wcscpy_s(strBufferFile, MAX_PATH, L"");
        if (strFile)
            wcscpy_s(strBufferFile, MAX_PATH, strFile);

        wcscpy_s(strBufferMsg, 1024, L"");
        if (nMsgLen > 0)
            swprintf_s(strBufferMsg, 1024, L"現在の呼び出し：%ls\n", strMsg);

        swprintf_s(strBuffer, 3000, L"ファイル名：%ls\n行番号：%ls\nエラーコードの意味：%ls\n%lsこのアプリケーションのデバッグを行いますか？",
            strBufferFile, strBufferLine, strBufferError, strBufferMsg);

        int nResult = MessageBoxW(GetForegroundWindow(), strBuffer, L"エラー", MB_YESNO | MB_ICONERROR);
        if (nResult == IDYES)
            DebugBreak();
    }

    return hr;
}