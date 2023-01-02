#include "CMainLoop.h"
#include "Utils.h"

#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"

#include <iostream>
#include <dwmapi.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dwmapi.lib")

void CMainLoop::Tick()
{
    RefreshContext();
    if (!_pLrcContext)
    {
        return;
    }
    __try
    {
        const auto pLrcData = *reinterpret_cast<PUCHAR*>(_pLrcContext + 0xC4);
        if (!pLrcData)
        {
            return;
        }
        const auto pCurrentLrc = *reinterpret_cast<PWCHAR*>(pLrcData + 0x8);
        if (!pCurrentLrc)
        {
            return;
        }
        const auto lrcLength = wcslen(pCurrentLrc);
        if (!lrcLength)
        {
            return;
        }
        const float progress = *reinterpret_cast<float*>(pLrcData + 0x4);
        if (progress < _lastProgress || progress == 0.f)
        {
            UpdateProgress(1.f);
            ShowNewLine(pCurrentLrc);
        }
        //UpdateProgress(progress);
        UpdateProgress(1.f);
        _lastProgress = progress;
    }
    __except (1)
    {
    }
}

void CMainLoop::Draw() const
{
    constexpr float fontSize = 20.f;
    constexpr auto clearColor = ImVec4(0.f, 0.f, 0.f, 0.f);

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    char szLrc[256];
    if (WideCharToMultiByte(CP_UTF8, NULL, _lastLrc.c_str(), -1, szLrc, sizeof(szLrc), nullptr, nullptr))
    {
        const auto lrcRect = _mainFont->CalcTextSizeA(fontSize, 420.f, 10000.f, szLrc);

        ImGui::GetForegroundDrawList()->AddText(_mainFont, fontSize, ImVec2(6.f, (_lastRectMSTaskSwWClass.bottom - _lastRectMSTaskSwWClass.top) * 0.5f - lrcRect.y * 0.5f), IM_COL32_WHITE, szLrc);
    }

    ImGui::Render();
    constexpr float clearColorWithAlpha[4] = { clearColor.x * clearColor.w, clearColor.y * clearColor.w, clearColor.z * clearColor.w, clearColor.w };
    _pd3dDeviceContext->OMSetRenderTargets(1, &_mainRenderTargetView, nullptr);
    _pd3dDeviceContext->ClearRenderTargetView(_mainRenderTargetView, clearColorWithAlpha);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    _pSwapChain->Present(1, 0); // Present with v sync
}

