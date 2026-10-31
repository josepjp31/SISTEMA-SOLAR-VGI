// InterficieUI.cpp : Implementacio dels components visuals (Selector de modes, Menus, Panells)

#include "InterficieUI.h"
#include "ImGui/imgui.h"
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <cmath>

// =========================================================================
// CATALEG DE DADES FISIQUES DELS 9 COSSOS DEL SISTEMA SOLAR (0: Sol .. 8: Neptu)
// =========================================================================
static const CosAstronomicDades kCatalegCossos[9] = {
    { "Sol",     696340.0,  0.000, 274.00,  609.12,   0.00 },
    { "Mercuri",   2439.7,  0.387,   3.70, 1407.60,   0.24 },
    { "Venus",     6051.8,  0.723,   8.87, 5832.50,   0.62 },
    { "Terra",     6371.0,  1.000,   9.81,   23.93,   1.00 },
    { "Mart",      3389.5,  1.524,   3.72,   24.62,   1.88 },
    { "Jupiter",  69911.0,  5.204,  24.79,    9.93,  11.86 },
    { "Saturn",   58232.0,  9.582,  10.44,   10.70,  29.45 },
    { "Ura",      25362.0, 19.218,   8.69,   17.24,  84.02 },
    { "Neptu",    24622.0, 30.110,  11.15,   16.11, 164.79 }
};

void InterficieUI::ActualitzarDadesCosActiu(EstatSimulacio& estat)
{
    int id = estat.cosEnfocatId;
    if (id < 0 || id >= 9) id = 0;
    estat.dadesCosActiu = kCatalegCossos[id];
}

// =========================================================================
// 1. INICIALITZACIO D'ESTIL VISUAL
// =========================================================================
void InterficieUI::InicialitzarEstil()
{
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 6.0f;
    style.FrameRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.PopupRounding = 6.0f;
    style.ScrollbarRounding = 4.0f;
    style.TabRounding = 4.0f;

    // Tema fosc adaptat a simulacio espacial
    ImGui::StyleColorsDark();

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg]           = ImVec4(0.08f, 0.10f, 0.14f, 0.94f);
    colors[ImGuiCol_Header]             = ImVec4(0.18f, 0.35f, 0.58f, 0.65f);
    colors[ImGuiCol_HeaderHovered]      = ImVec4(0.24f, 0.45f, 0.72f, 0.80f);
    colors[ImGuiCol_HeaderActive]       = ImVec4(0.28f, 0.52f, 0.82f, 1.00f);
    colors[ImGuiCol_Button]             = ImVec4(0.16f, 0.32f, 0.54f, 0.75f);
    colors[ImGuiCol_ButtonHovered]      = ImVec4(0.22f, 0.42f, 0.68f, 0.90f);
    colors[ImGuiCol_ButtonActive]       = ImVec4(0.14f, 0.28f, 0.48f, 1.00f);
    colors[ImGuiCol_FrameBg]            = ImVec4(0.12f, 0.16f, 0.22f, 0.80f);
    colors[ImGuiCol_FrameBgHovered]     = ImVec4(0.18f, 0.24f, 0.32f, 0.85f);
    colors[ImGuiCol_FrameBgActive]      = ImVec4(0.22f, 0.30f, 0.40f, 1.00f);
    colors[ImGuiCol_TitleBg]            = ImVec4(0.06f, 0.08f, 0.12f, 1.00f);
    colors[ImGuiCol_TitleBgActive]      = ImVec4(0.10f, 0.18f, 0.30f, 1.00f);
    colors[ImGuiCol_MenuBarBg]          = ImVec4(0.07f, 0.09f, 0.13f, 0.96f);
}

// =========================================================================
// 2. RENDERITZADOR PRINCIPAL
// =========================================================================
void InterficieUI::Renderitzar(EstatSimulacio& estat, Escenari*& currentEscenari, Escenari escenaris[], GLuint& shader_programID)
{
    // 1. Selector inicial de modes (2 quadrats centrals amb escalat adaptatiu)
    // En aquesta fase inicial nomes es mostren els quadrats dels modes sobre fons fosc.
    if (estat.mostrarSelectorModes) {
        DibuixarSelectorInicialModes(estat, currentEscenari, escenaris, shader_programID);
        return;
    }

    // 2. Un cop seleccionat el mode, s'activen la barra de menu superior i els panells corresponents
    DibuixarBarraMenuSuperior(estat, currentEscenari, escenaris, shader_programID);

    // 3. Selector de data/hora i timelapse (cantonada inferior dreta, nomes en mode escala real)
    if (estat.modeActual == ModeSimulacio::ESCALA_REAL && estat.mostrarPanellTemps) {
        DibuixarPanellDataHoraTimelapse(estat);
    }

    // 4. Fitxa astronomica HUD (telemetria de l'astre actiu)
    if (estat.mostrarFitxaHUD) {
        DibuixarFitxaAstronomicaHUD(estat.dadesCosActiu, &estat.mostrarFitxaHUD);
    }

    // 5. Dialeg "Quant al Simulador"
    if (estat.mostrarDialegQuantA) {
        ImGui::SetNextWindowSize(ImVec2(480, 260), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Quant al Simulador Interactiu del Sistema Solar", &estat.mostrarDialegQuantA, ImGuiWindowFlags_NoResize))
        {
            ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "SIMULADOR INTERACTIU DEL SISTEMA SOLAR");
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::TextWrapped("Projecte de Visualitzacio Grafica Interactiva (VGI) - Escola d'Enginyeria UAB.");
            ImGui::Spacing();
            ImGui::BulletText("Modes: Didactic / Intuitiu i Cientific a Escala Real.");
            ImGui::BulletText("Motor 3D: C++, OpenGL 4.6 Core Profile, GLFW 3.4, GLEW, GLM.");
            ImGui::BulletText("Interficie d'usuari: Dear ImGui v1.89+ (Immediate Mode GUI).");
            ImGui::BulletText("Efemerides astronomiques: NASA SPICE Toolkit / Algorismes Keplerians.");
            ImGui::Spacing();
            ImGui::Separator();
            if (ImGui::Button("Tancar", ImVec2(100, 28))) {
                estat.mostrarDialegQuantA = false;
            }
        }
        ImGui::End();
    }
}

