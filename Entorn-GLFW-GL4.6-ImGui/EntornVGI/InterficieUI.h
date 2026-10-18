// InterficieUI.h : Declaracio de tipus, estructures d'estat i metodes de la interficie d'usuari

#pragma once

#include <string>
#include <vector>
#include <ctime>
#include "framework.h"
#include "escenari.h"

// Modes de funcionament de la simulacio
enum class ModeSimulacio {
    INTUITIU,    // Mode Didactic / Intuitiu
    ESCALA_REAL  // Mode a Escala Real (Cientific)
};

// Modes de camera del simulador
enum class ModeCamera {
    ORBITAL_CENTRADOR,
    FIXA_INCLINADA_25,
    GLOBAL_SISTEMA,
    COET_PRIMERA_PERSONA
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

    void Validar() {
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
        acumulatSegons += (double)direccioTemps * factorVelocitatSegonsPerSegon * deltaSegons;
        int segonsPassats = (int)acumulatSegons;
        if (segonsPassats != 0) {
            acumulatSegons -= segonsPassats;
            AvancarSegonsInt(segonsPassats);
        }
    }

private:
    void AvancarSegonsInt(int s) {
        dataHora.segon += s;
        while (dataHora.segon >= 60) { dataHora.segon -= 60; dataHora.minut++; }
        while (dataHora.segon < 0)   { dataHora.segon += 60; dataHora.minut--; }
        while (dataHora.minut >= 60) { dataHora.minut -= 60; dataHora.hora++; }
        while (dataHora.minut < 0)   { dataHora.minut += 60; dataHora.hora--; }
        while (dataHora.hora >= 24)  { dataHora.hora -= 24; dataHora.dia++; }
        while (dataHora.hora < 0)    { dataHora.hora += 24; dataHora.dia--; }

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
    bool iluminacioSolarDirecta = true;  // Sol com a emissor directe
};

// Parametres de camera
struct CameraConfig {
    ModeCamera mode = ModeCamera::FIXA_INCLINADA_25;
    float angleInclinacio = 25.0f; // Inclinacio inicial de 25 graus
    float distZoom = 150.0f;
};

// Dades per a la fitxa astronomica HUD
struct CosAstronomicDades {
    std::string nom = "Terra";
    double radiKm = 6371.0;
    double distSolUA = 1.0;
    double gravetat = 9.81;
    double periodeRotacioHores = 23.93;
    double periodeOrbitalAnys = 1.0;
};

// Estat global de la interficie del simulador
struct EstatSimulacio {
    ModeSimulacio modeActual = ModeSimulacio::INTUITIU;
    bool mostrarSelectorModes = true;  // 2 quadrats centrals a l'inici (Selector Inicial)
    bool mostrarPanellTemps = false;   // Cantonada inferior dreta (mode escala)
    bool mostrarFitxaHUD = false;      // Fitxa astronomica HUD
    bool mostrarDialegQuantA = false;  // Dialeg Quant a
    bool recalculEfemeridesNecessari = false;
    bool origenReferenciaSol = true;    // Sol vs Planeta com a origen
    int cosEnfocatId = 0;              // Cos celeste enfocat per defecte (0 = Sol)

    EstatTempsSimulacio tempsSimulacio;
    OpcionsVisualitzacio opcionsVisualitzacio;
    CameraConfig cameraConfig;
    CosAstronomicDades dadesCosActiu;
};

// Classe principal d'interficie d'usuari Dear ImGui
class InterficieUI
{
public:
    // Inicialitzacio d'estils visuals (tema fosc espacial, arrodoniments)
    static void InicialitzarEstil();

    // Renderitzador principal que coordina tots els components
    static void Renderitzar(EstatSimulacio& estat, Escenari*& currentEscenari, Escenari escenaris[]);

    // 2.1 Selector Inicial de Modes (Modal Central amb 2 Quadrats)
    static void DibuixarSelectorInicialModes(EstatSimulacio& estat, Escenari*& currentEscenari, Escenari escenaris[]);

    // 2.2 Barra de Menu Superior Dinamica per Mode (Main Menu Bar)
    static void DibuixarBarraMenuSuperior(EstatSimulacio& estat, Escenari*& currentEscenari, Escenari escenaris[]);

    // 2.3 Selector de Data, Hora i Control de Timelapse (Cantonada Inferior Dreta)
    static void DibuixarPanellDataHoraTimelapse(EstatSimulacio& estat);

    // 2.6 Panell d'Informacio i Telemetria Astronomica (HUD Flotant)
    static void DibuixarFitxaAstronomicaHUD(const CosAstronomicDades& dades, bool* pObert);

    // 2.7 Projeccio d'Etiquetes 3D a Pantalla 2D amb ImDrawList
    static void DibuixarEtiquetaPlaneta(const std::string& nom, const glm::vec3& posicioMon, 
                                       const glm::mat4& viewMatrix, const glm::mat4& projMatrix, 
                                       const glm::vec4& viewport);
};
