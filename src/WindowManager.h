#pragma once

#include "pch.h"

class Game;

class WindowManager
{
public:
    WindowManager();
    ~WindowManager();

    bool Initialize(HINSTANCE hInstance, int nCmdShow, Game* game);
    void Shutdown();

    HWND GetWindowHandle() const
    {
        return m_hwnd;
    }
    bool IsFullscreen() const
    {
        return m_fullscreen;
    }

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
    HWND m_hwnd;
    HINSTANCE m_hInstance;
    Game* m_game;
    bool m_fullscreen;

    static bool s_in_sizemove;
    static bool s_in_suspend;
    static bool s_minimized;

    void ToggleFullscreen();
    bool CreateGameWindow(int nCmdShow);
    bool RegisterWindowClass();
};