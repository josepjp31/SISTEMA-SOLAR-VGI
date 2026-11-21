// InterficieUI.cpp : Implementacio dels components visuals (Selector de modes, Menus, Panells, Pop-ups i HUD)
//
// Guia rapida de manteniment:
// - Per modificar dades fisiques mostrades a la Fitxa de l'Astre -> kCatalegCossos.
// - Per afegir o editar opcions de menu -> DibuixarBarraMenuSuperior (blocs INTUITIU / ESCALA_REAL).
// - Per ajustar l'aspecte dels pop-ups espacials -> DibuixarMarcFinestraEspacial i InicialitzarEstil.
// - Per canviar els limits del timelapse -> kPosMarques / kValMarques i EstatTempsSimulacio (.h).

#include "InterficieUI.h"
#include "ImGui/imgui.h"
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <cmath>
#include <algorithm>

// Fonts sans-serif carregades per als panells i etiquetes (la font 0 queda com a base d'ImGui)
static ImFont* s_fontPanellRegular = nullptr; // Text estandard de controls i files (16 px)
static ImFont* s_fontPanellPetit   = nullptr; // Subtitols i etiquetes de marques (13.5 px)
static ImFont* s_fontPanellTitol   = nullptr; // Capcaleres de finestres en negreta (17.5 px)
static ImFont* s_fontPanellDigital = nullptr; // Visor gran d'hora i data simulada (24.5 px)
static ImVector<ImWchar> s_rangCaractersCatala; // Rang de glifs persistent per a accents, dieresi i simbols

// Comprova si un fitxer de tipografia existeix al disc abans d'intentar carregar-lo amb ImGui
static bool FitxerExisteix(const char* ruta)
{
    FILE* f = nullptr;
    if (fopen_s(&f, ruta, "rb") == 0 && f != nullptr) {
        fclose(f);
        return true;
    }
    return false;
}

// Aplica una font nomes si s'ha carregat correctament (evita errors si falta al sistema)
static void PushFontSegur(ImFont* f)
{
    if (f != nullptr) ImGui::PushFont(f);
}

// Restaura la font anterior sempre que PushFontSegur n'hagi aplicat una
static void PopFontSegur(ImFont* f)
{
    if (f != nullptr) ImGui::PopFont();
}

// =========================================================================
// CATALEG DE DADES FISIQUES DELS 9 COSSOS DEL SISTEMA SOLAR (0: Sol .. 8: Neptu)
// Ordre indexat igual que EstatSimulacio::cosEnfocatId i Escenari::m_objectList[0..8].
// =========================================================================
static const CosAstronomicDades kCatalegCossos[9] = {
    { "Sol",     696340.0,  0.000, 274.00,  609.12,   0.00 },
    { "Mercuri",   2439.7,  0.387,   3.70, 1407.60,   0.24 },
    { "Venus",     6051.8,  0.723,   8.87, 5832.50,   0.62 },
    { "Terra",     6371.0,  1.000,   9.81,   23.93,   1.00 },
    { "Mart",      3389.5,  1.524,   3.72,   24.62,   1.88 },
    { "Júpiter",  69911.0,  5.204,  24.79,    9.93,  11.86 },
    { "Saturn",   58232.0,  9.582,  10.44,   10.70,  29.45 },
    { "Urà",      25362.0, 19.218,   8.69,   17.24,  84.02 },
    { "Neptú",    24622.0, 30.110,  11.15,   16.11, 164.79 }
};

// Copia les dades del cataleg de l'astre enfocant cap a estat.dadesCosActiu per al HUD
void InterficieUI::ActualitzarDadesCosActiu(EstatSimulacio& estat)
{
    int id = estat.cosEnfocatId;
    if (id < 0 || id >= 9) id = 0; // Proteccio de rang (per defecte Sol)
    estat.dadesCosActiu = kCatalegCossos[id];
}

// =========================================================================
// 1. INICIALITZACIO D'ESTIL VISUAL I TIPOGRAFIES
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

    // Tema fosc base adaptat a simulacio espacial (colors blaus profunds i cian)
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

    // Carrega de fonts del sistema (Segoe UI o Arial) amb rang estès de caracters catalans
    ImGuiIO& io = ImGui::GetIO();
    if (io.Fonts != nullptr && io.Fonts->Fonts.Size == 0) {
        const char* rutaFontRegular = nullptr;
        const char* rutaFontNegreta = nullptr;
        if (FitxerExisteix("C:\\Windows\\Fonts\\segoeui.ttf")) {
            rutaFontRegular = "C:\\Windows\\Fonts\\segoeui.ttf";
            rutaFontNegreta = FitxerExisteix("C:\\Windows\\Fonts\\segoeuib.ttf")
                ? "C:\\Windows\\Fonts\\segoeuib.ttf"
                : rutaFontRegular;
        } else if (FitxerExisteix("C:\\Windows\\Fonts\\arial.ttf")) {
            rutaFontRegular = "C:\\Windows\\Fonts\\arial.ttf";
            rutaFontNegreta = FitxerExisteix("C:\\Windows\\Fonts\\arialbd.ttf")
                ? "C:\\Windows\\Fonts\\arialbd.ttf"
                : rutaFontRegular;
        }

        if (rutaFontRegular != nullptr) {
            ImFontGlyphRangesBuilder builder;
            builder.AddRanges(io.Fonts->GetGlyphRangesDefault());
            // Caracters catalans i unitats astronomiques necessaris als menus, panell i HUD
            builder.AddText("ÀÁÈÉÍÏÒÓÚÜÇàáèéíïòóúüç·—°²");
            builder.BuildRanges(&s_rangCaractersCatala);

            // La primera font registrada passa a ser la font per defecte de tota la interficie
            io.Fonts->AddFontFromFileTTF(rutaFontRegular, 15.5f, nullptr, s_rangCaractersCatala.Data);
            s_fontPanellPetit   = io.Fonts->AddFontFromFileTTF(rutaFontRegular, 13.5f, nullptr, s_rangCaractersCatala.Data);
            s_fontPanellRegular = io.Fonts->AddFontFromFileTTF(rutaFontRegular, 16.0f, nullptr, s_rangCaractersCatala.Data);
            s_fontPanellTitol   = io.Fonts->AddFontFromFileTTF(rutaFontNegreta, 17.5f, nullptr, s_rangCaractersCatala.Data);
            s_fontPanellDigital = io.Fonts->AddFontFromFileTTF(rutaFontRegular, 24.5f, nullptr, s_rangCaractersCatala.Data);
        } else {
            // Fallback a la font bitmap integrada d'ImGui si no es troben fonts TrueType
            io.Fonts->AddFontDefault();
        }
    }
}

// =========================================================================
// HELPERS VISUALS COMPARTITS (ESTIL FOSC ESPACIAL CIAN)
// =========================================================================

// Dibuixa vectorialmente la icona de reproduccio (triangle verd/cian) si esta en pausa,
// o les dues barres verticals de pausa (taronja) si la simulacio esta activa.
static void DibuixarIconaPausaReproduccio(ImDrawList* dl, ImVec2 centre, float alt, bool enPausa)
{
    float escala = alt / 46.0f;
    float hBar = 11.0f * escala;
    if (enPausa) {
        // Estat pausat: mostra unicament la icona de reprendre (>)
        ImU32 colPlay = IM_COL32(55, 245, 185, 255);
        float wPlay = 16.0f * escala;
        dl->AddTriangleFilled(
            ImVec2(centre.x + wPlay * 0.60f, centre.y),
            ImVec2(centre.x - wPlay * 0.45f, centre.y - hBar),
            ImVec2(centre.x - wPlay * 0.45f, centre.y + hBar),
            colPlay
        );
    } else {
        // Estat actiu: mostra unicament la icona de pausa (||)
        ImU32 colPausa = IM_COL32(255, 165, 55, 255);
        float wBar = 4.6f * escala;
        float sep = 4.2f * escala;
        dl->AddRectFilled(
            ImVec2(centre.x - sep - wBar, centre.y - hBar),
            ImVec2(centre.x - sep, centre.y + hBar),
            colPausa, 1.5f
        );
        dl->AddRectFilled(
            ImVec2(centre.x + sep, centre.y - hBar),
            ImVec2(centre.x + sep + wBar, centre.y + hBar),
            colPausa, 1.5f
        );
    }
}

// Dibuixa el marc comu dels pop-ups en estil espacial: resplendor cian exterior,
// capcalera amb titol centrat i boto de tancament ("X") connectat a *pObert.
static void DibuixarMarcFinestraEspacial(ImDrawList* dl, ImVec2 winPos, ImVec2 winSize, const char* titol, bool* pObert, const char* idBotoTancar, float alcadaCapcalera = 42.0f)
{
    ImVec2 winMax(winPos.x + winSize.x, winPos.y + winSize.y);

    // Resplendor exterior i vora lluminosa cian
    dl->PushClipRectFullScreen();
    dl->AddRect(ImVec2(winPos.x - 6.0f, winPos.y - 6.0f), ImVec2(winMax.x + 6.0f, winMax.y + 6.0f), IM_COL32(0, 195, 255, 16), 18.0f, 0, 3.0f);
    dl->AddRect(ImVec2(winPos.x - 3.0f, winPos.y - 3.0f), ImVec2(winMax.x + 3.0f, winMax.y + 3.0f), IM_COL32(0, 200, 255, 36), 16.0f, 0, 2.5f);
    dl->AddRect(winPos, winMax, IM_COL32(60, 205, 255, 225), 14.0f, 0, 1.6f);
    dl->PopClipRect();

    // Capcalera integrada
    dl->AddRectFilled(
        winPos,
        ImVec2(winMax.x, winPos.y + alcadaCapcalera),
        IM_COL32(22, 48, 78, 175),
        14.0f,
        ImDrawFlags_RoundCornersTop
    );
    dl->AddLine(
        ImVec2(winPos.x + 1.0f, winPos.y + alcadaCapcalera),
        ImVec2(winMax.x - 1.0f, winPos.y + alcadaCapcalera),
        IM_COL32(60, 190, 245, 145),
        1.2f
    );

    PushFontSegur(s_fontPanellTitol);
    ImVec2 midaTitol = ImGui::CalcTextSize(titol);
    dl->AddText(
        ImVec2(winPos.x + (winSize.x - midaTitol.x) * 0.5f, winPos.y + (alcadaCapcalera - midaTitol.y) * 0.5f),
        IM_COL32(195, 238, 255, 255),
        titol
    );
    PopFontSegur(s_fontPanellTitol);

    // Boto de tancament ("X") a la cantonada superior dreta
    const float midaClose = 28.0f;
    ImVec2 posClose(winMax.x - midaClose - 10.0f, winPos.y + (alcadaCapcalera - midaClose) * 0.5f);
    ImGui::SetCursorScreenPos(posClose);
    if (ImGui::InvisibleButton(idBotoTancar, ImVec2(midaClose, midaClose))) {
        if (pObert != nullptr) *pObert = false;
    }
    bool hoverClose = ImGui::IsItemHovered();
    if (hoverClose) {
        dl->AddRectFilled(posClose, ImVec2(posClose.x + midaClose, posClose.y + midaClose), IM_COL32(45, 130, 185, 95), 6.0f);
    }
    ImVec2 cClose(posClose.x + midaClose * 0.5f, posClose.y + midaClose * 0.5f);
    ImU32 colClose = hoverClose ? IM_COL32(255, 255, 255, 255) : IM_COL32(175, 225, 250, 235);
    dl->AddLine(ImVec2(cClose.x - 5.5f, cClose.y - 5.5f), ImVec2(cClose.x + 5.5f, cClose.y + 5.5f), colClose, 2.0f);
    dl->AddLine(ImVec2(cClose.x + 5.5f, cClose.y - 5.5f), ImVec2(cClose.x - 5.5f, cClose.y + 5.5f), colClose, 2.0f);
}

