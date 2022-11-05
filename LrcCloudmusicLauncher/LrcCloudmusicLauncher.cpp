#include <iostream>

#include <Windows.h>

int main()
{
    WCHAR CurrentDir[MAX_PATH] = { 0 };
    GetCurrentDirectoryW(MAX_PATH, CurrentDir);

    WCHAR LrcCloudmusicDLL[MAX_PATH + 20] = { 0 };
    wsprintfW(LrcCloudmusicDLL, L"%s\\LrcCloudmusic.dll", CurrentDir);

    while (true)
    {
        HWND hCloudmusicWindow = FindWindowW(L"DesktopLyrics", L"桌面歌词");
        if (hCloudmusicWindow)
        {
            DWORD CloudmusicPID = 0;
            if (GetWindowThreadProcessId(hCloudmusicWindow, &CloudmusicPID) && CloudmusicPID)
            {
                HANDLE hCloudmusicProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, CloudmusicPID);
                if (hCloudmusicProcess)
                {
                    PVOID LrcLoaderPtr = VirtualAllocEx(hCloudmusicProcess, 0, 4096, MEM_COMMIT, PAGE_READWRITE);
                    if (LrcLoaderPtr)
                    {
                        BOOL LoadSuccess = FALSE;

                        WriteProcessMemory(hCloudmusicProcess, LrcLoaderPtr, LrcCloudmusicDLL, MAX_PATH + 20, 0);
                        HANDLE hLrcLoaderThread = CreateRemoteThread(hCloudmusicProcess, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibraryW, LrcLoaderPtr, 0, 0);

                        if (hLrcLoaderThread)
                        {
                            WaitForSingleObject(hLrcLoaderThread, 10000);

                            DWORD LrcLoaderThreadExitCode = 0;
                            if (GetExitCodeThread(hLrcLoaderThread, &LrcLoaderThreadExitCode) && LrcLoaderThreadExitCode > 4096)
                            {
                                LoadSuccess = TRUE;

                                WaitForSingleObject(hCloudmusicProcess, INFINITE);
                            }
                            else
                            {
                                printf("Fail to query LrcLoader thread!");
                            }
                        }
                        else
                        {
                            printf("Fail to start LrcLoader thread!");
                        }

                        if (!LoadSuccess)
                        {
                            MessageBoxW(GetConsoleWindow(), L"An error occurred while loading LrcCloudmusic!", L"Error", MB_ICONWARNING);
                        }
                    }
                    else
                    {
                        printf("Fail to init LrcLoader!");
                    }
                }
                else
                {
                    printf("Fail to open process: %d\n", (int)CloudmusicPID);
                }
            }
        }

        Sleep(1000);
    }
}
