#include <iostream>
#include <Windows.h>
#include <queue>
#include <functional>
#include <thread>
#include <tuple>
#include <chrono>
#include "DeskDefender.h"
#include "resource.h"

#define WM_TRAYICON (WM_USER + 1)

int main()
{
    // Hide the console window
    ::ShowWindow(::GetConsoleWindow(), SW_HIDE);

    // Initalize variables
    NOTIFYICONDATAW nid = DeskDefender::createSystemTrayIcon();
    bool bStart = false;
    bool bFirstRun = true;

    std::queue<FnWithParams<CHAR>> queue;
    std::queue<FnWithParams<int, int>> mouseMoveQueue;

    POINT startingMousePosition;
    GetCursorPos(&startingMousePosition);

    // Main loop
    while (true) {
        if (GetAsyncKeyState(VK_NUMPAD9)) {
            DeskDefender::cleanOnExit(nid);
            return 0;
        } // Exit

        if (GetAsyncKeyState(VK_NUMPAD0)) {
            bStart = false;
            bFirstRun = true;
        }

        if (GetAsyncKeyState(VK_NUMPAD1)) {
            bStart = !bStart;
        }

        if (GetAsyncKeyState(VK_NUMPAD2)) {
            DeskDefender::fillMouseMoveQueue(mouseMoveQueue, 100, 100);
            DeskDefender::smoothMouseMove(mouseMoveQueue, 500);
        }

        if (bStart) {
            if (bFirstRun) {
                bFirstRun = false;
                Sleep(3000);
            }
            DeskDefender::pressKey('w');
            Sleep(100);
            DeskDefender::mouse_move(rand() % 100 + startingMousePosition.x, rand() % 100 + startingMousePosition.y);
        }

        // This exists to help prevent high CPU usage while idle
        Sleep(1);
    }
}

// Send a single character as keyboard input
void DeskDefender::pressKey(CHAR keyParam)
{
    SHORT key;
    UINT mappedkey;
    INPUT input = { 0 };
    key = VkKeyScan(keyParam);
    mappedkey = MapVirtualKey(LOBYTE(key), 0);
    input.type = INPUT_KEYBOARD;
    input.ki.dwFlags = KEYEVENTF_SCANCODE;
    input.ki.wScan = mappedkey;
    SendInput(1, &input, sizeof(input));
    Sleep(10);
    input.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
    SendInput(1, &input, sizeof(input));
}

// Moves the mouse from its current position to another over a given amount of time
void DeskDefender::smoothMouseMove(std::queue<FnWithParams<int, int>>& mouseMoveQueue, int timeInMs)
{
    // Set the time for the delay in ms, and clamp that time between 10 and 100
    int delay = timeInMs / mouseMoveQueue.size();
    delay = std::clamp(delay, 10, 100);
    
    FnWithParams<int, int> obj = mouseMoveQueue.front();

    while (!mouseMoveQueue.empty()) {
        auto fn = mouseMoveQueue.front().fn;
        auto args = mouseMoveQueue.front().args;
        // Call the function with its parameters
        std::apply(fn, args);
        // Remove the function from the queue
        mouseMoveQueue.pop();
        // Delay for 10ms before processing the next function
        Sleep(delay);
    }
}

// Returns a tuple of an interpolated value based on the alpha, 0 being 100% xy and 1 being 100% dxdy
std::tuple<int,int> DeskDefender::getNextMousePosLerp(int x, int y, int dx, int dy, float alpha)
{
    int newX, newY;

    newX = std::lerp(x, dx, alpha);
    newY = std::lerp(y, dy, alpha);

    std::tuple <int, int> tuple;
    tuple = std::make_tuple(newX, newY);

    return tuple;
}

// Cleanup upon exiting the app
void DeskDefender::cleanOnExit(NOTIFYICONDATAW nid)
{
    // The system tray icon will remain after closing the program if this is not included
    Shell_NotifyIcon(NIM_DELETE, &nid);
}

// Create and setup system tray icon
NOTIFYICONDATAW DeskDefender::createSystemTrayIcon()
{
    // Set up the notifyIconData structure
    NOTIFYICONDATAW nid = {};
    nid = {};
    nid.cbSize = sizeof(nid);
    nid.hWnd = GetConsoleWindow();
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
    nid.uCallbackMessage = WM_TRAYICON;
    //This is temp until I get a context menu
    wcscpy_s(nid.szTip, L"Num 0: Stop\nNum 1: Start\nNum 9: Exit");

    // Load the icon from the resource file
    HINSTANCE hInst = GetModuleHandle(NULL);
    HICON hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1));
    nid.hIcon = hIcon;

    // Add the icon to the system tray
    Shell_NotifyIcon(NIM_ADD, &nid);

    return nid;
}

// Fills the mouseMoveQueue with a list of positions
void DeskDefender::fillMouseMoveQueue(std::queue<FnWithParams<int, int>>& mouseMoveQueue, int x, int y)
{
    // Ensure we have an empty queue
    std::queue<FnWithParams<int, int>>().swap(mouseMoveQueue);

    float alpha = 0;

    while (alpha < 1)
    {
        POINT pos;
        GetCursorPos(&pos);
        mouseMoveQueue.push({ DeskDefender::mouse_move, DeskDefender::getNextMousePosLerp(pos.x, pos.y, x, y, alpha) });
        alpha += 1.f / 60.f;
    }
}

bool DeskDefender::mouse_move(int x, int y)
{
    INPUT input;
    input.type = INPUT_MOUSE;
    input.mi.mouseData = 0;
    input.mi.time = 0;
    input.mi.dx = x * (65536.0f / GetSystemMetrics(SM_CXSCREEN));
    input.mi.dy = y * (65536.0f / GetSystemMetrics(SM_CYSCREEN));
    input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_VIRTUALDESK | MOUSEEVENTF_ABSOLUTE;
    SendInput(1, &input, sizeof(input));
    return true;
}