// Finestra flotant "Quant al Simulador" (oberta des del menu Ajuda)
static void DibuixarDialegQuantA(bool* pObert)
{
    if (!pObert || !*pObert) return;

    const ImGuiViewport* vp = ImGui::GetMainViewport();
    const float ampleFinestra = 500.0f;
    const float alcadaFinestra = 356.0f;

    ImVec2 posInicial(
        vp->WorkPos.x + (vp->WorkSize.x - ampleFinestra) * 0.5f,
        vp->WorkPos.y + (vp->WorkSize.y - alcadaFinestra) * 0.5f
    );

    ImGui::SetNextWindowPos(posInicial, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(ampleFinestra, alcadaFinestra), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.90f);

    ImGuiWindowFlags flagsFinestra = ImGuiWindowFlags_NoTitleBar |
                                     ImGuiWindowFlags_NoResize |
                                     ImGuiWindowFlags_NoScrollbar |
                                     ImGuiWindowFlags_NoScrollWithMouse |
                                     ImGuiWindowFlags_NoCollapse |
                                     ImGuiWindowFlags_NoSavedSettings;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 14.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(10, 22, 40, 230));

    if (ImGui::Begin("QuantAlSimulador##PopUpEspacial", pObert, flagsFinestra))
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 winPos = ImGui::GetWindowPos();
        ImVec2 winSize = ImGui::GetWindowSize();
        const float alcadaCapcalera = 42.0f;

        DibuixarMarcFinestraEspacial(dl, winPos, winSize, "QUANT AL SIMULADOR", pObert, "##TancarQuantA", alcadaCapcalera);

        // Caixa interior informativa
        const float padLat = 22.0f;
        ImVec2 caixaMin(winPos.x + padLat, winPos.y + alcadaCapcalera + 16.0f);
        ImVec2 caixaMax(winPos.x + winSize.x - padLat, winPos.y + winSize.y - 62.0f);
        dl->AddRectFilled(caixaMin, caixaMax, IM_COL32(12, 28, 48, 232), 9.0f);
        dl->AddRect(caixaMin, caixaMax, IM_COL32(55, 165, 220, 165), 9.0f, 0, 1.2f);

        PushFontSegur(s_fontPanellRegular);
        const char* titolApp = "SIMULADOR DEL SISTEMA SOLAR";
        ImVec2 mTitolApp = ImGui::CalcTextSize(titolApp);
        dl->AddText(
            ImVec2(winPos.x + (winSize.x - mTitolApp.x) * 0.5f, caixaMin.y + 14.0f),
            IM_COL32(115, 225, 255, 255),
            titolApp
        );

        const char* linia1 = "Explora els planetes i les seves òrbites en 3D.";
        const char* linia2 = "• Mode Didàctic: vista clara de tot el sistema.";
        const char* linia3 = "• Mode a Escala Real: distàncies reals i control del temps.";
        const char* linia4 = "Projecte educatiu — Escola d'Enginyeria (UAB)";

        dl->AddText(ImVec2(caixaMin.x + 16.0f, caixaMin.y + 40.0f), IM_COL32(235, 245, 255, 250), linia1);
        dl->AddText(ImVec2(caixaMin.x + 16.0f, caixaMin.y + 64.0f), IM_COL32(195, 228, 250, 240), linia2);
        dl->AddText(ImVec2(caixaMin.x + 16.0f, caixaMin.y + 86.0f), IM_COL32(195, 228, 250, 240), linia3);

        // Separador i llista dels integrants del grup (Grup 05) en dues columnes
        float ySep = caixaMin.y + 112.0f;
        dl->AddLine(ImVec2(caixaMin.x + 14.0f, ySep), ImVec2(caixaMax.x - 14.0f, ySep), IM_COL32(55, 165, 220, 110), 1.0f);
        dl->AddText(ImVec2(caixaMin.x + 16.0f, ySep + 8.0f), IM_COL32(115, 225, 255, 255), "Integrants del grup (Grup 05):");

        static const char* integrantsCol1[3] = {
            "• Carles Peiró",
            "• Noa Gangolells Alcázar",
            "• Josep Montoro Pascual"
        };
        static const char* integrantsCol2[3] = {
            "• Lucas Luna Rodriguez",
            "• Abel Paiz Ilias",
            "• Ty Devia Ballesteros"
        };

        PushFontSegur(s_fontPanellPetit);
        float yMembres = ySep + 32.0f;
        float xCol1 = caixaMin.x + 18.0f;
        float xCol2 = caixaMin.x + (caixaMax.x - caixaMin.x) * 0.50f + 4.0f;
        for (int i = 0; i < 3; ++i) {
            dl->AddText(ImVec2(xCol1, yMembres + i * 20.0f), IM_COL32(225, 240, 255, 245), integrantsCol1[i]);
            dl->AddText(ImVec2(xCol2, yMembres + i * 20.0f), IM_COL32(225, 240, 255, 245), integrantsCol2[i]);
        }

        ImVec2 mLinia4 = ImGui::CalcTextSize(linia4);
        dl->AddText(
            ImVec2(winPos.x + (winSize.x - mLinia4.x) * 0.5f, caixaMax.y - mLinia4.y - 10.0f),
            IM_COL32(150, 195, 225, 210),
            linia4
        );
        PopFontSegur(s_fontPanellPetit);

        // Boto Tancar centrat a la base
        const float wBtn = 130.0f;
        const float hBtn = 32.0f;
        ImVec2 pBtn(winPos.x + (winSize.x - wBtn) * 0.5f, winPos.y + winSize.y - hBtn - 15.0f);
        ImVec2 pBtnMax(pBtn.x + wBtn, pBtn.y + hBtn);
        ImGui::SetCursorScreenPos(pBtn);
        if (ImGui::InvisibleButton("##BotoTancarQuantAFooter", ImVec2(wBtn, hBtn))) {
            *pObert = false;
        }
        bool hoverBtn = ImGui::IsItemHovered();
        dl->AddRectFilled(pBtn, pBtnMax, hoverBtn ? IM_COL32(24, 58, 94, 235) : IM_COL32(14, 36, 62, 220), 7.0f);
        dl->AddRect(pBtn, pBtnMax, hoverBtn ? IM_COL32(85, 225, 255, 245) : IM_COL32(55, 165, 220, 195), 7.0f, 0, 1.4f);
        ImVec2 mBtn = ImGui::CalcTextSize("Tancar");
        dl->AddText(
            ImVec2(pBtn.x + (wBtn - mBtn.x) * 0.5f, pBtn.y + (hBtn - mBtn.y) * 0.5f),
            IM_COL32(235, 248, 255, 255),
            "Tancar"
        );
        PopFontSegur(s_fontPanellRegular);
    }
    ImGui::End();

    ImGui::PopStyleColor();
    ImGui::PopStyleVar(3);
}

// =========================================================================
// 2. RENDERITZADOR PRINCIPAL DE LA INTERFICIE
// =========================================================================
void InterficieUI::Renderitzar(EstatSimulacio& estat, Escenari*& currentEscenari, Escenari escenaris[], GLuint& shader_programID, GLuint& shader_programIDNormalMap)
{
    // Fase 1: mentre el selector de modes estigui obert, s'oculta la barra de menu i tots els panells
    // per presentar les dues targetes centrals sobre el fons fosc.
    if (estat.mostrarSelectorModes) {
        DibuixarSelectorInicialModes(estat, currentEscenari, escenaris, shader_programID, shader_programIDNormalMap);
        return;
    }

    // Fase 2: barra de menu superior adaptada a estat.modeActual (INTUITIU o ESCALA_REAL)
    DibuixarBarraMenuSuperior(estat, currentEscenari, escenaris, shader_programID, shader_programIDNormalMap);

    // Fase 3: panell flotant de control de data, hora i timelapse (nomes visible en mode escala real)
    if (estat.modeActual == ModeSimulacio::ESCALA_REAL && estat.mostrarPanellTemps) {
        DibuixarPanellDataHoraTimelapse(estat);
    }

    // Fase 4: fitxa astronomica de l'astre enfocant (HUD a la cantonada superior dreta)
    if (estat.mostrarFitxaHUD) {
        DibuixarFitxaAstronomicaHUD(estat.dadesCosActiu, &estat.mostrarFitxaHUD);
    }

    // Fase 5: dialeg informatiu sobre el projecte
    if (estat.mostrarDialegQuantA) {
        DibuixarDialegQuantA(&estat.mostrarDialegQuantA);
    }
}

