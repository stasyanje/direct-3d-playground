//
// Main.cpp
//

#include "pch.h"
#include "src/Application.h"

extern "C"
{
    // Used to enable the "Agility SDK" components
    __declspec(dllexport) extern const UINT D3D12SDKVersion = D3D12_SDK_VERSION;
    __declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\";
}

LPCWSTR g_szAppName = L"direct-3d-playground";

void ExitGame() noexcept;

// Entry point
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    Application app;
    return app.Run(hInstance, nCmdShow);
}

// Exit helper
void ExitGame() noexcept
{
    PostQuitMessage(0);
}
