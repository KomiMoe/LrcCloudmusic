#include "CMainLoop.h"
#include "Utils.h"

#include <iostream>

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
        ShowNewLine(pCurrentLrc);
    }
    __except(1)
    {
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
    _lastLrc = std::move(currentLrc);
#ifdef _DEBUG
    wprintf(L"%ws\n", currentLrc.c_str());
#endif
}

CMainLoop::CMainLoop()
{
    //89 ?? ?? ?? ?? ?? FF ?? 04 FF 15 ?? ?? ?? ?? E8 ?? ?? ?? ?? 8B ?? E8 ?? ?? ?? ?? 83 3D ?? ?? ?? ?? 00 74
    constexpr unsigned char sig[] =
    {
        0x89, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xFF, 0xCC, 0x04, 0xFF, 0x15,
        0xCC, 0xCC, 0xCC, 0xCC, 0xE8, 0xCC, 0xCC, 0xCC, 0xCC, 0x8B, 0xCC,
        0xE8, 0xCC, 0xCC, 0xCC, 0xCC, 0x83, 0x3D, 0xCC, 0xCC, 0xCC, 0xCC,
        0x00, 0x74
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
    const auto ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<PUCHAR>(_hCloudMusic) + dosHeader->e_lfanew);

    const auto sizeOfImage = ntHeaders->OptionalHeader.SizeOfImage;
#ifdef _DEBUG
    wprintf(L"Size of cloudmusic.dll %d \n", sizeOfImage);
#endif

    const auto hitTarget = SearchVirtualMemory(reinterpret_cast<PUCHAR>(_hCloudMusic), sizeOfImage, sig, "x?????x?xxx????x????x?x????xx????xx");
#ifdef _DEBUG
    wprintf(L"Target %p \n", hitTarget);
#endif

    if (!hitTarget)
    {
        MessageBoxW(nullptr, L"Can not found call target", nullptr, NULL);
        return;
    }
    (const unsigned char*&)_fnGetLrcContext = hitTarget + target_offset + *reinterpret_cast<const signed int*>(hitTarget + target_offset) + 4;
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
        Tick();
        Sleep(10);
    }
}
