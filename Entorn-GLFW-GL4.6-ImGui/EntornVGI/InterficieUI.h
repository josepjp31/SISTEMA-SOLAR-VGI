// InterficieUI.h : Declaracio de tipus, estructures d'estat i metodes de la interficie d'usuari

#pragma once

#include <string>
#include <vector>
#include <ctime>
#include "framework.h"
#include "escenari.h"
#include "CatalegAstronomic.h"

// Modes de funcionament de la simulacio
enum class ModeSimulacio {
    INTUITIU,    // Mode Didactic / Intuitiu
    ESCALA_REAL  // Mode a Escala Real (Cientific)
};

// Modes de camera del simulador
enum class ModeCamera {
    ORBITAL_CENTRADOR,      // Camera Esferica lliure centrada en l'astre enfocat
    FIXA_INCLINADA_25,      // Camera Esferica amb inclinacio vertical fixa de 25 graus
    GLOBAL_SISTEMA,         // Vista global del sistema solar des de dalt
    COET_PRIMERA_PERSONA    // Camera Lliure en primera persona (mode Navega)
};

// Estructura per a dates i hores amb validacio de calendari gregoria
struct DataHoraSimulacio {
    int any = 2026;
    int mes = 10;
    int dia = 18;
    int hora = 12;
    int minut = 0;
    int segon = 0;

    static bool EsAnyDeTraspas(int a) {
        return (a % 4 == 0 && a % 100 != 0) || (a % 400 == 0);
    }

    static int DiesAlMes(int m, int a) {
        static const int dies[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
        if (m == 2 && EsAnyDeTraspas(a)) return 29;
        if (m >= 1 && m <= 12) return dies[m - 1];
        return 31;
    }

    // Retorna 0 (Dilluns) .. 6 (Diumenge) per a una data gregoriana
    static int DiaDeLaSetmana(int d, int m, int a) {
        static const int t[12] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };
        int y = a;
        if (m < 3) y -= 1;
        int dowSun0 = (y + y / 4 - y / 100 + y / 400 + t[m - 1] + d) % 7;
        if (dowSun0 < 0) dowSun0 += 7;
        return (dowSun0 + 6) % 7;
    }

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

// Control de temps i timelapse (Rang permes: 1 min/s a 20 dies/s)
struct EstatTempsSimulacio {
    DataHoraSimulacio dataHora;
    bool enPausa = false;
    int direccioTemps = 1; // +1: Endavant, -1: Enrere (Rebobinar)
    double factorVelocitatSegonsPerSegon = 60.0; // Minim inicial: 1 min/s (60 s/s)
    double acumulatSegons = 0.0;

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

    void EstablirJ2000() {
        dataHora = { 2000, 1, 1, 12, 0, 0 };
    }

    void AugmentarVelocitat() {
        factorVelocitatSegonsPerSegon *= 2.0;
        if (factorVelocitatSegonsPerSegon > 1728000.0) factorVelocitatSegonsPerSegon = 1728000.0;
    }

    void ReduirVelocitat() {
        factorVelocitatSegonsPerSegon *= 0.5;
        if (factorVelocitatSegonsPerSegon < 60.0) factorVelocitatSegonsPerSegon = 60.0;
    }

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

// Capes de renderitzat i opcions de visualitzacio
struct OpcionsVisualitzacio {
    bool mostrarOrbites = true;
    bool mostrarEtiquetes = true;
    bool mostrarMarcadorsPetits = true;
    bool mostrarSkybox = true;
    bool mostrarEixos = false;
    bool mostrarCinturoAsteroides = true;
    bool mostrarPuntsLagrange = false;
    bool anellsSaturnParticules = false; // false: disc pla amb textura, true: particules instanciades
    bool iluminacioSolarDirecta = true;  // Sol com a emissor directe + Llum #0 activa
    bool vistaClaraAmbient = false;      // Vista clara (llum ambient) desactivada per defecte
};

// Parametres de camera
struct CameraConfig {
    ModeCamera mode = ModeCamera::FIXA_INCLINADA_25;
    float angleInclinacio = 25.0f; // Inclinacio inicial de 25 graus
    float distZoom = 100.0f;
};

// Alias de compatibilitat cap al cataleg astronomic unificat
using CosAstronomicDades = DadesCosAstronomic;

// Estat global de la interficie del simulador
struct EstatSimulacio {
    ModeSimulacio modeActual = ModeSimulacio::INTUITIU;
    bool mostrarSelectorModes = true;  // 2 quadrats centrals a l'inici (Selector Inicial)
    bool mostrarPanellTemps = false;   // Cantonada inferior dreta (mode escala)
    bool mostrarFitxaHUD = false;      // Fitxa astronomica HUD
    bool mostrarDialegQuantA = false;  // Dialeg Quant a
    bool recalculEfemeridesNecessari = false;
    bool origenReferenciaSol = true;   // Sol vs Planeta com a origen
    int cosEnfocatId = 0;              // Cos celeste enfocat per defecte (0 = Sol)

    // Flags de sincronitzacio amb el motor OpenGL (main.cpp)
    bool configuracioInicialPendent = false;
    bool canviCameraPendent = false;

    EstatTempsSimulacio tempsSimulacio;
    OpcionsVisualitzacio opcionsVisualitzacio;
    CameraConfig cameraConfig;
    DadesCosAstronomic dadesCosActiu = CatalegAstronomic::GetCos(0);
};

// Classe principal d'interficie d'usuari Dear ImGui
class InterficieUI
{
public:
    // Inicialitzacio d'estils visuals (tema fosc espacial, arrodoniments)
    static void InicialitzarEstil();

    // Actualitza les dades astronomiques del HUD segons el cos seleccionat (0..10)
    static void ActualitzarDadesCosActiu(EstatSimulacio& estat);

    // Renderitzador principal que coordina tots els components
    static void Renderitzar(EstatSimulacio& estat, Escenari*& currentEscenari, Escenari* escenaris[2], GLuint& shader_programID);

    // 2.1 Selector Inicial de Modes (Modal Central amb 2 Quadrats)
    static void DibuixarSelectorInicialModes(EstatSimulacio& estat, Escenari*& currentEscenari, Escenari* escenaris[2], GLuint& shader_programID);

    // 2.2 Barra de Menu Superior Dinamica per Mode (Main Menu Bar)
    static void DibuixarBarraMenuSuperior(EstatSimulacio& estat, Escenari*& currentEscenari, Escenari* escenaris[2], GLuint& shader_programID);

    // 2.3 Selector de Data, Hora i Control de Timelapse (Cantonada Inferior Dreta)
    static void DibuixarPanellDataHoraTimelapse(EstatSimulacio& estat);

    // 2.6 Panell d'Informacio i Telemetria Astronomica (HUD Flotant)
    static void DibuixarFitxaAstronomicaHUD(const DadesCosAstronomic& dades, bool* pObert);

    // 2.7 Projeccio d'Etiquetes i Marcadors 3D a Pantalla 2D amb ImDrawList
    static void DibuixarEtiquetaPlaneta(const std::string& nom, const glm::vec3& posicioMon, 
                                       const glm::mat4& viewMatrix, const glm::mat4& projMatrix, 
                                       const glm::vec4& viewport);
    static void DibuixarMarcadorPlaneta(const std::string& nom, const glm::vec3& posicioMon,
                                        float radiCos, const glm::vec3& colorCos,
                                        bool esCosEnfocat, bool mostrarEtiqueta,
                                        const glm::mat4& viewMatrix, const glm::mat4& projMatrix,
                                        const glm::vec4& viewport);
};
