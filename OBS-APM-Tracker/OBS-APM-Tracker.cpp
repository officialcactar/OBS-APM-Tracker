#include <windows.h>

#include <fstream>

#include <vector>

#include <iomanip>

#include <iostream>

#include <chrono>

#include <thread>

#include <mutex>

const char* PATH = "./APM.txt"; // The path to the log file
const char* NAME = "APM-Tracker";

bool* KeyStates = (bool*)calloc(100, sizeof(bool));
std::mutex KeyHistoryMutex;
std::mutex KeyStateMutex;
std::vector < std::chrono::milliseconds > vect;
HHOOK KeyboardHook;
HHOOK MouseHook;
void KeyPressWriter();
void KeyPressRemover();
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);

int main(void) {
    FreeConsole();
    std::thread writer_thread(KeyPressWriter);
    std::thread remover_thread(KeyPressRemover);
    HINSTANCE hInstance = GetModuleHandle(NULL);
    HINSTANCE hInstance2 = GetModuleHandle(NULL);
    KeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)LowLevelKeyboardProc, hInstance, 0);
    MouseHook = SetWindowsHookEx(WH_MOUSE_LL, (HOOKPROC)LowLevelMouseProc, hInstance2, 0);

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
        KBDLLHOOKSTRUCT* kb = (KBDLLHOOKSTRUCT*)lParam;
        if ((wParam == WM_KEYDOWN && !KeyStates[kb->vkCode]) || wParam == WM_SYSKEYDOWN) {
            KeyHistoryMutex.lock();
            vect.push_back(duration_cast <std::chrono::milliseconds> (std::chrono::system_clock::now().time_since_epoch()));
            KeyHistoryMutex.unlock();

            KeyStateMutex.lock();
            KeyStates[kb->vkCode] = true;
            KeyStateMutex.unlock();
        }
        else if (wParam == WM_KEYUP && KeyStates[kb->vkCode]) {
            KeyStateMutex.lock();
            KeyStates[kb->vkCode] = false;
            KeyStateMutex.unlock();
        }
    }
    return CallNextHookEx(KeyboardHook, nCode, wParam, lParam);
}

LRESULT CALLBACK
LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        if (wParam == WM_LBUTTONDOWN || wParam == WM_RBUTTONDOWN) {
            KeyHistoryMutex.lock();
            vect.push_back(duration_cast <std::chrono::milliseconds> (std::chrono::system_clock::now().time_since_epoch()));
            KeyHistoryMutex.unlock();
        }
    }
    return CallNextHookEx(MouseHook, nCode, wParam, lParam);
}

void
KeyPressRemover() {
    while (1) {
        KeyHistoryMutex.lock();
        std::erase_if(vect, [](std::chrono::milliseconds x) {
            //Remove anything thats older than 5 seconds.
            return x < duration_cast <std::chrono::milliseconds> (std::chrono::system_clock::now().time_since_epoch() - std::chrono::seconds(5));
            });
        KeyHistoryMutex.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

}

void
KeyPressWriter() {
    std::ofstream APM_File;
    APM_File.open(PATH, std::ios::trunc);
    if (!APM_File) {
        if (MessageBoxA(NULL,
            "There was an error opening the APM.txt file.\n"
            "Please check file permissions.",
            NAME, MB_OK | MB_ICONWARNING))
            exit(1);
    }
    while (1) {
        APM_File.seekp(0);
        KeyHistoryMutex.lock();
        size_t APM = vect.size() * 12;
        KeyHistoryMutex.unlock();
        APM_File << "APM: " << std::setw(4) << std::left << APM;
        APM_File.flush();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    APM_File.close();
}