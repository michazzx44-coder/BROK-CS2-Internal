#pragma once
#include <Windows.h> // ЭТА СТРОКА ИСПРАВЛЯЕТ ОШИБКИ С VK_MBUTTON

struct Config {
    bool MenuOpen = true;
    int MenuKey = VK_MBUTTON; // Mouse 3

    bool ESPEnabled = true;
    bool ESPBox = true;
    bool ESPHealth = true;
    bool ESPSnaplines = true;
    bool ESPName = false; // ЭТА ПЕРЕМЕННАЯ ИСПРАВЛЯЕТ ОШИБКУ В MENU.CPP

    bool AimbotEnabled = false;
    int AimbotBone = 0; 
    float AimbotFOV = 100.0f;
    float AimbotSmooth = 2.0f;
    int AimbotKey = VK_LBUTTON;

    bool SilentAimEnabled = false;
    int SilentAimKey = VK_XBUTTON2; // Mouse 5

    bool BhopEnabled = false;
    int BhopKey = VK_SPACE;
};

extern Config cfg;