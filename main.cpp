// main.cpp — Точка входа и хуки
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <thread>
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "config.hpp"

// Объявляем функции из других файлов, чтобы main их видел
void RunESP();
void RunAimbot();
void RunSilentAim();
void RunBhop();
void RenderMenu();

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

typedef HRESULT(__stdcall* Present_t)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
Present_t oPresent = nullptr;
WNDPROC oWndProc = nullptr;

ID3D11Device* g_pDevice = nullptr;
ID3D11DeviceContext* g_pContext = nullptr;
ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;
HWND g_GameWindow = nullptr;

LRESULT __stdcall hkWndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (cfg.MenuOpen) {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam)) return true;
        if (uMsg == WM_MOUSEMOVE || uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONUP) return true;
    }
    return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) {
    if (!g_pDevice) {
        pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&g_pDevice);
        g_pDevice->GetImmediateContext(&g_pContext);
        
        DXGI_SWAP_CHAIN_DESC desc;
        pSwapChain->GetDesc(&desc);
        g_GameWindow = desc.OutputWindow;

        ID3D11Texture2D* pBackBuffer = nullptr;
        pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
        if (pBackBuffer) {
            g_pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
            pBackBuffer->Release();
        }

        ImGui::CreateContext();
        ImGui_ImplWin32_Init(g_GameWindow);
        ImGui_ImplDX11_Init(g_pDevice, g_pContext);

        oWndProc = (WNDPROC)SetWindowLongPtrW(g_GameWindow, GWLP_WNDPROC, (LONG_PTR)hkWndProc);
    }

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // Вызов всех функций
    RunESP();
    RunAimbot();
    RunSilentAim();
    RunBhop();
    RenderMenu();

    ImGui::Render();
    g_pContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    return oPresent(pSwapChain, SyncInterval, Flags);
}

void InitHook() {
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, DefWindowProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, L"Dummy", NULL };
    RegisterClassEx(&wc);
    HWND hWnd = CreateWindow(wc.lpszClassName, L"", WS_OVERLAPPEDWINDOW, 0, 0, 100, 100, NULL, NULL, wc.hInstance, NULL);

    DXGI_SWAP_CHAIN_DESC sd = { 0 };
    sd.BufferCount = 1;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;

    IDXGISwapChain* pSwapChain = nullptr;
    ID3D11Device* pDevice = nullptr;
    ID3D11DeviceContext* pContext = nullptr;
    D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, NULL, 0, D3D11_SDK_VERSION, &sd, &pSwapChain, &pDevice, NULL, &pContext);

    if (pSwapChain) {
        void** vTable = *(void***)pSwapChain;
        oPresent = (Present_t)vTable[8];
        DWORD oldProtect;
        VirtualProtect(&vTable[8], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect);
        vTable[8] = (void*)hkPresent;
        VirtualProtect(&vTable[8], sizeof(void*), oldProtect, &oldProtect);
        pSwapChain->Release();
    }
    if (pDevice) pDevice->Release();
    if (pContext) pContext->Release();
    DestroyWindow(hWnd);
    UnregisterClass(wc.lpszClassName, wc.hInstance);
}

DWORD WINAPI MainThread(LPVOID lpReserved) {
    InitHook();
    while (true) {
        if (GetAsyncKeyState(cfg.MenuKey) & 0x8000) {
            cfg.MenuOpen = !cfg.MenuOpen;
            Sleep(200);
        }
        Sleep(50);
    }
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(NULL, 0, MainThread, NULL, 0, NULL);
    }
    return TRUE;
}