#include <windows.h>
#include <fstream>
#include <vector>
#include <iomanip>
#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>

#define PATH "./APM.txt" // The path to the log file
#define NAME "APM-Tracker"

std::mutex mtx;
std::vector < std::chrono::milliseconds > vect;
std::ofstream APM_File;
HHOOK KeyboardHook;
HHOOK MouseHook;
void KeyPressWriter();
void KeyPressRemover();
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);

int main(void) {
    FreeConsole();
    std::thread writer_thread(KeyPressWriter);
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
            std::chrono::milliseconds time = duration_cast <std::chrono::milliseconds> (std::chrono::system_clock::now().time_since_epoch());
            mtx.lock();
            vect.push_back(time);
            mtx.unlock();
        }
    }
    return CallNextHookEx(KeyboardHook, nCode, wParam, lParam);
}

LRESULT CALLBACK
LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        if (wParam == WM_LBUTTONDOWN || wParam == WM_RBUTTONDOWN) {
            std::chrono::milliseconds time = duration_cast <std::chrono::milliseconds> (std::chrono::system_clock::now().time_since_epoch());
            mtx.lock();
            vect.push_back(time);
            mtx.unlock();
        }
    }
    return CallNextHookEx(MouseHook, nCode, wParam, lParam);
}

void
KeyPressRemover() {
    mtx.lock();
    std::cout << std::erase_if(vect, [](std::chrono::milliseconds x) {
        return x < duration_cast <std::chrono::milliseconds> (std::chrono::system_clock::now().time_since_epoch() - std::chrono::seconds(5));
        });
    mtx.unlock();
}

void
KeyPressWriter() {
    APM_File.open(PATH, std::ios::trunc);
    if (!APM_File) {
        if (MessageBoxA(NULL,
            "There was an error opening the APM.txt file.\n"
            "Please check file permissions.",
            NAME, MB_OK | MB_ICONWARNING))
            exit(1);
    }
    while (1) {
        KeyPressRemover();

        APM_File.seekp(0);
        size_t APM = vect.size() * 12;
        APM_File << "APM: " << std::setw(4) << std::left << APM;
        APM_File.flush();

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    APM_File.close();
}