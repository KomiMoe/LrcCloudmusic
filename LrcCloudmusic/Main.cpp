#include "CMainLoop.h"

#include <clocale>
#include <thread>
#include <Windows.h>

CMainLoop* mainLoop = nullptr;

BOOL APIENTRY DllMain(HMODULE hModule,
                      DWORD ul_reason_for_call,
                      LPVOID lpReserved
    )
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
            new std::thread([]
            {
#ifdef _DEBUG
                    AllocConsole();
                    setlocale(LC_ALL, "");
                    FILE* fBackup;
                    freopen_s(&fBackup, "CONOUT$", "w", stdout);
#endif
                mainLoop = new CMainLoop;
                mainLoop->Run();
            });
            break;
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            if (mainLoop)
            {
                delete mainLoop;
            }
            break;
        default:
            break;
    }
    return TRUE;
}
