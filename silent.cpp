#include <windows.h>
#include <cmath>
#include "math.hpp"
#include "config.hpp"
#include "cs2_offsets.hpp"

void RunSilentAim() {
    if (!cfg.SilentAimEnabled) return;
    
    uintptr_t clientBase = (uintptr_t)GetModuleHandleA("client.dll");
    if (!clientBase) return;

    uintptr_t localPlayer = *(uintptr_t*)(clientBase + Offsets::dwLocalPlayerPawn);
    if (!localPlayer) return;

    // Читаем команду нашего игрока
    int myTeam = *(int*)(localPlayer + Offsets::m_iTeamNum);

    Matrix4x4 viewMatrix = *(Matrix4x4*)(clientBase + Offsets::dwViewMatrix);
    int screenW = GetSystemMetrics(SM_CXSCREEN), screenH = GetSystemMetrics(SM_CYSCREEN);
    Vector2 screenCenter = { (float)screenW / 2.0f, (float)screenH / 2.0f };

    float closestDist = cfg.AimbotFOV;
    Vector3 bestTarget = { 0, 0, 0 };
    bool hasTarget = false;

    for (int i = 1; i <= 64; i++) {
        uintptr_t entity = *(uintptr_t*)(clientBase + Offsets::dwEntityList + i * 0x78);
        if (!entity) continue;

        int health = *(int*)(entity + Offsets::m_iHealth);
        if (health <= 0 || health > 100) continue;

        // ПРОВЕРКА НА СВОИХ
        int entityTeam = *(int*)(entity + Offsets::m_iTeamNum);
        if (entityTeam == myTeam) continue;

        Vector3 origin = *(Vector3*)(entity + Offsets::m_vOldOrigin);
        float boneOffset = (cfg.AimbotBone == 0) ? 72.0f : (cfg.AimbotBone == 1 ? 65.0f : 50.0f);
        Vector3 bonePos = { origin.x, origin.y, origin.z + boneOffset };

        Vector2 boneScreen;
        if (WorldToScreen(bonePos, boneScreen, viewMatrix, screenW, screenH)) {
            float dist = sqrt(pow(boneScreen.x - screenCenter.x, 2) + pow(boneScreen.y - screenCenter.y, 2));
            if (dist < closestDist) {
                closestDist = dist;
                bestTarget = bonePos;
                hasTarget = true;
            }
        }
    }

    if (hasTarget) {
        Vector3 myPos = *(Vector3*)(localPlayer + Offsets::m_vOldOrigin);
        Vector3 angles = CalcAngle(myPos, bestTarget);
        uintptr_t viewAngles = clientBase + Offsets::dwViewAngles;
        
        // Подмена углов (пули летят в цель, а прицел смотрит куда хочешь)
        *(Vector3*)(viewAngles) = angles;
    }
}