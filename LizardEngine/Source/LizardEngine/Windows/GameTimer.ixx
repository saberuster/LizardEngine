module;

#include "windows.h"
#pragma warning(disable : 5050)
export module LizardEngine.Windows : GameTimer;
import std.core;

export class GameTimer
{

public:
    GameTimer()
        : baseTime(0), currTime(0), preTime(0), deltaTime(0.0), secondsPerCount(0.0){};

    void Tick()
    {
        QueryPerformanceCounter((LARGE_INTEGER *)&currTime);

        deltaTime = (currTime - preTime) * secondsPerCount;

        preTime = currTime;

        if (deltaTime < 0.0)
        {
            deltaTime = 0.0;
        }
    }

    void Reset()
    {
        std::int64_t time{0};
        QueryPerformanceCounter((LARGE_INTEGER *)&time);
        baseTime = time;
        currTime = time;
        preTime = time;
        std::int64_t countsPerSecond;
        QueryPerformanceFrequency((LARGE_INTEGER *)&countsPerSecond);
        secondsPerCount = 1 / (double)countsPerSecond;
    }

    double GetDeltaTime() const { return deltaTime; }

private:
    std::int64_t baseTime;
    std::int64_t currTime;
    std::int64_t preTime;
    double deltaTime;
    double secondsPerCount;
};