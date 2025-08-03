#pragma once

#include "pch.h"

class Game;
class WindowManager;

class Application
{
public:
    Application();
    ~Application();

    int Run(HINSTANCE hInstance, int nCmdShow);

private:
    bool Initialize(HINSTANCE hInstance, int nCmdShow);
    void Shutdown();
    int MessageLoop();
    
    std::unique_ptr<Game> m_game;
    std::unique_ptr<WindowManager> m_windowManager;
};