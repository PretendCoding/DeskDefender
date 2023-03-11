#pragma once

#include <Windows.h>
#include <iostream>
#include <vector>

// Define a type for the function signature with variable arguments
template<typename... Args>
using FnType = std::function<void(Args...)>;

// Define a struct to hold the function and its parameters
template<typename... Args>
struct FnWithParams {
	FnType<Args...> fn;
	std::tuple<Args...> args;
};

class DeskDefender
{
public:
	static void pressKey(CHAR keyParam);
	static bool mouse_move(int x, int y);
	static void smoothMouseMove(std::queue<FnWithParams<int, int>>& mouseMoveQueue, int timeInMs);
	static std::tuple<int, int> getNextMousePosLerp(int x, int y, int dx, int dy, float alpha);
	static void cleanOnExit(NOTIFYICONDATAW nid);
	static NOTIFYICONDATAW createSystemTrayIcon();
	static void fillMouseMoveQueue(std::queue<FnWithParams<int, int>>& mouseMoveQueue, int x, int y);
};