// =========================================================================
// HELPER PER A ELEMENTS DE LLISTA AMB PUNT (BULLET) I SALT DE LINIA AUTOMATIC
// =========================================================================
// Escriu un element de llista amb vinyeta i ajust de linia automatic per evitar talls en finestres estretes
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
void InterficieUI::DibuixarSelectorInicialModes(EstatSimulacio& estat, Escenari*& currentEscenari, Escenari escenaris[], GLuint& shader_programID, GLuint& shader_programIDNormalMap)
{
    if (!estat.mostrarSelectorModes) return;

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    float viewW = viewport->WorkSize.x;
    float viewH = viewport->WorkSize.y;

    // Factor d'escala responsive en funcio de l'amplada de la finestra (finestra normal, 1080p, 1440p, 4K)
    float factorEscala = 1.0f;
    if (viewW >= 2400.0f)      factorEscala = 1.35f;
    else if (viewW >= 1700.0f) factorEscala = 1.20f;
    else if (viewW >= 1200.0f) factorEscala = 1.10f;
    else if (viewW >= 900.0f)  factorEscala = 1.00f;
    else                       factorEscala = 0.90f;

    float ampleModal = 840.0f * factorEscala;
    float alcadaModal = 520.0f * factorEscala;

    // Dimensions del contenidor modal adaptades amb limits minim i maxim per evitar desbordaments
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

    // Aillament visual: fons fosc complet sobre tota la pantalla mentre no s'ha triat mode
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

        // Capcalera principal centrada
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
        float ampleTitol = ImGui::CalcTextSize("SIMULADOR INTERACTIU DEL SISTEMA SOLAR").x;
        ImGui::SetCursorPosX((midaModal.x - ampleTitol) * 0.5f);
        ImGui::Text("SIMULADOR INTERACTIU DEL SISTEMA SOLAR");
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(185, 215, 250, 255));
        float ampleSub = ImGui::CalcTextSize("Trieu el mode de representació per iniciar la simulació").x;
        ImGui::SetCursorPosX((midaModal.x - ampleSub) * 0.5f);
        ImGui::Text("Trieu el mode de representació per iniciar la simulació");
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Separator, IM_COL32(50, 80, 130, 180));
        ImGui::Separator();
        ImGui::PopStyleColor();
        ImGui::Spacing();

        // Geometria simetrica per a les dues targetes i el boto inferior de cada una
        float espaiCapcalera = ImGui::GetCursorPosY();
        float margeInferior = 18.0f * factorEscala;
        float alcadaTargeta = midaModal.y - espaiCapcalera - margeInferior;
        if (alcadaTargeta < 200.0f) alcadaTargeta = 200.0f;

        float separacio = 24.0f * factorEscala;
        float margeLateral = 24.0f * factorEscala;
        float ampleTargeta = (midaModal.x - (margeLateral * 2.0f + separacio)) * 0.5f;

        ImVec2 midaTargeta(ampleTargeta, alcadaTargeta);
        float alcadaBoto = 44.0f * factorEscala;

        // Amplada del boto dimensionada a partir de l'etiqueta mes llarga per evitar talls
        float textBotoMax = ImGui::CalcTextSize("INICIAR MODE A ESCALA REAL").x;
        float ampleBoto = (midaTargeta.x * 0.85f > textBotoMax + 32.0f) ? (midaTargeta.x * 0.85f) : (textBotoMax + 32.0f);
        if (ampleBoto > 420.0f * factorEscala) ampleBoto = 420.0f * factorEscala;
        if (ampleBoto > midaTargeta.x - 16.0f) ampleBoto = midaTargeta.x - 16.0f;

        // TARGETA 1: MODE DIDACTIC / INTUITIU (tons blaus/cian)
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
            ImGui::Text("   MODE DIDÀCTIC / INTUÏTIU");
            ImGui::PopStyleColor();

            ImGui::PushStyleColor(ImGuiCol_Separator, IM_COL32(0, 170, 230, 150));
            ImGui::Separator();
            ImGui::PopStyleColor();
            ImGui::Spacing();

            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(210, 230, 255, 255));
            ImGui::TextWrapped("Exploració visual adaptada per a l'aprenentatge:");
            ImGui::PopStyleColor();
            ImGui::Spacing();

            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(235, 245, 255, 255));
            DibuixarPuntLlista("Òrbites circulars coplanars.");
            DibuixarPuntLlista("Distàncies i radis ajustats visualment.");
            DibuixarPuntLlista("Tots els planetes visibles simultàniament.");
            DibuixarPuntLlista("Velocitats d'òrbita proporcionals.");
            DibuixarPuntLlista("Càmera orbital inicial a 25 graus.");
            DibuixarPuntLlista("Focus directe a qualsevol planeta.");
            ImGui::PopStyleColor();

            // Posicionament resilient: fixa el boto a la base si hi ha espai, o just sota la llista
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
            if (ImGui::Button("INICIAR MODE DIDÀCTIC", ImVec2(ampleBoto, alcadaBoto)))
            {
                // Configura l'estat per al Mode Didactic, tanca el selector i demana sincronitzar OpenGL
                estat.modeActual = ModeSimulacio::INTUITIU;
                estat.mostrarSelectorModes = false;
                estat.mostrarPanellTemps = false;
                estat.cosEnfocatId = 0;
                estat.cameraConfig.angleInclinacio = 25.0f;
                estat.cameraConfig.mode = ModeCamera::FIXA_INCLINADA_25;
                estat.configuracioInicialPendent = true;
                estat.canviCameraPendent = true;
                ActualitzarDadesCosActiu(estat);

                // Creacio i vinculacio de l'escenari didactic (index 0) amb els shaders actius
                if (escenaris != nullptr)
                {
                    currentEscenari = &escenaris[0];
                    currentEscenari->creaEscenari(shader_programID, shader_programIDNormalMap);
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
            ImGui::Text("   MODE A ESCALA REAL");
            ImGui::PopStyleColor();

            ImGui::PushStyleColor(ImGuiCol_Separator, IM_COL32(255, 165, 2, 150));
            ImGui::Separator();
            ImGui::PopStyleColor();
            ImGui::Spacing();

            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 230, 205, 255));
            ImGui::TextWrapped("Simulació astronòmica fidel a la realitat:");
            ImGui::PopStyleColor();
            ImGui::Spacing();

            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 245, 230, 255));
            DibuixarPuntLlista("Distàncies i mides reals dels planetes.");
            DibuixarPuntLlista("Òrbites reals amb inclinació natural.");
            DibuixarPuntLlista("Control complet de Data, Hora i Velocitat.");
            DibuixarPuntLlista("Velocitat regulable: 1 min/s a 20 dies/s.");
            DibuixarPuntLlista("Possibilitat d'avançar o rebobinar.");
            DibuixarPuntLlista("Focus directe a cada astre.");
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
                // Activa el mode a escala real amb el panell temporal obert i camera orbital lliure
                estat.modeActual = ModeSimulacio::ESCALA_REAL;
                estat.mostrarSelectorModes = false;
                estat.mostrarPanellTemps = true;
                estat.cosEnfocatId = 0;
                estat.cameraConfig.mode = ModeCamera::ORBITAL_CENTRADOR;
                estat.configuracioInicialPendent = true;
                estat.canviCameraPendent = true;
                ActualitzarDadesCosActiu(estat);

                // Creacio i vinculacio de l'escenari a escala real (index 1)
                if (escenaris != nullptr)
                {
                    currentEscenari = &escenaris[1];
                    currentEscenari->creaEscenari(shader_programID, shader_programIDNormalMap);
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
// Construeix la barra superior amb opcions filtrades segons estat.modeActual.
// Per afegir un nou menu o opcio, seguiu l'estructura BeginMenu/MenuItem dins
// del bloc corresponent (INTUITIU o ESCALA_REAL) i propagueu SetWindowFontScale.
void InterficieUI::DibuixarBarraMenuSuperior(EstatSimulacio& estat, Escenari*& currentEscenari, Escenari escenaris[], GLuint& shader_programID, GLuint& shader_programIDNormalMap)
{
    const ImGuiViewport* vp = ImGui::GetMainViewport();
    float viewW = vp->WorkSize.x;

    // Interpolacio lineal [0..1.25] per escalar tipografia i marges entre 900px i 1920px+
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

    // Noms mostrables dels 9 astres indexats segons Escenari::m_objectList[0..8]
    static const char* nomsPlanetesMenu[9] = {
        "Sol (Centre)", "Mercuri", "Venus", "Terra",
        "Mart", "Júpiter", "Saturn", "Urà", "Neptú"
    };

    if (estat.modeActual == ModeSimulacio::INTUITIU)
    {
        // ==================== MENUS MODE DIDACTIC / INTUITIU ====================
        // 1) Canvi de mode, control rapid de pausa, reinici de vista i sortida
        if (ImGui::BeginMenu("Escenari"))
        {
            ImGui::SetWindowFontScale(factorFontMenu);
            if (ImGui::MenuItem("Canviar de Mode...")) {
                estat.mostrarSelectorModes = true;
            }
            if (ImGui::MenuItem("Canviar a Mode a Escala Real")) {
                estat.modeActual = ModeSimulacio::ESCALA_REAL;
                estat.mostrarPanellTemps = true;
                estat.configuracioInicialPendent = true;
                estat.canviCameraPendent = true;
                if (escenaris != nullptr) {
                    currentEscenari = &escenaris[1];
                    currentEscenari->creaEscenari(shader_programID, shader_programIDNormalMap);
                }
            }
            ImGui::Separator();
            if (ImGui::MenuItem(estat.tempsSimulacio.enPausa ? "Reprendre Moviment" : "Pausar Moviment", "Espai")) {
                estat.tempsSimulacio.enPausa = !estat.tempsSimulacio.enPausa;
            }
            if (ImGui::MenuItem("Reiniciar Vista", "R")) {
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

        // 2) Seleccio de l'astre sobre el qual orbita la camera (0=Sol .. 8=Neptu)
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
            if (ImGui::MenuItem("Vista Global del Sol", "Home")) {
                estat.cosEnfocatId = 0;
                estat.cameraConfig.mode = ModeCamera::GLOBAL_SISTEMA;
                estat.canviCameraPendent = true;
                ActualitzarDadesCosActiu(estat);
            }
            ImGui::EndMenu();
        }

        // 3) Tipus de camera disponibles al Mode Didactic (orbital, 25 graus, pilota coet i global)
        if (ImGui::BeginMenu("Càmera"))
        {
            ImGui::SetWindowFontScale(factorFontMenu);
            if (ImGui::MenuItem("Càmera Orbital", nullptr, estat.cameraConfig.mode == ModeCamera::ORBITAL_CENTRADOR)) {
                estat.cameraConfig.mode = ModeCamera::ORBITAL_CENTRADOR;
                estat.canviCameraPendent = true;
            }
            if (ImGui::MenuItem("Vista Inclinada (25°)", nullptr, estat.cameraConfig.mode == ModeCamera::FIXA_INCLINADA_25)) {
                estat.cameraConfig.mode = ModeCamera::FIXA_INCLINADA_25;
                estat.cameraConfig.angleInclinacio = 25.0f;
                estat.canviCameraPendent = true;
            }
            // Activa la càmera de seguiment en mode coet (CAM_NAVEGA)
            if (ImGui::MenuItem("Pilota coet", nullptr, estat.cameraConfig.mode == ModeCamera::COET_PRIMERA_PERSONA)) {
                estat.cameraConfig.mode = ModeCamera::COET_PRIMERA_PERSONA;
                estat.canviCameraPendent = true;
            }
            if (ImGui::MenuItem("Vista Global del Sistema", nullptr, estat.cameraConfig.mode == ModeCamera::GLOBAL_SISTEMA)) {
                estat.cosEnfocatId = 0;
                estat.cameraConfig.mode = ModeCamera::GLOBAL_SISTEMA;
                estat.canviCameraPendent = true;
                ActualitzarDadesCosActiu(estat);
            }
            ImGui::EndMenu();
        }

        // 4) Capes visuals i il-luminacio (sincronitzades automaticament a SincronitzarEstatSimuladorAmbOpenGL)
        if (ImGui::BeginMenu("Visualització"))
        {
            ImGui::SetWindowFontScale(factorFontMenu);
            ImGui::MenuItem("Mostrar Òrbites", nullptr, &estat.opcionsVisualitzacio.mostrarOrbites);
            ImGui::MenuItem("Mostrar Noms dels Planetes", nullptr, &estat.opcionsVisualitzacio.mostrarEtiquetes);
            ImGui::MenuItem("Llum del Sol", nullptr, &estat.opcionsVisualitzacio.iluminacioSolarDirecta);
            ImGui::MenuItem("Vista Clara (Sense ombres)", nullptr, &estat.opcionsVisualitzacio.vistaClaraAmbient);
            ImGui::Separator();

            if (ImGui::BeginMenu("Anells de Saturn")) {
                ImGui::SetWindowFontScale(factorFontMenu);
                if (ImGui::MenuItem("Disc llis amb textura", nullptr, !estat.opcionsVisualitzacio.anellsSaturnParticules)) {
                    estat.opcionsVisualitzacio.anellsSaturnParticules = false;
                }
                if (ImGui::MenuItem("Anells amb partícules", nullptr, estat.opcionsVisualitzacio.anellsSaturnParticules)) {
                    estat.opcionsVisualitzacio.anellsSaturnParticules = true;
                }
                ImGui::EndMenu();
            }

            ImGui::MenuItem("Fons d'Estrelles", nullptr, &estat.opcionsVisualitzacio.mostrarSkybox);
            ImGui::MenuItem("Mostrar Eixos de Guia", nullptr, &estat.opcionsVisualitzacio.mostrarEixos);
            ImGui::EndMenu();
        }

        // 5) Telemetria de l'astre i dialeg informatiu
        if (ImGui::BeginMenu("Ajuda"))
        {
            ImGui::SetWindowFontScale(factorFontMenu);
            ImGui::MenuItem("Mostrar Fitxa de l'Astre", nullptr, &estat.mostrarFitxaHUD);
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
        // 1) Commutacio de mode, reinici de vista centrada al Sol i sortida
        if (ImGui::BeginMenu("Escenari"))
        {
            ImGui::SetWindowFontScale(factorFontMenu);
            if (ImGui::MenuItem("Canviar de Mode...")) {
                estat.mostrarSelectorModes = true;
            }
            if (ImGui::MenuItem("Canviar a Mode Didàctic")) {
                estat.modeActual = ModeSimulacio::INTUITIU;
                estat.mostrarPanellTemps = false;
                estat.cameraConfig.mode = ModeCamera::FIXA_INCLINADA_25;
                estat.configuracioInicialPendent = true;
                estat.canviCameraPendent = true;
                if (escenaris != nullptr) {
                    currentEscenari = &escenaris[0];
                    currentEscenari->creaEscenari(shader_programID, shader_programIDNormalMap);
                }
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Reiniciar Vista", "R")) {
                estat.cosEnfocatId = 0;
                estat.origenReferenciaSol = true;
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

        // 2) Visibilitat del panell flotant de temps, pausa/direccio, dates i presets de velocitat
        if (ImGui::BeginMenu("Temps & Data"))
        {
            ImGui::SetWindowFontScale(factorFontMenu);
            ImGui::MenuItem("Mostrar Control del Temps", nullptr, &estat.mostrarPanellTemps);
            ImGui::Separator();
            if (ImGui::MenuItem(estat.tempsSimulacio.enPausa ? "Reprendre Simulació" : "Pausar Simulació", "Espai")) {
                estat.tempsSimulacio.enPausa = !estat.tempsSimulacio.enPausa;
            }
            if (ImGui::MenuItem("Invertir Sentit del Temps", "Backspace")) {
                estat.tempsSimulacio.direccioTemps *= -1;
            }
            ImGui::Separator();
            if (ImGui::BeginMenu("Dates Destacades")) {
                ImGui::SetWindowFontScale(factorFontMenu);
                if (ImGui::MenuItem("Data i Hora Actual")) {
                    estat.tempsSimulacio.EstablirDataActual();
                    estat.recalculEfemeridesNecessari = true;
                }
                if (ImGui::MenuItem("Any 2000 (1 Gen 2000)")) {
                    estat.tempsSimulacio.EstablirJ2000();
                    estat.recalculEfemeridesNecessari = true;
                }
                if (ImGui::MenuItem("Eclipsi Solar Total (12 Ago 2026)")) {
                    estat.tempsSimulacio.dataHora = { 2026, 8, 12, 17, 30, 0 };
                    estat.recalculEfemeridesNecessari = true;
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Velocitats Ràpides")) {
                ImGui::SetWindowFontScale(factorFontMenu);
                if (ImGui::MenuItem("1 minut / s")) estat.tempsSimulacio.factorVelocitatSegonsPerSegon = 60.0;
                if (ImGui::MenuItem("1 hora / s"))  estat.tempsSimulacio.factorVelocitatSegonsPerSegon = 3600.0;
                if (ImGui::MenuItem("1 dia / s"))   estat.tempsSimulacio.factorVelocitatSegonsPerSegon = 86400.0;
                if (ImGui::MenuItem("10 dies / s")) estat.tempsSimulacio.factorVelocitatSegonsPerSegon = 864000.0;
                if (ImGui::MenuItem("20 dies / s")) estat.tempsSimulacio.factorVelocitatSegonsPerSegon = 1728000.0;
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }

        // 3) Seleccio d'origen/astre centrat i modalitat de camera (incloent Pilota coet)
        if (ImGui::BeginMenu("Origen & Càmeres"))
        {
            ImGui::SetWindowFontScale(factorFontMenu);
            ImGui::TextDisabled("Centrar Vista en:");
            if (ImGui::MenuItem("El Sol", nullptr, estat.origenReferenciaSol && estat.cosEnfocatId == 0)) {
                estat.origenReferenciaSol = true;
                estat.cosEnfocatId = 0;
                estat.cameraConfig.mode = ModeCamera::ORBITAL_CENTRADOR;
                estat.canviCameraPendent = true;
                ActualitzarDadesCosActiu(estat);
            }
            if (ImGui::BeginMenu("El Planeta Seleccionat")) {
                ImGui::SetWindowFontScale(factorFontMenu);
                for (int i = 1; i < 9; ++i) {
                    bool seleccionat = (!estat.origenReferenciaSol && estat.cosEnfocatId == i);
                    if (ImGui::MenuItem(nomsPlanetesMenu[i], nullptr, seleccionat)) {
                        estat.cosEnfocatId = i;
                        estat.origenReferenciaSol = false;
                        estat.cameraConfig.mode = ModeCamera::ORBITAL_CENTRADOR;
                        estat.canviCameraPendent = true;
                        ActualitzarDadesCosActiu(estat);
                    }
                }
                ImGui::EndMenu();
            }

            ImGui::Separator();
            ImGui::TextDisabled("Tipus de Càmera:");
            if (ImGui::MenuItem("Càmera Orbital", nullptr, estat.cameraConfig.mode == ModeCamera::ORBITAL_CENTRADOR)) {
                estat.cameraConfig.mode = ModeCamera::ORBITAL_CENTRADOR;
                estat.canviCameraPendent = true;
            }
            if (ImGui::MenuItem("Vista Inclinada (25°)", nullptr, estat.cameraConfig.mode == ModeCamera::FIXA_INCLINADA_25)) {
                estat.cameraConfig.mode = ModeCamera::FIXA_INCLINADA_25;
                estat.cameraConfig.angleInclinacio = 25.0f;
                estat.canviCameraPendent = true;
            }
            if (ImGui::MenuItem("Pilota coet", nullptr, estat.cameraConfig.mode == ModeCamera::COET_PRIMERA_PERSONA)) {
                estat.cameraConfig.mode = ModeCamera::COET_PRIMERA_PERSONA;
                estat.canviCameraPendent = true;
            }
            if (ImGui::MenuItem("Vista Global del Sistema", nullptr, estat.cameraConfig.mode == ModeCamera::GLOBAL_SISTEMA)) {
                estat.cosEnfocatId = 0;
                estat.origenReferenciaSol = true;
                estat.cameraConfig.mode = ModeCamera::GLOBAL_SISTEMA;
                estat.canviCameraPendent = true;
                ActualitzarDadesCosActiu(estat);
            }
            ImGui::EndMenu();
        }

        // 4) Capes visuals del renderitzat en mode a escala real
        if (ImGui::BeginMenu("Visualització"))
        {
            ImGui::SetWindowFontScale(factorFontMenu);
            ImGui::MenuItem("Mostrar Òrbites", nullptr, &estat.opcionsVisualitzacio.mostrarOrbites);
            ImGui::MenuItem("Mostrar Noms dels Planetes", nullptr, &estat.opcionsVisualitzacio.mostrarEtiquetes);
            ImGui::MenuItem("Llum del Sol", nullptr, &estat.opcionsVisualitzacio.iluminacioSolarDirecta);
            ImGui::MenuItem("Vista Clara (Sense ombres)", nullptr, &estat.opcionsVisualitzacio.vistaClaraAmbient);
            ImGui::Separator();
            ImGui::MenuItem("Fons d'Estrelles", nullptr, &estat.opcionsVisualitzacio.mostrarSkybox);
            ImGui::MenuItem("Mostrar Eixos de Guia", nullptr, &estat.opcionsVisualitzacio.mostrarEixos);
            ImGui::EndMenu();
        }

        // 5) Telemetria de l'astre i dialeg informatiu
        if (ImGui::BeginMenu("Ajuda"))
        {
            ImGui::SetWindowFontScale(factorFontMenu);
            ImGui::MenuItem("Mostrar Fitxa de l'Astre", nullptr, &estat.mostrarFitxaHUD);
            ImGui::Separator();
            if (ImGui::MenuItem("Quant al Simulador...")) {
                estat.mostrarDialegQuantA = true;
            }
            ImGui::EndMenu();
        }
    }

    // ==================== ZONA DRETA: MODE, PAUSA/REPRENDRE, CANVIAR I FPS ====================
    // Calcula l'amplada exacta de tots els widgets de la dreta per ancorar-los sense solapaments
    float ampleTotal = ImGui::GetWindowWidth();

    const char* etiquetaMode = (estat.modeActual == ModeSimulacio::INTUITIU) 
        ? "[MODE DIDÀCTIC]" 
        : "[ESCALA REAL]";
    ImVec4 colorMode = (estat.modeActual == ModeSimulacio::INTUITIU)
        ? ImVec4(0.2f, 0.9f, 0.4f, 1.0f)
        : ImVec4(1.0f, 0.75f, 0.2f, 1.0f);

    float fps = ImGui::GetIO().Framerate;
    char bufferFps[32];
    snprintf(bufferFps, sizeof(bufferFps), "%.1f FPS", fps);

    float frameH = ImGui::GetFrameHeight();
    float textH = ImGui::GetTextLineHeight();
    float altBtnPausa = textH + 6.0f;
    if (altBtnPausa > frameH - 4.0f) altBtnPausa = frameH - 4.0f;
    if (altBtnPausa < 18.0f) altBtnPausa = 18.0f;
    float ampleBtnPausa = altBtnPausa * 1.25f;

    float ampleEtiqueta = ImGui::CalcTextSize(etiquetaMode).x;
    float ampleBotoCanviar = ImGui::CalcTextSize("Canviar").x + (padX * 2.0f) + 12.0f;
    float ampleFps = ImGui::CalcTextSize(bufferFps).x;
    float espaiSeparadors = padX * 4.2f;

    float ampleWidgetsDreta = ampleEtiqueta + ampleBtnPausa + ampleBotoCanviar + ampleFps + espaiSeparadors + 24.0f;

    float xActual = ImGui::GetCursorPosX();
    float xDesti = ampleTotal - ampleWidgetsDreta;

    if (xDesti > xActual + 15.0f) {
        ImGui::SameLine(xDesti);
    } else {
        ImGui::SameLine();
    }

    ImGui::TextColored(colorMode, "%s", etiquetaMode);
    ImGui::SameLine();

    // Boto compacte de Pausa / Reprendre a la barra superior abans de "Canviar"
    ImGui::BeginGroup();
    ImVec2 posSlot = ImGui::GetCursorScreenPos();
    ImGui::Dummy(ImVec2(ampleBtnPausa, frameH));
    ImVec2 pPausaMin(posSlot.x, posSlot.y + (frameH - altBtnPausa) * 0.5f);
    ImVec2 pPausaMax(pPausaMin.x + ampleBtnPausa, pPausaMin.y + altBtnPausa);
    ImGui::SetCursorScreenPos(pPausaMin);
    if (ImGui::InvisibleButton("##BotoPausaBarraMenu", ImVec2(ampleBtnPausa, altBtnPausa))) {
        estat.tempsSimulacio.enPausa = !estat.tempsSimulacio.enPausa;
    }
    bool hoverPausaBar = ImGui::IsItemHovered();
    if (hoverPausaBar) {
        ImGui::SetTooltip(estat.tempsSimulacio.enPausa ? "Reprendre simulació (Espai)" : "Pausar simulació (Espai)");
    }
    ImDrawList* dlBar = ImGui::GetWindowDrawList();
    ImU32 bgPausaBar = hoverPausaBar ? IM_COL32(22, 68, 76, 238) : IM_COL32(16, 48, 60, 225);
    dlBar->AddRectFilled(pPausaMin, pPausaMax, bgPausaBar, 5.5f);
    dlBar->AddRect(pPausaMin, pPausaMax, IM_COL32(55, 238, 180, 240), 5.5f, 0, 1.3f);
    DibuixarIconaPausaReproduccio(
        dlBar,
        ImVec2((pPausaMin.x + pPausaMax.x) * 0.5f, (pPausaMin.y + pPausaMax.y) * 0.5f),
        altBtnPausa * 1.15f,
        estat.tempsSimulacio.enPausa
    );
    ImGui::EndGroup();

    ImGui::SameLine();

    // Reobre el selector inicial per canviar entre Mode Didactic i Escala Real
    if (ImGui::Button("Canviar")) {
        estat.mostrarSelectorModes = true;
    }
    ImGui::SameLine();

    // Comptador de FPS (verd si > 45 FPS, vermell si cau per sota)
    ImGui::TextColored(fps > 45.0f ? ImVec4(0.3f, 1.0f, 0.3f, 1.0f) : ImVec4(1.0f, 0.4f, 0.4f, 1.0f), 
                       "%s", bufferFps);

    ImGui::EndMainMenuBar();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
}

// =========================================================================
// HELPERS DEL PANELL DE CONTROL DE TEMPS I VELOCITAT
// =========================================================================
// Marques equiespaiades del slider [0..1] amb mapeig logaritmic per trams de velocitat
static const int kNumMarquesSlider = 5;
static const float kPosMarques[kNumMarquesSlider] = { 0.00f, 0.25f, 0.50f, 0.75f, 1.00f };
static const double kValMarques[kNumMarquesSlider] = { 60.0, 3600.0, 86400.0, 864000.0, 1728000.0 };
static const char* kEtiquetesMarques[kNumMarquesSlider] = {
    "1 min/s", "1 h/s", "1 dia/s", "10 dies/s", "20 dies/s"
};

// Converteix una velocitat (en segons simulats/segon) a posicio normalitzada [0..1] del slider
static float VelocitatANormalitzat(double vel)
{
    if (vel <= kValMarques[0]) return 0.0f;
    if (vel >= kValMarques[kNumMarquesSlider - 1]) return 1.0f;
    for (int i = 0; i < kNumMarquesSlider - 1; ++i) {
        if (vel <= kValMarques[i + 1]) {
            double ratio = std::log(vel / kValMarques[i]) / std::log(kValMarques[i + 1] / kValMarques[i]);
            return kPosMarques[i] + (float)ratio * (kPosMarques[i + 1] - kPosMarques[i]);
        }
    }
    return 1.0f;
}

// Inversa de VelocitatANormalitzat: transforma una posicio normalitzada [0..1] del slider a segons/segon
static double NormalitzatAVelocitat(float u)
{
    if (u <= 0.0f) return kValMarques[0];
    if (u >= 1.0f) return kValMarques[kNumMarquesSlider - 1];
    for (int i = 0; i < kNumMarquesSlider - 1; ++i) {
        if (u <= kPosMarques[i + 1]) {
            double localT = (double)((u - kPosMarques[i]) / (kPosMarques[i + 1] - kPosMarques[i]));
            return kValMarques[i] * std::pow(kValMarques[i + 1] / kValMarques[i], localT);
        }
    }
    return kValMarques[kNumMarquesSlider - 1];
}

// Formata la velocitat de simulacio en unitats llegibles (minuts, hores o dies/s) i afegeix l'estat (PAUSADA/ENRERE)
static void FormatarVelocitatActual(double vel, bool enPausa, int direccioTemps, char* outBuf, size_t bufSize)
{
    char base[96];
    if (vel >= 86400.0 - 1.0) {
        double dies = vel / 86400.0;
        if (std::fabs(dies - 1.0) < 0.02) {
            snprintf(base, sizeof(base), "1 DIA PER SEGON");
        } else if (std::fabs(dies - std::round(dies)) < 0.08) {
            snprintf(base, sizeof(base), "%.0f DIES PER SEGON", std::round(dies));
        } else {
            snprintf(base, sizeof(base), "%.1f DIES PER SEGON", dies);
        }
    } else if (vel >= 3600.0 - 1.0) {
        double hores = vel / 3600.0;
        if (std::fabs(hores - 1.0) < 0.02) {
            snprintf(base, sizeof(base), "1 HORA PER SEGON");
        } else if (std::fabs(hores - std::round(hores)) < 0.08) {
            snprintf(base, sizeof(base), "%.0f HORES PER SEGON", std::round(hores));
        } else {
            snprintf(base, sizeof(base), "%.1f HORES PER SEGON", hores);
        }
    } else {
        double mins = vel / 60.0;
        if (std::fabs(mins - 1.0) < 0.02) {
            snprintf(base, sizeof(base), "1 MINUT PER SEGON");
        } else if (std::fabs(mins - std::round(mins)) < 0.08) {
            snprintf(base, sizeof(base), "%.0f MINUTS PER SEGON", std::round(mins));
        } else {
            snprintf(base, sizeof(base), "%.1f MINUTS PER SEGON", mins);
        }
    }

    const char* estatTxt = "";
    if (enPausa) {
        estatTxt = " (PAUSADA)";
    } else if (direccioTemps < 0) {
        estatTxt = " (ENRERE)";
    }
    snprintf(outBuf, bufSize, "VELOCITAT ACTUAL: %s%s", base, estatTxt);
}

// Interpola linealment dos colors RGBA empacats (ImU32) segons t en [0..1] per al degradat del slider
static ImU32 InterpolarColor(ImU32 colA, ImU32 colB, float t)
{
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    int rA = (colA >> IM_COL32_R_SHIFT) & 0xFF;
    int gA = (colA >> IM_COL32_G_SHIFT) & 0xFF;
    int bA = (colA >> IM_COL32_B_SHIFT) & 0xFF;
    int aA = (colA >> IM_COL32_A_SHIFT) & 0xFF;
    int rB = (colB >> IM_COL32_R_SHIFT) & 0xFF;
    int gB = (colB >> IM_COL32_G_SHIFT) & 0xFF;
    int bB = (colB >> IM_COL32_B_SHIFT) & 0xFF;
    int aB = (colB >> IM_COL32_A_SHIFT) & 0xFF;
    int r = (int)(rA + (rB - rA) * t);
    int g = (int)(gA + (gB - gA) * t);
    int b = (int)(bA + (bB - bA) * t);
    int a = (int)(aA + (aB - aA) * t);
    return IM_COL32(r, g, b, a);
}

// Icona vectorial de rellotge per al boto de seleccio d'hora
static void DibuixarIconaRellotge(ImDrawList* dl, ImVec2 centre, float radi, ImU32 col)
{
    dl->AddCircle(centre, radi, col, 28, 2.2f);
    dl->AddLine(centre, ImVec2(centre.x, centre.y - radi * 0.56f), col, 2.2f);
    dl->AddLine(centre, ImVec2(centre.x + radi * 0.45f, centre.y + radi * 0.25f), col, 2.2f);
    dl->AddCircleFilled(centre, 1.8f, col);
}

// Icona vectorial de calendari (marc + anelles + graella 3x2) per al boto de seleccio de data
static void DibuixarIconaCalendari(ImDrawList* dl, ImVec2 centre, float mida, ImU32 col)
{
    float hw = mida * 0.50f;
    float hh = mida * 0.46f;
    ImVec2 pMin(centre.x - hw, centre.y - hh + 2.0f);
    ImVec2 pMax(centre.x + hw, centre.y + hh + 2.0f);

    dl->AddRect(pMin, pMax, col, 3.0f, 0, 2.0f);
    float yBar = pMin.y + mida * 0.28f;
    dl->AddLine(ImVec2(pMin.x, yBar), ImVec2(pMax.x, yBar), col, 1.8f);

    // Anelles superiors
    dl->AddLine(ImVec2(centre.x - hw * 0.48f, pMin.y - 3.5f), ImVec2(centre.x - hw * 0.48f, pMin.y + 3.0f), col, 2.0f);
    dl->AddLine(ImVec2(centre.x + hw * 0.48f, pMin.y - 3.5f), ImVec2(centre.x + hw * 0.48f, pMin.y + 3.0f), col, 2.0f);

    // Graella de dies (3x2)
    float startX = pMin.x + mida * 0.24f;
    float stepX  = (pMax.x - pMin.x - mida * 0.48f) / 2.0f;
    float startY = yBar + (pMax.y - yBar) * 0.34f;
    float stepY  = (pMax.y - yBar) * 0.36f;
    for (int r = 0; r < 2; ++r) {
        for (int c = 0; c < 3; ++c) {
            ImVec2 pDot(startX + c * stepX, startY + r * stepY);
            dl->AddRectFilled(ImVec2(pDot.x - 1.7f, pDot.y - 1.7f), ImVec2(pDot.x + 1.7f, pDot.y + 1.7f), col, 0.5f);
        }
    }
}

// Icona vectorial de doble fletxa (<< amb dir=-1 per rebobinar, >> amb dir=+1 per avancar)
static void DibuixarIconaFletxesDobles(ImDrawList* dl, ImVec2 centre, float mida, int dir, ImU32 col)
{
    float s = (float)dir; // -1 esquerra, +1 dreta
    float wTri = mida * 0.42f;
    float hTri = mida * 0.48f;
    float offset = mida * 0.22f;

    for (int k = 0; k < 2; ++k) {
        float cx = centre.x + (k == 0 ? -offset : offset) * s;
        ImVec2 tip(cx + wTri * 0.55f * s, centre.y);
        ImVec2 top(cx - wTri * 0.55f * s, centre.y - hTri);
        ImVec2 bot(cx - wTri * 0.55f * s, centre.y + hTri);
        dl->AddTriangleFilled(tip, top, bot, col);
    }
}

// Pop-up emergent per editar manualment Hora, Minut i Segon (o aplicar presets 00:00 / 12:00 / Ara)
static void DibuixarPopupSelectorHora(EstatSimulacio& estat, bool& canviDataSollicitat)
{
    ImGui::PushStyleColor(ImGuiCol_PopupBg, IM_COL32(12, 26, 46, 248));
    ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(50, 200, 255, 220));
    ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 10.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14.0f, 12.0f));

    if (ImGui::BeginPopup("PopupSelectorHora")) {
        PushFontSegur(s_fontPanellRegular);
        DataHoraSimulacio& dt = estat.tempsSimulacio.dataHora;

        ImGui::TextColored(ImVec4(0.35f, 0.88f, 1.0f, 1.0f), "SELECCIONAR HORA");
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Text("Hora:"); ImGui::SameLine(85.0f);
        ImGui::SetNextItemWidth(110.0f);
        if (ImGui::InputInt("##HoraInput", &dt.hora, 1, 3)) { dt.Validar(); canviDataSollicitat = true; }

        ImGui::Text("Minut:"); ImGui::SameLine(85.0f);
        ImGui::SetNextItemWidth(110.0f);
        if (ImGui::InputInt("##MinutInput", &dt.minut, 1, 5)) { dt.Validar(); canviDataSollicitat = true; }

        ImGui::Text("Segon:"); ImGui::SameLine(85.0f);
        ImGui::SetNextItemWidth(110.0f);
        if (ImGui::InputInt("##SegonInput", &dt.segon, 1, 10)) { dt.Validar(); canviDataSollicitat = true; }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("00:00:00", ImVec2(72, 26))) {
            dt.hora = 0; dt.minut = 0; dt.segon = 0; canviDataSollicitat = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("12:00:00", ImVec2(72, 26))) {
            dt.hora = 12; dt.minut = 0; dt.segon = 0; canviDataSollicitat = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Ara", ImVec2(50, 26))) {
            estat.tempsSimulacio.EstablirDataActual(); canviDataSollicitat = true;
        }

        ImGui::Spacing();
        if (ImGui::Button("Tancar", ImVec2(206, 26))) {
            ImGui::CloseCurrentPopup();
        }
        PopFontSegur(s_fontPanellRegular);
        ImGui::EndPopup();
    }

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(2);
}

// Pop-up emergent amb graella mensual gregoriana per seleccionar Dia, Mes i Any
static void DibuixarPopupSelectorData(EstatSimulacio& estat, bool& canviDataSollicitat)
{
    static const char* nomsMesosComplets[12] = {
        "Gener", "Febrer", "Març", "Abril", "Maig", "Juny",
        "Juliol", "Agost", "Setembre", "Octubre", "Novembre", "Desembre"
    };

    ImGui::PushStyleColor(ImGuiCol_PopupBg, IM_COL32(12, 26, 46, 248));
    ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(245, 155, 50, 220));
    ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 10.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14.0f, 12.0f));

    if (ImGui::BeginPopup("PopupSelectorData")) {
        PushFontSegur(s_fontPanellRegular);
        DataHoraSimulacio& dt = estat.tempsSimulacio.dataHora;

        ImGui::TextColored(ImVec4(1.0f, 0.70f, 0.25f, 1.0f), "SELECCIONAR DATA");
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("<##MesPrev", ImVec2(26, 26))) {
            dt.mes--;
            if (dt.mes < 1) { dt.mes = 12; dt.any--; }
            dt.Validar();
            canviDataSollicitat = true;
        }
        ImGui::SameLine();

        ImGui::SetNextItemWidth(105.0f);
        int mesIdx = dt.mes - 1;
        if (ImGui::Combo("##MesCombo", &mesIdx, nomsMesosComplets, IM_ARRAYSIZE(nomsMesosComplets))) {
            dt.mes = mesIdx + 1;
            dt.Validar();
            canviDataSollicitat = true;
        }
        ImGui::SameLine();

        if (ImGui::Button(">##MesSeg", ImVec2(26, 26))) {
            dt.mes++;
            if (dt.mes > 12) { dt.mes = 1; dt.any++; }
            dt.Validar();
            canviDataSollicitat = true;
        }
        ImGui::SameLine();

        ImGui::SetNextItemWidth(86.0f);
        if (ImGui::InputInt("##AnyInput", &dt.any, 1, 10)) {
            dt.Validar();
            canviDataSollicitat = true;
        }

        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.65f, 0.85f, 0.98f, 1.0f), "  Dl    Dt    Dc    Dj    Dv    Ds    Dg");

        int primerDiaSetmana = DataHoraSimulacio::DiaDeLaSetmana(1, dt.mes, dt.any);
        int diesMes = DataHoraSimulacio::DiesAlMes(dt.mes, dt.any);

        int col = 0;
        for (int b = 0; b < primerDiaSetmana; ++b) {
            ImGui::Dummy(ImVec2(32.0f, 24.0f));
            ImGui::SameLine(0.0f, 4.0f);
            col++;
        }
        for (int d = 1; d <= diesMes; ++d) {
            char bId[12];
            snprintf(bId, sizeof(bId), "%d##d%d", d, d);
            bool actiu = (d == dt.dia);
            if (actiu) {
                ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(235, 135, 30, 235));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(255, 160, 55, 255));
            }
            if (ImGui::Button(bId, ImVec2(32.0f, 24.0f))) {
                dt.dia = d;
                dt.Validar();
                canviDataSollicitat = true;
                ImGui::CloseCurrentPopup();
            }
            if (actiu) ImGui::PopStyleColor(2);

            col++;
            if (col < 7 && d < diesMes) {
                ImGui::SameLine(0.0f, 4.0f);
            } else {
                col = 0;
            }
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("Avui", ImVec2(66, 26))) {
            estat.tempsSimulacio.EstablirDataActual();
            canviDataSollicitat = true;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Any 2000", ImVec2(82, 26))) {
            estat.tempsSimulacio.EstablirJ2000();
            canviDataSollicitat = true;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Tancar", ImVec2(88, 26))) {
            ImGui::CloseCurrentPopup();
        }

        PopFontSegur(s_fontPanellRegular);
        ImGui::EndPopup();
    }

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(2);
}

