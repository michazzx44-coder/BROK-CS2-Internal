#include "imgui.h"
#include "config.hpp"
#include <string>
#include <vector>

// Вспомогательная функция для отрисовки красивых блоков (как в Lumin)
void DrawSection(const char* title, bool& isOpen, void (*contentFunc)()) {
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.12f, 0.12f, 0.14f, 1.0f));
    ImGui::BeginChild(title, ImVec2(0, 0), ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_Borders);
    
    // Заголовок секции
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
    if (ImGui::Selectable(title, isOpen, ImGuiSelectableFlags_None, ImVec2(0, 25))) {
        isOpen = !isOpen;
    }
    ImGui::PopStyleColor();

    if (isOpen) {
        ImGui::Separator();
        ImGui::Spacing();
        contentFunc();
        ImGui::Spacing();
    }
    
    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
    ImGui::Spacing();
}

void RenderAimContent() {
    ImGui::Checkbox("Enable Aimbot", &cfg.AimbotEnabled);
    ImGui::SliderFloat("FOV", &cfg.AimbotFOV, 10.0f, 500.0f, "%.0f");
    ImGui::SliderFloat("Smooth", &cfg.AimbotSmooth, 1.0f, 10.0f, "%.1f");
    const char* bones[] = { "Head", "Neck", "Chest" };
    ImGui::Combo("Hitbox", &cfg.AimbotBone, bones, IM_ARRAYSIZE(bones));
    
    ImGui::Separator();
    ImGui::Checkbox("Silent Aim", &cfg.SilentAimEnabled);
}

void RenderVisualsContent() {
    ImGui::Checkbox("Enable ESP", &cfg.ESPEnabled);
    ImGui::Checkbox("Box", &cfg.ESPBox);
    ImGui::Checkbox("Health Bar", &cfg.ESPHealth);
    ImGui::Checkbox("Snaplines", &cfg.ESPSnaplines);
}

void RenderMovementContent() {
    ImGui::Checkbox("Bunny Hop", &cfg.BhopEnabled);
    ImGui::TextDisabled("Hold Space to auto-jump.");
}

void RenderPlayerContent() {
    static float dummy_scale = 100.0f;
    ImGui::SliderFloat("GUI Scale", &dummy_scale, 100.0f, 200.0f, "%.0f%%");
    ImGui::Checkbox("Player Labels", &cfg.ESPName);
}

void RenderMenu() {
    if (!cfg.MenuOpen) return;

    // --- НАСТРОЙКА СТИЛЯ ---
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 8.0f;
    style.ChildRounding = 6.0f;
    style.FrameRounding = 4.0f;
    style.PopupRounding = 4.0f;
    style.ScrollbarRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.WindowPadding = ImVec2(0, 0);
    
    // Цвета (Темный фон, фиолетовый акцент)
    ImVec4 bg_color = ImVec4(0.08f, 0.08f, 0.09f, 1.0f);
    ImVec4 sidebar_color = ImVec4(0.06f, 0.06f, 0.07f, 1.0f);
    ImVec4 accent_color = ImVec4(0.55f, 0.45f, 0.95f, 1.0f);
    
    style.Colors[ImGuiCol_WindowBg] = bg_color;
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0, 0, 0, 0);
    style.Colors[ImGuiCol_Text] = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
    
    // Акценты
    style.Colors[ImGuiCol_CheckMark] = accent_color;
    style.Colors[ImGuiCol_SliderGrab] = accent_color;
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.65f, 0.55f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.15f, 0.15f, 0.18f, 1.0f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.25f, 0.25f, 0.3f, 1.0f);
    style.Colors[ImGuiCol_ButtonActive] = accent_color;
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.12f, 0.12f, 0.14f, 1.0f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.18f, 0.18f, 0.2f, 1.0f);

    // Главное окно
    ImGui::SetNextWindowSize(ImVec2(750, 500), ImGuiCond_FirstUseEver);
    ImGui::Begin("BROK", &cfg.MenuOpen, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

    // --- САЙДБАР ---
    ImGui::PushStyleColor(ImGuiCol_ChildBg, sidebar_color);
    ImGui::BeginChild("Sidebar", ImVec2(180, 0), true);
    
    // Логотип BROK
    ImGui::SetCursorPos(ImVec2(20, 20));
    ImGui::PushStyleColor(ImGuiCol_Text, accent_color);
    ImGui::Text("B R O K");
    ImGui::PopStyleColor();
    ImGui::SetCursorPosX(20);
    ImGui::TextDisabled("Internal V1.0");
    
    ImGui::SetCursorPosY(80); 
    
    static int active_tab = 0;
    const char* tabs[] = { "Player", "Combat", "Visuals", "Movement" };
    
    for (int i = 0; i < IM_ARRAYSIZE(tabs); i++) {
        bool is_selected = (active_tab == i);
        if (is_selected) ImGui::PushStyleColor(ImGuiCol_Text, accent_color);
        else ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
        
        ImGui::SetCursorPosX(20);
        if (ImGui::Selectable(tabs[i], is_selected, 0, ImVec2(140, 35))) {
            active_tab = i;
        }
        ImGui::PopStyleColor();
    }
    
    ImGui::EndChild();
    ImGui::PopStyleColor();
    
    // --- КОНТЕНТ ---
    ImGui::SameLine();
    ImGui::BeginChild("Content", ImVec2(0, 0), false);
    
    // Верхняя панель
    ImGui::SetCursorPos(ImVec2(20, 15));
    ImGui::BeginChild("TopBar", ImVec2(0, 40), false);
    ImGui::SetCursorPosX(10);
    ImGui::SetNextItemWidth(200);
    static char search_buf[64] = "";
    ImGui::InputTextWithHint("##search", "Search", search_buf, IM_ARRAYSIZE(search_buf));
    
    ImGui::SameLine(ImGui::GetWindowWidth() - 250);
    ImGui::TextDisabled("Profile   Loadout   Match");
    ImGui::EndChild();
    
    ImGui::Separator();
    ImGui::Spacing();

    // Наполнение вкладок
    ImGui::SetCursorPosX(20);
    ImGui::BeginChild("TabContent", ImVec2(0, 0), false);
    
    // Разделение на две колонки
    ImGui::Columns(2, "ContentColumns", false);
    
    static bool open_sections[10] = { true, true, true, true, true, true, true, true, true, true };

    if (active_tab == 0) { // Player
        DrawSection("Profile Settings", open_sections[0], RenderPlayerContent);
    }
    else if (active_tab == 1) { // Combat
        DrawSection("Aimbot Settings", open_sections[1], RenderAimContent);
        ImGui::NextColumn();
        DrawSection("Weapon Settings", open_sections[2], []() {
            ImGui::TextDisabled("No recoil / No spread (Soon)");
        });
    }
    else if (active_tab == 2) { // Visuals
        DrawSection("Player ESP", open_sections[3], RenderVisualsContent);
        ImGui::NextColumn();
        DrawSection("World ESP", open_sections[4], []() {
            ImGui::TextDisabled("Dropped weapons, vehicles (Soon)");
        });
    }
    else if (active_tab == 3) { // Movement
        DrawSection("Movement Exploits", open_sections[5], RenderMovementContent);
    }

    ImGui::Columns(1);
    ImGui::EndChild();
    
    ImGui::EndChild();
    ImGui::End();
}