void CMainLoop::UpdateWindow()
{
    GetWindowRect(_hMSTaskSwWClass, &_rectMSTaskSwWClass);
    if (memcmp(&_lastRectMSTaskSwWClass, &_rectMSTaskSwWClass, sizeof(_rectMSTaskSwWClass)))
    {
        SetWindowPos(_lrcWindow, HWND_TOP, _rectMSTaskSwWClass.right, 0, 420, _rectMSTaskSwWClass.bottom - _rectMSTaskSwWClass.top, NULL);
        _lastRectMSTaskSwWClass = _rectMSTaskSwWClass;
    }
    MSG msg;
    while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void CMainLoop::RefreshContext()
{
    if (!_fnGetLrcContext)
    {
        return;
    }
    _pLrcContext = _fnGetLrcContext();
}

void CMainLoop::ShowNewLine(PWCHAR pCurrentLrc)
{
    const std::wstring currentLrc(pCurrentLrc);
    if (currentLrc == _lastLrc)
    {
        return;
    }
    _processedPos = 0;
    _lastLrc = std::move(currentLrc);
    #ifdef _DEBUG
    wprintf(L"\n");
    //wprintf(L"%ws\n", currentLrc.c_str());
    #endif
}

void CMainLoop::UpdateProgress(float progress)
{
    if (progress > 1.f)
    {
        return;
    }
    const auto lineLength = _lastLrc.length();
    const auto printPos = static_cast<size_t>(ceil(static_cast<float>(lineLength) * progress));
    if (printPos <= _processedPos)
    {
        return;
    }
#ifdef _DEBUG
    const auto printWorlds = _lastLrc.substr(_processedPos, printPos - _processedPos);
    wprintf(L"%ws", printWorlds.c_str());
#endif
    _processedPos = printPos;
}

CMainLoop::CMainLoop()
{
    //89 ?? ?? ?? ?? ?? FF ?? 04 FF 15 ?? ?? ?? ?? E8 ?? ?? ?? ?? 8B ?? E8 ?? ?? ?? ?? 83 3D ?? ?? ?? ?? 00 74
    constexpr unsigned char sig[] =
    {
        0x89,
        0xCC,
        0xCC,
        0xCC,
        0xCC,
        0xCC,
        0xFF,
        0xCC,
        0x04,
        0xFF,
        0x15,
        0xCC,
        0xCC,
        0xCC,
        0xCC,
        0xE8,
        0xCC,
        0xCC,
        0xCC,
        0xCC,
        0x8B,
        0xCC,
        0xE8,
        0xCC,
        0xCC,
        0xCC,
        0xCC,
        0x83,
        0x3D,
        0xCC,
        0xCC,
        0xCC,
        0xCC,
        0x00,
        0x74
    };
    constexpr size_t target_offset = 16;

    _hCloudMusic = GetModuleHandleW(L"cloudmusic.dll");
    #ifdef _DEBUG
    wprintf(L"Module cloudmusic.dll %p\n", _hCloudMusic);
    #endif

    if (!_hCloudMusic)
    {
        MessageBoxW(nullptr, L"Can not found cloudmusic.dll", nullptr, NULL);
        return;
    }
    const auto dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(_hCloudMusic);
    const auto ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(
        reinterpret_cast<PUCHAR>(_hCloudMusic) + dosHeader->e_lfanew);

    const auto sizeOfImage = ntHeaders->OptionalHeader.SizeOfImage;
    #ifdef _DEBUG
    wprintf(L"Size of cloudmusic.dll %d \n", sizeOfImage);
    #endif

    const auto hitTarget = SearchVirtualMemory(reinterpret_cast<PUCHAR>(_hCloudMusic),
                                               sizeOfImage,
                                               sig,
                                               "x?????x?xxx????x????x?x????xx????xx");
    #ifdef _DEBUG
    wprintf(L"Target %p \n", hitTarget);
    #endif

    if (!hitTarget)
    {
        MessageBoxW(nullptr, L"Can not found call target", nullptr, NULL);
        return;
    }
    (const unsigned char*&)_fnGetLrcContext = hitTarget + target_offset + *reinterpret_cast<const signed int*>(
                                                  hitTarget + target_offset) + 4;
    #ifdef _DEBUG
    wprintf(L"GetContextCall %p \n", _fnGetLrcContext);
    #endif

    RefreshContext();
    #ifdef _DEBUG
    wprintf(L"LrcContext %p \n", _pLrcContext);
    #endif

    if (!_pLrcContext)
    {
        return;
    }
    _mutex = new std::mutex;
    _initialized = true;

    _hShellTray = FindWindow(L"Shell_TrayWnd", nullptr);
    if (!_hShellTray)
    {
        MessageBoxW(nullptr, L"Can not found task bar window", nullptr, NULL);
        return;
    }
    _hReBarWindow32 = FindWindowEx(_hShellTray, nullptr, L"ReBarWindow32", nullptr);
    if (!_hReBarWindow32)
    {
        MessageBoxW(nullptr, L"Can not found task bar window", nullptr, NULL);
        return;
    }
    _hMSTaskSwWClass = FindWindowEx(_hReBarWindow32, nullptr, L"MSTaskSwWClass", nullptr);
    if (!_hMSTaskSwWClass)
    {
        MessageBoxW(nullptr, L"Can not found task bar window", nullptr, NULL);
        return;
    }

    GetWindowRect(_hMSTaskSwWClass, &_rectMSTaskSwWClass);
    std::cout << "MSTaskSwWindow rect " << _rectMSTaskSwWClass.left << " " << _rectMSTaskSwWClass.top << " " << _rectMSTaskSwWClass.right << " " << _rectMSTaskSwWClass.bottom << std::endl;

    const WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, DefWindowProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"Cloud Music Lrc Clz", nullptr};
    RegisterClassEx(&wc);
    _lrcWindow = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT, wc.lpszClassName, L"Cloud Music Lrc", WS_POPUP, 
                                 _rectMSTaskSwWClass.right, 0, 420, _rectMSTaskSwWClass.bottom - _rectMSTaskSwWClass.top, nullptr, nullptr, wc.hInstance, nullptr);
    
    CreateDeviceD3D();
    ShowWindow(_lrcWindow, SW_SHOW);
    SetParent(_lrcWindow, _hShellTray);
    SetLayeredWindowAttributes(_lrcWindow, RGB(0, 0, 0), 0, LWA_COLORKEY);
    constexpr MARGINS margins{-1};
    DwmExtendFrameIntoClientArea(_lrcWindow, &margins);
    ShowWindow(_lrcWindow, SW_SHOW);
    ::UpdateWindow(_lrcWindow);

    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    _mainFont = ImGui::GetIO().Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\msyhbd.ttc", 30.f, nullptr, ImGui::GetIO().Fonts->GetGlyphRangesChineseFull());
    ImGui_ImplWin32_Init(_lrcWindow);
    ImGui_ImplDX11_Init(_pd3dDevice, _pd3dDeviceContext);
}

bool CMainLoop::CreateDeviceD3D()
{
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = _lrcWindow;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    constexpr UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    constexpr D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    if (D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &_pSwapChain, &_pd3dDevice, &featureLevel, &_pd3dDeviceContext) != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CMainLoop::CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    _pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    _pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &_mainRenderTargetView);
    pBackBuffer->Release();
}

CMainLoop::~CMainLoop()
{
    _initialized = false;
    _mutex->lock();
    _mutex->unlock();
    delete _mutex;
}

void CMainLoop::Run()
{
    _mutex->lock();
    while (true)
    {
        if (!_initialized)
        {
            _mutex->unlock();
            break;
        }
        UpdateWindow();
        Tick();
        Draw();
    }
}
