#pragma once
#include <mutex>
#include <Windows.h>

class CMainLoop
{
protected:
    HMODULE _hCloudMusic = nullptr;
    PUCHAR(__stdcall* _fnGetLrcContext)() = nullptr;
    PUCHAR _pLrcContext = nullptr;

    std::wstring _lastLrc;

    std::mutex* _mutex;
    bool _initialized = false;

    void Tick();
    void RefreshContext();
    void ShowNewLine(PWCHAR pCurrentLrc);

public:
    CMainLoop();
    void Run();
};
