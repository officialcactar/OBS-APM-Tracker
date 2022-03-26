//{{NO_DEPENDENCIES}}
// Microsoft Visual C++ generated include file.
// Used by OBS-APM-Tracker.rc
//

// Next default values for new objects
// 
#ifdef APSTUDIO_INVOKED
#ifndef APSTUDIO_READONLY_SYMBOLS
#define _APS_NEXT_RESOURCE_VALUE        105
#define _APS_NEXT_COMMAND_VALUE         40001
#define _APS_NEXT_CONTROL_VALUE         1001
#define _APS_NEXT_SYMED_VALUE           101
#endif
#endif

const char* PATH = "./APM.txt"; // The path to the log file
const char* NAME = "APM-Tracker";

bool* KeyStates = new bool[0xFE]{};
std::mutex KeyHistoryMutex;
std::mutex KeyStateMutex;
std::vector < std::chrono::milliseconds > vect;
HHOOK KeyboardHook;
HHOOK MouseHook;
void exiting();
void KeyPressWriter();
void KeyPressRemover();
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);
