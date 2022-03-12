#include <windows.h>

#include <iostream>

#include <fstream>


#define PATH "./APM.txt" // The path to the log file
#define NAME "APM-Tracker"
using namespace std;

ofstream file;

HHOOK hookk;
HHOOK hookm;
MSG msg;

DWORD ThreadId;
HANDLE ThreadHandle;

DWORD WINAPI writer(LPVOID lpdwThreadParam);
DWORD WINAPI remover(LPVOID lpdwThreadParam);

DWORD WINAPI messagebox(LPVOID lpdwThreadParam);

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);

int KeyCount = 0;

int main(void) {

    FreeConsole();

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
            ThreadHandle = CreateThread(NULL, 0, remover, NULL, 0, &ThreadId);
            KeyCount++;
        }
    }
    return CallNextHookEx(hookk, nCode, wParam, lParam);
}

LRESULT CALLBACK
LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        if (wParam == WM_LBUTTONDOWN || wParam == WM_RBUTTONDOWN) {
            ThreadHandle = CreateThread(NULL, 0, remover, NULL, 0, &ThreadId);
            KeyCount++;
        }
    }
    return CallNextHookEx(hookm, nCode, wParam, lParam);
}

DWORD WINAPI
remover(LPVOID lpdwThreadParam) {
    Sleep(5000);
    KeyCount--;
    return 0;
}

DWORD WINAPI
writer(LPVOID lpdwThreadParam) {
    while (1) {
        file.open(PATH, ios::trunc);
        if (!file) {
            if (MessageBoxA(NULL,
                "There was an error opening the APM.txt file.\n"
                "Please check file permissions then report on github if not resolved.",
                NAME, MB_OK | MB_ICONWARNING))
                return 1;
        }
        file << "APM: " << KeyCount * 12.0;
        file.close();
        Sleep(1000);
    }
    return 1;
}