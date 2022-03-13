#include <windows.h>

#include <iostream>

#include <fstream>

#include <vector>

#include <map>

#include <iostream>

#include <vector>

#include <chrono>

#include <thread>

#define PATH "./APM.txt" // The path to the log file
#define NAME "APM-Tracker"
//using namespace std;

std::ofstream file;

bool Race_Prevention = true;

HHOOK hookk;
HHOOK hookm;
MSG msg;

DWORD ThreadId;
HANDLE ThreadHandle;

DWORD WINAPI writer(LPVOID lpdwThreadParam);
DWORD WINAPI remover(LPVOID lpdwThreadParam);

long long int CurrentEpochTime();

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);

int KeyCount = 0;
std::vector < long long int > vect;

int main(void) {
    FreeConsole();

    ThreadHandle = CreateThread(NULL, 0, remover, NULL, 0, &ThreadId);
    ThreadHandle = CreateThread(NULL, 0, writer, NULL, 0, &ThreadId);
    HINSTANCE hInstance = GetModuleHandle(NULL);
    HHOOK KeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)LowLevelKeyboardProc, hInstance, 0);
    HHOOK MouseHook = SetWindowsHookEx(WH_MOUSE_LL, (HOOKPROC)LowLevelMouseProc, hInstance, 0);

    if (MessageBoxA(NULL,
        "APM-Tracker is now running!\n\n"
        "Please create a TEXT source in OBS and link the APM.txt file.\n\n"
        "Press the 'OK' button when you would like to exit.",
        NAME, MB_OK | MB_ICONINFORMATION))
        exit(0);

}

LRESULT CALLBACK
LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        if (wParam == WM_KEYDOWN) {
            long long int time = CurrentEpochTime();
            while (1) {
                if (!Race_Prevention) {
                    vect.push_back(time);
                    break;
                }
            }
        }
    }
    return CallNextHookEx(hookk, nCode, wParam, lParam);
}

LRESULT CALLBACK
LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        if (wParam == WM_LBUTTONDOWN || wParam == WM_RBUTTONDOWN) {
            long long int time = CurrentEpochTime();
            while (1) {
                if (!Race_Prevention) {
                    vect.push_back(time);
                    break;
                }
            }
        }
    }
    return CallNextHookEx(hookm, nCode, wParam, lParam);
}

DWORD WINAPI
remover(LPVOID lpdwThreadParam) {
    while (1) {
        Race_Prevention = true;
        std::erase_if(vect, [](int x) {
            return x < CurrentEpochTime() - 5;
            });
        Race_Prevention = false;
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
    return 1;
}

DWORD WINAPI
writer(LPVOID lpdwThreadParam) {
    while (1) {
        file.open(PATH, std::ios::trunc);
        if (!file) {
            if (MessageBoxA(NULL,
                "There was an error opening the APM.txt file.\n"
                "Please check file permissions then report on github if not resolved.",
                NAME, MB_OK | MB_ICONWARNING))
                return 1;
        }
        file << "APM: " << vect.size() * 12.0;
        file.close();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 1;
}

long long int
CurrentEpochTime() {
    FILETIME ft = {
      0
    };
    GetSystemTimeAsFileTime(&ft);
    LARGE_INTEGER li = {
      0
    };
    li.LowPart = ft.dwLowDateTime;
    li.HighPart = ft.dwHighDateTime;
    return (li.QuadPart / 10000000 - 11644473600LL);
}