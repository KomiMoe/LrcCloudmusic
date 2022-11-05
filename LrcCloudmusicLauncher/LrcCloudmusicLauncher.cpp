#include <iostream>

#include <Windows.h>

int main()
{
    while (true)
    {
        HWND hCloudmusicWindow = FindWindowW(L"DesktopLyrics", L"桌面歌词");
        if (hCloudmusicWindow)
        {
            DWORD CloudmusicPID = 0;
            if (GetWindowThreadProcessId(hCloudmusicWindow, &CloudmusicPID) && CloudmusicPID)
            {
                HANDLE hCloudmusic = OpenProcess(PROCESS_ALL_ACCESS, FALSE, CloudmusicPID);
                if (hCloudmusic)
                {
                    HANDLE hLrcLoaderThread = CreateRemoteThread(hCloudmusicWindow, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, 0, 0, 0);
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
