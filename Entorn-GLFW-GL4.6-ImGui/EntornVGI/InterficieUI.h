// InterficieUI.h : Declaracio de tipus, estructures d'estat i metodes de la interficie d'usuari (Dear ImGui)
//
// Arquitectura general de la UI:
// - EstatSimulacio actua com a punt unic de veritat compartit entre ImGui (InterficieUI.cpp)
//   i el motor OpenGL/GLFW (main.cpp).
// - Els canvis fets des dels menus o botons activen flags de sincronitzacio
//   (configuracioInicialPendent, canviCameraPendent) que main.cpp processa a cada frame.

#pragma once

#include <string>
#include <vector>
#include <ctime>
#include "framework.h"
#include "escenari.h"

// Modes de funcionament de la simulacio (determinen quin Escenari i quins menus es mostren)
enum class ModeSimulacio {
    INTUITIU,    // Mode Didactic / Intuitiu: orbites circulars i radis/distancies ajustats visualment (escenaris[0])
    ESCALA_REAL  // Mode a Escala Real (Cientific): escenari realista amb control de data, hora i timelapse (escenaris[1])
};

// Modes de camera disponibles al simulador (mapejats a CAM_ESFERICA o CAM_NAVEGA dins main.cpp)
enum class ModeCamera {
    ORBITAL_CENTRADOR,      // Camera esferica lliure (CAM_ESFERICA) centrada sobre l'astre enfocant (cosEnfocatId)
    FIXA_INCLINADA_25,      // Camera esferica amb inclinacio vertical blocada a 25 graus (OPV.alfa = 25)
    GLOBAL_SISTEMA,         // Vista zenital allunyada (R = 550, alfa = 65) centrada al Sol per veure tot el sistema
    COET_PRIMERA_PERSONA    // Mode "Pilota coet" (CAM_NAVEGA): nau Milano pilotable amb teclat/ratoli i camera de seguiment
};

// Estructura per a dates i hores amb validacio automatica de calendari gregoria
struct DataHoraSimulacio {
    int any = 2026;
    int mes = 10;
    int dia = 18;
    int hora = 12;
    int minut = 0;
    int segon = 0;

    // Comprova si un any es de traspas segons el calendari gregoria
    static bool EsAnyDeTraspas(int a) {
        return (a % 4 == 0 && a % 100 != 0) || (a % 400 == 0);
    }

    // Retorna el nombre de dies que te el mes (1..12) per a l'any indicat
    static int DiesAlMes(int m, int a) {
        static const int dies[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
        if (m == 2 && EsAnyDeTraspas(a)) return 29;
        if (m >= 1 && m <= 12) return dies[m - 1];
        return 31;
    }

    // Retorna 0 (Dilluns) .. 6 (Diumenge) per a una data gregoriana (algoritme de Sakamoto adaptat a dilluns=0)
    static int DiaDeLaSetmana(int d, int m, int a) {
        static const int t[12] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };
        int y = a;
        if (m < 3) y -= 1;
        int dowSun0 = (y + y / 4 - y / 100 + y / 400 + t[m - 1] + d) % 7;
        if (dowSun0 < 0) dowSun0 += 7;
        return (dowSun0 + 6) % 7;
    }

    // Ajusta mes, dia, hora, minut i segon als seus rangs valids (respectant anys de traspas)
    void Validar() {
        if (mes < 1) mes = 1; if (mes > 12) mes = 12;
        int maxD = DiesAlMes(mes, any);
        if (dia < 1) dia = 1; if (dia > maxD) dia = maxD;
        if (hora < 0) hora = 0; if (hora > 23) hora = 23;
        if (minut < 0) minut = 0; if (minut > 59) minut = 59;
        if (segon < 0) segon = 0; if (segon > 59) segon = 59;
    }
};

// Control de temps i timelapse (Rang permès: 1 h/s [3.600 s/s] a 50 dies/s [4.320.000 s/s]; per defecte: 1 dia/s)
struct EstatTempsSimulacio {
    static constexpr double VELOCITAT_MIN_S_PER_S     = 3600.0;    // 1 hora/s
    static constexpr double VELOCITAT_DEFECTE_S_PER_S = 86400.0;   // 1 dia/s (valor inicial per defecte)
    static constexpr double VELOCITAT_MAX_S_PER_S     = 4320000.0; // 50 dies/s

    DataHoraSimulacio dataHora;
    bool enPausa = false;                                           // true atura l'avanç del rellotge i del moviment orbital
    int direccioTemps = 1;                                          // +1: Endavant, -1: Enrere (Rebobinar)
    double factorVelocitatSegonsPerSegon = VELOCITAT_DEFECTE_S_PER_S; // Per defecte: 1 dia/s (86.400 s/s)
    double acumulatSegons = 0.0;                                    // Fracció de segons pendents d'acumular entre frames

