#include "pch.h"
#include "WindowManager.h"
#include "Renderer.h"

extern LPCWSTR g_szAppName;

bool WindowManager::s_in_sizemove = false;
bool WindowManager::s_in_suspend = false;
bool WindowManager::s_minimized = false;

WindowManager::WindowManager() :
    m_hwnd(nullptr),
    m_hInstance(nullptr),
    m_renderer(nullptr),
    m_fullscreen(false)
{
}

WindowManager::~WindowManager()
{
    Shutdown();
}

bool WindowManager::Initialize(HINSTANCE hInstance, int nCmdShow, Renderer* renderer)
{
    m_hInstance = hInstance;
    m_renderer = renderer;

    if (!RegisterWindowClass())
        return false;

    if (!CreateRendererWindow(nCmdShow))
        return false;

    return true;
}

void WindowManager::Shutdown()
{
    if (m_hwnd)
    {
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
    }
}

bool WindowManager::RegisterWindowClass()
{
    WNDCLASSEXW wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEXW);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = m_hInstance;
    wcex.hIcon = LoadIconW(m_hInstance, L"IDI_ICON");
    wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wcex.lpszClassName = L"direct_3d_playgroundWindowClass";
    wcex.hIconSm = LoadIconW(wcex.hInstance, L"IDI_ICON");

    return RegisterClassExW(&wcex) != 0;
}

bool WindowManager::CreateRendererWindow(int nCmdShow)
{
    int w, h;
    m_renderer->GetDefaultSize(w, h);

    RECT rc = {0, 0, static_cast<LONG>(w), static_cast<LONG>(h)};
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

    m_hwnd = CreateWindowExW(
        0,
        L"direct_3d_playgroundWindowClass",
        g_szAppName,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        rc.right - rc.left,
        rc.bottom - rc.top,
        nullptr,
        nullptr,
        m_hInstance,
        m_renderer
    );

    if (!m_hwnd)
        return false;

    ShowWindow(m_hwnd, nCmdShow);
    GetClientRect(m_hwnd, &rc);
    m_renderer->Initialize(m_hwnd, rc.right - rc.left, rc.bottom - rc.top);

    return true;
}

void WindowManager::ToggleFullscreen()
{
    if (m_fullscreen)
    {
        SetWindowLongPtr(m_hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
        SetWindowLongPtr(m_hwnd, GWL_EXSTYLE, 0);

        int width = 800;
        int height = 600;
        if (m_renderer)
            m_renderer->GetDefaultSize(width, height);

        ShowWindow(m_hwnd, SW_SHOWNORMAL);
        SetWindowPos(m_hwnd, HWND_TOP, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
    }
    else
    {
        SetWindowLongPtr(m_hwnd, GWL_STYLE, WS_POPUP);
        SetWindowLongPtr(m_hwnd, GWL_EXSTYLE, WS_EX_TOPMOST);
        SetWindowPos(m_hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
        ShowWindow(m_hwnd, SW_SHOWMAXIMIZED);
    }

    m_fullscreen = !m_fullscreen;
}

LRESULT CALLBACK WindowManager::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    auto renderer = reinterpret_cast<Renderer*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    switch (message)
    {
    case WM_CREATE:
        if (lParam)
        {
            auto params = reinterpret_cast<LPCREATESTRUCTW>(lParam);
            SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(params->lpCreateParams));
        }
        break;

    case WM_PAINT:
        if (s_in_sizemove && renderer)
        {
            renderer->Tick();
        }
        else
        {
            PAINTSTRUCT ps;
            std::ignore = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
        }
        break;

    case WM_DISPLAYCHANGE:
        if (renderer)
        {
            renderer->OnDisplayChange();
        }
        break;

    case WM_MOVE:
        if (renderer)
        {
            renderer->OnWindowMoved();
        }
        break;

    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
        {
            if (!s_minimized)
            {
                s_minimized = true;
                if (!s_in_suspend && renderer)
                    renderer->OnSuspending();
                s_in_suspend = true;
            }
        }
        else if (s_minimized)
        {
            s_minimized = false;
            if (s_in_suspend && renderer)
                renderer->OnResuming();
            s_in_suspend = false;
        }
        else if (!s_in_sizemove && renderer)
        {
            renderer->OnWindowSizeChanged(LOWORD(lParam), HIWORD(lParam));
        }
        break;

    case WM_ENTERSIZEMOVE:
        s_in_sizemove = true;
        break;

    case WM_EXITSIZEMOVE:
        s_in_sizemove = false;
        if (renderer)
        {
            RECT rc;
            GetClientRect(hWnd, &rc);
            renderer->OnWindowSizeChanged(rc.right - rc.left, rc.bottom - rc.top);
        }
        break;

    case WM_GETMINMAXINFO:
        if (lParam)
        {
            auto info = reinterpret_cast<MINMAXINFO*>(lParam);
            info->ptMinTrackSize.x = 320;
            info->ptMinTrackSize.y = 200;
        }
        break;

    case WM_ACTIVATEAPP:
        if (renderer)
        {
            if (wParam)
            {
                renderer->OnActivated();
            }
            else
            {
                renderer->OnDeactivated();
            }
        }
        break;

    case WM_POWERBROADCAST:
        switch (wParam)
        {
        case PBT_APMQUERYSUSPEND:
            if (!s_in_suspend && renderer)
                renderer->OnSuspending();
            s_in_suspend = true;
            return TRUE;

        case PBT_APMRESUMESUSPEND:
            if (!s_minimized)
            {
                if (s_in_suspend && renderer)
                    renderer->OnResuming();
                s_in_suspend = false;
            }
            return TRUE;

        default:
            break;
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_SYSKEYDOWN:
        if (wParam == VK_RETURN && (lParam & 0x60000000) == 0x20000000)
        {
            // Find WindowManager instance by searching for the window handle
            // This is a simplified approach - in a real application you might store the instance
            // differently
            static bool s_fullscreen = false;
            if (s_fullscreen)
            {
                SetWindowLongPtr(hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
                SetWindowLongPtr(hWnd, GWL_EXSTYLE, 0);

                int width = 800;
                int height = 600;
                if (renderer)
                    renderer->GetDefaultSize(width, height);

                ShowWindow(hWnd, SW_SHOWNORMAL);
                SetWindowPos(hWnd, HWND_TOP, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
            }
            else
            {
                SetWindowLongPtr(hWnd, GWL_STYLE, WS_POPUP);
                SetWindowLongPtr(hWnd, GWL_EXSTYLE, WS_EX_TOPMOST);
                SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
                ShowWindow(hWnd, SW_SHOWMAXIMIZED);
            }
            s_fullscreen = !s_fullscreen;
        }
        break;

    case WM_MENUCHAR:
        return MAKELRESULT(0, MNC_CLOSE);

    default:
        break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}