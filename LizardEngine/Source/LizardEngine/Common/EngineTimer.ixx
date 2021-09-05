export module LizardEngine.Common : EngineTimer;

import : Logger;

export class EngineTimer
{
public:
    EngineTimer() {}

    float TotalTime() const;
    float DeltaTime() const;

    void Reset();
    void Start();
    void Stop();
    void Tick();

private:
};