    // Sincronitza la data/hora simulada amb el rellotge local del sistema operatiu
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

    // Situa la simulació a l'època astronòmica estàndard J2000 (1 de gener de 2000 a les 12:00:00)
    void EstablirJ2000() {
        dataHora = { 2000, 1, 1, 12, 0, 0 };
    }

    // Duplica la velocitat de simulació fins al límit superior de 50 dies/s
    void AugmentarVelocitat() {
        factorVelocitatSegonsPerSegon *= 2.0;
        if (factorVelocitatSegonsPerSegon > VELOCITAT_MAX_S_PER_S) factorVelocitatSegonsPerSegon = VELOCITAT_MAX_S_PER_S;
    }

    // Redueix a la meitat la velocitat de simulació fins al límit inferior d'1 h/s
    void ReduirVelocitat() {
        factorVelocitatSegonsPerSegon *= 0.5;
        if (factorVelocitatSegonsPerSegon < VELOCITAT_MIN_S_PER_S) factorVelocitatSegonsPerSegon = VELOCITAT_MIN_S_PER_S;
    }

    // Acumula el temps transcorregut del frame (deltaSegons) multiplicat pel timelapse i aplica els segons sencers
    void AvancarSegons(double deltaSegons) {
        if (enPausa) return;
        if (deltaSegons > 0.25) deltaSegons = 0.25; // Topall de seguretat davant pauses del sistema
        acumulatSegons += (double)direccioTemps * factorVelocitatSegonsPerSegon * deltaSegons;
        long long segonsPassats = (long long)acumulatSegons;
        if (segonsPassats != 0) {
            acumulatSegons -= (double)segonsPassats;
            AvancarSegonsInt(segonsPassats);
        }
    }

private:
    // Suma o resta un nombre enter de segons propagant l'arrossegament cap a minuts, hores, dies, mesos i anys
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

// Capes de renderitzat i opcions de visualitzacio commutables des del menu "Visualitzacio"
struct OpcionsVisualitzacio {
    bool mostrarOrbites = true;          // Dibuixa les trajectories orbitals (consumit a main.cpp -> dibuixa_EscenaGL)
    bool mostrarEtiquetes = true;        // Projecta els noms 2D sobre els cossos (consumit a main.cpp -> draw_Menu_ImGui)
    bool mostrarMarcadorsPetits = true;  // Indicadors per a planetes petits en escales grans
    bool mostrarSkybox = true;           // Activa/desactiva el cubemap d'estrelles (sincronitzat amb SkyBoxCube)
    bool mostrarEixos = false;           // Activa/desactiva els eixos XYZ del mon (sincronitzat amb eixos)
    bool mostrarCinturoAsteroides = true;
    bool mostrarPuntsLagrange = false;
    bool anellsSaturnParticules = false; // false: disc pla amb textura, true: particules instanciades
    bool iluminacioSolarDirecta = true;  // Sol com a emissor directe + Llum #0 activa (sw_material[0] i llumGL[0].encesa)
    bool vistaClaraAmbient = false;      // Vista clara (llum ambient global sense ombres fortes, sw_material[1])
};

// Parametres de configuracio activa de la camera
struct CameraConfig {
    ModeCamera mode = ModeCamera::FIXA_INCLINADA_25;
    float angleInclinacio = 25.0f; // Inclinacio inicial de 25 graus (OPV.alfa)
    float distZoom = 100.0f;
};

// Dades fisiques de l'astre seleccionat per mostrar a la Fitxa Astronomica (HUD)
struct CosAstronomicDades {
    std::string nom = "Sol";
    double radiKm = 696340.0;
    double distSolUA = 0.0;
    double gravetat = 274.0;
    double periodeRotacioHores = 609.12;
    double periodeOrbitalAnys = 0.0;
    std::string tipusEtiqueta = "Estrella";          // Categoria legible per a la Fitxa HUD
    std::string distTextPersonalitzat = "";          // Text alternatiu (p. ex. satel-lits o naus)
};

// Estat global de la interficie del simulador (instancia unica g_EstatSimulador a main.cpp)
struct EstatSimulacio {
    ModeSimulacio modeActual = ModeSimulacio::INTUITIU; // Mode de simulació actiu (INTUITIU o ESCALA_REAL)
    bool mostrarSelectorModes = true;  // Quan és true, mostra el modal inicial amb 2 targetes i bloqueja l'escena 3D
    bool mostrarPanellTemps = true;    // Mostra el pop-up de control temporal (cantonada inferior dreta en ambdós modes)
    bool mostrarFitxaHUD = false;      // Mostra la fitxa flotant amb dades físiques de l'astre enfocat
    bool mostrarDialegQuantA = false;  // Mostra el diàleg informatiu "Quant al Simulador"
    bool recalculEfemeridesNecessari = false; // Flag per notificar canvi manual de data/hora a futurs càlculs d'efemèrides
    bool origenReferenciaSol = true;   // true: origen al Sol; false: centrat en el planeta seleccionat
    CelestialBodyId cosEnfocatId = CB_SUN;              // Identificador de l'astre enfocat (Sol, planetes, satèl·lits...)