// =========================================================================
// 2.3 CONTROL DE TEMPS I VELOCITAT (POP-UP FLOTANT EN MODE ESCALA REAL)
// =========================================================================
// Panell ancorat inicialment a la cantonada inferior dreta amb:
// 1) Marc espacial i titol; 2) Visor digital d'hora/data amb pop-ups;
// 3) Botons de reproduccio (<<, ||/>, >>); 4) Slider logaritmic de velocitat;
// 5) Dreceres rapides (1h/s, 1dia/s, 20dies/s); 6) Indicador d'estat inferior.
void InterficieUI::DibuixarPanellDataHoraTimelapse(EstatSimulacio& estat)
{
    if (estat.modeActual != ModeSimulacio::ESCALA_REAL || !estat.mostrarPanellTemps) return;

    const ImGuiViewport* vp = ImGui::GetMainViewport();
    const float ampleFinestra = 580.0f;
    const float alcadaFinestra = 382.0f;
    const float margeViewport = 20.0f;

    ImVec2 posInicial(
        vp->WorkPos.x + vp->WorkSize.x - ampleFinestra - margeViewport,
        vp->WorkPos.y + vp->WorkSize.y - alcadaFinestra - margeViewport
    );
    if (posInicial.x < vp->WorkPos.x + 8.0f) posInicial.x = vp->WorkPos.x + 8.0f;
    if (posInicial.y < vp->WorkPos.y + 8.0f) posInicial.y = vp->WorkPos.y + 8.0f;

    ImGui::SetNextWindowPos(posInicial, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(ampleFinestra, alcadaFinestra), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.90f);

    ImGuiWindowFlags flagsFinestra = ImGuiWindowFlags_NoTitleBar |
                                     ImGuiWindowFlags_NoResize |
                                     ImGuiWindowFlags_NoScrollbar |
                                     ImGuiWindowFlags_NoScrollWithMouse |
                                     ImGuiWindowFlags_NoCollapse |
                                     ImGuiWindowFlags_NoSavedSettings;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 14.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(10, 22, 40, 230));

    if (ImGui::Begin("ControlTempsVelocitat##PanellFlotant", &estat.mostrarPanellTemps, flagsFinestra))
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 winPos = ImGui::GetWindowPos();
        ImVec2 winSize = ImGui::GetWindowSize();
        const float alcadaCapcalera = 42.0f;

        DibuixarMarcFinestraEspacial(dl, winPos, winSize, "CONTROL DE TEMPS I VELOCITAT", &estat.mostrarPanellTemps, "##BotoTancarPanell", alcadaCapcalera);

        DataHoraSimulacio& dt = estat.tempsSimulacio.dataHora;
        bool canviDataSollicitat = false;

        // -----------------------------------------------------------------
        // 2. VISOR D'HORA I DATA SIMULADA (RELLOTGE | CAIXA DIGITAL | CALENDARI)
        // -----------------------------------------------------------------
        const float yVisor = winPos.y + alcadaCapcalera + 14.0f;
        const float alcadaCaixa = 66.0f;
        const float ampleCaixa = 376.0f;
        const float midaBotoLateral = 50.0f;
        const float gapVisor = 16.0f;
        const float ampleTotalVisor = midaBotoLateral * 2.0f + ampleCaixa + gapVisor * 2.0f;
        const float xIniciVisor = winPos.x + (winSize.x - ampleTotalVisor) * 0.5f;
        const float yBotoLateral = yVisor + (alcadaCaixa - midaBotoLateral) * 0.5f;

        // Botó esquerra: Icona de rellotge (cian) -> Selector d'hora
        ImVec2 posRellotge(xIniciVisor, yBotoLateral);
        ImGui::SetCursorScreenPos(posRellotge);
        if (ImGui::InvisibleButton("##BotoSelectorHora", ImVec2(midaBotoLateral, midaBotoLateral))) {
            ImGui::OpenPopup("PopupSelectorHora");
        }
        bool hoverRellotge = ImGui::IsItemHovered();
        bool actiuRellotge = ImGui::IsPopupOpen("PopupSelectorHora");
        ImU32 bgRellotge = (hoverRellotge || actiuRellotge) ? IM_COL32(20, 58, 92, 235) : IM_COL32(14, 34, 58, 220);
        ImU32 voraRellotge = (hoverRellotge || actiuRellotge) ? IM_COL32(85, 235, 255, 255) : IM_COL32(45, 195, 245, 210);
        dl->AddRectFilled(posRellotge, ImVec2(posRellotge.x + midaBotoLateral, posRellotge.y + midaBotoLateral), bgRellotge, 9.0f);
        dl->AddRect(posRellotge, ImVec2(posRellotge.x + midaBotoLateral, posRellotge.y + midaBotoLateral), voraRellotge, 9.0f, 0, 1.5f);
        DibuixarIconaRellotge(
            dl,
            ImVec2(posRellotge.x + midaBotoLateral * 0.5f, posRellotge.y + midaBotoLateral * 0.5f),
            13.5f,
            IM_COL32(65, 225, 255, 255)
        );

        // Caixa central digital: Etiqueta + HH:MM:SS — DD / Mes / AAAA
        ImVec2 posCaixa(xIniciVisor + midaBotoLateral + gapVisor, yVisor);
        ImVec2 maxCaixa(posCaixa.x + ampleCaixa, posCaixa.y + alcadaCaixa);
        dl->AddRectFilled(posCaixa, maxCaixa, IM_COL32(12, 28, 48, 232), 8.0f);
        dl->AddRect(posCaixa, maxCaixa, IM_COL32(55, 165, 220, 180), 8.0f, 0, 1.3f);

        PushFontSegur(s_fontPanellPetit);
        const char* titolVisor = "HORA I DATA SIMULADA";
        ImVec2 midaTitolVisor = ImGui::CalcTextSize(titolVisor);
        dl->AddText(
            ImVec2(posCaixa.x + (ampleCaixa - midaTitolVisor.x) * 0.5f, posCaixa.y + 8.0f),
            IM_COL32(170, 220, 245, 240),
            titolVisor
        );
        PopFontSegur(s_fontPanellPetit);

        static const char* mesosCurts[12] = {
            "Gen", "Feb", "Mar", "Abr", "Mai", "Jun",
            "Jul", "Ago", "Set", "Oct", "Nov", "Des"
        };
        int idxMes = dt.mes - 1;
        if (idxMes < 0 || idxMes >= 12) idxMes = 0;

        char bufDataHora[64];
        snprintf(bufDataHora, sizeof(bufDataHora),
                 "%02d:%02d:%02d  —  %02d / %s / %04d",
                 dt.hora, dt.minut, dt.segon, dt.dia, mesosCurts[idxMes], dt.any);

        PushFontSegur(s_fontPanellDigital);
        ImVec2 midaDigital = ImGui::CalcTextSize(bufDataHora);
        dl->AddText(
            ImVec2(posCaixa.x + (ampleCaixa - midaDigital.x) * 0.5f, posCaixa.y + 28.0f),
            IM_COL32(240, 248, 255, 255),
            bufDataHora
        );
        PopFontSegur(s_fontPanellDigital);

        // Botó dreta: Icona de calendari (taronja/ambre) -> Selector de data
        ImVec2 posCalendari(maxCaixa.x + gapVisor, yBotoLateral);
        ImGui::SetCursorScreenPos(posCalendari);
        if (ImGui::InvisibleButton("##BotoSelectorData", ImVec2(midaBotoLateral, midaBotoLateral))) {
            ImGui::OpenPopup("PopupSelectorData");
        }
        bool hoverCalendari = ImGui::IsItemHovered();
        bool actiuCalendari = ImGui::IsPopupOpen("PopupSelectorData");
        ImU32 bgCalendari = (hoverCalendari || actiuCalendari) ? IM_COL32(62, 42, 22, 235) : IM_COL32(14, 34, 58, 220);
        ImU32 voraCalendari = (hoverCalendari || actiuCalendari) ? IM_COL32(255, 185, 80, 255) : IM_COL32(240, 150, 50, 210);
        dl->AddRectFilled(posCalendari, ImVec2(posCalendari.x + midaBotoLateral, posCalendari.y + midaBotoLateral), bgCalendari, 9.0f);
        dl->AddRect(posCalendari, ImVec2(posCalendari.x + midaBotoLateral, posCalendari.y + midaBotoLateral), voraCalendari, 9.0f, 0, 1.5f);
        DibuixarIconaCalendari(
            dl,
            ImVec2(posCalendari.x + midaBotoLateral * 0.5f, posCalendari.y + midaBotoLateral * 0.5f),
            24.0f,
            IM_COL32(250, 165, 55, 255)
        );

        DibuixarPopupSelectorHora(estat, canviDataSollicitat);
        DibuixarPopupSelectorData(estat, canviDataSollicitat);

        // -----------------------------------------------------------------
        // 3. CONTROLS DE REPRODUCCIO (<< | || / > | >>)
        // -----------------------------------------------------------------
        const float ySecRepro = yVisor + alcadaCaixa + 14.0f;
        PushFontSegur(s_fontPanellPetit);
        const char* titolRepro = "CONTROLS DE REPRODUCCIÓ";
        ImVec2 midaTitolRepro = ImGui::CalcTextSize(titolRepro);
        dl->AddText(
            ImVec2(winPos.x + (winSize.x - midaTitolRepro.x) * 0.5f, ySecRepro),
            IM_COL32(190, 228, 250, 245),
            titolRepro
        );
        PopFontSegur(s_fontPanellPetit);

        const float yBotonsRepro = ySecRepro + 22.0f;
        const float wSideBtn = 56.0f;
        const float hSideBtn = 42.0f;
        const float wCenterBtn = 96.0f;
        const float hCenterBtn = 46.0f;
        const float gapRepro = 16.0f;
        const float ampleRepro = wSideBtn * 2.0f + wCenterBtn + gapRepro * 2.0f;
        const float xIniciRepro = winPos.x + (winSize.x - ampleRepro) * 0.5f;

        bool esEnrereActiu = (!estat.tempsSimulacio.enPausa && estat.tempsSimulacio.direccioTemps < 0);
        bool esEndavantActiu = (!estat.tempsSimulacio.enPausa && estat.tempsSimulacio.direccioTemps > 0);

        // Botó 1: Doble fletxa enrere (<<)
        ImVec2 posEnrere(xIniciRepro, yBotonsRepro + (hCenterBtn - hSideBtn) * 0.5f);
        ImGui::SetCursorScreenPos(posEnrere);
        if (ImGui::InvisibleButton("##BotoRebobinar", ImVec2(wSideBtn, hSideBtn))) {
            estat.tempsSimulacio.direccioTemps = -1;
            estat.tempsSimulacio.enPausa = false;
        }
        bool hoverEnrere = ImGui::IsItemHovered();
        ImU32 bgEnrere = esEnrereActiu
            ? IM_COL32(48, 38, 28, 235)
            : (hoverEnrere ? IM_COL32(28, 54, 88, 235) : IM_COL32(16, 36, 62, 220));
        ImU32 voraEnrere = esEnrereActiu
            ? IM_COL32(255, 165, 55, 240)
            : (hoverEnrere ? IM_COL32(95, 200, 255, 215) : IM_COL32(55, 145, 205, 175));
        dl->AddRectFilled(posEnrere, ImVec2(posEnrere.x + wSideBtn, posEnrere.y + hSideBtn), bgEnrere, 8.0f);
        dl->AddRect(posEnrere, ImVec2(posEnrere.x + wSideBtn, posEnrere.y + hSideBtn), voraEnrere, 8.0f, 0, 1.4f);
        DibuixarIconaFletxesDobles(
            dl,
            ImVec2(posEnrere.x + wSideBtn * 0.5f, posEnrere.y + hSideBtn * 0.5f),
            20.0f,
            -1,
            IM_COL32(255, 150, 45, 255)
        );

        // Botó 2: Combinat Pausa / Repren ("|| / >") amb vora ressaltada verd/cian
        ImVec2 posPausaPlay(xIniciRepro + wSideBtn + gapRepro, yBotonsRepro);
        ImGui::SetCursorScreenPos(posPausaPlay);
        if (ImGui::InvisibleButton("##BotoPausaRepren", ImVec2(wCenterBtn, hCenterBtn))) {
            estat.tempsSimulacio.enPausa = !estat.tempsSimulacio.enPausa;
        }
        bool hoverPausaPlay = ImGui::IsItemHovered();
        dl->AddRect(
            ImVec2(posPausaPlay.x - 2.0f, posPausaPlay.y - 2.0f),
            ImVec2(posPausaPlay.x + wCenterBtn + 2.0f, posPausaPlay.y + hCenterBtn + 2.0f),
            IM_COL32(45, 235, 180, hoverPausaPlay ? 70 : 38),
            10.0f, 0, 2.5f
        );
        ImU32 bgPausaPlay = hoverPausaPlay ? IM_COL32(22, 68, 76, 238) : IM_COL32(16, 48, 60, 225);
        dl->AddRectFilled(posPausaPlay, ImVec2(posPausaPlay.x + wCenterBtn, posPausaPlay.y + hCenterBtn), bgPausaPlay, 9.0f);
        dl->AddRect(posPausaPlay, ImVec2(posPausaPlay.x + wCenterBtn, posPausaPlay.y + hCenterBtn), IM_COL32(55, 238, 180, 240), 9.0f, 0, 1.8f);
        DibuixarIconaPausaReproduccio(
            dl,
            ImVec2(posPausaPlay.x + wCenterBtn * 0.5f, posPausaPlay.y + hCenterBtn * 0.5f),
            hCenterBtn,
            estat.tempsSimulacio.enPausa
        );

        // Botó 3: Doble fletxa endavant (>>)
        ImVec2 posEndavant(posPausaPlay.x + wCenterBtn + gapRepro, yBotonsRepro + (hCenterBtn - hSideBtn) * 0.5f);
        ImGui::SetCursorScreenPos(posEndavant);
        if (ImGui::InvisibleButton("##BotoEndavant", ImVec2(wSideBtn, hSideBtn))) {
            estat.tempsSimulacio.direccioTemps = 1;
            estat.tempsSimulacio.enPausa = false;
        }
        bool hoverEndavant = ImGui::IsItemHovered();
        ImU32 bgEndavant = esEndavantActiu
            ? IM_COL32(48, 38, 28, 235)
            : (hoverEndavant ? IM_COL32(28, 54, 88, 235) : IM_COL32(16, 36, 62, 220));
        ImU32 voraEndavant = esEndavantActiu
            ? IM_COL32(255, 165, 55, 240)
            : (hoverEndavant ? IM_COL32(95, 200, 255, 215) : IM_COL32(55, 145, 205, 175));
        dl->AddRectFilled(posEndavant, ImVec2(posEndavant.x + wSideBtn, posEndavant.y + hSideBtn), bgEndavant, 8.0f);
        dl->AddRect(posEndavant, ImVec2(posEndavant.x + wSideBtn, posEndavant.y + hSideBtn), voraEndavant, 8.0f, 0, 1.4f);
        DibuixarIconaFletxesDobles(
            dl,
            ImVec2(posEndavant.x + wSideBtn * 0.5f, posEndavant.y + hSideBtn * 0.5f),
            20.0f,
            +1,
            IM_COL32(255, 150, 45, 255)
        );

        // -----------------------------------------------------------------
        // 4. CONTROL LLISCANT (SLIDER) DE VELOCITAT
        // -----------------------------------------------------------------
        const float ySecSlider = yBotonsRepro + hCenterBtn + 14.0f;
        PushFontSegur(s_fontPanellPetit);
        const char* titolSlider = "VELOCITAT DE LA SIMULACIÓ";
        ImVec2 midaTitolSlider = ImGui::CalcTextSize(titolSlider);
        dl->AddText(
            ImVec2(winPos.x + (winSize.x - midaTitolSlider.x) * 0.5f, ySecSlider),
            IM_COL32(190, 228, 250, 245),
            titolSlider
        );
        PopFontSegur(s_fontPanellPetit);

        const float ampleTrack = 456.0f;
        const float alcadaTrack = 9.0f;
        const float alcadaHitSlider = 26.0f;
        const float xTrack = winPos.x + (winSize.x - ampleTrack) * 0.5f;
        const float yTrackCenter = ySecSlider + 30.0f;
        ImVec2 trackMin(xTrack, yTrackCenter - alcadaTrack * 0.5f);
        ImVec2 trackMax(xTrack + ampleTrack, yTrackCenter + alcadaTrack * 0.5f);

        ImGui::SetCursorScreenPos(ImVec2(xTrack - 8.0f, yTrackCenter - alcadaHitSlider * 0.5f));
        ImGui::InvisibleButton("##SliderVelocitatGrad", ImVec2(ampleTrack + 16.0f, alcadaHitSlider));
        if (ImGui::IsItemActive()) {
            float mouseX = ImGui::GetIO().MousePos.x;
            float uNou = (mouseX - xTrack) / ampleTrack;
            if (uNou < 0.0f) uNou = 0.0f;
            if (uNou > 1.0f) uNou = 1.0f;
            estat.tempsSimulacio.factorVelocitatSegonsPerSegon = NormalitzatAVelocitat(uNou);
        }
        bool hoverSlider = ImGui::IsItemHovered() || ImGui::IsItemActive();

        float uActual = VelocitatANormalitzat(estat.tempsSimulacio.factorVelocitatSegonsPerSegon);
        float thumbX = xTrack + uActual * ampleTrack;

        // Fons fosc de la barra
        dl->AddRectFilled(trackMin, trackMax, IM_COL32(12, 26, 46, 235), 4.5f);

        // Omplert actiu degradat de cian (esquerra) cap a taronja/ambre (dreta)
        if (thumbX > trackMin.x + 1.0f) {
            ImU32 colCian = IM_COL32(35, 228, 255, 235);
            ImU32 colAmbre = IM_COL32(248, 155, 48, 235);
            ImU32 colThumb = InterpolarColor(colCian, colAmbre, uActual);
            dl->PushClipRect(trackMin, ImVec2(thumbX, trackMax.y), true);
            dl->AddRectFilledMultiColor(
                trackMin,
                ImVec2(thumbX, trackMax.y),
                colCian, colThumb, colThumb, colCian
            );
            dl->PopClipRect();
        }

        // Vora bicolor (cian a l'esquerra del tirador, taronja/ambre a la dreta)
        if (thumbX > trackMin.x + 1.0f) {
            dl->PushClipRect(ImVec2(trackMin.x - 1.0f, trackMin.y - 1.0f), ImVec2(thumbX, trackMax.y + 1.0f), true);
            dl->AddRect(trackMin, trackMax, IM_COL32(60, 230, 255, 235), 4.5f, 0, 1.4f);
            dl->PopClipRect();
        }
        if (thumbX < trackMax.x - 1.0f) {
            dl->PushClipRect(ImVec2(thumbX, trackMin.y - 1.0f), ImVec2(trackMax.x + 1.0f, trackMax.y + 1.0f), true);
            dl->AddRect(trackMin, trackMax, IM_COL32(238, 148, 48, 225), 4.5f, 0, 1.4f);
            dl->PopClipRect();
        }

        // Marques i etiquetes de l'escala
        PushFontSegur(s_fontPanellPetit);
        for (int i = 0; i < kNumMarquesSlider; ++i) {
            float tx = xTrack + kPosMarques[i] * ampleTrack;
            ImU32 colMarca = (kPosMarques[i] <= uActual + 0.001f)
                ? IM_COL32(165, 238, 255, 240)
                : IM_COL32(150, 185, 215, 200);
            if (i >= 2 && std::fabs(kPosMarques[i] - uActual) < 0.03f) {
                colMarca = IM_COL32(255, 180, 80, 255);
            }
            dl->AddLine(ImVec2(tx, trackMax.y + 3.0f), ImVec2(tx, trackMax.y + 8.0f), colMarca, 1.3f);

            ImVec2 mTxt = ImGui::CalcTextSize(kEtiquetesMarques[i]);
            float xTxt = tx - mTxt.x * 0.5f;
            dl->AddText(ImVec2(xTxt, trackMax.y + 10.0f), colMarca, kEtiquetesMarques[i]);
        }
        PopFontSegur(s_fontPanellPetit);

        // Tirador vertical translúcid
        const float wThumb = 14.0f;
        const float hThumb = 24.0f;
        ImVec2 thumbMin(thumbX - wThumb * 0.5f, yTrackCenter - hThumb * 0.5f);
        ImVec2 thumbMax(thumbX + wThumb * 0.5f, yTrackCenter + hThumb * 0.5f);
        dl->AddRect(
            ImVec2(thumbMin.x - 1.5f, thumbMin.y - 1.5f),
            ImVec2(thumbMax.x + 1.5f, thumbMax.y + 1.5f),
            IM_COL32(95, 230, 255, hoverSlider ? 95 : 48),
            5.0f, 0, 2.0f
        );
        dl->AddRectFilled(thumbMin, thumbMax, IM_COL32(170, 235, 255, hoverSlider ? 150 : 115), 4.5f);
        dl->AddRect(thumbMin, thumbMax, IM_COL32(205, 248, 255, 245), 4.5f, 0, 1.5f);

        // -----------------------------------------------------------------
        // 5. DRECERES DE VELOCITAT (3 BOTONS RECTANGULARS)
        // -----------------------------------------------------------------
        const float ySecDreceres = yTrackCenter + 34.0f;
        PushFontSegur(s_fontPanellPetit);
        const char* titolDreceres = "DRECERES DE VELOCITAT";
        ImVec2 midaTitolDreceres = ImGui::CalcTextSize(titolDreceres);
        dl->AddText(
            ImVec2(winPos.x + (winSize.x - midaTitolDreceres.x) * 0.5f, ySecDreceres),
            IM_COL32(190, 228, 250, 245),
            titolDreceres
        );
        PopFontSegur(s_fontPanellPetit);

        struct DreceraVel {
            const char* etiqueta;
            double valorSegons;
        };
        static const DreceraVel kDreceres[3] = {
            { "1 HORA / S", 3600.0 },
            { "1 DIA / S",  86400.0 },
            { "20 DIES / S", 1728000.0 }
        };

        const float yBotonsDrecera = ySecDreceres + 22.0f;
        const float wDrecera = 148.0f;
        const float hDrecera = 32.0f;
        const float gapDrecera = 14.0f;
        const float ampleDreceres = wDrecera * 3.0f + gapDrecera * 2.0f;
        const float xIniciDreceres = winPos.x + (winSize.x - ampleDreceres) * 0.5f;

        PushFontSegur(s_fontPanellRegular);
        for (int i = 0; i < 3; ++i) {
            ImVec2 pBtn(xIniciDreceres + i * (wDrecera + gapDrecera), yBotonsDrecera);
            ImVec2 pBtnMax(pBtn.x + wDrecera, pBtn.y + hDrecera);

            char btnId[32];
            snprintf(btnId, sizeof(btnId), "##DreceraVel%d", i);
            ImGui::SetCursorScreenPos(pBtn);
            if (ImGui::InvisibleButton(btnId, ImVec2(wDrecera, hDrecera))) {
                estat.tempsSimulacio.factorVelocitatSegonsPerSegon = kDreceres[i].valorSegons;
            }

            bool hoverBtn = ImGui::IsItemHovered();
            double diffRel = std::fabs(estat.tempsSimulacio.factorVelocitatSegonsPerSegon - kDreceres[i].valorSegons) / kDreceres[i].valorSegons;
            bool actiuBtn = (diffRel < 0.015);

            ImU32 bgBtn = actiuBtn
                ? IM_COL32(26, 72, 112, 235)
                : (hoverBtn ? IM_COL32(24, 54, 88, 230) : IM_COL32(14, 34, 58, 215));
            ImU32 voraBtn = actiuBtn
                ? IM_COL32(75, 225, 255, 245)
                : (hoverBtn ? IM_COL32(80, 190, 245, 210) : IM_COL32(55, 140, 200, 175));

            dl->AddRectFilled(pBtn, pBtnMax, bgBtn, 7.0f);
            dl->AddRect(pBtn, pBtnMax, voraBtn, 7.0f, 0, actiuBtn ? 1.6f : 1.2f);

            ImVec2 mTxt = ImGui::CalcTextSize(kDreceres[i].etiqueta);
            dl->AddText(
                ImVec2(pBtn.x + (wDrecera - mTxt.x) * 0.5f, pBtn.y + (hDrecera - mTxt.y) * 0.5f),
                actiuBtn ? IM_COL32(255, 255, 255, 255) : IM_COL32(220, 240, 255, 245),
                kDreceres[i].etiqueta
            );
        }
        PopFontSegur(s_fontPanellRegular);

        // -----------------------------------------------------------------
        // 6. INDICADOR D'ESTAT INFERIOR (FOOTER)
        // -----------------------------------------------------------------
        char bufFooter[128];
        FormatarVelocitatActual(
            estat.tempsSimulacio.factorVelocitatSegonsPerSegon,
            estat.tempsSimulacio.enPausa,
            estat.tempsSimulacio.direccioTemps,
            bufFooter,
            sizeof(bufFooter)
        );

        PushFontSegur(s_fontPanellRegular);
        ImVec2 midaFooter = ImGui::CalcTextSize(bufFooter);
        const float yFooter = winPos.y + winSize.y - midaFooter.y - 16.0f;
        dl->AddText(
            ImVec2(winPos.x + (winSize.x - midaFooter.x) * 0.5f, yFooter),
            IM_COL32(232, 246, 255, 255),
            bufFooter
        );
        PopFontSegur(s_fontPanellRegular);

        if (canviDataSollicitat) {
            estat.recalculEfemeridesNecessari = true;
        }
    }
    ImGui::End();

    ImGui::PopStyleColor();
    ImGui::PopStyleVar(3);
}