// =========================================================================
// HELPER PER A ELEMENTS DE LLISTA AMB PUNT (BULLET) I SALT DE LINIA AUTOMATIC
// =========================================================================
static void DibuixarPuntLlista(const char* text)
{
    ImGui::Bullet();
    ImGui::SameLine();
    ImGui::TextWrapped("%s", text);
    ImGui::Spacing();
}

// =========================================================================
// 2.1 SELECTOR INICIAL DE MODES (MODAL CENTRAL ESCALABLE AMB 2 QUADRATS)
// =========================================================================
void InterficieUI::DibuixarSelectorInicialModes(EstatSimulacio& estat, Escenari*& currentEscenari, Escenari escenaris[], GLuint& shader_programID)
{
    if (!estat.mostrarSelectorModes) return;

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    float viewW = viewport->WorkSize.x;
    float viewH = viewport->WorkSize.y;

    float factorEscala = 1.0f;
    if (viewW >= 2400.0f)      factorEscala = 1.35f;
    else if (viewW >= 1700.0f) factorEscala = 1.20f;
    else if (viewW >= 1200.0f) factorEscala = 1.10f;
    else if (viewW >= 900.0f)  factorEscala = 1.00f;
    else                       factorEscala = 0.90f;

    float ampleModal = 840.0f * factorEscala;
    float alcadaModal = 520.0f * factorEscala;

    if (viewW >= 1600.0f) {
        ampleModal = (viewW * 0.62f < 1200.0f) ? 1200.0f : (viewW * 0.62f);
        if (ampleModal > 1350.0f) ampleModal = 1350.0f;
        alcadaModal = (viewH * 0.74f < 720.0f) ? 720.0f : (viewH * 0.74f);
        if (alcadaModal > 820.0f) alcadaModal = 820.0f;
    } else if (viewW >= 1100.0f) {
        ampleModal = (viewW * 0.75f < 950.0f) ? 950.0f : (viewW * 0.75f);
        if (ampleModal > 1050.0f) ampleModal = 1050.0f;
        alcadaModal = (viewH * 0.75f < 560.0f) ? 560.0f : (viewH * 0.75f);
        if (alcadaModal > 660.0f) alcadaModal = 660.0f;
    } else {
        ampleModal = viewW * 0.95f;
        alcadaModal = viewH * 0.92f;
    }

    ImVec2 midaModal(ampleModal, alcadaModal);
    ImVec2 posModal(viewport->WorkPos.x + (viewW - midaModal.x) * 0.5f,
                    viewport->WorkPos.y + (viewH - midaModal.y) * 0.5f);

    // Dibuixar un fons fosc complet sobre tota la pantalla (sense sistema solar darrere)
    ImDrawList* bgDrawList = ImGui::GetBackgroundDrawList();
    if (bgDrawList != nullptr)
    {
        bgDrawList->AddRectFilled(
            viewport->Pos, 
            ImVec2(viewport->Pos.x + viewport->Size.x, viewport->Pos.y + viewport->Size.y), 
            IM_COL32(8, 12, 20, 255)
        );
    }

    ImGui::SetNextWindowPos(posModal, ImGuiCond_Always);
    ImGui::SetNextWindowSize(midaModal, ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.97f);

    ImGuiWindowFlags flagsModal = ImGuiWindowFlags_NoDecoration | 
                                  ImGuiWindowFlags_NoMove | 
                                  ImGuiWindowFlags_NoSavedSettings;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(14, 20, 32, 252));
    ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(50, 80, 130, 230));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(24.0f * factorEscala, 20.0f * factorEscala));

    if (ImGui::Begin("SelectorModesModal", nullptr, flagsModal))
    {
        ImGui::SetWindowFontScale(factorEscala);

        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
        float ampleTitol = ImGui::CalcTextSize("SIMULADOR INTERACTIU DEL SISTEMA SOLAR").x;
        ImGui::SetCursorPosX((midaModal.x - ampleTitol) * 0.5f);
        ImGui::Text("SIMULADOR INTERACTIU DEL SISTEMA SOLAR");
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(185, 215, 250, 255));
        float ampleSub = ImGui::CalcTextSize("Trieu el mode de representacio per iniciar la simulacio").x;
        ImGui::SetCursorPosX((midaModal.x - ampleSub) * 0.5f);
        ImGui::Text("Trieu el mode de representacio per iniciar la simulacio");
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Separator, IM_COL32(50, 80, 130, 180));
        ImGui::Separator();
        ImGui::PopStyleColor();
        ImGui::Spacing();

        float espaiCapcalera = ImGui::GetCursorPosY();
        float margeInferior = 18.0f * factorEscala;
        float alcadaTargeta = midaModal.y - espaiCapcalera - margeInferior;
        if (alcadaTargeta < 200.0f) alcadaTargeta = 200.0f;

        float separacio = 24.0f * factorEscala;
        float margeLateral = 24.0f * factorEscala;
        float ampleTargeta = (midaModal.x - (margeLateral * 2.0f + separacio)) * 0.5f;

        ImVec2 midaTargeta(ampleTargeta, alcadaTargeta);
        float alcadaBoto = 44.0f * factorEscala;

        float textBotoMax = ImGui::CalcTextSize("INICIAR MODE A ESCALA REAL").x;
        float ampleBoto = (midaTargeta.x * 0.85f > textBotoMax + 32.0f) ? (midaTargeta.x * 0.85f) : (textBotoMax + 32.0f);
        if (ampleBoto > 420.0f * factorEscala) ampleBoto = 420.0f * factorEscala;
        if (ampleBoto > midaTargeta.x - 16.0f) ampleBoto = midaTargeta.x - 16.0f;

        // TARGETA 1: MODE DIDACTIC / INTUITIU
        ImGui::SetCursorPosX(margeLateral);
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 10.0f * factorEscala);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(18.0f * factorEscala, 16.0f * factorEscala));
        ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(18, 32, 54, 250));
        ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(0, 190, 255, 230));
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(235, 245, 255, 255));

        ImGuiWindowFlags flagsTargeta = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
        if (ImGui::BeginChild("CardModeIntuitiu", midaTargeta, true, flagsTargeta))
        {
            ImGui::SetWindowFontScale(factorEscala);
            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 225, 255, 255));
            ImGui::Text("   MODE DIDACTIC / INTUITIU");
            ImGui::PopStyleColor();

            ImGui::PushStyleColor(ImGuiCol_Separator, IM_COL32(0, 170, 230, 150));
            ImGui::Separator();
            ImGui::PopStyleColor();
            ImGui::Spacing();

            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(210, 230, 255, 255));
            ImGui::TextWrapped("Exploracio visual adaptada per a l'aprenentatge:");
            ImGui::PopStyleColor();
            ImGui::Spacing();

            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(235, 245, 255, 255));
            DibuixarPuntLlista("Orbites circulars coplanars.");
            DibuixarPuntLlista("Distancies i radis ajustats visualment.");
            DibuixarPuntLlista("Tots els planetes visibles simultaniament.");
            DibuixarPuntLlista("Velocitats d'orbita proporcionals.");
            DibuixarPuntLlista("Camera orbital inicial a 25 graus.");
            DibuixarPuntLlista("Focus directe a qualsevol planeta.");
            ImGui::PopStyleColor();

            float padY = 16.0f * factorEscala;
            float espaiFinal = ImGui::GetStyle().ItemSpacing.y;
            float yBotoDesejat = midaTargeta.y - alcadaBoto - padY - espaiFinal - 4.0f;
            float cursorYActual = ImGui::GetCursorPosY();
            if (yBotoDesejat > cursorYActual + 8.0f) {
                ImGui::SetCursorPosY(yBotoDesejat);
            } else {
                ImGui::Spacing();
            }

            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 120, 215, 255));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(0, 160, 255, 255));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(0, 90, 180, 255));

            ImGui::SetCursorPosX((midaTargeta.x - ampleBoto) * 0.5f);
            if (ImGui::Button("INICIAR MODE DIDACTIC", ImVec2(ampleBoto, alcadaBoto)))
            {
                estat.modeActual = ModeSimulacio::INTUITIU;
                estat.mostrarSelectorModes = false;
                estat.mostrarPanellTemps = false;
                estat.cosEnfocatId = 0;
                estat.cameraConfig.angleInclinacio = 25.0f;
                estat.cameraConfig.mode = ModeCamera::FIXA_INCLINADA_25;
                estat.configuracioInicialPendent = true;
                estat.canviCameraPendent = true;
                ActualitzarDadesCosActiu(estat);

                // Creacio de l'escenari corresponent (index 0: Mode Didactic)
                if (escenaris != nullptr)
                {
                    currentEscenari = &escenaris[0];
                    currentEscenari->creaEscenari(shader_programID);
                }
            }
            ImGui::PopStyleColor(4);
        }
        ImGui::EndChild();
        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar(2);

        // TARGETA 2: MODE A ESCALA REAL (CIENTIFIC)
        ImGui::SameLine(margeLateral + midaTargeta.x + separacio);
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 10.0f * factorEscala);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(18.0f * factorEscala, 16.0f * factorEscala));
        ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(46, 32, 18, 250));
        ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(255, 165, 2, 230));
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 245, 230, 255));

        if (ImGui::BeginChild("CardModeEscalaReal", midaTargeta, true, flagsTargeta))
        {
            ImGui::SetWindowFontScale(factorEscala);
            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 195, 45, 255));
            ImGui::Text("   MODE A ESCALA REAL (CIENTIFIC)");
            ImGui::PopStyleColor();

            ImGui::PushStyleColor(ImGuiCol_Separator, IM_COL32(255, 165, 2, 150));
            ImGui::Separator();
            ImGui::PopStyleColor();
            ImGui::Spacing();

            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 230, 205, 255));
            ImGui::TextWrapped("Simulacio astronomica fidel a la fisica real:");
            ImGui::PopStyleColor();
            ImGui::Spacing();

            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 245, 230, 255));
            DibuixarPuntLlista("Distancies i radis astronomics reals.");
            DibuixarPuntLlista("Orbites el-liptiques precises (SPICE).");
            DibuixarPuntLlista("Selector complet de Data, Hora i Efemerides.");
            DibuixarPuntLlista("Timelapse regulable: 1 min/s a 20 dies/s.");
            DibuixarPuntLlista("Possibilitat d'avancar o rebobinar.");
            DibuixarPuntLlista("Camera coet en 1a persona i marcadors.");
            ImGui::PopStyleColor();

            float padY = 16.0f * factorEscala;
            float espaiFinal = ImGui::GetStyle().ItemSpacing.y;
            float yBotoDesejat = midaTargeta.y - alcadaBoto - padY - espaiFinal - 4.0f;
            float cursorYActual = ImGui::GetCursorPosY();
            if (yBotoDesejat > cursorYActual + 8.0f) {
                ImGui::SetCursorPosY(yBotoDesejat);
            } else {
                ImGui::Spacing();
            }

            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(215, 115, 0, 255));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(255, 145, 10, 255));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(180, 90, 0, 255));

            ImGui::SetCursorPosX((midaTargeta.x - ampleBoto) * 0.5f);
            if (ImGui::Button("INICIAR MODE A ESCALA REAL", ImVec2(ampleBoto, alcadaBoto)))
            {
                estat.modeActual = ModeSimulacio::ESCALA_REAL;
                estat.mostrarSelectorModes = false;
                estat.mostrarPanellTemps = true;
                estat.cosEnfocatId = 0;
                estat.cameraConfig.mode = ModeCamera::ORBITAL_CENTRADOR;
                estat.configuracioInicialPendent = true;
                estat.canviCameraPendent = true;
                ActualitzarDadesCosActiu(estat);

                // Creacio de l'escenari corresponent (index 1: Mode a Escala Real)
                if (escenaris != nullptr)
                {
                    currentEscenari = &escenaris[1];
                    currentEscenari->creaEscenari(shader_programID);
                }
            }
            ImGui::PopStyleColor(4);
        }
        ImGui::EndChild();
        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar(2);
    }
    ImGui::End();
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(3);
}

