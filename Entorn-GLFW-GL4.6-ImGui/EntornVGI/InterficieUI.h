// InterficieUI.h — Arquitectura: UI
// Defineix l'estat global del simulador i la classe InterficieUI (selector inicial, menús, panell timelapse, HUD i marcadors 2D ImGui).

#pragma once

#include <string>
#include <vector>
#include <ctime>
#include "framework.h"
#include "escenari.h"
#include "CatalegAstronomic.h"

// Modes de funcionament de la simulació
enum class ModeSimulacio {
    INTUITIU,    // Mode Didàctic / Intuïtiu
    ESCALA_REAL  // Mode a Escala Real (Científic)
};

// Modes de càmera del simulador
enum class ModeCamera {
    ORBITAL_CENTRADOR,      // Càmera esfèrica lliure centrada en l'astre enfocat
    FIXA_INCLINADA_25,      // Càmera esfèrica amb inclinació vertical fixa de 25 graus
    GLOBAL_SISTEMA,         // Vista global del sistema solar des de dalt
    COET_PRIMERA_PERSONA    // Càmera lliure en primera persona (mode Navega)
};

// Estructura per a dates i hores gregòries amb validació de rang
struct DataHoraSimulacio {
    int any = 2026;    // [any gregorià, 1550..2649]
    int mes = 10;      // [mes, 1..12]
    int dia = 18;      // [dia del mes, 1..DiesAlMes(mes, any)]
    int hora = 12;     // [hores UTC, 0..23]
    int minut = 0;     // [minuts, 0..59]
    int segon = 0;     // [segons, 0..59]

    // Paràmetres: `a` any gregorià. Retorn: true si `a` és any de traspàs.
    static bool EsAnyDeTraspas(int a) {
        return (a % 4 == 0 && a % 100 != 0) || (a % 400 == 0);
    }

    // Paràmetres: `m` [1..12], `a` any gregorià. Retorn: nombre de dies del mes [28..31].
    static int DiesAlMes(int m, int a) {
        static const int dies[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
        if (m == 2 && EsAnyDeTraspas(a)) return 29;
        if (m >= 1 && m <= 12) return dies[m - 1];
        return 31;
    }

    // Paràmetres: `d` [1..31], `m` [1..12], `a` any gregorià.
    // Retorn: índex del dia de la setmana [0 = Dilluns .. 6 = Diumenge].
    static int DiaDeLaSetmana(int d, int m, int a) {
        static const int t[12] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };
        int y = a;
        if (m < 3) y -= 1;
        int dowSun0 = (y + y / 4 - y / 100 + y / 400 + t[m - 1] + d) % 7;
        if (dowSun0 < 0) dowSun0 += 7;
        return (dowSun0 + 6) % 7;
    }

    // Ajusta tots els camps de la data/hora als seus límits vàlids (any [1550..2649], mes [1..12], dia [1..maxD], h/m/s).
    void Validar() {
        if (any < 1550) any = 1550; if (any > 2649) any = 2649;
        if (mes < 1) mes = 1; if (mes > 12) mes = 12;
        int maxD = DiesAlMes(mes, any);
        if (dia < 1) dia = 1; if (dia > maxD) dia = maxD;
        if (hora < 0) hora = 0; if (hora > 23) hora = 23;
        if (minut < 0) minut = 0; if (minut > 59) minut = 59;
        if (segon < 0) segon = 0; if (segon > 59) segon = 59;
    }
};

// Control de temps i timelapse (rang permès de velocitat: 1 min/s a 20 dies/s)
struct EstatTempsSimulacio {
    DataHoraSimulacio dataHora;                     // Data i hora actuals de la simulació [UTC]
    bool enPausa = false;                           // Estat de pausa temporal [true = pausat, false = actiu]
    int direccioTemps = 1;                          // Sentit temporal {-1: Enrere/Rebobinar, +1: Endavant}
    double factorVelocitatSegonsPerSegon = 60.0;    // [s_simulats / s_real, 60.0..1728000.0] (inicial: 1 min/s)
    double acumulatSegons = 0.0;                    // [s_simulats] Acumulador sub-segon entre frames

    // Estableix `dataHora` a l'instant local actual del rellotge del sistema.
    void EstablirDataActual() {
        time_t t = time(nullptr);
        tm ltm;
        localtime_s(&ltm, &t);
        dataHora.any = 1900 + ltm.tm_year;
        dataHora.mes = 1 + ltm.tm_mon;
        dataHora.dia = ltm.tm_mday;
        dataHora.hora = ltm.tm_hour;
        dataHora.minut = ltm.tm_min;
        dataHora.segon = ltm.tm_sec;
    }

    // Restableix `dataHora` a l'època de referència J2000 (2000-01-01 12:00:00).
    void EstablirJ2000() {
        dataHora = { 2000, 1, 1, 12, 0, 0 };
    }

