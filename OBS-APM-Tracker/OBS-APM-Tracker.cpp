#include <windows.h>
#include <fstream>
#include <vector>
#include <chrono>
#include <thread>

#define PATH "./APM.txt" // The path to the log file
#define NAME "APM-Tracker"

std::vector < int > vect;
std::ofstream APM_File;
bool RacePrevention = true;
HHOOK KeyboardHook;
HHOOK MouseHook;
DWORD ThreadId;
HANDLE ThreadHandle;
void KeyPressWriter();
void KeyPressRemover();
int CurrentEpochTime();
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);

int main(void) {
    FreeConsole();

    std::thread writer_thread(KeyPressWriter);
    std::thread remover_thread(KeyPressRemover);

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
            int time = CurrentEpochTime();
            while (1) {
                if (!RacePrevention) {
                    vect.push_back(time);
                    break;
                }
            }
        }
    }
    return CallNextHookEx(KeyboardHook, nCode, wParam, lParam);
}

LRESULT CALLBACK
LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        if (wParam == WM_LBUTTONDOWN || wParam == WM_RBUTTONDOWN) {
            int time = CurrentEpochTime();
            while (1) {
                if (!RacePrevention) {
                    vect.push_back(time);
                    break;
                }
            }
        }
    }
    return CallNextHookEx(MouseHook, nCode, wParam, lParam);
}

void
KeyPressRemover() {
    while (1) {
        RacePrevention = true;
        std::erase_if(vect, [](int x) {
            return x < CurrentEpochTime() - 5;
            });
        RacePrevention = false;
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

void
KeyPressWriter() {
    while (1) {
        APM_File.open(PATH, std::ios::trunc);
        if (!APM_File) {
            if (MessageBoxA(NULL,
                "There was an error opening the APM.txt file.\n"
                "Please check file permissions then report on github if not resolved.",
                NAME, MB_OK | MB_ICONWARNING))
                exit(1);
        }
        APM_File << "APM: " << vect.size() * 12.0;
        APM_File.close();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int
CurrentEpochTime() {
    FILETIME ft = { 0 };
    GetSystemTimeAsFileTime(&ft);
    LARGE_INTEGER li = { 0 };
    li.LowPart = ft.dwLowDateTime;
    li.HighPart = ft.dwHighDateTime;
    return (int)(li.QuadPart / 10000000 - 11644473600LL);
}