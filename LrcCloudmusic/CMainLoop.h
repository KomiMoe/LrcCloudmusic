#pragma once

#include "imgui/imgui.h"

#include <mutex>
#include <Windows.h>
#include <d3d11.h>

class CMainLoop
{
protected:
    HMODULE _hCloudMusic = nullptr;
    PUCHAR(__stdcall* _fnGetLrcContext)() = nullptr;
    PUCHAR _pLrcContext = nullptr;

    HWND _lrcWindow = nullptr;
    HWND _hShellTray = nullptr;
    HWND _hReBarWindow32 = nullptr;
    HWND _hMSTaskSwWClass = nullptr;

    RECT _rectMSTaskSwWClass{};
    RECT _lastRectMSTaskSwWClass{};

    std::wstring _lastLrc;
    float _lastProgress;
    size_t _processedPos;

    std::mutex* _mutex;
    bool _initialized = false;

    ID3D11Device* _pd3dDevice = nullptr;
    ID3D11DeviceContext* _pd3dDeviceContext = nullptr;
    IDXGISwapChain* _pSwapChain = nullptr;
    ID3D11RenderTargetView* _mainRenderTargetView = nullptr;

    ImFont* _mainFont = nullptr;

    void Tick();
    void Draw() const;
    bool CreateDeviceD3D();
    void CreateRenderTarget();
    void UpdateWindow();
    void RefreshContext();
    void ShowNewLine(PWCHAR pCurrentLrc);
    void UpdateProgress(float progress);

public:
    CMainLoop();
    ~CMainLoop();
    void Run();
};