// =========================================================================
// 2.2 BARRA DE MENU SUPERIOR DINAMICA PER MODE (MAIN MENU BAR)
// =========================================================================
void InterficieUI::DibuixarBarraMenuSuperior(EstatSimulacio& estat, Escenari*& currentEscenari, Escenari escenaris[], GLuint& shader_programID)
{
    const ImGuiViewport* vp = ImGui::GetMainViewport();
    float viewW = vp->WorkSize.x;

    float t = (viewW - 900.0f) / (1920.0f - 900.0f);
    if (t < 0.0f) t = 0.0f;
    if (t > 1.25f) t = 1.25f;

    float factorFontMenu = 1.0f + 0.35f * t;
    float padY = 5.0f + 7.0f * t;
    float padX = 10.0f + 12.0f * t;

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(padX, padY));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(padX * 1.15f, padY));
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(235, 245, 255, 255));

    if (!ImGui::BeginMainMenuBar())
    {
        ImGui::PopStyleColor();
        ImGui::PopStyleVar(2);
        return;
    }

    ImGui::SetWindowFontScale(factorFontMenu);

    static const char* nomsPlanetesMenu[9] = {
        "Sol (Centre)", "Mercuri", "Venus", "Terra",
        "Mart", "Jupiter", "Saturn", "Ura", "Neptu"
    };

    if (estat.modeActual == ModeSimulacio::INTUITIU)
    {
        // ==================== MENUS MODE DIDACTIC / INTUITIU ====================
        if (ImGui::BeginMenu("Escenari"))
        {
            ImGui::SetWindowFontScale(factorFontMenu);
            if (ImGui::MenuItem("Canviar de Mode (Selector inicial)...")) {
                estat.mostrarSelectorModes = true;
            }
            if (ImGui::MenuItem("Commutar a Mode a Escala Real")) {
                estat.modeActual = ModeSimulacio::ESCALA_REAL;
                estat.mostrarPanellTemps = true;
                estat.configuracioInicialPendent = true;
                estat.canviCameraPendent = true;
                if (escenaris != nullptr) {
                    currentEscenari = &escenaris[1];
                    currentEscenari->creaEscenari(shader_programID);
                }
            }
            ImGui::Separator();
            if (ImGui::MenuItem(estat.tempsSimulacio.enPausa ? "Reprendre Moviment Orbital" : "Pausar Moviment Orbital", "Espai")) {
                estat.tempsSimulacio.enPausa = !estat.tempsSimulacio.enPausa;
            }
            if (ImGui::MenuItem("Reiniciar Vista Didactica", "R")) {
                estat.cosEnfocatId = 0;
                estat.cameraConfig.angleInclinacio = 25.0f;
                estat.cameraConfig.mode = ModeCamera::FIXA_INCLINADA_25;
                estat.canviCameraPendent = true;
                ActualitzarDadesCosActiu(estat);
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Sortir", "Alt+F4")) {
                GLFWwindow* ctx = glfwGetCurrentContext();
                if (ctx) glfwSetWindowShouldClose(ctx, GLFW_TRUE);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Planetes & Focus"))
        {
            ImGui::SetWindowFontScale(factorFontMenu);
            for (int i = 0; i < 9; ++i) {
                bool seleccionat = (estat.cosEnfocatId == i);
                if (ImGui::MenuItem(nomsPlanetesMenu[i], nullptr, seleccionat)) {
                    estat.cosEnfocatId = i;
                    estat.cameraConfig.mode = ModeCamera::ORBITAL_CENTRADOR;
                    estat.canviCameraPendent = true;
                    ActualitzarDadesCosActiu(estat);
                }
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Tornar a la Vista Global del Sol", "Home")) {
                estat.cosEnfocatId = 0;
                estat.cameraConfig.mode = ModeCamera::GLOBAL_SISTEMA;
                estat.canviCameraPendent = true;
                ActualitzarDadesCosActiu(estat);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Camera"))
        {
            ImGui::SetWindowFontScale(factorFontMenu);
            if (ImGui::MenuItem("Camera Orbital Centrada (Esferica)", nullptr, estat.cameraConfig.mode == ModeCamera::ORBITAL_CENTRADOR)) {
                estat.cameraConfig.mode = ModeCamera::ORBITAL_CENTRADOR;
                estat.canviCameraPendent = true;
            }
            if (ImGui::MenuItem("Camera Fixa Inicial (Inclinada 25 graus)", nullptr, estat.cameraConfig.mode == ModeCamera::FIXA_INCLINADA_25)) {
                estat.cameraConfig.mode = ModeCamera::FIXA_INCLINADA_25;
                estat.cameraConfig.angleInclinacio = 25.0f;
                estat.canviCameraPendent = true;
            }
            if (ImGui::MenuItem("Camera Lliure (Navega)", nullptr, estat.cameraConfig.mode == ModeCamera::COET_PRIMERA_PERSONA)) {
                estat.cameraConfig.mode = ModeCamera::COET_PRIMERA_PERSONA;
                estat.canviCameraPendent = true;
            }
            if (ImGui::MenuItem("Vista Global del Sistema", nullptr, estat.cameraConfig.mode == ModeCamera::GLOBAL_SISTEMA)) {
                estat.cosEnfocatId = 0;
                estat.cameraConfig.mode = ModeCamera::GLOBAL_SISTEMA;
                estat.canviCameraPendent = true;
                ActualitzarDadesCosActiu(estat);
            }
            ImGui::Separator();
            ImGui::TextDisabled("Controls de moviment:");
            ImGui::BulletText("Ratoli (Clic Esquerre) / Fletxes: Gir orbital");
            ImGui::BulletText("Roda del Ratoli / Amunt-Avall: Zoom");
            ImGui::BulletText("Mode Lliure (Navega): Fletxes + W/S/A/D");
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Visualitzacio"))
        {
            ImGui::SetWindowFontScale(factorFontMenu);
            ImGui::MenuItem("Dibuixar Orbites Circulars", nullptr, &estat.opcionsVisualitzacio.mostrarOrbites);
            ImGui::MenuItem("Mostrar Noms i Etiquetes 2D", nullptr, &estat.opcionsVisualitzacio.mostrarEtiquetes);
            ImGui::MenuItem("Il-luminacio Solar Directa", nullptr, &estat.opcionsVisualitzacio.iluminacioSolarDirecta);
            ImGui::MenuItem("Vista Clara (Llum Ambient)", nullptr, &estat.opcionsVisualitzacio.vistaClaraAmbient);
            ImGui::Separator();

            if (ImGui::BeginMenu("Model d'Anells de Saturn")) {
                ImGui::SetWindowFontScale(factorFontMenu);
                if (ImGui::MenuItem("Disc pla amb textura", nullptr, !estat.opcionsVisualitzacio.anellsSaturnParticules)) {
                    estat.opcionsVisualitzacio.anellsSaturnParticules = false;
                }
                if (ImGui::MenuItem("Particules instanciades (gluDisc)", nullptr, estat.opcionsVisualitzacio.anellsSaturnParticules)) {
                    estat.opcionsVisualitzacio.anellsSaturnParticules = true;
                }
                ImGui::EndMenu();
            }

            ImGui::MenuItem("Fons Espacial (Skybox d'estrelles)", nullptr, &estat.opcionsVisualitzacio.mostrarSkybox);
            ImGui::MenuItem("Eixos de Coordenades", nullptr, &estat.opcionsVisualitzacio.mostrarEixos);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Ajuda"))
        {
            ImGui::SetWindowFontScale(factorFontMenu);
            ImGui::MenuItem("Mostrar Fitxa de l'Astre (HUD)", nullptr, &estat.mostrarFitxaHUD);
            ImGui::Separator();
            if (ImGui::MenuItem("Quant al Simulador...")) {
                estat.mostrarDialegQuantA = true;
            }
            ImGui::EndMenu();
        }
    }
    else if (estat.modeActual == ModeSimulacio::ESCALA_REAL)
    {
        // ==================== MENUS MODE A ESCALA REAL ====================
        if (ImGui::BeginMenu("Escenari"))
        {
            ImGui::SetWindowFontScale(factorFontMenu);
            if (ImGui::MenuItem("Canviar de Mode (Selector inicial)...")) {
                estat.mostrarSelectorModes = true;
            }
            if (ImGui::MenuItem("Commutar a Mode Didactic")) {
                estat.modeActual = ModeSimulacio::INTUITIU;
                estat.mostrarPanellTemps = false;
                estat.cameraConfig.mode = ModeCamera::FIXA_INCLINADA_25;
                estat.configuracioInicialPendent = true;
                estat.canviCameraPendent = true;
                if (escenaris != nullptr) {
                    currentEscenari = &escenaris[0];
                    currentEscenari->creaEscenari(shader_programID);
                }
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Reiniciar Simulacio a Escala", "R")) {
                estat.cosEnfocatId = 0;
                estat.cameraConfig.mode = ModeCamera::ORBITAL_CENTRADOR;
                estat.canviCameraPendent = true;
                ActualitzarDadesCosActiu(estat);
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Sortir", "Alt+F4")) {
                GLFWwindow* ctx = glfwGetCurrentContext();
                if (ctx) glfwSetWindowShouldClose(ctx, GLFW_TRUE);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Efemerides & Temps"))
        {
            ImGui::SetWindowFontScale(factorFontMenu);
            ImGui::MenuItem("Mostrar Panell Data/Hora (Dreta Inferior)", nullptr, &estat.mostrarPanellTemps);
            ImGui::Separator();
            if (ImGui::MenuItem(estat.tempsSimulacio.enPausa ? "Reprendre Temps" : "Pausar Temps", "Espai")) {
                estat.tempsSimulacio.enPausa = !estat.tempsSimulacio.enPausa;
            }
            if (ImGui::MenuItem("Invertir Sentit (Enrere / Endavant)", "Backspace")) {
                estat.tempsSimulacio.direccioTemps *= -1;
            }
            ImGui::Separator();
            if (ImGui::BeginMenu("Dreceres de Data")) {
                ImGui::SetWindowFontScale(factorFontMenu);
                if (ImGui::MenuItem("Data Actual del Sistema (Ara)")) {
                    estat.tempsSimulacio.EstablirDataActual();
                }
                if (ImGui::MenuItem("Epoca J2000.0 (1 Gen 2000 12:00 UTC)")) {
                    estat.tempsSimulacio.EstablirJ2000();
                }
                if (ImGui::MenuItem("Eclipsi Solar Total (12 Agost 2026)")) {
                    estat.tempsSimulacio.dataHora = { 2026, 8, 12, 17, 30, 0 };
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Velocitats Rapides de Timelapse")) {
                ImGui::SetWindowFontScale(factorFontMenu);
                if (ImGui::MenuItem("Minim: 1 min/s")) estat.tempsSimulacio.factorVelocitatSegonsPerSegon = 60.0;
                if (ImGui::MenuItem("1 hora/s")) estat.tempsSimulacio.factorVelocitatSegonsPerSegon = 3600.0;
                if (ImGui::MenuItem("1 dia/s")) estat.tempsSimulacio.factorVelocitatSegonsPerSegon = 86400.0;
                if (ImGui::MenuItem("5 dies/s")) estat.tempsSimulacio.factorVelocitatSegonsPerSegon = 432000.0;
                if (ImGui::MenuItem("Maxim: 20 dies/s")) estat.tempsSimulacio.factorVelocitatSegonsPerSegon = 1728000.0;
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Origen & Cameres"))
        {
            ImGui::SetWindowFontScale(factorFontMenu);
            ImGui::TextDisabled("Punt d'Origen de Referencia:");
            if (ImGui::RadioButton("Heliocentric (Sol al centre)", estat.origenReferenciaSol)) {
                estat.origenReferenciaSol = true;
                estat.cosEnfocatId = 0;
                estat.canviCameraPendent = true;
                ActualitzarDadesCosActiu(estat);
            }
            if (ImGui::RadioButton("Planeta Seleccionat com a Centre", !estat.origenReferenciaSol)) {
                estat.origenReferenciaSol = false;
                estat.canviCameraPendent = true;
            }

            ImGui::Separator();
            ImGui::TextDisabled("Modes de Camera:");
            if (ImGui::MenuItem("Camera Orbital Centrada", nullptr, estat.cameraConfig.mode == ModeCamera::ORBITAL_CENTRADOR)) {
                estat.cameraConfig.mode = ModeCamera::ORBITAL_CENTRADOR;
                estat.canviCameraPendent = true;
            }
            if (ImGui::MenuItem("Camera Fixa Inclinada (25 graus)", nullptr, estat.cameraConfig.mode == ModeCamera::FIXA_INCLINADA_25)) {
                estat.cameraConfig.mode = ModeCamera::FIXA_INCLINADA_25;
                estat.cameraConfig.angleInclinacio = 25.0f;
                estat.canviCameraPendent = true;
            }
            if (ImGui::MenuItem("Camera Coet en 1a Persona (Navega)", nullptr, estat.cameraConfig.mode == ModeCamera::COET_PRIMERA_PERSONA)) {
                estat.cameraConfig.mode = ModeCamera::COET_PRIMERA_PERSONA;
                estat.canviCameraPendent = true;
            }
            if (ImGui::MenuItem("Vista Global del Sistema", nullptr, estat.cameraConfig.mode == ModeCamera::GLOBAL_SISTEMA)) {
                estat.cosEnfocatId = 0;
                estat.cameraConfig.mode = ModeCamera::GLOBAL_SISTEMA;
                estat.canviCameraPendent = true;
                ActualitzarDadesCosActiu(estat);
            }

            ImGui::Separator();
            if (ImGui::BeginMenu("Centrar Camera a...")) {
                ImGui::SetWindowFontScale(factorFontMenu);
                for (int i = 0; i < 9; ++i) {
                    bool seleccionat = (estat.cosEnfocatId == i);
                    if (ImGui::MenuItem(nomsPlanetesMenu[i], nullptr, seleccionat)) {
                        estat.cosEnfocatId = i;
                        estat.origenReferenciaSol = (i == 0);
                        estat.cameraConfig.mode = ModeCamera::ORBITAL_CENTRADOR;
                        estat.canviCameraPendent = true;
                        ActualitzarDadesCosActiu(estat);
                    }
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Visualitzacio Realista"))
        {
            ImGui::SetWindowFontScale(factorFontMenu);
            ImGui::MenuItem("Orbites El-liptiques Reals (SPICE)", nullptr, &estat.opcionsVisualitzacio.mostrarOrbites);
            ImGui::MenuItem("Mostrar Noms i Etiquetes 2D", nullptr, &estat.opcionsVisualitzacio.mostrarEtiquetes);
            ImGui::MenuItem("Marcadors Lluminosos (Halos Cossos Petits)", nullptr, &estat.opcionsVisualitzacio.mostrarMarcadorsPetits);
            ImGui::MenuItem("Il-luminacio Solar Directa", nullptr, &estat.opcionsVisualitzacio.iluminacioSolarDirecta);
            ImGui::MenuItem("Vista Clara (Llum Ambient)", nullptr, &estat.opcionsVisualitzacio.vistaClaraAmbient);
            ImGui::MenuItem("Cinturo d'Asteroides (Instancing)", nullptr, &estat.opcionsVisualitzacio.mostrarCinturoAsteroides);
            ImGui::MenuItem("Punts de Lagrange (L1 - L5)", nullptr, &estat.opcionsVisualitzacio.mostrarPuntsLagrange);
            ImGui::MenuItem("Fons Espacial (Skybox)", nullptr, &estat.opcionsVisualitzacio.mostrarSkybox);
            ImGui::MenuItem("Eixos de l'Ecliptica", nullptr, &estat.opcionsVisualitzacio.mostrarEixos);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Dades Cientifiques"))
        {
            ImGui::SetWindowFontScale(factorFontMenu);
            ImGui::TextDisabled("Font d'Efemerides:");
            ImGui::BulletText("NASA SPICE Toolkit for C");
            ImGui::Separator();
            ImGui::MenuItem("Fitxa Tecnica i Telemetria (HUD)", nullptr, &estat.mostrarFitxaHUD);
            ImGui::Separator();
            if (ImGui::MenuItem("Quant al Simulador...")) {
                estat.mostrarDialegQuantA = true;
            }
            ImGui::EndMenu();
        }
    }

    // ==================== ZONA DRETA: INDICADOR DE MODE I RENDIMENT ====================
    float ampleTotal = ImGui::GetWindowWidth();

    const char* etiquetaMode = (estat.modeActual == ModeSimulacio::INTUITIU) 
        ? "[MODE DIDACTIC / INTUITIU]" 
        : "[MODE A ESCALA REAL]";
    ImVec4 colorMode = (estat.modeActual == ModeSimulacio::INTUITIU)
        ? ImVec4(0.2f, 0.9f, 0.4f, 1.0f)
        : ImVec4(1.0f, 0.75f, 0.2f, 1.0f);

    float fps = ImGui::GetIO().Framerate;
    char bufferFps[32];
    snprintf(bufferFps, sizeof(bufferFps), "%.1f FPS", fps);

    float ampleEtiqueta = ImGui::CalcTextSize(etiquetaMode).x;
    float ampleBotoCanviar = ImGui::CalcTextSize("Canviar").x + (padX * 2.0f) + 12.0f;
    float ampleFps = ImGui::CalcTextSize(bufferFps).x;
    float espaiSeparadors = padX * 3.0f;

    float ampleWidgetsDreta = ampleEtiqueta + ampleBotoCanviar + ampleFps + espaiSeparadors + 20.0f;

    float xActual = ImGui::GetCursorPosX();
    float xDesti = ampleTotal - ampleWidgetsDreta;

    if (xDesti > xActual + 15.0f) {
        ImGui::SameLine(xDesti);
    } else {
        ImGui::SameLine();
    }

    ImGui::TextColored(colorMode, "%s", etiquetaMode);
    ImGui::SameLine();

    if (ImGui::Button("Canviar")) {
        estat.mostrarSelectorModes = true;
    }
    ImGui::SameLine();

    ImGui::TextColored(fps > 45.0f ? ImVec4(0.3f, 1.0f, 0.3f, 1.0f) : ImVec4(1.0f, 0.4f, 0.4f, 1.0f), 
                       "%s", bufferFps);

    ImGui::EndMainMenuBar();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
}

// =========================================================================
// 2.3 SELECTOR DE DATA, HORA I TIMELAPSE (CANTONADA INFERIOR DRETA)
// =========================================================================
void InterficieUI::DibuixarPanellDataHoraTimelapse(EstatSimulacio& estat)
{
    if (estat.modeActual != ModeSimulacio::ESCALA_REAL || !estat.mostrarPanellTemps) return;

    const ImGuiViewport* vp = ImGui::GetMainViewport();
    float ampleFinestra = 420.0f;
    float alcadaFinestra = 390.0f;
    float marge = 15.0f;

    ImVec2 pos(vp->WorkPos.x + vp->WorkSize.x - ampleFinestra - marge,
               vp->WorkPos.y + vp->WorkSize.y - alcadaFinestra - marge);

    ImGui::SetNextWindowPos(pos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(ampleFinestra, alcadaFinestra), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.88f);

    ImGuiWindowFlags flagsFinestra = ImGuiWindowFlags_NoMove | 
                                     ImGuiWindowFlags_NoResize | 
                                     ImGuiWindowFlags_NoSavedSettings;

    if (ImGui::Begin("Efemerides i Control de Temps##PanellDretaInferior", &estat.mostrarPanellTemps, flagsFinestra))
    {
        DataHoraSimulacio& dt = estat.tempsSimulacio.dataHora;
        bool canviDataSollicitat = false;

        // SECCIO 1: DATA DE SIMULACIO
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(100, 200, 255, 255));
        ImGui::Text("DATA DE SIMULACIO (Efemerides UTC)");
        ImGui::PopStyleColor();
        ImGui::Separator();

        static const char* nomsMesos[] = {
            "Gener", "Febrer", "Marc", "Abril", "Maig", "Juny",
            "Juliol", "Agost", "Setembre", "Octubre", "Novembre", "Desembre"
        };

        ImGui::SetNextItemWidth(80.0f);
        if (ImGui::InputInt("##Any", &dt.any, 1, 10)) { dt.Validar(); canviDataSollicitat = true; }
        ImGui::SameLine();

        ImGui::SetNextItemWidth(105.0f);
        int mesIdx = dt.mes - 1;
        if (ImGui::Combo("##Mes", &mesIdx, nomsMesos, IM_ARRAYSIZE(nomsMesos))) {
            dt.mes = mesIdx + 1;
            dt.Validar();
            canviDataSollicitat = true;
        }
        ImGui::SameLine();

        ImGui::SetNextItemWidth(65.0f);
        if (ImGui::InputInt("##Dia", &dt.dia, 1, 5)) { dt.Validar(); canviDataSollicitat = true; }
        ImGui::SameLine();

        if (ImGui::Button("Calendari...")) {
            ImGui::OpenPopup("PopupCalendariMes");
        }

        if (ImGui::BeginPopup("PopupCalendariMes")) {
            ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "%s %d", nomsMesos[dt.mes - 1], dt.any);
            ImGui::Separator();
            ImGui::TextDisabled(" Dl  Dt  Dc  Dj  Dv  Ds  Dg");

            int diesMes = DataHoraSimulacio::DiesAlMes(dt.mes, dt.any);
            for (int d = 1; d <= diesMes; ++d) {
                char bId[8];
                snprintf(bId, sizeof(bId), "%2d", d);
                if (d == dt.dia) ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(30, 144, 255, 255));

                if (ImGui::Button(bId, ImVec2(24, 22))) {
                    dt.dia = d;
                    canviDataSollicitat = true;
                    ImGui::CloseCurrentPopup();
                }
                if (d == dt.dia) ImGui::PopStyleColor();

                if (d % 7 != 0 && d < diesMes) ImGui::SameLine();
            }
            ImGui::EndPopup();
        }

        // SECCIO 2: HORA DE SIMULACIO
        ImGui::Spacing();
        ImGui::Text("Hora:"); ImGui::SameLine();
        ImGui::SetNextItemWidth(60.0f);
        if (ImGui::DragInt("##H", &dt.hora, 0.2f, 0, 23, "%02d h")) { dt.Validar(); canviDataSollicitat = true; }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(60.0f);
        if (ImGui::DragInt("##M", &dt.minut, 0.2f, 0, 59, "%02d m")) { dt.Validar(); canviDataSollicitat = true; }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(60.0f);
        if (ImGui::DragInt("##S", &dt.segon, 0.2f, 0, 59, "%02d s")) { dt.Validar(); canviDataSollicitat = true; }

        ImGui::SameLine();
        if (ImGui::SmallButton("Ara")) { estat.tempsSimulacio.EstablirDataActual(); canviDataSollicitat = true; }
        ImGui::SameLine();
        if (ImGui::SmallButton("J2000")) { estat.tempsSimulacio.EstablirJ2000(); canviDataSollicitat = true; }

        // SECCIO 3: TRANSPORT I CONTROL DE VELOCITAT (1 min/s a 20 dies/s)
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(100, 200, 255, 255));
        ImGui::Text("CONTROL DE VELOCITAT (1 min/s - 20 dies/s)");
        ImGui::PopStyleColor();
        ImGui::Separator();

        bool esEnrere = (estat.tempsSimulacio.direccioTemps < 0);
        if (esEnrere) ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(180, 90, 20, 255));
        if (ImGui::Button("<< Enrere", ImVec2(85, 26))) {
            estat.tempsSimulacio.direccioTemps = -1;
            estat.tempsSimulacio.enPausa = false;
        }
        if (esEnrere) ImGui::PopStyleColor();
        ImGui::SameLine();

        if (estat.tempsSimulacio.enPausa) {
            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(30, 150, 60, 255));
            if (ImGui::Button("Reproduir", ImVec2(95, 26))) {
                estat.tempsSimulacio.enPausa = false;
            }
            ImGui::PopStyleColor();
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(190, 50, 50, 255));
            if (ImGui::Button("Pausa", ImVec2(95, 26))) {
                estat.tempsSimulacio.enPausa = true;
            }
            ImGui::PopStyleColor();
        }
        ImGui::SameLine();

        bool esEndavant = (estat.tempsSimulacio.direccioTemps > 0 && !estat.tempsSimulacio.enPausa);
        if (esEndavant) ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(30, 120, 200, 255));
        if (ImGui::Button("Endavant >>", ImVec2(95, 26))) {
            estat.tempsSimulacio.direccioTemps = 1;
            estat.tempsSimulacio.enPausa = false;
        }
        if (esEndavant) ImGui::PopStyleColor();
        ImGui::SameLine();

        if (ImGui::Button("-", ImVec2(30, 26))) {
            estat.tempsSimulacio.ReduirVelocitat();
        }
        ImGui::SameLine();
        if (ImGui::Button("+", ImVec2(30, 26))) {
            estat.tempsSimulacio.AugmentarVelocitat();
        }

        float velActual = (float)estat.tempsSimulacio.factorVelocitatSegonsPerSegon;
        char bufferFormatVel[32];
        if (velActual < 3600.0f) {
            snprintf(bufferFormatVel, sizeof(bufferFormatVel), "%.1f min/s", velActual / 60.0f);
        } else if (velActual < 86400.0f) {
            snprintf(bufferFormatVel, sizeof(bufferFormatVel), "%.2f hores/s", velActual / 3600.0f);
        } else {
            snprintf(bufferFormatVel, sizeof(bufferFormatVel), "%.2f dies/s", velActual / 86400.0f);
        }

        ImGui::Text("Velocitat simulada:");
        ImGui::SetNextItemWidth(ampleFinestra - 30.0f);
        if (ImGui::SliderFloat("##SliderVelocitat", &velActual, 60.0f, 1728000.0f, bufferFormatVel, ImGuiSliderFlags_Logarithmic)) {
            estat.tempsSimulacio.factorVelocitatSegonsPerSegon = (double)velActual;
        }

        ImGui::TextDisabled("Valors fixos:");
        if (ImGui::SmallButton("1 min/s"))  estat.tempsSimulacio.factorVelocitatSegonsPerSegon = 60.0;
        ImGui::SameLine();
        if (ImGui::SmallButton("1 hora/s")) estat.tempsSimulacio.factorVelocitatSegonsPerSegon = 3600.0;
        ImGui::SameLine();
        if (ImGui::SmallButton("1 dia/s"))  estat.tempsSimulacio.factorVelocitatSegonsPerSegon = 86400.0;
        ImGui::SameLine();
        if (ImGui::SmallButton("5 dies/s")) estat.tempsSimulacio.factorVelocitatSegonsPerSegon = 432000.0;
        ImGui::SameLine();
        if (ImGui::SmallButton("20 dies/s (Max)")) estat.tempsSimulacio.factorVelocitatSegonsPerSegon = 1728000.0;

        ImGui::Spacing();
        ImGui::Separator();
        if (estat.tempsSimulacio.enPausa) {
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Estat: SIMULACIO PAUSADA");
        } else {
            ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.5f, 1.0f), 
                "Estat: %s a %s", 
                esEnrere ? "REBOBINANT (Enrere)" : "AVANCANT (Endavant)", 
                bufferFormatVel);
        }

        if (canviDataSollicitat) {
            estat.recalculEfemeridesNecessari = true;
        }
    }
    ImGui::End();
}

// =========================================================================
// 2.6 PANELL D'INFORMACIO I TELEMETRIA ASTRONOMICA (HUD FLOTANT)
// =========================================================================
void InterficieUI::DibuixarFitxaAstronomicaHUD(const CosAstronomicDades& dades, bool* pObert)
{
    if (!pObert || !*pObert) return;

    const ImGuiViewport* vp = ImGui::GetMainViewport();
    ImVec2 pos(vp->WorkPos.x + vp->WorkSize.x - 330.0f, vp->WorkPos.y + 40.0f);
    ImGui::SetNextWindowPos(pos, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(310.0f, 290.0f), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.85f);

    if (ImGui::Begin("Fitxa Astronomica##HUD", pObert, ImGuiWindowFlags_NoResize))
    {
        ImGui::TextColored(ImVec4(0.3f, 0.85f, 1.0f, 1.0f), "%s", dades.nom.c_str());
        ImGui::Separator();

        ImGui::Text("Radi equatorial:");       ImGui::SameLine(175); ImGui::Text("%.1f km", dades.radiKm);
        ImGui::Text("Distancia al Sol:");      ImGui::SameLine(175); ImGui::Text("%.3f UA", dades.distSolUA);
        ImGui::Text("Gravetat superficial:");  ImGui::SameLine(175); ImGui::Text("%.2f m/s2", dades.gravetat);
        ImGui::Text("Periode de rotacio:");    ImGui::SameLine(175); ImGui::Text("%.2f h", dades.periodeRotacioHores);
        ImGui::Text("Periode orbital:");       ImGui::SameLine(175); ImGui::Text("%.2f anys", dades.periodeOrbitalAnys);
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextDisabled("Dades fisiques del cataleg del sistema");
    }
    ImGui::End();
}

// =========================================================================
// 2.7 PROJECCIO D'ETIQUETES 3D A PANTALLA 2D AMB IMDRAWLIST
// =========================================================================
void InterficieUI::DibuixarEtiquetaPlaneta(const std::string& nom, const glm::vec3& posicioMon, 
                                          const glm::mat4& viewMatrix, const glm::mat4& projMatrix, 
                                          const glm::vec4& viewport)
{
    glm::vec3 posPantalla = glm::project(posicioMon, viewMatrix, projMatrix, viewport);

    if (posPantalla.z > 0.0f && posPantalla.z < 1.0f) {
        ImDrawList* drawList = ImGui::GetBackgroundDrawList();
        ImVec2 posText(posPantalla.x, viewport.w - posPantalla.y);

        drawList->AddCircleFilled(posText, 3.5f, IM_COL32(0, 200, 255, 255));
        drawList->AddText(ImVec2(posText.x + 8.0f, posText.y - 7.0f), 
                          IM_COL32(245, 245, 245, 230), nom.c_str());
    }
}
