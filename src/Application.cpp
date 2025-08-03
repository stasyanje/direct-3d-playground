#include "pch.h"
#include "Application.h"
#include "Game.h"

using namespace DirectX;

Application::Application()
    : m_game(nullptr)
    , m_windowManager(nullptr)
{
}

Application::~Application()
{
    Shutdown();
}

int Application::Run(HINSTANCE hInstance, int nCmdShow)
{
    if (!XMVerifyCPUSupport())
        return 1;

    if (FAILED(CoInitializeEx(nullptr, COINITBASE_MULTITHREADED)))
        return 1;

    HRESULT hr = XGameRuntimeInitialize();
    if (FAILED(hr))
    {
        if (hr == E_GAMERUNTIME_DLL_NOT_FOUND || hr == E_GAMERUNTIME_VERSION_MISMATCH || hr == HRESULT_FROM_WIN32(ERROR_SERVICE_DOES_NOT_EXIST))
        {
            extern LPCWSTR g_szAppName;
            std::ignore = MessageBoxW(nullptr, L"Game Runtime is not installed on this system or needs updating.", g_szAppName, MB_ICONERROR | MB_OK);
        }
        return 1;
    }

    if (!Initialize(hInstance, nCmdShow))
        return 1;

    int result = MessageLoop();
    
    Shutdown();
    XGameRuntimeUninitialize();
    
    return result;
}

bool Application::Initialize(HINSTANCE hInstance, int nCmdShow)
{
    m_game = std::make_unique<Game>();
    m_windowManager = std::make_unique<WindowManager>();
    
    if (!m_windowManager->Initialize(hInstance, nCmdShow, m_game.get()))
        return false;
        
    return true;
}

void Application::Shutdown()
{
    if (m_windowManager)
    {
        m_windowManager->Shutdown();
        m_windowManager.reset();
    }
    
    if (m_game)
    {
        m_game.reset();
    }
}

int Application::MessageLoop()
{
    MSG msg = {};
    while (WM_QUIT != msg.message)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            m_game->Tick();
        }
    }
    
    return static_cast<int>(msg.wParam);
}