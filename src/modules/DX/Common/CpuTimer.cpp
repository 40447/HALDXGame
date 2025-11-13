#include "WinMin.h"
#include "CpuTimer.h"

CpuTimer::CpuTimer()
{
    __int64 countsPerSec{};
    QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
    m_SecondsPerCount = 1.0 / (double)countsPerSec;
}


float CpuTimer::TotalTime()const
{
    /////////////////////中国語////////////////////////////////
    // 如果调用了Stop()，暂停中的这段时间我们不需要计入。此外
    // m_StopTime - m_BaseTime可能会包含之前的暂停时间，为
    // 此我们可以从m_StopTime减去之前累积的暂停的时间
    //
    //                     |<-- 暂停的时间 -->|
    // ----*---------------*-----------------*------------*------------*------> time
    //  m_BaseTime       m_StopTime        startTime     m_StopTime    m_CurrTime
    ////////////////////////日本語/////////////////////////////////
    // Stop() が呼び出された場合、その間の一時停止時間は計測時間に含めない。
    // また、m_StopTime - m_BaseTime は過去の一時停止時間を含んでいる可能性があるため、
    // それを差し引く必要がある。
    //
    //                     ｜<-- 一時停止時間 -->｜
    // ----*---------------*-----------------*------------*------------*------> time
    //  m_BaseTime       m_StopTime        startTime     m_StopTime    m_CurrTime


    if( m_Stopped )
    {
        return (float)(((m_StopTime - m_PausedTime)-m_BaseTime)*m_SecondsPerCount);
    }
    ////////////////////中国語////////////////////////////////
    // m_CurrTime - m_BaseTime包含暂停时间，但我们不想将它计入。
    // 为此我们可以从m_CurrTime减去之前累积的暂停的时间
    //
    //  (m_CurrTime - m_PausedTime) - m_BaseTime 
    //
    //                     |<-- 暂停的时间 -->|
    // ----*---------------*-----------------*------------*------> time
    //  m_BaseTime       m_StopTime        startTime     m_CurrTime
    ////////////////////////日本語/////////////////////////////////
    // m_CurrTime - m_BaseTime は一時停止時間を含んでいるが、これを計測時間に含めたくない。
    // そのため、m_CurrTime から累積された一時停止時間を差し引く必要がある。
    //
    //  (m_CurrTime - m_PausedTime) - m_BaseTime
    //
    //                     ｜<-- 一時停止時間 -->｜
    // ----*---------------*-----------------*------------*------> time
    //  m_BaseTime       m_StopTime        startTime     m_CurrTime

    else
    {
        return (float)(((m_CurrTime-m_PausedTime)-m_BaseTime)*m_SecondsPerCount);
    }
}

float CpuTimer::DeltaTime()const
{
    return (float)m_DeltaTime;
}

void CpuTimer::Reset()
{
    __int64 currTime{};
    QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

    m_BaseTime = currTime;
    m_PrevTime = currTime;
    m_StopTime = 0;
    m_PausedTime = 0;   // 複数回 Reset を行う場合は、これを 0 にリセットする必要がある
    m_Stopped  = false;
}

void CpuTimer::Start()
{
    __int64 startTime{};
    QueryPerformanceCounter((LARGE_INTEGER*)&startTime);


    // 累积暂停开始到暂停结束的这段时间
    //
    //                     |<-------d------->|
    // ----*---------------*-----------------*------------> time
    //  m_BaseTime       m_StopTime        startTime     

    // 一時停止の開始から再開までの累積時間
    //
    //                     ｜<-------d------->｜
    // ----*---------------*-----------------*------------> time
    //  m_BaseTime       m_StopTime        startTime

    if( m_Stopped )
    {
        m_PausedTime += (startTime - m_StopTime);

        m_PrevTime = startTime;
        m_StopTime = 0;
        m_Stopped  = false;
    }
}

void CpuTimer::Stop()
{
    if( !m_Stopped )
    {
        __int64 currTime{};
        QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

        m_StopTime = currTime;
        m_Stopped  = true;
    }
}

void CpuTimer::Tick()
{
    if( m_Stopped )
    {
        m_DeltaTime = 0.0;
        return;
    }

    __int64 currTime{};
    QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
    m_CurrTime = currTime;

    // 現在の Tick と前回の Tick とのフレーム間隔
    m_DeltaTime = (m_CurrTime - m_PrevTime)*m_SecondsPerCount;

    m_PrevTime = m_CurrTime;

    if(m_DeltaTime < 0.0)
    {
        m_DeltaTime = 0.0;
    }
}

bool CpuTimer::IsStopped() const
{
    return m_Stopped;
}