    // Flags de sincronització amb el motor OpenGL (llegits i netejats a SincronitzarEstatSimuladorAmbOpenGL de main.cpp)
    bool configuracioInicialPendent = false; // Només al sortir del selector inicial (configura shaders, llums, skybox)
    bool canviModeDirectePendent = false;    // Activat pel botó "Canviar" o menú ràpid: commuta escenari conservant vista
    bool canviCameraPendent = false;         // Activat quan l'usuari canvia manualment d'astre o mode de càmera

    // Snapshot de la vista abans d'un canvi directe de mode per conservar exactament l'enquadrament
    struct EstatVistaTransicio {
        bool valid = false;
        CelestialBodyId cosAnterior = CB_SUN;
        ModeCamera modeCameraAnterior = ModeCamera::FIXA_INCLINADA_25;
        double radiCosAnterior = 1.0;       // Escala del cos enfocat a l'escenari d'origen
        double distAbsoluta = 100.0;        // OPV.R abans del canvi
        double ratioZoomRelatiu = 4.5;      // OPV.R / radiCosAnterior (clau per comparar escales diferents)
        double alfa = 25.0;                 // Angle vertical de camera esferica
        double beta = 0.0;                  // Angle horitzontal de camera esferica
        double radiSistemaAnterior = 210.0; // Per escalar proporcionalment en GLOBAL_SISTEMA o COET
    };
    EstatVistaTransicio transicioVista;

    EstatTempsSimulacio tempsSimulacio;
    OpcionsVisualitzacio opcionsVisualitzacio;
    CameraConfig cameraConfig;
    CosAstronomicDades dadesCosActiu;
};

// Classe principal d'interficie d'usuari Dear ImGui (metodes estatiques sense estat propi; tot l'estat viu a EstatSimulacio)
class InterficieUI
{
public:
    // Inicialitza l'estil visual (tema fosc espacial, arrodoniments) i carrega les fonts sans-serif amb suport catala
    static void InicialitzarEstil();

    // Actualitza estat.dadesCosActiu a partir del cataleg intern (amb fallback segur des d'Escenari)
    static void ActualitzarDadesCosActiu(EstatSimulacio& estat, const Escenari* escenari = nullptr);

    // Commuta directament entre INTUITIU i ESCALA_REAL conservant vista, camera, focus i temps
    static void SolicitarCanviModeDirecte(EstatSimulacio& estat, ModeSimulacio modeDesti);

    // Coordina i dibuixa tots els components actius de la UI a cada frame (cridada des de draw_Menu_ImGui a main.cpp)
    static void Renderitzar(EstatSimulacio& estat, Escenari*& currentEscenari, Escenari escenaris[], GLuint& shader_programID, GLuint& shader_programIDNormalMap);

    // 2.1 Selector Inicial de Modes: modal central responsive amb 2 targetes (Mode Didactic i Mode a Escala Real)
    static void DibuixarSelectorInicialModes(EstatSimulacio& estat, Escenari*& currentEscenari, Escenari escenaris[], GLuint& shader_programID, GLuint& shader_programIDNormalMap);

    // 2.2 Barra de Menu Superior Dinamica: construeix els menus segons estat.modeActual i la zona dreta (estat, pausa, Canviar, FPS)
    static void DibuixarBarraMenuSuperior(EstatSimulacio& estat, Escenari*& currentEscenari, Escenari escenaris[], GLuint& shader_programID, GLuint& shader_programIDNormalMap);

    // 2.3 Panell de Control de Temps i Velocitat (cantonada inferior dreta, disponible en ambdós modes)
    static void DibuixarPanellDataHoraTimelapse(EstatSimulacio& estat);

    // 2.6 Panell d'Informacio i Telemetria Astronomica (HUD flotant a la cantonada superior dreta)
    static void DibuixarFitxaAstronomicaHUD(const CosAstronomicDades& dades, bool* pObert);

    // 2.7 Projeccio d'Etiquetes 3D a Pantalla 2D amb glm::project i ImDrawList (dibuixada sobre el fons del viewport)
    static void DibuixarEtiquetaPlaneta(const std::string& nom, const glm::vec3& posicioMon, 
                                       const glm::mat4& viewMatrix, const glm::mat4& projMatrix, 
                                       const glm::vec4& viewport);
};
