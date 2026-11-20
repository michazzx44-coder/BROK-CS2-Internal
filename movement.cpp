#include <windows.h>
#include "config.hpp"
#include "cs2_offsets.hpp"

void RunBhop() {
    if (!cfg.BhopEnabled) return;

    uintptr_t clientBase = (uintptr_t)GetModuleHandleA("client.dll");
    if (!clientBase) return;

    uintptr_t localPlayer = *(uintptr_t*)(clientBase + Offsets::dwLocalPlayerPawn);
    if (!localPlayer) return;

    // Проверяем, находимся ли мы на земле (флаг FL_ONGROUND = 1)
    int flags = *(int*)(localPlayer + Offsets::m_fFlags);
    bool isOnGround = (flags & 1) != 0;

    // Если на земле и зажата кнопка прыжка - ебашим прыжок
    if (isOnGround && (GetAsyncKeyState(cfg.BhopKey) & 0x8000)) {
        INPUT input = { 0 };
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = VK_SPACE;
        
        // Нажимаем пробел
        SendInput(1, &input, sizeof(INPUT));
        
        // Отпускаем пробел (чтобы игра зарегала клик)
        input.ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(1, &input, sizeof(INPUT));
    }
}