// =========================================================================
// 2.6 FITXA DE L'ASTRE (HUD FLOTANT)
// =========================================================================
// Mostra les dades fisiques de estat.dadesCosActiu (actualitzades amb ActualitzarDadesCosActiu)
// a la cantonada superior dreta mentre estat.mostrarFitxaHUD sigui true.
void InterficieUI::DibuixarFitxaAstronomicaHUD(const CosAstronomicDades& dades, bool* pObert)
{
    if (!pObert || !*pObert) return;

    const ImGuiViewport* vp = ImGui::GetMainViewport();
    const float ampleFinestra = 340.0f;
    const float alcadaFinestra = 276.0f;

    ImVec2 pos(vp->WorkPos.x + vp->WorkSize.x - ampleFinestra - 20.0f, vp->WorkPos.y + 44.0f);
    ImGui::SetNextWindowPos(pos, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(ampleFinestra, alcadaFinestra), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.90f);

    ImGuiWindowFlags flagsFinestra = ImGuiWindowFlags_NoTitleBar |
                                     ImGuiWindowFlags_NoResize |
                                     ImGuiWindowFlags_NoScrollbar |
                                     ImGuiWindowFlags_NoScrollWithMouse |
                                     ImGuiWindowFlags_NoCollapse |
                                     ImGuiWindowFlags_NoSavedSettings;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 14.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(10, 22, 40, 230));

    if (ImGui::Begin("FitxaAstre##HUD", pObert, flagsFinestra))
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 winPos = ImGui::GetWindowPos();
        ImVec2 winSize = ImGui::GetWindowSize();
        const float alcadaCapcalera = 42.0f;

        DibuixarMarcFinestraEspacial(dl, winPos, winSize, "FITXA DE L'ASTRE", pObert, "##TancarFitxaAstre", alcadaCapcalera);

        // Nom de l'astre destacat
        PushFontSegur(s_fontPanellTitol);
        ImVec2 midaNom = ImGui::CalcTextSize(dades.nom.c_str());
        dl->AddText(
            ImVec2(winPos.x + (winSize.x - midaNom.x) * 0.5f, winPos.y + alcadaCapcalera + 12.0f),
            IM_COL32(105, 225, 255, 255),
            dades.nom.c_str()
        );
        PopFontSegur(s_fontPanellTitol);

        // Caixa interior de dades simples
        const float padLat = 18.0f;
        ImVec2 caixaMin(winPos.x + padLat, winPos.y + alcadaCapcalera + 42.0f);
        ImVec2 caixaMax(winPos.x + winSize.x - padLat, winPos.y + winSize.y - 18.0f);
        dl->AddRectFilled(caixaMin, caixaMax, IM_COL32(12, 28, 48, 232), 9.0f);
        dl->AddRect(caixaMin, caixaMax, IM_COL32(55, 165, 220, 165), 9.0f, 0, 1.2f);

        char valRadi[48], valDist[48], valGrav[48], valDia[48], valAny[48];
        snprintf(valRadi, sizeof(valRadi), "%.0f km", dades.radiKm);
        if (dades.distSolUA <= 0.0001) {
            snprintf(valDist, sizeof(valDist), "Centre del sistema");
        } else {
            snprintf(valDist, sizeof(valDist), "%.2f UA", dades.distSolUA);
        }
        snprintf(valGrav, sizeof(valGrav), "%.1f m/s²", dades.gravetat);
        snprintf(valDia, sizeof(valDia), "%.1f h", dades.periodeRotacioHores);
        if (dades.periodeOrbitalAnys <= 0.0001) {
            snprintf(valAny, sizeof(valAny), "—");
        } else {
            snprintf(valAny, sizeof(valAny), "%.2f anys", dades.periodeOrbitalAnys);
        }

        struct FilaDada {
            const char* etiqueta;
            const char* valor;
        };
        FilaDada files[5] = {
            { "Radi:",            valRadi },
            { "Distància al Sol:", valDist },
            { "Gravetat:",        valGrav },
            { "Durada del dia:",  valDia },
            { "Volta al Sol:",    valAny }
        };

        PushFontSegur(s_fontPanellRegular);
        float startY = caixaMin.y + 15.0f;
        float pasY = 34.0f;
        for (int i = 0; i < 5; ++i) {
            float y = startY + i * pasY;
            dl->AddText(ImVec2(caixaMin.x + 16.0f, y), IM_COL32(175, 218, 245, 240), files[i].etiqueta);
            ImVec2 mVal = ImGui::CalcTextSize(files[i].valor);
            dl->AddText(ImVec2(caixaMax.x - mVal.x - 16.0f, y), IM_COL32(242, 249, 255, 255), files[i].valor);
            if (i < 4) {
                dl->AddLine(
                    ImVec2(caixaMin.x + 14.0f, y + 25.0f),
                    ImVec2(caixaMax.x - 14.0f, y + 25.0f),
                    IM_COL32(45, 110, 165, 75),
                    1.0f
                );
            }
        }
        PopFontSegur(s_fontPanellRegular);
    }
    ImGui::End();

    ImGui::PopStyleColor();
    ImGui::PopStyleVar(3);
}

// =========================================================================
// 2.7 PROJECCIO D'ETIQUETES 3D A PANTALLA 2D AMB IMDRAWLIST
// =========================================================================
// Projecta una posicio 3D del mon cap a coordenades de pantalla (invertint Y respecte OpenGL)
// i dibuixa un punt cian + el nom de l'astre al fons del viewport quan es davant la camera.
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
