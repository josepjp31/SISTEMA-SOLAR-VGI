# Referència Tècnica Completa de Fitxers, Declaracions, Funcions i Shaders

> **Directori font principal:** `Entorn-GLFW-GL4.6-ImGui/EntornVGI/`  
> **Document complementari:** Per a la visió arquitectònica, diagrames de flux, guia de càmeres/controls i receptari «On tocar...?», consulteu la [Guia de l'Entorn i Arquitectura](./GUIA_ARQUITECTURA_ENTORN.md).

---

## Índex

1. [Cercador ràpid per funcionalitat](#1-cercador-ràpid-per-funcionalitat)
2. [Nucli i configuració global (`constants.h`, `framework.h`, `main.h`, `main.cpp`)](#2-nucli-i-configuració-global)
3. [Simulació del Sistema Solar i efemèrides (`escenari`, `CelestialBodyData`)](#3-simulació-del-sistema-solar-i-efemèrides)
4. [Escena, visualització, càmeres i materials (`escena`, `visualitzacio`, `material`)](#4-escena-visualització-càmeres-i-materials)
5. [Interfície d'usuari (`InterficieUI`)](#5-interfície-dusuari-interficieui)
6. [Recursos, models 3D, shaders, geometria i quaternions (`GestorRecursos`, `objLoader`, `shader`, `glut_geometry`, `quatern`)](#6-recursos-models-3d-shaders-geometria-i-quaternions)
7. [Referència completa dels 7 programes de shaders GLSL (`shaders/`)](#7-referència-completa-dels-7-programes-de-shaders-glsl)
8. [Referència de kernels SPICE (`cbdata/`) i textures (`textures/`, `docs/planetes/`)](#8-referència-de-kernels-spice-i-textures)

---

## 1. Cercador ràpid per funcionalitat

| Què busques? | Fitxer | Símbol(s) clau |
|---|---|---|
| Bucle principal, inicialització GLFW/GLEW/ImGui | `main.cpp` | `main()` (`L1890`), `InitGL()` (`L41`), `OnPaint()` (`L838`) |
| Sincronització UI ↔ OpenGL / canvi de mode directe | `main.cpp` | `SincronitzarEstatSimuladorAmbOpenGL()` (`L510`) |
| Conversió data gregoriana → segons J2000 | `main.cpp` | `toJ2000Seconds()` (`L1869`) |
| Callbacks de teclat, ratolí, roda, resize, nau | `main.cpp` | `OnKeyDown` (`L1200`), `Teclat_Navega` (`L1306`), `OnMouseButton` (`L1365`), `OnMouseMove` (`L1429`), `OnMouseWheel` (`L1687`), `OnSize` (`L353`) |
| Límits de zoom i enfocament per astre | `main.cpp` | `ObtenirDadesEnfocament` (`L454`), `CalcularLimitsCameraPerCos` (`L478`) |
| Estat compartit de tota l'aplicació | `InterficieUI.h` / `main.cpp` | `struct EstatSimulacio` (`InterficieUI.h:202`), `g_EstatSimulador` (`main.cpp:33`) |
| Rellotge, calendari gregorià i timelapse | `InterficieUI.h` / `.cpp` | `DataHoraSimulacio` (`.h:32`), `EstatTempsSimulacio` (`.h:75`), `DibuixarPanellDataHoraTimelapse` (`.cpp:1530`) |
| Menús superiors, selector de modes, HUD, etiquetes | `InterficieUI.cpp` | `Renderitzar` (`L411`), `DibuixarSelectorInicialModes` (`L454`), `DibuixarBarraMenuSuperior` (`L864`), `DibuixarFitxaAstronomicaHUD` (`L1961`), `DibuixarEtiquetaPlaneta` (`L2103`) |
| Catàleg de dades educatives dels 16 cossos | `InterficieUI.cpp` | `kCatalegCossos` (`L50-67`), `ActualitzarDadesCosActiu` (`L83`) |
| Graf d'escena del Sistema Solar (planetes, òrbites, anells) | `escenari.h` / `.cpp` | `Object`, `EsferaInt`, `AnellInt`, `Escenari::creaEscenari` (`L318`), `dibuixaEscenari` (`L417`), `updateVariables` (`L488`), `creaOrbita` (`L295`) |
| IDs NAIF i càlculs d'efemèrides NASA CSPICE | `CelestialBodyData.h` / `.cpp` | `enum CelestialBodyId`, `cbList`, `loadCspiceKernels`, `getCBstate`, `getCBposMat`, `getCBrotMat`, `getOrbitStates` |
| Dibuix de Skybox, eixos i nau Milano | `escena.cpp` | `dibuixa_Eixos` (`L22`), `dibuixa_Skybox` (`L41`), `dibuixa_EscenaGL` (`L81`) |
| Projecció perspectiva, càmeres i llums OpenGL | `visualitzacio.cpp` | `Projeccio_Perspectiva` (`L318`), `Vista_Esferica` (`L339`), `Vista_Navega` (`L418`), `Iluminacio` (`L25`) |
| Cache compartit de textures, Skybox, VAOs i uniforms | `GestorRecursos.h` / `.cpp` | `GestorRecursos`, `CacheUniformsShader`, `ParametresIluminacioEscenari` |
| Carregador `.obj` / `.mtl` / `.mbin` de la nau Milano | `objLoader.h` / `.cpp` | `COBJModel`, `OBJPreloadData`, `LoadToCPU`, `UploadFromCPU`, `draw_TriVAO_OBJ` |
| Esfera amb tangents, disc d'anell i registre `VAOList` | `glut_geometry.h` / `.cpp` | `loadgluSphere_EBO`, `loadgluDisk_EBO`, `loadgluPartialDisk_EBO`, `load_TRIANGLES_EBOver2`, `draw_TriEBO_Object` |
| Compilació i gestió de shaders GLSL | `shader.h` / `.cpp` | `Shader::loadFileShaders`, `Shader::DeleteProgram`, setters `set*` |

---

## 2. Nucli i configuració global

### 2.1 `constants.h` (218 línies)

Capçalera de constants, macros i estructures base compartides per tot el motor (guard `CONST_H`, L8–9).

#### Macros i constants
- **Càmeres** (L12–15): `CAP` (`' '`), `CAM_ESFERICA` (`'E'`), `CAM_GEODE` (`'G'`), `CAM_NAVEGA` (`'N'`).
- **Projeccions** (L18–22): `AXONOM` (`'A'`), `ORTO` (`'O'`), `PERSPECT` (`'P'`), `IMA_PICK` (`3`), `PASSEIG_V` (`4`).
- **Polars** (L26–28): `POLARZ` (`'Z'`), `POLARY` (`'Y'`), `POLARX` (`'X'`).
- **Objectes** (L31–58): llegat EntornVGI (`ARC`, `CAMIO`, `CILINDRE`, `CUB`, `CUB_RGB`, corbes `C_BEZIER`/`C_LEMNISCATA`/`C_LEMNISCATA2D`/`C_BSPLINE`/`C_HERMITTE`/`C_CATMULL_ROM`, `ESFERA`, `O_FRACTAL`, `HIDROAVIO`, `ICOSAEDRE`, `MATRIUP`, `MATRIUP_VAO`, `ROBOT`, `TETERA`, `TEXTE_BITMAP`, `TEXTE_STROKE`, `TIE`, `TORUS`, `VAIXELL`, `OBJ3DS`, `OBJOBJ`) + **`SISTEMA_SOLAR` (`'$'`, L58)**.
- **Il·luminació** (L61–64): `PUNTS` (`'P'`), `FILFERROS` (`'w'`), `PLANA` (`'f'`), `SUAU` (`'s'`).
- **Frustum** (L68–69): `const double p_near = 0.01;`, `const double p_far = 50000.0;`.
- **Picking (llegat)** (L72–75): `PICKCAP` (`0`), `PICKFAR` (`1`), `PICKVAIXELL` (`2`), `PICKHIDRO` (`3`).
- **Textures** (L79–91): `NUM_MAX_TEXTURES` (`15`), `TEXTURA_BRICKS`..`TEXTURA_FITXERIMA` (`'I'`).
- **Llums** (L94): `const int NUM_MAX_LLUMS = 8;`.
- **Shaders** (L97–104): `CAP_SHADER` (`' '`), `FLAT_SHADER` (`'f'`), `GOURAUD_SHADER` (`'G'`), `PHONG_SHADER` (`'P'`), **`NORMAL_MAP_SHADER` (`'M'`, L101)**, `FILE_SHADER` (`'F'`), `PROG_BINARY_SHADER` (`'p'`), `PROG_BINARY_SHADERW` (`'W'`).
- **Matemàtiques, Grid i Timer** (L107–117): `PI` (`3.14159`), `TWOPI`, `PID2`, `GRID_SIZE` (`50`), `PAS_GRID` (`1`), `elapsedTime` (`0.004f`).

#### Estructures (`struct`)
| Struct | Línies | Camps |
|---|---:|---|
| `CSize` | 120–124 | `GLint cx, cy;` |
| `CPoint` | 127–131 | `GLint x, y;` |
| `CPunt3D` | 134–140 | `GLdouble x, y, z, w;` |
| `CMask3D` | 143–149 | `bool x, y, z, w;` |
| `CColor` | 152–158 | `GLdouble r, g, b, a;` |
| `CEsfe3D` | 161–166 | `GLdouble R, alfa, beta;` |
| `CVAO` | 169–176 | `GLuint vaoId, vboId, eboId; GLint nVertexs, nIndices;` |
| `INSTANCIA` | 179–184 | `CPunt3D VTras, VScal, VRota;` |
| `CAtenua` | 187–192 | `GLdouble a, b, c;` |
| `MATERIAL` | 195–202 | `GLfloat ambient[4], diffuse[4], specular[4], emission[4], shininess;` |
| `LLUM` | 205–216 | `bool encesa; CPunt3D posicio; CColor difusa, especular; CAtenua atenuacio; bool restringida; CPunt3D spotdirection; GLfloat spotcoscutoff, spotexponent;` |

---

### 2.2 `framework.h` (58 línies)

Capçalera comuna (`#pragma once`, L5) que centralitza:
- Macros: `VC_EXTRALEAN` (L8), `GLEW_NO_GLU` (L16).
- Includes: `<gl/glew.h>`, `<gl/wglew.h>`, `<GLFW/glfw3.h>`, `<glm/glm.hpp>`, `<glm/ext.hpp>`, `<glm/gtx/quaternion.hpp>`, `<glm/gtx/euler_angles.hpp>`, `"glut_geometry.h"`, `"gl/SOIL2.h"`, `<stdio.h>`, `<stdlib.h>`, `<math.h>`, `<time.h>`, `<vector>`, `<string>`, `"constants.h"`.
- `using namespace glm;` (L32) i desactivació de warnings MSVC `4244, 4305, 4473, 4099, 4996` (L54–58).

---

### 2.3 `main.h` (188 línies) i `main.cpp` (2159 línies)

#### Variables globals principals
- **Finestra / GLFW (`main.h`)**: `primary` (L16), `mode` (L19), `window` (L22), `fullscreen` (L47), `w, h` (L132), `width_old, height_old` (L133).
- **Càmera i Projecció (`main.h`)**: `camera` (L29), `mobil`, `zzoom`, `zzoomO`, `satelit` (L30–33), `cam_Esferica[3]` (L36), `OPV` (L37), `Vis_Polar` (L38), `ViewMatrix` (L39), `n[3]`, `opvN`, `angleZ` (L42–44), `pan`, `fact_pan`, `tr_cpv`, `tr_cpvF` (L48, L57–59), `projeccio` (L63), `ProjectionMatrix` (L64), `pas`, `mida` (L142–143).
- **Shaders (`main.h`)**: `shaderEixos`/`eixos_programID` (L50–51), `shader_SkyBoxC`/`skC_programID`/`shaderSkyBox` (L69–73), `shader` (L113), `shader_programID` (L114), `instancing_programID` (L115), `shader_programIDNormalMap` (L116), `shaderLightingNormalMap` (L117), `shaderLighting` (L118).
- **Llums, Materials, Textures i Visibilitat (`main.h`)**: `front_faces`, `test_vis`, `oculta`, `back_line` (L91–94), `ifixe`, `ilum2sides`, `ilumina` (L97–100), `sw_material[5]`, `sw_material_old[5]` (L101–103), `textura`, `t_textura`, `tFlag_invert_Y` (L104–106), `llum_ambient`, `llumGL[NUM_MAX_LLUMS]` (L109–110), `c_fons`, `clear_colorB`, `clear_colorO`, `col_obj` (L136–139).
- **Escena, Ratolí i Objectes (`main.h`)**: `eixos`, `eixos_Id`, `grid`, `hgrid` (L49, L52–54), `objecte` (L65), `SkyBoxCube`, `skC_VAOID`, `cubemapTexture` (L68, L71–72), `transf`, `trasl`, `rota`, `escal`, `fact_Tras`, `fact_Rota`, `TG`, `TGF`, `transX/Y/Z`, `GTMatrix` (L76–88), `ObOBJ` (L121), `m_PosEAvall`, `m_PosDAvall`, `m_ButoEAvall`, `m_ButoDAvall`, `m_EsfeEAvall`, `m_EsfeIncEAvall` (L124–129), `show_demo_window`, `show_another_window` (L25–26).
- **Estat del Simulador i Coet (`main.cpp`)**:
  - `Escenari* escenariActual = nullptr;` (L28), `Escenari* escenaris = nullptr;` (L29).
  - `EstatSimulacio g_EstatSimulador;` (L33).
  - `glm::vec3 posMilano(120.0f, 0.0f, 0.0f);` (L36), `float angleMilano = 0.0f;` (L37), `float velocitatMilano = 0.5f;` (L38, sense ús), `float inclinacioMilano = 0.0f;` (L39).
  - Precàrrega async coet (`static`, L381–386): `g_coetPreloadThread`, `g_coetPreloadData`, `g_coetPreloadStarted`, `g_coetPreloadDone`, `g_coetPreloadSuccess`, `kMilanoObjPath = "textures/coet/milano_gotg.obj"`.
  - Struct local `LimitsCameraCos` (`main.cpp:471-476`): `{ double distDefecte, minR, maxR, pasZoom; }`.
  - Macro `#define glCheckError() glCheckError_(__FILE__, __LINE__)` (`main.cpp:1824`).

#### Inventari complet de funcions (32 funcions)

| Funció | Signatura exacta | `.h` | `.cpp` | Descripció |
|---|---|---:|---:|---|
| `InitGL` | `void InitGL()` | 149 | 41–329 | Inicialitza globals OpenGL/EntornVGI, `initVAOList()`, compila `eixos`, `skybox`, `phong_shdrML` i `normal_mapping`, i crea el VAO del cub Skybox. |
| `GetGLVersion` | `void GetGLVersion(int* major, int* minor)` | 151 | 332–351 | Llegeix `GL_MAJOR_VERSION` i `GL_MINOR_VERSION` via OpenGL. |
| `OnSize` | `void OnSize(GLFWwindow* window, int width, int height)` | 154 | 353–365 | Callback GLFW de redimensionament; actualitza `w` i `h`. |
| `OnShaderNormalMap` | `void OnShaderNormalMap()` | — | 369–376 | Elimina i recarrega `.\shaders\normal_mapping.{vert,frag}` a `shader_programIDNormalMap`. |
| `IniciarPrecarregaCoet` | `static void IniciarPrecarregaCoet()` | — | 389–399 | Llança `g_coetPreloadThread` per executar `COBJModel::LoadToCPU(kMilanoObjPath, g_coetPreloadData)`. |
| `FinalitzarCarregaCoetGPU` | `static void FinalitzarCarregaCoetGPU(bool esperarSiCal)` | — | 403–444 | Instancia `ObOBJ` i puja geometria/textura de la Milano a GPU (`UploadFromCPU`, o fallback `LoadModel`). |
| `AssegurarCoetCarregat` | `static void AssegurarCoetCarregat()` | — | 447–450 | Garanteix càrrega síncrona de la Milano (`FinalitzarCarregaCoetGPU(true)`). |
| `ObtenirDadesEnfocament` | `static bool ObtenirDadesEnfocament(const Escenari* escenari, CelestialBodyId id, glm::vec3& outPosicio, double& outRadi, std::string* outNom = nullptr)` | — | 454–467 | Consulta posició (`m_trMatrix[3]`), radi (`m_scale`) i nom d'un objecte enfocable a `escenari`. |
| `CalcularLimitsCameraPerCos` | `static LimitsCameraCos CalcularLimitsCameraPerCos(CelestialBodyId id, double radiCos)` | — | 478–496 | Calcula `distDefecte`, `minR`, `maxR` i `pasZoom` proporcionals a `radiCos` (diferenciant `CB_SUN`). |
| `LimitarRang` | `static double LimitarRang(double valor, double minim, double maxim)` | — | 498–501 | Acota `valor` a `[minim, maxim]`. |
| `SincronitzarEstatSimuladorAmbOpenGL` | `void SincronitzarEstatSimuladorAmbOpenGL()` | — | 510–835 | Pont per frame UI ↔ OpenGL: resol `configuracioInicialPendent`, `canviModeDirectePendent` (conservant zoom relatiu), `recalculEfemeridesNecessari`, opcions de visualització i `canviCameraPendent`. |
| `OnPaint` | `void OnPaint(GLFWwindow* window)` | 156 | 838–964 | Renderitzat 3D per frame: neteja si hi ha selector de modes; sinó sincronitza estat, calcula `ProjectionMatrix`, `ViewMatrix` (centrada en cos o coet) i crida `configura_Escena()` + `dibuixa_Escena()`. |
| `configura_Escena` | `void configura_Escena()` | 157 | 968–972 | Calcula `GTMatrix = instancia(transf, TG, TGF)`. |
| `dibuixa_Escena` | `void dibuixa_Escena()` | 158 | 975–1004 | Crida `dibuixa_Skybox`, `dibuixa_Eixos`, empaqueta `ParametresIluminacioEscenari` i crida `dibuixa_EscenaGL`. |
| `draw_Menu_ImGui` | `void draw_Menu_ImGui()` | 161 | 1012–1083 | Construeix frame ImGui (`InterficieUI::Renderitzar`), projecta etiquetes 2D (ocultant satèl·lits a `<24 px` del pare) i crida `ImGui::Render()`. |
| `OnCameraNavega` | `void OnCameraNavega()` | 164 | 1086–1126 | Activa `CAM_NAVEGA`, carrega la nau Milano (`AssegurarCoetCarregat`) i reinicia paràmetres de navegació. |
| `OnVistaSkyBox` | `void OnVistaSkyBox()` | 165 | 1129–1145 | Assegura `skC_programID`, `skC_VAOID` (`CUBE_SKYBOX`) i `cubemapTexture` (`GestorRecursos::ObtenirTexturaSkybox`). |
| `OnProjeccioPerspectiva` | `void OnProjeccioPerspectiva()` | 166 | 1152–1157 | Activa `PERSPECT`, `mobil = true`, `zzoom = true`, `zzoomO = false`. |
| `OnShaderPhong` | `void OnShaderPhong()` | 167 | 1160–1183 | Recarrega `.\shaders\phong_shdrML.{vert,frag}` i activa `PHONG_SHADER` / `SUAU`. |
| `OnKeyDown` | `void OnKeyDown(GLFWwindow* window, int key, int scancode, int action, int mods)` | 170 | 1200–1297 | Callback GLFW de teclat: `Esc`, dreceres (`Space`, `Backspace`, `R`, `Home`, `Tab`), fletxes en `CAM_ESFERICA` i delegació a `Teclat_Navega` en `CAM_NAVEGA`. |
| `OnKeyUp` | `void OnKeyUp(GLFWwindow* window, int key, int scancode, int action, int mods)` | 171 | 1299–1300 | Stub buit (no registrat). |
| `OnTextDown` | `void OnTextDown(GLFWwindow* window, unsigned int codepoint)` | 172 | 1302–1303 | Stub buit (registre comentat a L2008). |
| `Teclat_Navega` | `void Teclat_Navega(int key, int action)` | 175 | 1306–1354 | Pilotatge de la Milano (`W/S/↑/↓` avanç 3D, `A/D/←/→` yaw `±5°`, `Q/E` alçada Z `±2.0`). |
| `OnMouseButton` | `void OnMouseButton(GLFWwindow* window, int button, int action, int mods)` | 178 | 1365–1420 | Callback de botons del ratolí; actualitza `m_ButoEAvall`/`m_ButoDAvall` i posicions d'inici d'arrossegament. |
| `OnMouseMove` | `void OnMouseMove(GLFWwindow* window, double xpos, double ypos)` | 179 | 1429–1680 | Callback de moviment de cursor: òrbita esfèrica (bloqueig `alfa=25°` en `FIXA_INCLINADA_25`), orientació Milano (`angleMilano`/`inclinacioMilano`), zoom dret, pan i TG. |
| `OnMouseWheel` | `void OnMouseWheel(GLFWwindow* window, double xoffset, double yoffset)` | 180 | 1687–1759 | Callback de roda: zoom proporcional a l'astre en `CAM_ESFERICA` o avanç llegat en `CAM_NAVEGA`. |
| `OnFull_Screen` | `void OnFull_Screen(GLFWmonitor* monitor, GLFWwindow* window)` | 183 | 1764–1786 | Commuta pantalla completa via `glfwSetWindowMonitor` (no invocada). |
| `error_callback` | `void error_callback(int code, const char* description)` | 185 | 1790–1798 | Imprimeix errors GLFW a `stderr`. |
| `glCheckError_` | `GLenum glCheckError_(const char* file, int line)` | 186 | 1801–1823 | Drena `glGetError()` i registra errors OpenGL a `stderr`. |
| `glDebugOutput` | `void APIENTRY glDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)` | 187 | 1826–1866 | Callback de depuració síncrona OpenGL 4.6 (filtra notificacions informatives). |
| `toJ2000Seconds` | `double toJ2000Seconds(int year, int month, int day, int hour = 0, int minute = 0, double second = 0.0)` | — *(fwd L378)* | 1869–1887 | Converteix data/hora gregoriana a segons des de J2000.0 (`JD 2451545.0`, offset `-2453069.5`). |
| `main` | `int main()` | — | 1890–2149 | Inicialització completa, bucle principal i alliberament ordenat de recursos. |

---

## 3. Simulació del Sistema Solar i efemèrides

### 3.1 `escenari.h` (276 línies) i `escenari.cpp` (522 línies)

#### Tipus i classes declarats a `escenari.h`
- **`enum class CategoriaObjecte`** (L11–18): `ESTRELLA`, `PLANETA`, `PLANETA_NAN`, `SATELLIT_NATURAL`, `ASTRE_ARTIFICIAL`, `ORNAMENTAL`.
- **`struct ContextRenderEscenari`** (L21–38): `shaderPhongID`, `shaderNormalMapID`, `shaderActualID`, `mostrarOrbites`, `iluminacioSolar`, `vistaClara`, `sw_mat[5]`, `matriuVista`, `cachePhong`, `cacheNormalMap`, i mètode inline `cacheActual()` (L33–37).
- **`class Object`** (L41–132): node base abstracte del graf d'escena.
  - Camps: `m_textureID`, `m_normalMapID`, `m_propietariTextures`, `m_trMatrix`, `m_scale`, `m_nom`, `m_id`, `m_categoria`, `m_enfocable`, `m_mostrarEtiqueta`, `m_ordreMenu`, `m_sons`, `m_parent`, `m_sw_mat[5]`.
- **`class EsferaInt : public Object`** (L134–169):
  - Camps: `m_radiCos` (ref. a `m_scale`), `m_radiOrbita`, `m_angleOrbitaInicial`, `m_angleOrbita`, `m_velAngular`, `m_angleRotacioInicial`, `m_angleRotacio`, `m_velRotacio`, `m_inclinacioEix`, `m_inclinacioOrbita`.
- **`class AnellInt : public Object`** (L171–218):
  - Camps: `m_radiExtern` (ref. a `m_scale`), `m_radiIntern`, `m_inclinacioEix`, `CVAO m_vao`.
- **`class Escenari`** (L221–276):
  - Camps: `bool m_inicialitzat`, `std::unordered_map<CelestialBodyId, Object*> m_objectList`.

#### Funcions i mètodes complets

| Classe / Àmbit | Signatura exacta | `.h` | `.cpp` | Descripció |
|---|---|---:|---:|---|
| `ContextRenderEscenari` | `CacheUniformsShader* cacheActual() const` | 33–37 | inline | Retorna `cacheNormalMap` si està actiu el shader de normal map; sinó `cachePhong`. |
| `Object` | `Object()` / `Object(const std::string& nom, double scale)` | 44–45 | inline | Constructors (còpia/assignació `= delete` a L46–47). |
| `Object` | `virtual ~Object()` | 48–54 | inline | Allibera textures pròpies (`unloadTexture`/`unloadNormMap`) i elimina recursivament `m_sons`. |
| `Object` | `void unloadTexture()` / `void unloadNormMap()` | 57–58 | inline | Només crida `glDeleteTextures` si `m_propietariTextures == true`. |
| `Object` | `GLuint loadTexture(const char* filepath)` / `GLuint loadNormMap(const char* filepath)` | 59–92 | inline | Càrrega individual amb `loadIMA_SOIL` (`m_propietariTextures = true`, anisotropia `8x`). |
| `Object` | `bool addSon(Object* s)` | 93–100 | inline | Enllaça fill `s` a `m_sons` i assigna `s->m_parent = this`. |
| `Object` | `virtual void drawObject(ContextRenderEscenari& ctx, const glm::mat4& matriuPare) = 0` | 102 | — | Dibuix polimòrfic del node. |
| `Object` | `virtual void update(double tempsJ2000, const glm::mat4& matriuPare)` | 106–110 | inline | Desa `m_trMatrix = matriuPare` i propaga als fills. |
| `EsferaInt` | `EsferaInt(const std::string& nom, double radiCos, double radiOrbita, double angleOrbita, double velAngular, double angleRotacio, double velRotacio, double inclinacioEix = 0.0, double inclinacioOrbita = 0.0)` | 152–162 | inline | Inicialitza paràmetres físics/cinemàtics en graus. |
| `EsferaInt` | `void drawObject(ContextRenderEscenari& ctx, const glm::mat4& matriuPare) override` | 166 | 74–177 | Dibuixa òrbita (`GLUT_TORUS`, `GL_LINE_LOOP`, `shaderPhongID`), renderitza esfera (`GLU_SPHERE`, `shaderNormalMapID`, `uNormalMatrixPV`, `GL_TEXTURE0/1`) i propaga als fills. |
| `EsferaInt` | `void update(double tempsJ2000, const glm::mat4& matriuPare) override` | 167 | 179–202 | Actualitza `m_angleOrbita` i `m_angleRotacio`, aplica inclinació orbital i calcula `m_trMatrix` abans de propagar als fills. |
| `AnellInt` | `AnellInt(const std::string& nom, double radiIntern, double radiExtern, double inclinacioEix = 0.0)` | 180–189 | inline | Crea disc `loadgluDisk_EBO(m_radiIntern, m_radiExtern, 80, 5)` com a `ORNAMENTAL` no enfocable. |
| `AnellInt` | `~AnellInt()` | 191–214 | inline | Allibera `vboId`, `eboId` i `vaoId` de `m_vao`. |
| `AnellInt` | `void update(double tempsJ2000, const glm::mat4& matriuPare) override` | 217 | 204–209 | Copia `m_trMatrix = matriuPare` i propaga als fills. |
| `AnellInt` | `void drawObject(ContextRenderEscenari& ctx, const glm::mat4& matriuPare) override` | 216 | 211–289 | Dibuixa l'anell amb `shaderPhongID`, `sw_twoSided = GL_TRUE`, `GL_BLEND` i `glDepthMask(GL_FALSE)`. |
| `Escenari` | `void creaEscenari(GLuint& shader_programID, GLuint& shaderNormalMapID)` | 232 | 318–413 | Configura samplers, assegura VAOs (`GestorRecursos`), crea els 10 astres + `AnellInt`, construeix l'arbre i vincula textures compartides. |
| `Escenari` | `void dibuixaEscenari(GLuint sh_programID, GLuint shaderNormalMapID, glm::mat4 MatriuVista, glm::mat4 MatriuTG, bool sw_mat[5], bool mostrarOrbites = true, const ParametresIluminacioEscenari* ilum = nullptr)` | 235 | 417–482 | Actualitza `s_cachePhong`/`s_cacheNormalMap`, sincronitza shader normal map des de CPU, recorre arrels i restaura estat GL. |
| `Escenari` | `void updateVariables(double et = 0, double dt = 0.016, bool enPausa = false, int direccioTemps = 1, bool escalaReal = false, double factorVelocitat = 60.0, bool forcarActualitzacio = false)` | 240 | 488–502 | Si `!enPausa || forcarActualitzacio`, executa `update(et, mat4(1.0f))` sobre les arrels. |
| `Escenari` | `bool esInicialitzat() const` | 242 | inline | Retorna `m_inicialitzat`. |
| `Escenari` | `Object* obtenirObjecte(CelestialBodyId id) const` | 245–248 | inline | Cerca segura a `m_objectList` sense inserir claus nul·les. |
| `Escenari` | `std::vector<const Object*> obtenirObjectesEnfocables() const` | 251 | 504–520 | Retorna tots els objectes enfocables ordenats per `m_ordreMenu` (desempat `m_nom`). |
| `Escenari` | `virtual double obtenirRadiReferenciaSistema() const` | 254 | inline | Retorna `210.0`. |
| `Escenari` | `void alliberaEscenari()` | 257–272 | inline | Destrueix totes les arrels (i fills) i reinicia `m_objectList` i `m_inicialitzat`. |
| *(static)* | `static void activarShaderSiCal(ContextRenderEscenari& ctx, GLuint shaderObjectiu)` | — | 8–15 | Evita `glUseProgram` redundants. |
| *(static)* | `static void aplicarMaterialCache(const CacheUniformsShader& cache, const bool sw_mater[5])` | — | 18–36 | Envia material blanc/negre i `sw_intensity`/`sw_material` usant ubicacions cacheades. |
| *(static)* | `static void SincronitzarShaderNormalMap(const CacheUniformsShader& cache, const glm::mat4& matriuVista, const ParametresIluminacioEscenari& ilum)` | — | 39–72 | Sincronitza `projectionMatrix`, `viewMatrix`, `normalMatrix`, ambient i 8 llums al shader `normal_mapping` sense `glGetUniform*`. |
| *(global)* | `CVAO creaOrbita()` | *(decl. `GestorRecursos.cpp:14`)* | 295–315 | Crea cercle unitari de 512 vèrtexs al pla XY amb `load_TRIANGLES_VAO` (dibuixat com `GL_LINE_LOOP`). |

---

### 3.2 `CelestialBodyData.h` (78 línies) i `CelestialBodyData.cpp` (191 línies)

- **`enum CelestialBodyId`** (`.h:5-43`):
  - `CB_SOLAR_SYSTEM_BARYCENTER = 0`, `CB_SUN = 10`
  - Planetes: `CB_MERCURY = 199`, `CB_VENUS = 299`, `CB_EARTH = 399`, `CB_MARS = 4`, `CB_JUPITER = 5`, `CB_SATURN = 6`, `CB_URANUS = 7`, `CB_NEPTUNE = 8`
  - Planetes nans: `CB_CERES = 2000001`, `CB_PLUTO = 999`, `CB_PLUTO_BARYCENTER = 9`
  - Satèl·lits: `CB_MOON = 301`, `CB_CHARON = 901` *(comentats: Phobos, Deimos, Io, Europa, Ganymede, Callisto, Titan, Enceladus, Triton, Nix..Styx)*
  - Naus/sentinelles: `CB_VOYAGER1 = -31`, `CB_VOYAGER2 = -32`, `CB_MILANO = -1000`, `CB_NONE = -99999`
- **Structs i globals**:
  - `struct CB` (`.h:45-52`): `std::string m_name, m_id; bool m_hasRotFrame; std::string m_rotationFrame; CelestialBodyId m_orbitedObject;`
  - `struct CState3D` (`.h:58-65`): `double x, y, z, vx, vy, vz;` + 4 constructors inline.
  - `const double KM_TO_AU = 1.0 / 149597870.7;` (`.h:54`)
  - `CelestialBodyId globalSystemOrigin = CB_SUN;` (`.h:55`, `.cpp:14`)
  - `std::unordered_map<CelestialBodyId, CB> cbList` (`.h:56`, `.cpp:16-37`, 16 cossos; inclou marcs `IAU_*`; no inclou `CB_VOYAGER2` ni `CB_MILANO`).
  - `std::string kernels[]` (`.cpp:39-49`, 9 fitxers de `cbdata\`).

| Funció | Signatura exacta | `.h` | `.cpp` | Descripció |
|---|---|---:|---:|---|
| `loadCspiceKernels` | `void loadCspiceKernels()` | 77 | 51–57 | Carrega els 9 kernels amb `furnsh_c` i posa errors CSPICE en mode `RETURN` / `NONE`. *(No cridada encara).* |
| `unloadCspiceKernels` | `void unloadCspiceKernels()` | 78 | 59–62 | Crida `kclear_c()`. *(No cridada encara).* |
| `getCBstate` | `CState3D getCBstate(CelestialBodyId body, CelestialBodyId origin, double et)` | 69 | 64–87 | Consulta `spkezr_c(..., et, "J2000", "NONE", ...)` en km i km/s; en error fa `reset_c()` i retorna zeros. |
| `mulState` | `CState3D mulState(CState3D& state, double val)` | 67 | 89–94 | Multiplica in-place els 6 components de `state` per `val`. |
| `getCBposMat` | `glm::mat4 getCBposMat(CelestialBodyId body, CelestialBodyId origin, double et)` | 70 | 96–101 | Retorna matriu de translació en km a partir de `getCBstate`. |
| `getCBrotMat` | `glm::mat4 getCBrotMat(CelestialBodyId body, double et)` | 71 | 103–121 | Consulta `pxform_c("J2000", m_rotationFrame, et, rot)` i transposa 3×3 a `glm::mat4`. |
| `getOrbitStates` | `std::vector<CState3D> getOrbitStates(CelestialBodyId body, double et, int nStates)` | 73 | 123–172 | Llegeix `GM` (`bodvrd_c`), calcula període osculatriu (`oscltx_c`) i mostra `nStates + 1` estats en UA. |

---

## 4. Escena, visualització, càmeres i materials

### 4.1 `escena.h` (33 línies) i `escena.cpp` (196 línies)

- **Variables externes (`escena.h:29-31`)**: `extern glm::vec3 posMilano;`, `extern float angleMilano;`, `extern float inclinacioMilano;`.

| Funció | Signatura exacta | `.h` | `.cpp` | Descripció |
|---|---|---:|---:|---|
| `dibuixa_Eixos` | `void dibuixa_Eixos(GLuint ax_programID, bool eix, GLuint axis_Id, CMask3D reixa, CPunt3D hreixa, glm::mat4 MatriuProjeccio, glm::mat4 MatriuVista)` | 18 | 22–38 | Activa `ax_programID`, envia projecció i vista, i crida `draw_Eixos(axis_Id)` si `eix == true`. |
| `dibuixa_Skybox` | `void dibuixa_Skybox(GLuint sk_programID, GLuint cmTexture, char eix_Polar, glm::mat4 MatriuProjeccio, glm::mat4 MatriuVista)` | 21 | 41–76 | Renderitza el cub Skybox (`CUBE_SKYBOX`) amb vista sense translació, rotació segons `eix_Polar`, escala `5000`, `GL_LEQUAL` i `glDepthMask(GL_FALSE)`. |
| `dibuixa_EscenaGL` | `void dibuixa_EscenaGL(GLuint sh_programID, GLuint shader_programIDNormalMap, glm::mat4 MatriuVista, glm::mat4 MatriuTG, bool sw_mat[5], Escenari* escenariActual, COBJModel* objecteOBJ, bool mostrarOrbites = true, bool mostrarMilano = true, const ParametresIluminacioEscenari* ilum = nullptr)` | 25–26 | 81–196 | Cacheja 6 uniforms del shader base, delega a `escenariActual->dibuixaEscenari(...)` i, si `mostrarMilano && objecteOBJ`, transforma i dibuixa la nau Milano respecte a `centreModel(5.05, 0.71, -7.55)` amb `GL_CULL_FACE`. |

### 4.2 `visualitzacio.h` (49 línies) i `visualitzacio.cpp` (751 línies)

| Funció | Signatura exacta | `.h` | `.cpp` | Descripció |
|---|---|---:|---:|---|
| `Iluminacio` | `.cpp`: `void Iluminacio(GLint sh_programID, char ilumin, bool ifix, bool ilu2sides, bool ll_amb, LLUM* lumin, char obj, bool frnt_fcs, bool bc_lin, int step)` *(a `.h:15` manca `sh_programID`)* | 15 | 25–312 | Envia `LightModelAmbient`, `fixedLight` i les 8 llums `LightSource[0..7]` al shader i configura `glPolygonMode` segons `ilumin`. |
| `Projeccio_Perspectiva` | `glm::mat4 Projeccio_Perspectiva(GLuint sh_programID, int minx, int miny, GLsizei w, GLsizei h, double zoom)` | 20 | 318–335 | Configura `glViewport`, calcula `glm::perspective(60°, w/h, p_near, p_far)` i la puja a `"projectionMatrix"`. |
| `Vista_Esferica` | `glm::mat4 Vista_Esferica(GLuint sh_programID, CEsfe3D opv, char VPol, bool pant, CPunt3D tr, CPunt3D trF, CColor col_fons, CColor col_object, char objecte, double mida, int step, bool frnt_fcs, bool oculta, bool testv, bool bck_ln, char iluminacio, bool llum_amb, LLUM* lumi, bool ifix, bool il2sides, bool eix, CMask3D reixa, CPunt3D hreixa)` | 24–27 | 339–412 | Neteja framebuffer (`Fons`), aplica `Iluminacio`, calcula `cam` i `up` en esfèriques (`POLARZ/Y/X`), construeix `ViewMatrix`, configura `GL_CULL_FACE`/`GL_DEPTH_TEST` i la retorna. |
| `Vista_Navega` | `glm::mat4 Vista_Navega(GLuint sh_programID, CPunt3D pv, GLdouble n[3], GLdouble v[3], ...)` | 29–33 | 418–462 | Neteja framebuffer, aplica `Iluminacio`, construeix `lookAt(pv, n, v)` + pan i configura estats GL. |
| `instancia` | `glm::mat4 instancia(bool TR, INSTANCIA tg, INSTANCIA tgF)` | 35 | 467–500 | Construeix matriu de transformació geomètrica a partir de `tg` i `tgF`. |
| `deixos` | `GLuint deixos(void)` | 38 | 504–578 | Crea VAO+VBO dels 3 eixos de longitud 300 (`location 0` posició, `location 1` color). |
| `draw_Eixos` | `void draw_Eixos(GLuint vaoId)` | 39 | 581–594 | Dibuixa `GL_LINES` (6 vèrtexs) del VAO d'eixos. |
| `Fons` | `void Fons(CColor int_fons)` | 42 | 598–605 | Neteja `GL_COLOR_BUFFER_BIT` i `GL_DEPTH_BUFFER_BIT` i fa `glFlush()`. |
| `loadIMA_SOIL` | `GLint loadIMA_SOIL(const char *filename)` | 46 | 617–660 | Carrega síncronament una textura 2D RGBA via SOIL2 amb mipmaps i `GL_REPEAT`. |
| `loadCubemap` | `unsigned int loadCubemap(std::vector<std::string> faces)` | 47 | 672–704 | Carrega síncronament 6 cares RGB cap a `GL_TEXTURE_CUBE_MAP` (`GL_LINEAR`, `GL_CLAMP_TO_EDGE`). |
| `SetTextureParameters` | `void SetTextureParameters(GLint num, GLint IDText, bool extensT, bool filterT, bool blendT, bool genMipmpT)` | 48 | 715–750 | Configura unitat de textura, wrap, filtres i generació de mipmaps. |

### 4.3 `material.h` (21 línies) i `material.cpp` (109 línies)

| Funció | Signatura exacta | `.h` | `.cpp` | Descripció |
|---|---|---:|---:|---|
| `SeleccionaColorMaterial` | `void SeleccionaColorMaterial(GLuint sh_programID, CColor c_obj, bool sw_mater[5])` | 17 | 14–108 | Crida `SetColor4d`, envia `sw_intensity` (`sw_mater[0..3]`), `sw_material` (`sw_mater[4]`), `material.{emission,ambient,diffuse,specular}` i `material.shininess` (`32.0f` o `0.0f`). |

---

## 5. Interfície d'usuari (`InterficieUI`)

Fitxers: `InterficieUI.h` (269 línies) i `InterficieUI.cpp` (2117 línies).

### 5.1 Enums i estructures d'estat (`InterficieUI.h`)

- **`enum class ModeSimulacio`** (L18–21): `INTUITIU`, `ESCALA_REAL`.
- **`enum class ModeCamera`** (L24–29): `ORBITAL_CENTRADOR`, `FIXA_INCLINADA_25`, `GLOBAL_SISTEMA`, `COET_PRIMERA_PERSONA`.
- **`struct DataHoraSimulacio`** (L32–72): `any=2026, mes=10, dia=18, hora=12, minut=0, segon=0`.
  - Mètodes inline: `EsAnyDeTraspas(int)` (L41), `DiesAlMes(int, int)` (L46), `DiaDeLaSetmana(int, int, int)` (L54, Sakamoto `0=Dl..6=Dg`), `Validar()` (L64).
- **`struct EstatTempsSimulacio`** (L75–166):
  - Constants: `VELOCITAT_MIN_S_PER_S = 3600.0`, `VELOCITAT_DEFECTE_S_PER_S = 86400.0`, `VELOCITAT_MAX_S_PER_S = 4320000.0` (L76–78).
  - Camps: `dataHora`, `enPausa=false`, `direccioTemps=1`, `factorVelocitatSegonsPerSegon=86400.0`, `acumulatSegons=0.0`.
  - Mètodes inline: `EstablirDataActual()` (L87), `EstablirJ2000()` (L100), `AugmentarVelocitat()` (L105), `ReduirVelocitat()` (L111), `AvancarSegons(double)` (L117, limita `dt <= 0.25s`), `AvancarSegonsInt(long long)` (L130, privat).
- **`struct OpcionsVisualitzacio`** (L169–180): `mostrarOrbites=true`, `mostrarEtiquetes=true`, `mostrarMarcadorsPetits=true`, `mostrarSkybox=true`, `mostrarEixos=false`, `mostrarCinturoAsteroides=true`, `mostrarPuntsLagrange=false`, `anellsSaturnParticules=false`, `iluminacioSolarDirecta=true`, `vistaClaraAmbient=false`.
- **`struct CameraConfig`** (L183–187): `mode=FIXA_INCLINADA_25`, `angleInclinacio=25.0f`, `distZoom=100.0f`.
- **`struct CosAstronomicDades`** (L190–199): `nom`, `radiKm`, `distSolUA`, `gravetat`, `periodeRotacioHores`, `periodeOrbitalAnys`, `tipusEtiqueta`, `distTextPersonalitzat`.
- **`struct EstatSimulacio`** (L202–235):
  - `modeActual`, `mostrarSelectorModes=true`, `mostrarPanellTemps=true`, `mostrarFitxaHUD=false`, `mostrarDialegQuantA=false`, `recalculEfemeridesNecessari=false`, `origenReferenciaSol=true`, `cosEnfocatId=CB_SUN`.
  - Flags: `configuracioInicialPendent`, `canviModeDirectePendent`, `canviCameraPendent`.
  - Substruct `EstatVistaTransicio transicioVista` (L218–229): `valid`, `cosAnterior`, `modeCameraAnterior`, `radiCosAnterior`, `distAbsoluta`, `ratioZoomRelatiu`, `alfa`, `beta`, `radiSistemaAnterior`.
  - Substructs: `tempsSimulacio`, `opcionsVisualitzacio`, `cameraConfig`, `dadesCosActiu`.

### 5.2 Variables estàtiques i catàleg (`InterficieUI.cpp`)
- Fonts (L18–22): `s_fontPanellRegular` (16 px), `s_fontPanellPetit` (13.5 px), `s_fontPanellTitol` (17.5 px bold), `s_fontPanellDigital` (24.5 px), `s_rangCaractersCatala`.
- `kCatalegCossos` (L50–67): mapa de 16 cossos (`CB_SUN`, 8 planetes, `CB_MOON`, `CB_PLUTO`, `CB_CERES`, `CB_CHARON`, `CB_VOYAGER1`, `CB_VOYAGER2`, `CB_MILANO`).
- Marques del slider (L1199–1204): `kNumMarquesSlider=5`, `kPosMarques`, `kValMarques` (`3600`..`4320000`), `kEtiquetesMarques`.

### 5.3 Mètodes públics de `InterficieUI` (9 mètodes)

| Mètode | Signatura exacta | `.h` | `.cpp` | Descripció |
|---|---|---:|---:|---|
| `InicialitzarEstil` | `static void InicialitzarEstil()` | 242 | 128–191 | Aplica paleta fosca/cian, arrodoniments i carrega les 5 fonts TTF amb glifs catalans. |
| `ActualitzarDadesCosActiu` | `static void ActualitzarDadesCosActiu(EstatSimulacio& estat, const Escenari* escenari = nullptr)` | 245 | 83–113 | Omple `estat.dadesCosActiu` des de `kCatalegCossos` amb fallback a `escenari->obtenirObjecte(id)` o `CB_SUN`. |
| `SolicitarCanviModeDirecte` | `static void SolicitarCanviModeDirecte(EstatSimulacio& estat, ModeSimulacio modeDesti)` | 248 | 117–123 | Commuta `estat.modeActual` i activa `estat.canviModeDirectePendent = true`. |
| `Renderitzar` | `static void Renderitzar(EstatSimulacio& estat, Escenari*& currentEscenari, Escenari escenaris[], GLuint& shader_programID, GLuint& shader_programIDNormalMap)` | 251 | 411–437 | Orquestra per frame el selector modal o bé la barra superior + panells actius. |
| `DibuixarSelectorInicialModes` | `static void DibuixarSelectorInicialModes(EstatSimulacio& estat, Escenari*& currentEscenari, Escenari escenaris[], GLuint& shader_programID, GLuint& shader_programIDNormalMap)` | 254 | 454–720 | Finestra modal responsive amb targetes de Mode Didàctic i Mode a Escala Real. |
| `DibuixarBarraMenuSuperior` | `static void DibuixarBarraMenuSuperior(EstatSimulacio& estat, Escenari*& currentEscenari, Escenari escenaris[], GLuint& shader_programID, GLuint& shader_programIDNormalMap)` | 257 | 864–1193 | Barra superior dinàmica per mode + zona dreta (badge, pausa, botó `"Canviar"`, FPS). |
| `DibuixarPanellDataHoraTimelapse` | `static void DibuixarPanellDataHoraTimelapse(EstatSimulacio& estat)` | 260 | 1530–1954 | Panell flotant (`580×382`) amb visor digital, pop-ups hora/calendari, controls `<< || >>` i slider logarítmic. |
| `DibuixarFitxaAstronomicaHUD` | `static void DibuixarFitxaAstronomicaHUD(const CosAstronomicDades& dades, bool* pObert)` | 263 | 1961–2096 | Fitxa flotant (`350×294`) amb nom, categoria i 5 magnituds astronòmiques. |
| `DibuixarEtiquetaPlaneta` | `static void DibuixarEtiquetaPlaneta(const std::string& nom, const glm::vec3& posicioMon, const glm::mat4& viewMatrix, const glm::mat4& projMatrix, const glm::vec4& viewport)` | 266–268 | 2103–2117 | Projecta `posicioMon` via `glm::project` i dibuixa punt cian + text al `BackgroundDrawList`. |

### 5.4 Funcions estàtiques internes de `InterficieUI.cpp` (19 helpers)
- Utilitats: `FitxerExisteix` (L25), `PushFontSegur` (L36), `PopFontSegur` (L42), `NomCategoriaLlegible` (L69), `InterpolarColor` (L1277).
- Dibuix vectorial i finestres: `DibuixarIconaPausaReproduccio` (L199), `DibuixarMarcFinestraEspacial` (L233), `DibuixarDialegQuantA` (L286), `DibuixarPuntLlista` (L443), `DibuixarIconaRellotge` (L1297), `DibuixarIconaCalendari` (L1306), `DibuixarIconaFletxesDobles` (L1335).
- Submenús i pop-ups: `DibuixarLlistaFocusObjectes` (L726), `DibuixarMenuTempsIData` (L805), `DibuixarPopupSelectorHora` (L1352), `DibuixarPopupSelectorData` (L1408).
- Slider de velocitat: `VelocitatANormalitzat` (L1207), `NormalitzatAVelocitat` (L1221), `FormatarVelocitatActual` (L1235).

---

## 6. Recursos, models 3D, shaders, geometria i quaternions

### 6.1 `GestorRecursos.h` (108 línies) i `GestorRecursos.cpp` (489 línies)

- **Structs (`GestorRecursos.h`)**:
  - `ImatgeCPU` (L18–24): `pixels`, `width`, `height`, `channels`, `rutaNormalitzada`.
  - `ParametresIluminacioEscenari` (L27–33): `matriuProjeccio`, `const LLUM* llums`, `numLlums`, `llumAmbient`, `llumFixa`.
  - `CacheUniformsShader` (L36–76): `programID`, 21 `GLint loc*` (matrius, samplers, flags, material), array `LocLlum locLlums[8]` (10 `GLint` per llum) i `ActualitzarSiCal(GLuint id)` (`.h:75`, `.cpp:231-272`).
- **Estat intern (`GestorRecursos.cpp`, namespace anònim L16–229)**: `kTexturesPrecarrega2D[18]`, `kRutesSkybox[6]`, `s_cacheTextures2D`, `s_defaultNormalMapID`, `s_cubemapID`, `s_mutex`, `s_cuaTexturesCPU`, `s_skyboxCaresCPU[6]`, `s_skyboxCPUReady`, `s_precarregaThread`, atòmics `s_precarregaIniciada`/`s_precarregaAcabada`/`s_cancelarPrecarrega`.
- **Helpers interns (`.cpp`)**: `NormalitzarRuta` (L65), `PujarImatge2DAGPU` (L77), `PujarSkyboxAGPU` (L121), `AlliberarImatgeCPU` (L155), `FonsPrecarregaEscena` (L167).

| Mètode públic (`GestorRecursos`) | `.h` | `.cpp` | Descripció |
|---|---:|---:|---|
| `static void IniciarPrecarregaEscena()` | 82 | 274–281 | Inicia fil `s_precarregaThread` per descodificar Skybox i 18 textures 2D a RAM. |
| `static void ProcessarCuaPujadaGPU(int maxTexturesPerFrame = 3)` | 85 | 283–330 | Puja cubemap si és llest i fins a `maxTexturesPerFrame` textures 2D per frame. |
| `static void FinalitzarPrecarregaGPU()` | 88 | 332–341 | Fa `join()` del fil i puja totes les textures pendents (`ProcessarCuaPujadaGPU(1000)`). |
| `static GLuint ObtenirTextura2D(const char* ruta)` | 91 | 343–395 | Cerca en 3 nivells: cache GPU → cua CPU → càrrega síncrona SOIL2. |
| `static GLuint ObtenirNormalMapPerDefecte()` | 94 | 397–409 | Genera/retorna textura 1×1 plana `{128, 128, 255}`. |
| `static GLuint ObtenirTexturaSkybox()` | 97 | 411–425 | Retorna `s_cubemapID` (puja cua o fallback síncron `loadCubemap`). |
| `static bool SkyboxLlestAGPU()` | 100 | 427–430 | Retorna `s_cubemapID != 0`. |
| `static void AssegurarVAOsEscenari()` | 103 | 432–444 | Crea un sol cop `VAOList[GLU_SPHERE]` (`loadgluSphere_EBO(1,40,40)`) i `VAOList[GLUT_TORUS]` (`creaOrbita()`). |
| `static void AlliberarTot()` | 107 | 446–489 | Cancel·la fil, allibera RAM, fa `glDeleteTextures` i crida `deleteVAOList` (`GLU_SPHERE`, `GLU_DISK`, `GLUT_TORUS`, `CUBE_SKYBOX`). |

---

### 6.2 `objLoader.h` (155 línies) i `objLoader.cpp` (1135 línies)

- **Constant**: `MAX_SIZE_VAOLIST = 125` (`.h:23`).
- **Structs (`.h`)**: `Vector3D` (L26), `Vector2D` (L33), `OBJFileInfo` (L39), `Face` (L48), `Material` (L57), `VertexOBJ` (L70 — 32 B: `px,py,pz, nx,ny,nz, u,v`), `OBJSubMeshCPU` (L77), `OBJTextureCPU` (L84), `OBJPreloadData` (L93).
- **Format `.mbin` (`.cpp:69-88`)**: `MBinHeader` (`magic="MBIN"`, `version=1`, `srcSize`, `srcMtime`, `numMaterials`, `numSubmeshes`) + `MBinMaterial[]` + submalles (`matIdx`, `nVerts`, `nIndices`, blocs crus de `VertexOBJ` i `uint32_t`).
- **Classe `COBJModel`** (`.h:102-146`):
  - **Públics**: `COBJModel()` (`.h:107`/`.cpp:619`), `~COBJModel()` (`.h:108`/`.cpp:635`), `LoadToCPU(const char*, OBJPreloadData&)` (`.h:112`/`.cpp:660`), `UploadFromCPU(OBJPreloadData&)` (`.h:113`/`.cpp:843`), `FreePreloadData(OBJPreloadData&)` (`.h:114`/`.cpp:641`), `LoadModel(char*)` (`.h:106`/`.cpp:964`), `draw_TriVAO_OBJ(GLuint)` (`.h:119`/`.cpp:1117`), `netejaVAOList_OBJ()` (`.h:117`/`.cpp:1048`), `netejaTextures_OBJ()` (`.h:118`/`.cpp:1057`), `DrawModel(int)` (`.h:105`/`.cpp:1009`), `EliminaLlista(int)` (`.h:109`/`.cpp:1024`).
  - **Privats**: `updateUniformCache` (`.cpp:972`), `UseMaterial_ShaderID` (`.cpp:984`), `initVAOList_OBJ` (`.cpp:1030`), `Set_VAOList_OBJ` (`.cpp:1042`), `deleteVAOList_OBJ` (`.cpp:1070`), `draw_TriVAO_Object_OBJ` (`.cpp:1103`).
  - **Exportades**: `InitObject()` (`.h:149`/`.cpp:1014`), `UnInitObject(COBJModel*)` (`.h:152`/`.cpp:1019`).
  - **Helpers interns (`.cpp:90-612`)**: `ExtractBasePath`, `IsAbsoluteOrDrivePath`, `BuildJoinPath`, `BuildMBinPath`, `InitDefaultMaterial`, `SkipSpaces`, `ExtractTrimmedSpan`, `ReadEntireFile`, `LoadMaterialLibCPU`, `ResolveObjIndex`, `ParseFaceCornerToken`, `ComputeFaceNormal`, `BuildIndexedSubMesh` (camí directe 1:1 o hash FNV-1a), `TryLoadMBin`, `SaveMBin`.

---

### 6.3 `shader.h` (73 línies) i `shader.cpp` (571 línies)

Classe `Shader` (`.h:15-70`) amb camp públic `unsigned int programID` (L21, inicialitzat a `-1`).
- **Cicle de vida**: `Shader()` (`.h:27`/`.cpp:21`), `~Shader()` (`.h:30`/`.cpp:28`), `loadFileShaders(string, string)` (`.h:33`/`.cpp:39`), `loadFileShadersG(string, string, string)` (`.h:34`/`.cpp:203` — conté bug a L314 on carrega `file_Geom` a `vs`), `DisplayMessageBox` (`.h:36`/`.cpp:406`), `textFileRead` (`.h:37`/`.cpp:439`), `releaseAllShaders` (`.h:38`/`.cpp:469`), `getProgramID` (`.h:41`/`.cpp:479`), `use` (`.h:44`/`.cpp:489`), `DeleteProgram` (`.h:67`/`.cpp:567`).
- **Setters d'uniforms**: `setBool` (L48/500), `setInt` (L49/505), `setFloat` (L50/510), `setDouble` (L51/515, cast a `float`), `setBool3` (L54/521), `setInt3` (L55/526), `setFloat3` (L56/531), `setDouble3` (L57/536), `setFloat4` (L60/542), `setDouble4` (L61/547), `setMatrix3fv` (L64/553), `setMatrix4fv` (L65/558).

---

### 6.4 `glut_geometry.h` (2925 línies) i `glut_geometry.cpp` (~9600 línies)

- **Emmagatzematge global**: `CVAO VAOList[MAX_SIZE_VAOID]` (`MAX_SIZE_VAOID = 50`, `.h:84`) i `GLdouble cColor[4]`.
  - Índexs utilitzats pel Sistema Solar: `CUBE_SKYBOX (0)`, `GLUT_TORUS (6, reutilitzat per a l'òrbita unitària)`, `GLU_DISK (15)`, `GLU_SPHERE (17)`.
- **Layout del VBO**: blocs contigus en `GL_DOUBLE`: `location 0` (3 pos), `1` (4 color), `2` (3 normal), `3` (2 UV), `4` (3 tangent, només a `load_TRIANGLES_EBOver2`). Excepció: `loadCubeSkybox_VAO` usa `GL_FLOAT` (`location 0`).
- **Funcions clau**:
  - **Gestió de VAOList**: `SetColor4d` (`.h:142`), `Set_VAOList` (`.h:143`), `Set_vaoId`/`vboId`/`eboId`/`nVertexs` (`.h:144-147`), `Get_VAOList` (`.h:150`), `Get_VAOId` (`.h:151`), `initVAOList` (`.h:154`), `netejaVAOList` (`.h:155`), `deleteVAOList` (`.h:156`), `draw_TriVAO_Object` (`.h:159`), `draw_LinVAO_Object` (`.h:160`), `draw_TriEBO_Object` (`.h:161`), `draw_LinEBO_Object` (`.h:162`).
  - **Skybox**: `loadCubeSkybox_VAO` (`.h:176`/`.cpp:677`), `drawCubeSkybox` (`.h:177`/`.cpp:740`).
  - **Esferes**: `loadgluSphere_VAO` (`.h:2633`/`.cpp:5071`), **`loadgluSphere_EBO`** (`.h:2634`/`.cpp:5222` — calcula tangents Gram-Schmidt per a normal mapping i crida `load_TRIANGLES_EBOver2`), `loadglutSolidSphere_VAO/EBO` (`.h:181-182`).
  - **Discs / Anells**: `loadgluDisk_EBO` (`.h:2626`/`.cpp:4699`) → delega a **`loadgluPartialDisk_EBO`** (`.h:2630`/`.cpp:4887`, UVs polars radials `u=(r-rIn)/(rOut-rIn)`, `v=i/slices`).
  - **Corbes Hermitte amb `CState3D`** (`.h:2734` inclou `CelestialBodyData.h`): `draw_Hermitte_Curve` (`.h:2746`/`.cpp:7354`), `load_Hermitte_Curve_VAO` (`.h:2747`/`.cpp:7459`), `load_Hermitte_Curve_EBO` (`.h:2748`/`.cpp:7673`).
  - **Constructors genèrics**: `load_LINES_VAO/EBO` (`.h:2905, 2907`), `load_TRIANGLES_VAO/EBO` (`.h:2909, 2911`), **`load_TRIANGLES_EBOver2`** (`.h:2912`/`.cpp:9465`, amb `tangents` a `location 4`).

---

### 6.5 `quatern.h` (61 línies) i `quatern.cpp` (827 línies)

Struct `GL_Quat { GLdouble w, x, y, z; }` (`.h:18-23`), constants `NFRAMES_Q=25`, `MAX_KEYFRAMES_Q=3` (`.h:12-13`), `DELTA=1e-6` (`.cpp:16`).
- **25 funcions** (llegat EntornVGI): `EixAngleToQuat` (`.h:27`/`.cpp:40`), `QuatToEixAngle` (L29/97), `QuatSetValue` (L30/134), `QuatToMatrix` (L31/188), `EulerToQuat` (L33/231), `MatrixToQuat` (L34/267 — discrepància `double` vs `double*`), `QuatLerp` (L37/419), `QuatSlerp` (L38/357), `QuatNormalize` (L41/463), `QuatScaleAngle` (L42/482), `QuatInverse` (L43/495), `QuatSetFromAx` (L44/519), `QuatMul` (L46/608), `QuatAdd` (L47/625), `QuatSub` (L48/640), `QuatDiv` (L49/655), `QuatCopy` (L50/678), `QuatSquare` (L51/690), `QuatSqrt` (L52/706), `QuatDot` (L53/737), `QuatLength` (L54/744), `QuatNegate` (L55/753), `QuatExp` (L56/767), `QuatLog` (L57/787), `QuatLnDif` (L58/809).

---

## 7. Referència completa dels 7 programes de shaders GLSL

Tots els shaders utilitzen **`#version 460 core`** i es troben a `Entorn-GLFW-GL4.6-ImGui/EntornVGI/shaders/`. Cap shader implementa encara textures dia/nit, núvols ni mapes especulars separats.

### Structs GLSL comuns (en `flat`, `gouraud`, `gouraud_inst`, `phong`, `normal_mapping`)
- `Light` (`MaxLights = 8`): `bool sw_light`, `vec4 position`, `vec4 ambient`, `vec4 diffuse`, `vec4 specular`, `vec3 attenuation` (`a,b,c`), `bool restricted`, `vec3 spotDirection`, `float spotCosCutoff`, `float spotExponent`.
- `Material`: `vec4 emission`, `vec4 ambient`, `vec4 diffuse`, `vec4 specular`, `float shininess`.

### 7.1 `eixos.vert` / `eixos.frag` *(Actiu)*
- **Atributs**: `0: vec3 in_Vertex`, `1: vec4 in_Color`.
- **Uniforms**: `mat4 projectionMatrix`, `mat4 viewMatrix`.
- **Algorisme**: `gl_Position = projectionMatrix * viewMatrix * vec4(in_Vertex, 1.0)`; `FragColor = VertexColor` (sense `modelMatrix` ni il·luminació).

### 7.2 `skybox.VERT` / `skybox.FRAG` *(Actiu)*
- **Atributs**: `0: vec3 in_Vertex`.
- **Uniforms**: `mat4 projectionMatrix`, `mat4 viewMatrix`, `mat4 modelMatrix`, `samplerCube skybox`.
- **Algorisme**: `TexCoords = in_Vertex`; `pos = projectionMatrix * viewMatrix * modelMatrix * vec4(in_Vertex, 1.0)`; `gl_Position = pos.xyww` (fixa profunditat NDC a `1.0`); `FragColor = texture(skybox, TexCoords)`.

### 7.3 `phong_shdrML.vert` / `phong_shdrML.frag` *(Actiu — òrbites, anells de Saturn, nau Milano)*
- **Atributs**: `0: vec3 in_Vertex`, `1: vec4 in_Color`, `2: vec3 in_Normal`, `3: vec2 in_TexCoord`.
- **Uniforms**:
  - Matrius: `projectionMatrix`, `viewMatrix`, `modelMatrix`, `normalMatrix` (FS, per a `spotDirectionPV`), `bool useNormalMatrixPV = false`, `mat3 uNormalMatrixPV` (optimització CPU→VS).
  - Textura: `sampler2D texture0` (unitat 0), `bool textur`, `bool flag_invert_y` (VS), `bool modulate`.
  - Il·luminació: `bool fixedLight`, `bool sw_material`, `bvec4 sw_intensity` (`[0]` emissió, `[1]` ambient, `[2]` difusa, `[3]` especular), `bool sw_twoSided = false`, `vec4 LightModelAmbient`, `Light LightSource[8]`, `Material material`.
- **Algorisme**:
  - **VS**: transforma posició (`vertexPV`) i normal (`vertexNormalPV` via `uNormalMatrixPV` si actiu, altrament `transpose(inverse(mat3(mv)))`); inverteix `UV.y` si `flag_invert_y`.
  - **FS (`phongModel`)**: il·luminació per píxel en View Space amb fins a 8 llums puntuals (`w == 1`, atenuació `1 / (a*d² + b*d + c)`) o direccionals, focus de Warn (`restricted`), difusa `sw_twoSided ? (0.35 + 0.65 * abs(dot(L, N))) : max(dot(L, N), 0.0)`, especular Phong (`pow(max(dot(R, V), 0), shininess)`), i modulació de `texture0` (`MODULATE` o `DECAL`).

### 7.4 `normal_mapping.vert` / `normal_mapping.frag` *(Actiu — totes les esferes `EsferaInt`)*
- **Atributs**: `0: in_Vertex`, `1: in_Color`, `2: in_Normal`, `3: in_TexCoord`, **`4: vec3 in_Tangent`**.
- **Uniforms**: tots els de `phong_shdrML` + **`sampler2D normalMap`** (unitat 1).
- **Algorisme**:
  - **VS**: construeix la base ortonormal `TBN_PV = mat3(T, B, N)` en View Space (`N = normalize(normalMatrixPV * in_Normal)`, `T` re-ortogonalitzada per Gram-Schmidt, `B = normalize(cross(N, T))`).
  - **FS**: mostreja `normalMap` amb força de relleu `0.3`:
    ```glsl
    vec2 nxy = (texture(normalMap, vertexTexCoord).rg * 2.0 - 1.0) * 0.3;
    vec3 normalTangent = vec3(nxy, sqrt(max(1.0 - dot(nxy, nxy), 0.0)));
    vec3 N = normalize(TBN_PV * normalTangent);
    ```
    Avalua `phongModel(N, ...)` idèntic a `phong_shdrML.frag` i aplica `texture0`.

### 7.5 `flat_shdrML.vert` / `flat_shdrML.frag` *(Llegat EntornVGI, inactiu)*
- **Atributs**: `0..3`. **Uniforms**: `normalMatrix`, `projectionMatrix`, `viewMatrix`, `modelMatrix`, `texture0`, `textur`, `flag_invert_y`, `modulate`, `fixedLight`, `sw_material`, `sw_intensity`, `LightModelAmbient`, `LightSource[8]`, `material`.
- **Algorisme**: calcula `flatModel` al VS en View Space i passa `flat out vec4 VertexColor` al FS (sense interpolació).

### 7.6 `gouraud_shdrML.vert` / `gouraud_shdrML.frag` *(Llegat EntornVGI, inactiu)*
- **Atributs**: `0..3`. **Uniforms**: idèntics a `flat_shdrML`, però calcula `normalMatrix = transpose(inverse(viewMatrix * modelMatrix))` dins el VS.
- **Algorisme**: calcula `gouraudModel` al VS i interpola suaument `VertexColor` al FS.

### 7.7 `gouraud_shdrML_inst.vert` / `gouraud_shdrML_inst.frag` *(Incomplet, no compila)*
- **Atributs**: `0..3` + `layout (location = 4) in ???? in_instanceMatrix;` (`vert:45`, marcador incomplet).
- **Estat**: aplica `in_instanceMatrix` a `vertexPV` (`L73`) però no a `normalMatrix` (`L70`) ni a `gl_Position` (`L188`). El fragment shader és idèntic a `gouraud_shdrML.frag`.

---

## 8. Referència de kernels SPICE i textures

### 8.1 Kernels NASA SPICE (`EntornVGI/cbdata/`)

| Fitxer | Tipus | Contingut principal | Cobertura temporal (ET, `SUMMARY.txt`) |
|---|---|---|---|
| `naif0012.tls` | LSK | Segons de traspàs (Leapseconds) | General |
| `pck00011.tpc` | PCK | Constants d'orientació / rotació IAU i radis | General |
| `gm_de440.tpc` | PCK | Paràmetres gravitacionals `GM` per a `bodvrd_c`/`oscltx_c` | General |
| `de440.bsp` | SPK | Sol, baricentres 1..9, Mercuri (199), Venus (299), Terra (399), Lluna (301) | `1549-12-31` → `2650-01-25` |
| `plu060.bsp` | SPK | Sistema Plutó (999), Caront (901), Nix..Styx (902..905) | `1800-01-02` → `2199-12-30` |
| `1000036.bsp` | SPK | Cometa Halley (`1000036`) | `1800-01-02` → `2199-12-30` |
| `codes_300ast_20100725.bsp` | SPK | 300 asteroides (incloent `2000001` Ceres, `2000004` Vesta...) | `1799-12-30` → `2199-12-13` |
| `vgr1.x2100.bsp` | SPK | Sonda Voyager 1 (`-31`) respecte al Sol | `1981-01-01` → `2100-01-01` |
| `vgr2.x2100.bsp` | SPK | Sonda Voyager 2 (`-32`) respecte a Neptú / Sol | `1989-08-29` → `2100-01-01` |

### 8.2 Inventari de textures runtime (`EntornVGI/textures/`)

- **Precarregades i actives via `GestorRecursos` (18 textures 2D + 6 cares Skybox):**
  - Albedo: `textures/sun/2k_sun.jpg`, `Mercury/2k_mercury.jpg`, `Venus/2k_venus_surface.jpg`, `Earth/earth2048.bmp`, `Mars/2k_mars.jpg`, `Jupiter/2k_jupiter.jpg`, `Saturn/2k_saturn.jpg`, `Uranus/2k_uranus.jpg`, `Neptune/2k_neptune.jpg`, `Moon/2k_moon.jpg`, `Saturn/2k_saturn_ring_alpha.png`.
  - Normal maps: `textures/planet/NormalTextures/NormalMap{Mercury,Venus,Earth,Mars,Jupiter,Saturn,Neptune}.png`.
  - Skybox (`+X,-X,+Y,-Y,+Z,-Z`): `textures/skybox/{right,left,top,bottom,front,back}.png`.
- **Carregades per `COBJModel` (Nau Milano):**
  - `textures/coet/milano_gotg.obj` (cache `milano_gotg.mbin`), `milano_gotg.mtl`, `texture-1.png`.
- **Presents a disc però no utilitzades encara en runtime:**
  - `textures/planet/Earth/2k_earth_{daymap,clouds,nightmap}.jpg`, `2k_earth_specular_map.tif`.
  - `textures/planet/Venus/2k_venus_atmosphere.jpg`.
  - `textures/skybox/*.jpg` (el runtime usa els `.png`).
  - `Assets/rock/{rock.mtl,rock.png,NormalMapRock.png}`.
  - `docs/planetes/planetes.json` i `docs/planetes/textures/` (catàleg documental amb atribució Solar System Scope CC BY 4.0).