    // Duplica la velocitat temporal fins al màxim de 20 dies/s (1.728.000 s/s).
    void AugmentarVelocitat() {
        factorVelocitatSegonsPerSegon *= 2.0;
        if (factorVelocitatSegonsPerSegon > 1728000.0) factorVelocitatSegonsPerSegon = 1728000.0;
    }

    // Redueix a la meitat la velocitat temporal fins al mínim d'1 min/s (60 s/s).
    void ReduirVelocitat() {
        factorVelocitatSegonsPerSegon *= 0.5;
        if (factorVelocitatSegonsPerSegon < 60.0) factorVelocitatSegonsPerSegon = 60.0;
    }

    // Paràmetres: `deltaSegons` [s reals] temps transcorregut des del darrer frame (es limita a 0.25 s).
    // Avança `dataHora` segons `direccioTemps * factorVelocitatSegonsPerSegon` si no està en pausa.
    void AvancarSegons(double deltaSegons) {
        if (enPausa) return;
        if (deltaSegons > 0.25) deltaSegons = 0.25;
        acumulatSegons += (double)direccioTemps * factorVelocitatSegonsPerSegon * deltaSegons;
        long long segonsPassats = (long long)acumulatSegons;
        if (segonsPassats != 0) {
            acumulatSegons -= (double)segonsPassats;
            AvancarSegonsInt(segonsPassats);
        }
    }

private:
    void AvancarSegonsInt(long long s) {
        long long totalSeg = (long long)dataHora.segon + s;
        long long extraMin = totalSeg / 60;
        long long remSeg = totalSeg % 60;
        if (remSeg < 0) { remSeg += 60; extraMin--; }
        dataHora.segon = (int)remSeg;

        long long totalMin = (long long)dataHora.minut + extraMin;
        long long extraHores = totalMin / 60;
        long long remMin = totalMin % 60;
        if (remMin < 0) { remMin += 60; extraHores--; }
        dataHora.minut = (int)remMin;

        long long totalHores = (long long)dataHora.hora + extraHores;
        long long extraDies = totalHores / 24;
        long long remHores = totalHores % 24;
        if (remHores < 0) { remHores += 24; extraDies--; }
        dataHora.hora = (int)remHores;

        dataHora.dia += (int)extraDies;

        while (true) {
            int dMes = DataHoraSimulacio::DiesAlMes(dataHora.mes, dataHora.any);
            if (dataHora.dia > dMes) {
                dataHora.dia -= dMes;
                dataHora.mes++;
                if (dataHora.mes > 12) { dataHora.mes = 1; dataHora.any++; }
            } else if (dataHora.dia < 1) {
                dataHora.mes--;
                if (dataHora.mes < 1) { dataHora.mes = 12; dataHora.any--; }
                dataHora.dia += DataHoraSimulacio::DiesAlMes(dataHora.mes, dataHora.any);
            } else {
                break;
            }
        }
    }
};

// Capes de renderitzat i opcions de visualització
struct OpcionsVisualitzacio {
    bool mostrarOrbites = true;              // Mostra o oculta les òrbites 3D dels astres
    bool mostrarEtiquetes = true;            // Mostra o oculta les etiquetes de text 2D dels planetes
    bool mostrarMarcadorsPetits = true;      // Mostra o oculta el halo/marcador 2D quan el cos és petit en pantalla
    bool mostrarSkybox = true;               // Mostra o oculta el fons estel·lar cubemap (Skybox)
    bool mostrarEixos = false;               // Mostra o oculta els eixos de coordenades del món (X, Y, Z)
    bool mostrarCinturoAsteroides = true;     // Flag de visualització del cinturó d'asteroides
    bool mostrarPuntsLagrange = false;       // Flag de visualització dels punts de Lagrange
    bool anellsSaturnParticules = false;     // false: disc pla amb textura alpha; true: mode partícules
    bool iluminacioSolarDirecta = true;      // Sol amb emissió pròpia + LightSource[0] activa
    bool vistaClaraAmbient = false;          // Llum ambient global per il·luminar les cares nocturnes
};

// Paràmetres de càmera de la interfície
struct CameraConfig {
    ModeCamera mode = ModeCamera::FIXA_INCLINADA_25; // Mode actiu de càmera
    float angleInclinacio = 25.0f;                   // [graus, 0.0..90.0] Inclinació vertical inicial
    float distZoom = 100.0f;                         // [unitats OpenGL, > 0] Distància de zoom de referència
};

// Àlies de compatibilitat cap al catàleg astronòmic unificat
using CosAstronomicDades = DadesCosAstronomic;

// Estat global compartit entre la interfície ImGui, el bucle principal i els escenaris
struct EstatSimulacio {
    ModeSimulacio modeActual = ModeSimulacio::INTUITIU; // Mode actiu (INTUITIU o ESCALA_REAL)
    bool mostrarSelectorModes = true;                   // true mentre es mostra el selector modal inicial
    bool mostrarPanellTemps = false;                    // true si el panell de data/hora i timelapse és visible
    bool mostrarFitxaHUD = false;                       // true si la fitxa astronòmica HUD del cos actiu és oberta
    bool mostrarDialegQuantA = false;                   // true si el diàleg modal "Quant a" és obert
    bool recalculEfemeridesNecessari = false;           // true quan l'usuari edita la data/hora i cal recalcular SPICE
    bool origenReferenciaSol = true;                    // true: Sol com a origen; false: planeta enfocat com a origen
    int cosEnfocatId = 0;                               // Índex del cos enfocat [0 = Sol .. 10 = Lluna]

