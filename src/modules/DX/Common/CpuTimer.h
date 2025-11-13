//***************************************************************************************
// CpuTimer.h by Frank Luna (C) 2011 All Rights Reserved.
// Modify name from GameTimer.cpp
// CPU計測タイマー
//***************************************************************************************

#pragma once

class CpuTimer
{
public:
    CpuTimer();

    float TotalTime()const;     // Reset() 呼び出し後の経過時間（停止中の時間は含まない）を返す
    float DeltaTime()const;     // フレーム間の経過時間を返す

    void Reset();               // 計測を開始する前、またはリセットが必要なときに呼び出す
    void Start();               // 計測を開始または一時停止を解除するときに呼び出す
    void Stop();                // 一時停止が必要なときに呼び出す
    void Tick();                // 毎フレームの冒頭で呼び出す
    bool IsStopped() const;     // タイマーが一時停止中または終了しているかを確認する

private:
    double m_SecondsPerCount = 0.0;
    double m_DeltaTime = -1.0;

    __int64 m_BaseTime = 0;
    __int64 m_PausedTime = 0;
    __int64 m_StopTime = 0;
    __int64 m_PrevTime = 0;
    __int64 m_CurrTime = 0;

    bool m_Stopped = false;
};