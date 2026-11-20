// loader.cpp — BROK Premium Loader (GUI) - Resizable & Absolute Path Fix
// build: g++ -O2 -DUNICODE -D_UNICODE -o BROK_Loader.exe loader.cpp imgui.cpp imgui_draw.cpp imgui_tables.cpp imgui_widgets.cpp imgui_impl_win32.cpp imgui_impl_dx11.cpp -lgdi32 -luser32 -ld3d11 -ldxgi -ld3dcompiler -ldxguid -lole32 -luuid -limm32 -ldwmapi -mwindows -static -static-libgcc -static-libstdc++
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <TlHelp32.h>
#include <string>
#include <iostream>
#include <vector>

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// --- DX11 ---
ID3D11Device* g_pd3dDevice = nullptr;
ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
IDXGISwapChain* g_pSwapChain = nullptr;
ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;
HWND g_hWnd = nullptr;

// --- ЛОГИКА ЛОАДЕРА ---
bool g_LoggedIn = false;
char g_KeyInput[64] = "";
std::string g_Status = "Waiting for authorization...";
bool g_KeyError = false;

void CleanupDeviceD3D() {
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

DWORD GetProcessID(const wchar_t* processName) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return 0;
    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(PROCESSENTRY32W);
    if (Process32FirstW(hSnapshot, &pe)) {
        do {
            if (_wcsicmp(pe.szExeFile, processName) == 0) {
                CloseHandle(hSnapshot); return pe.th32ProcessID;
            }
        } while (Process32NextW(hSnapshot, &pe));
    }
    CloseHandle(hSnapshot); return 0;
}

// Функция инжекта (теперь принимает полный путь)
bool InjectDLL(DWORD pid, const std::string& dllPath) {
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProcess) return false;

    void* pMemory = VirtualAllocEx(hProcess, nullptr, dllPath.length() + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!pMemory) { CloseHandle(hProcess); return false; }

    WriteProcessMemory(hProcess, pMemory, dllPath.c_str(), dllPath.length() + 1, nullptr);
    LPTHREAD_START_ROUTINE pLoadLibrary = (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
    
    HANDLE hThread = CreateRemoteThread(hProcess, nullptr, 0, pLoadLibrary, pMemory, 0, nullptr);
    if (hThread) WaitForSingleObject(hThread, INFINITE);

    VirtualFreeEx(hProcess, pMemory, 0, MEM_RELEASE);
    if (hThread) CloseHandle(hThread);
    CloseHandle(hProcess);
    return true;
}

// --- UI ---
void RenderLoaderUI() {
    // Стиль
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 0.0f;
    style.FrameRounding = 4.0f;
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.08f, 0.09f, 1.0f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.55f, 0.45f, 0.95f, 1.0f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.65f, 0.55f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_Text] = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);
    
    // Окно ImGui теперь занимает 100% места внутри Win32 окна
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::Begin("BROK LOADER", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
    
    // Логотип
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2 - 40);
    ImGui::SetCursorPosY(30);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.55f, 0.45f, 0.95f, 1.0f));
    ImGui::Text("B R O K");
    ImGui::PopStyleColor();
    
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2 - 70);
    ImGui::TextDisabled("CS2 Private Build");
    ImGui::SetCursorPosY(80);
    
    ImGui::Separator();
    ImGui::Spacing();

    if (!g_LoggedIn) {
        // Экран входа
        ImGui::SetCursorPosX(50);
        ImGui::Text("Enter License Key:");
        
        ImGui::SetCursorPosX(50);
        ImGui::SetNextItemWidth(ImGui::GetWindowWidth() - 100);
        
        if (g_KeyError) ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.5f, 0.0f, 0.0f, 0.5f));
        ImGui::InputText("##key", g_KeyInput, IM_ARRAYSIZE(g_KeyInput));
        if (g_KeyError) ImGui::PopStyleColor();
        
        ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 100);
        ImGui::SetCursorPosX(50);
        if (ImGui::Button("ACTIVATE", ImVec2(ImGui::GetWindowWidth() - 100, 40))) {
            if (std::string(g_KeyInput) == "BROK-DEMO-1234") {
                g_LoggedIn = true;
                g_Status = "Authorized successfully. Waiting for game...";
                g_KeyError = false;
            } else {
                g_KeyError = true;
                g_Status = "Invalid license key.";
            }
        }
    } else {
        // Главный экран
        ImGui::SetCursorPosX(50);
        ImGui::TextColored(ImVec4(0.55f, 0.45f, 0.95f, 1.0f), "Status: %s", g_Status.c_str());
        ImGui::Spacing();
        ImGui::Separator();
        
        ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 160);
        ImGui::SetCursorPosX(50);
        
        if (ImGui::Button("INJECT CHEAT", ImVec2(ImGui::GetWindowWidth() - 100, 50))) {
            DWORD pid = GetProcessID(L"cs2.exe");
            if (pid == 0) {
                g_Status = "ERROR: cs2.exe not found. Launch the game first.";
            } else {
                g_Status = "Found CS2. Injecting...";
                
                // ПОЛНЫЙ ПУТЬ К DLL
                char currentDir[MAX_PATH];
                GetCurrentDirectoryA(MAX_PATH, currentDir);
                std::string fullPath = std::string(currentDir) + "\\cs2_internal.dll";
                
                if (InjectDLL(pid, fullPath)) {
                    g_Status = "SUCCESS! Cheat injected. Press Mouse 3 in game.";
                } else {
                    g_Status = "ERROR: Injection failed. Run as Administrator.";
                }
            }
        }
        
        ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 90);
        ImGui::SetCursorPosX(50);
        if (ImGui::Button("EXIT", ImVec2(ImGui::GetWindowWidth() - 100, 30))) {
            ExitProcess(0);
        }
    }

    ImGui::End();
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) return true;
    if (msg == WM_DESTROY) { PostQuitMessage(0); return 0; }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

bool CreateDeviceD3D(HWND hWnd) {
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    if (D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
        return false;

    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
    return true;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, L"BROK_LOADER", NULL };
    RegisterClassEx(&wc);
    
    // Создаем окно с рамкой (WS_OVERLAPPEDWINDOW), чтобы его можно было тянуть и менять размер
    g_hWnd = CreateWindow(wc.lpszClassName, L"BROK Loader", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 100, 100, 450, 350, NULL, NULL, wc.hInstance, NULL);
    
    if (!CreateDeviceD3D(g_hWnd)) {
        CleanupDeviceD3D();
        UnregisterClass(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ShowWindow(g_hWnd, SW_SHOWDEFAULT);
    UpdateWindow(g_hWnd);

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    ImGui_ImplWin32_Init(g_hWnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        RenderLoaderUI();

        ImGui::Render();
        const float clear_color[4] = { 0.08f, 0.08f, 0.09f, 1.0f };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0);
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    CleanupDeviceD3D();
    DestroyWindow(g_hWnd);
    UnregisterClass(wc.lpszClassName, wc.hInstance);
    return 0;
}