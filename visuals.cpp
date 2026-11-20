#include <windows.h>
#include <string>
#include "math.hpp"
#include "config.hpp"
#include "cs2_offsets.hpp"
#include "imgui.h"

void RunESP() {
    if (!cfg.ESPEnabled) return;
    
    uintptr_t clientBase = (uintptr_t)GetModuleHandleA("client.dll");
    if (!clientBase) return;

    uintptr_t localPlayer = *(uintptr_t*)(clientBase + Offsets::dwLocalPlayerPawn);
    if (!localPlayer) return;

    Matrix4x4 viewMatrix = *(Matrix4x4*)(clientBase + Offsets::dwViewMatrix);
    int screenW = GetSystemMetrics(SM_CXSCREEN), screenH = GetSystemMetrics(SM_CYSCREEN);
    ImDrawList* draw = ImGui::GetBackgroundDrawList();

    for (int i = 1; i <= 64; i++) {
        uintptr_t entity = *(uintptr_t*)(clientBase + Offsets::dwEntityList + i * 0x78);
        if (!entity) continue;

        int health = *(int*)(entity + Offsets::m_iHealth);
        if (health <= 0 || health > 100) continue;

        int team = *(int*)(entity + Offsets::m_iTeamNum);
        int myTeam = *(int*)(localPlayer + Offsets::m_iTeamNum);
        if (team == myTeam) continue;

        Vector3 origin = *(Vector3*)(entity + Offsets::m_vOldOrigin);
        Vector2 screenPos;

        if (WorldToScreen(origin, screenPos, viewMatrix, screenW, screenH)) {
            if (cfg.ESPBox) {
                draw->AddRect(ImVec2(screenPos.x - 20, screenPos.y - 40), ImVec2(screenPos.x + 20, screenPos.y + 10), IM_COL32(255, 0, 0, 255), 0.0f, 0, 2.0f);
            }
            if (cfg.ESPHealth) {
                draw->AddText(ImVec2(screenPos.x - 20, screenPos.y - 55), IM_COL32(0, 255, 0, 255), std::to_string(health).c_str());
            }
            if (cfg.ESPSnaplines) {
                draw->AddLine(ImVec2(screenW / 2.0f, (float)screenH), ImVec2(screenPos.x, screenPos.y), IM_COL32(255, 0, 0, 150), 1.5f);
            }
        }
    }
}