    // Flags de sincronització amb el motor OpenGL (main.cpp)
    bool configuracioInicialPendent = false;            // true quan s'ha iniciat o commutat d'escenari i cal reinicialitzar GL
    bool canviCameraPendent = false;                    // true quan cal aplicar un nou mode de càmera o focus a OPV

    EstatTempsSimulacio tempsSimulacio;                 // Rellotge i paràmetres del timelapse
    OpcionsVisualitzacio opcionsVisualitzacio;          // Interruptors de capes de renderitzat i il·luminació
    CameraConfig cameraConfig;                          // Configuració de càmera seleccionada a la UI
    DadesCosAstronomic dadesCosActiu = CatalegAstronomic::GetCos(0); // Còpia de lectura de les dades del cos enfocat per al HUD
};

// Classe principal d'interfície d'usuari Dear ImGui
class InterficieUI
{
public:
    // Precondició: context ImGui creat (`ImGui::CreateContext`).
    // Configura el tema fosc espacial, arrodoniments i carrega les fonts sans-serif amb suport català.
    static void InicialitzarEstil();

    // Paràmetres: `estat` (actualitza `estat.dadesCosActiu` amb l'entrada del catàleg corresponent a `estat.cosEnfocatId`).
    static void ActualitzarDadesCosActiu(EstatSimulacio& estat);

    // Precondició: frame ImGui iniciat (`ImGui::NewFrame`), vector `escenaris[2]` inicialitzat.
    // Paràmetres: `estat`, `currentEscenari`, `escenaris[2]`, `shader_programID`.
    // Coordina tots els components UI visibles segons el mode actiu.
    static void Renderitzar(EstatSimulacio& estat, Escenari*& currentEscenari, Escenari* escenaris[2], GLuint& shader_programID);

    // Precondició: frame ImGui iniciat.
    // Mostra el selector inicial amb 2 targetes ("Mode Didàctic" i "Mode a Escala Real") i commuta `currentEscenari`.
    static void DibuixarSelectorInicialModes(EstatSimulacio& estat, Escenari*& currentEscenari, Escenari* escenaris[2], GLuint& shader_programID);

    // Precondició: frame ImGui iniciat.
    // Dibuixa la barra de menú superior dinàmica adaptada a `estat.modeActual` (Escenaris, Càmera, Astres, Visualització, Ajuda).
    static void DibuixarBarraMenuSuperior(EstatSimulacio& estat, Escenari*& currentEscenari, Escenari* escenaris[2], GLuint& shader_programID);

    // Precondició: frame ImGui iniciat.
    // Dibuixa el panell inferior dret de control de data/hora, reproducció/pausa i slider logarítmic de velocitat.
    static void DibuixarPanellDataHoraTimelapse(EstatSimulacio& estat);

    // Precondició: frame ImGui iniciat i `pObert != nullptr`.
    // Paràmetres: `dades` del cos astronòmic a mostrar, `pObert` punter al flag de visibilitat del HUD.
    static void DibuixarFitxaAstronomicaHUD(const DadesCosAstronomic& dades, bool* pObert);

    // Precondició: frame ImGui iniciat; matrius corresponents al frame renderitzat.
    // Paràmetres: `nom`, `posicioMon` [unitats OpenGL], `viewMatrix`, `projMatrix`, `viewport` (0, 0, w, h) [píxels].
    // Projecta una etiqueta de text 2D amb punt cian sobre la posició de l'astre si està davant de la càmera.
    static void DibuixarEtiquetaPlaneta(const std::string& nom, const glm::vec3& posicioMon, 
                                       const glm::mat4& viewMatrix, const glm::mat4& projMatrix, 
                                       const glm::vec4& viewport);

    // Precondició: frame ImGui iniciat; matrius corresponents al frame renderitzat.
    // Paràmetres: `nom`, `posicioMon` [u], `radiCos` [u], `colorCos` [RGB 0..1], `esCosEnfocat`, `mostrarEtiqueta`,
    // `viewMatrix`, `projMatrix`, `viewport` [píxels].
    // Dibuixa el marcador/halo 2D quan el diàmetre aparent en píxels és <= 25 px i mostra l'etiqueta si escau.
    static void DibuixarMarcadorPlaneta(const std::string& nom, const glm::vec3& posicioMon,
                                        float radiCos, const glm::vec3& colorCos,
                                        bool esCosEnfocat, bool mostrarEtiqueta,
                                        const glm::mat4& viewMatrix, const glm::mat4& projMatrix,
                                        const glm::vec4& viewport);
};
