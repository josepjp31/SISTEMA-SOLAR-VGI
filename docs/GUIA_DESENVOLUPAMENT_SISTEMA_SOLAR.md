# Guia de Desenvolupament i Adequació per al Simulador del Sistema Solar

> **Assignatura:** Visualització Gràfica Interactiva (VGI) – Escola d'Enginyeria, UAB  
> **Projecte:** Simulador Gràfic Interactiu del Sistema Solar (Encàrrec de l'IEEC - ABP Grup 05)  
> **Objectiu del document:** Definir de manera exhaustiva quins components de l'entorn base cal **mantenir**, quins cal **eliminar/desactivar**, quins cal **modificar/editar** i quins cal **crear de nou** per transformar el codi docent en el simulador astronòmic final.  
> **Data:** Octubre 2026  

---

## 1. Resum Executiu d'Adequació

L'entorn base **EntornVGI** ([`Entorn-GLFW-GL4.6-ImGui/EntornVGI/`](../Entorn-GLFW-GL4.6-ImGui/EntornVGI/)) proporciona una infraestructura sòlida d'**OpenGL 4.6 Modern Core Profile**, gestió de finestres amb GLFW, suport matricial amb GLM i una interfície immediata amb Dear ImGui. Tanmateix, va ser concebut com una eina d'aprenentatge generalista plena d'objectes de laboratori (com la tetera, cubs RGB, corbes paramètriques docents o una nau Star Wars).

Per desenvolupar el **Simulador del Sistema Solar** complint tots els requisits i casos d'ús (Casos 1 al 6 detallats a [`Casos d'ús_ Simulador del Sistema Solar.md`](./Casos%20d'%C3%BAs_%20Simulador%20del%20Sistema%20Solar.md)), cal dur a terme una transformació arquitectònica guiada:

```
+-----------------------------------------------------------------------------------------+
|                                    ESTAT ACTUAL                                         |
|  Entorn VGI Generalista: Tetera, Cub, Primitives soltes, Shaders docents, Menús demo    |
+--------------------------------------------+--------------------------------------------+
                                             |
             +-------------------------------+-------------------------------+
             |                               |                               |
             v                               v                               v
    [QUÈ CAL MANTENIR]             [QUÈ CAL ELIMINAR]              [QUÈ CAL CREAR DE NOU]
    - Bucle GLFW & ImGui           - Objectes Tie, Tetera, Cub     - Classes Escenari & CosCeleste
    - Abstracció Shaders           - Corbes docents (Lemniscata)   - Mòdul Efemèrides (NASA JPL)
    - Geometria Esfera & Skybox    - Shaders obsolets (Flat/Gour)  - Càmeres Orbitals Centrades
    - Càrrega textures (SOIL2)     - Menús docents innecessaris    - Traçador d'Òrbites & Lagrange
    - Quaternions & GLM                                            - Cinturó Asteroides (Instanced)
             |                               |                               |
             +-------------------------------+-------------------------------+
                                             |
                                             v
+-----------------------------------------------------------------------------------------+
|                                      ESTAT FINAL                                        |
|  Simulador Astronòmic: Escales reals/visuals, Efemèrides, Càmeres interactives,         |
|  Il·luminació central solar, Textures planetàries en alta resolució, UI espacial        |
+-----------------------------------------------------------------------------------------+
```

### 1.1. Filosofia de Disseny i Referents de Visualització
El projecte pren com a referència directa eines consolidades de visualització astronòmica interactiva com **OpenSpace** ([openspaceproject.com](https://www.openspaceproject.com)) i **SpaceEngine** ([spaceengine.org](https://spaceengine.org)):
* **Estratègia de priorització d'escenes:** 
  1. *Versió Intuïtiva (Prioritària):* Vista esquemàtica i no a escala estricta per fer immediatament visibles i navegables el Sol i els 9 planetes principals (incloent Plutó) amb les seves mides i distàncies ajustades per a una exploració còmoda.
  2. *Versió a Escala Real:* Vista amb proporcions físiques autèntiques, acompanyada d'indicadors visuals (marcadors o halos) per localitzar els planetes menors que d'altra manera serien imperceptibles a gran distància.
  3. *Mode Nau Espacial (Reserva):* Càmera lliure simulant un coet/nau espacial, que es planteja com a extensió secundària un cop enllestides i consolidades les dues primeres escenes.
* **Paradigma d'interactivitat de càmera (*Focus & Orbit*):** La vista general inicial situa una càmera orbital al voltant del Sol. En fer clic sobre qualsevol planeta (o seleccionar-lo al menú), la càmera canvia el seu centre d'atenció per orbitar directament al voltant del cos seleccionat, permetent inspeccionar-ne la superfície, rotació i satèl·lits.
* **Estètica orbital:** Renderitzat net de les trajectòries orbitals com a línies lluminoses contínues que l'usuari pot commutar a voluntat.

---

## 2. Què cal MANTENIR de l'Entorn Base

Els següents components són robustos, utilitzen les APIs correctes d'OpenGL 4.6 i han de ser conservats íntegrament com a fonament del projecte:

### 2.1. Arquitectura del Bucle Principal i Finestra ([`main.cpp`](../Entorn-GLFW-GL4.6-ImGui/EntornVGI/main.cpp))
* **Bucle GLFW:** La inicialització de finestra (`glfwCreateWindow`), el context OpenGL Core Profile 4.6 i l'intercanvi de buffers (`glfwSwapBuffers`) funcionen correctament i no necessiten cap reescriptura.
* **Depurador d'OpenGL:** La crida de retorn `glDebugOutput` configurada a `main()` és imprescindible per detectar errors de GPU (variables *uniform* no trobades, buffers invàlids o fallades de shaders) durant el desenvolupament.
* **Integració de Dear ImGui:** El cicle de vida d'ImGui (`ImGui_ImplGlfw_NewFrame()`, `ImGui_ImplOpenGL3_NewFrame()`, `ImGui::NewFrame()` i `ImGui_ImplOpenGL3_RenderDrawData()`) està perfectament configurat i preparat per allotjar els nous controls astronòmics.

### 2.2. Classe d'Abstracció de Shaders ([`shader.cpp`](../Entorn-GLFW-GL4.6-ImGui/EntornVGI/shader.cpp) / [`shader.h`](../Entorn-GLFW-GL4.6-ImGui/EntornVGI/shader.h))
* La classe `Shader` és completament reusable: encapsula la lectura de codi font GLSL, la compilació, l'enllaç del programa i l'assignació de variables *uniform* (`setMatrix4fv`, `setFloat`, `setBool`, `setInt`, `setFloat4`).

### 2.3. Geometria Procedimental Esfèrica i Toroidal ([`glut_geometry.cpp`](../Entorn-GLFW-GL4.6-ImGui/EntornVGI/glut_geometry.cpp))
* **Esfera amb buffers VAO i EBO:** `loadgluSphere_EBO` i `draw_TriEBO_Object(GLU_SPHERE)` generen una esfera amb índexs, normals i coordenades UV completes. Aquesta és la base geomètrica fonamental per a tots els planetes, satèl·lits naturals i el Sol.
* **Torus amb buffers VAO i EBO:** `glutSolidTorus_VAO` i `draw_TriEBO_Object(GLUT_TORUS)` són ideals per modelar els anells de Saturn o altres anells planetaris.
* **Línies 3D:** Els mecanismes de `draw_LinVAO_Object` i `draw_LinEBO_Object` són directament aprofitables per renderitzar les corbes de les trajectòries orbitals el·líptiques.

### 2.4. Subsistema de Skybox Cúbic ([`escena.cpp`](../Entorn-GLFW-GL4.6-ImGui/EntornVGI/escena.cpp) i [`visualitzacio.cpp`](../Entorn-GLFW-GL4.6-ImGui/EntornVGI/visualitzacio.cpp))
* Les funcions `loadCubemap()`, `loadCubeSkybox_VAO()` i `dibuixa_Skybox()` juntament amb els shaders [`skybox.VERT`](../Entorn-GLFW-GL4.6-ImGui/EntornVGI/shaders/skybox.VERT) i [`skybox.FRAG`](../Entorn-GLFW-GL4.6-ImGui/EntornVGI/shaders/skybox.FRAG) estan completament acabades. Només caldrà substituir les 6 imatges de prova per un mapa de l'espai profund / Via Làctia.

### 2.5. Càrrega de Textures amb SOIL2 ([`visualitzacio.cpp`](../Entorn-GLFW-GL4.6-ImGui/EntornVGI/visualitzacio.cpp))
* `loadIMA_SOIL()` i `SetTextureParameters()` permeten carregar imatges JPG, PNG i BMP directament a la memòria de textura de la GPU amb filtratge bilineal/trilineal i generació automàtica de Mipmaps.

### 2.6. Quaternions i Àlgebra GLM ([`quatern.cpp`](../Entorn-GLFW-GL4.6-ImGui/EntornVGI/quatern.cpp))
* El mòdul `quatern` i la integració amb GLM són imprescindibles per controlar la nau espacial (evitant el bloqueig de cardan o *Gimbal Lock* en orientacions 3D arbitràries) i calcular la rotació pròpia dels planetes amb els seus angles d'inclinació axial respecte a l'eclíptica.

---

## 3. Què cal ELIMINAR, DEPRECAR o DESACTIVAR

Per mantenir el codi net, evitar dispersions i guanyar rendiment, s'han de retirar o desactivar els elements següents que no tenen relació amb l'astronomia:

| Element / Funció | Fitxer Original | Raó per Eliminar o Desactivar |
|---|---|---|
| **Nau Star Wars (`tie()`, `Alas()`, `Motor()`, `Canon()`, `Cuerpo()`, `Cabina()`)** | [`escena.cpp`](../Entorn-GLFW-GL4.6-ImGui/EntornVGI/escena.cpp) (línies 644–1190) | Model docent codificat manualment mitjançant desenes de transformacions primitives. No té utilitat en la simulació del Sistema Solar. |
| **Objecte Arc i Mar Fractal (`arc()`, `loadSea_VAO()`)** | [`escena.cpp`](../Entorn-GLFW-GL4.6-ImGui/EntornVGI/escena.cpp) (línies 460–640) | Geometria docent d'una porta amb un pla ondulat marí. |
| **Primitives de prova (`TETERA`, `CUB`, `CUB_RGB`, `MATRIUP`, `MATRIUP_VAO`)** | [`escena.cpp`](../Entorn-GLFW-GL4.6-ImGui/EntornVGI/escena.cpp) | Objectes de laboratori d'anàlisi de rendiment que no s'han de renderitzar en el simulador final. |
| **Corbes matemàtiques docents (`C_LEMNISCATA`, `C_HERMITTE`, `C_CATMULL_ROM`)** | [`escena.cpp`](../Entorn-GLFW-GL4.6-ImGui/EntornVGI/escena.cpp) | Dibuix de corbes no astronòmiques i els seus respectius triedres de Darboux i Frenet docents. |
| **Shaders Obsolets (`flat_shdrML.*`, `gouraud_shdrML.*`)** | [`shaders/`](../Entorn-GLFW-GL4.6-ImGui/EntornVGI/shaders/) | L'ombrejat pla (*Flat*) i l'interpolat per vèrtex (*Gouraud*) produeixen artefactes visuals i mala il·luminació en esferes. Tota la visualització ha de convergir en el shader Phong per píxel o un shader PBR específic. |
| **Finestres de demostració de Dear ImGui (`show_demo_window`, `show_another_window`)** | [`main.cpp`](../Entorn-GLFW-GL4.6-ImGui/EntornVGI/main.cpp) | Sobrecàrrega visual innecessària que embruta la vista de l'usuari. Cal desactivar-les per defecte (`show_demo_window = false`). |
| **Menús docents innecessaris a `ShowEntornVGIWindow`** | [`main.cpp`](../Entorn-GLFW-GL4.6-ImGui/EntornVGI/main.cpp) | Seccions de fractals, matriu de primitives, triedre de Darboux o selecció d'objectes com la tetera han de ser substituïdes pels controls astronòmics. |

---

## 4. Què cal MODIFICAR i ADAPTAR (Refactoring)

A continuació es detallen els canvis necessaris als fitxers existents per acoblar la nova funcionalitat:

### 4.1. Modificacions a [`escena.cpp`](../Entorn-GLFW-GL4.6-ImGui/EntornVGI/escena.cpp)
* **Reemplaçar el bloc de selecció de `dibuixa_EscenaGL()`:**
  En lloc d'avaluar si l'objecte és un cub o una tetera, aquesta funció ha de delegar en el renderitzador del Sistema Solar:
  ```cpp
  void dibuixa_EscenaGL(...) {
      // 1. Dibuixar el Sol al centre (emissiu, sense rebre ombra externa)
      // 2. Dibuixar cada planeta amb la seva textura, inclinació axial i rotació
      // 3. Dibuixar les llunes associades al voltant del seu respectiu planeta
      // 4. Si està actiu: dibuixar les línies de les trajectòries orbitals
      // 5. Si està actiu: dibuixar el cinturó d'asteroides
      // 6. Si està actiu: dibuixar els marcadors dels punts de Lagrange
  }
  ```
* **Aplicació de la Jerarquia de Transformacions amb GLM:**
  Cal construir la jerarquia de matrius acumulatives:
  $$M_{\text{Planeta}} = M_{\text{TG}} \times T(\text{posició orbital}) \times R(\text{inclinació axial}) \times R(\text{rotació pròpia})$$
  $$M_{\text{Lluna}} = M_{\text{Planeta}} \times T(\text{òrbita lunar}) \times R(\text{rotació lunar})$$

### 4.2. Modificacions a [`main.cpp`](../Entorn-GLFW-GL4.6-ImGui/EntornVGI/main.cpp)
* **Configuració inicial a `InitGL()`:**
  * Fixar la càmera inicial orbital a una distància que permeti contemplar el Sol i els planetes interiors.
  * Activar el Skybox espacial per defecte (`SkyBoxCube = true`).
  * Fixar el color de fons a negre espacial $(0.0, 0.0, 0.0, 1.0)$.
  * Carregar per defecte el shader `phong_shdrML` com a shader principal.
  * Inicialitzar la font de llum `llumGL[0]` a la posició central $(0,0,0)$ amb intensitat blanca pura, representant la llum emesa pel Sol.
* **Control del Temps a `OnTimer()` (Cas d'ús 2):**
  * La variable `time` i la funció `OnTimer()` han de governar el temps astronòmic de la simulació:
    * Variable de velocitat: `velocitatTemps` (permetent valors com $0\times$ [pausa], $1\times$ [temps real], $100\times$, $10000\times$, etc.).
    * Avanç del temps: actualització de la data/hora simulada (dia, mes, any, hora, minut, segon) en funció del $\Delta t$ transcorregut.
    * Recàlcul de les posicions i rotacions dels astres segons el nou instant de temps.
* **Interfície ImGui (`draw_Menu_ImGui()` / `ShowEntornVGIWindow()`):**
  Substituir els desplegables docents pels panells específics de l'encàrrec de l'IEEC:
  1. **Panell d'Efemèrides i Temps (Cas d'ús 1 i 2):** Selector de data (dia/mes/any) i hora (hora/minut/segon), botons de Reproducció/Pausa, barres lliscants (*sliders*) de velocitat temporal (*timelapse* ràpid, lent, temps real).
  2. **Panell de Càmeres i Navegació (Cas d'ús 3):** Selector de cos diana (Sol, Mercuri, Venus, Terra, Mart, Júpiter, Saturn, Urà, Neptú, Plutó i Lluna). Aquest canvi de focus orbital funciona tant des del menú com fent clic directe sobre el planeta a la finestra gràfica (mitjançant *ray casting* o selecció interactiva). La càmera lliure de nau espacial es manté com a mode addicional en reserva.
  3. **Panell de Visualització i Escales (Cas d'ús 4):** Commutador entre Escala Didàctica/Intuïtiva (prioritària per a la comprensió de l'usuari) i Escala Real (proporcions físiques autèntiques), caselles per activar/desactivar el traçat d'òrbites, etiquetes identificatives i marcadors visuals per a planetes petits.
  4. **Panell de Cossos Menors (Cas d'ús 5 i 6):** Controls per activar el cinturó d'asteroides, visualitzar els punts de Lagrange i activar el mode de simulació física/col·lisions als anells.

### 4.3. Modificacions a [`visualitzacio.cpp`](../Entorn-GLFW-GL4.6-ImGui/EntornVGI/visualitzacio.cpp)
* **Model d'Il·luminació Solar a `Iluminacio()`:**
  * La Llum 0 ha de residir a l'origen $(0,0,0)$ en coordenades món (`llumGL[0].posicio = (0,0,0,1)`).
  * Cal configurar el paràmetre `fixedLight = true` al shader perquè la font de llum romangui estàtica al Sol i no es mogui en girar la càmera.
* **Càmera Orbital Centrable (`Vista_Esferica`):**
  * Modificar `Vista_Esferica` perquè no miri exclusivament a l'origen $(0,0,0)$, sinó que rebi un punt d'atenció objectiu variable $(T_x, T_y, T_z)$ corresponent a la posició actual del planeta o satèl·lit seleccionat.

### 4.4. Modificacions a Shaders ([`shaders/phong_shdrML.frag`](../Entorn-GLFW-GL4.6-ImGui/EntornVGI/shaders/phong_shdrML.frag))
* **Material Emissiu per al Sol:**
  * Donar suport a un component emissiu actiu que mostri la textura solar amb la màxima brillantor sense que el Sol quedi afectat per càlculs d'ombra exterior.

---

## 5. Què cal CREAR de Nou (Arquitectura de Nous Mòduls)

Per aconseguir un disseny modular, mantenible i professional en C++, es recomana crear les següents classes dedicades:

```
+-----------------------------------------------------------------------------------+
|                            ESCENARI (SISTEMA SOLAR)                               |
|                           Classe: CEscenari                                       |
|  - Data/Hora actual (Efemèrides JPL / SOFA)                                       |
|  - Escala activa (REAL vs. INTUÏTIVA)                                             |
|  - Arbre de Cossos Celestes (arrel: Sol)                                          |
|  - Càmera orbital activa i objectiu diana                                         |
+-----------------------------------------+-----------------------------------------+
                                          |
                                          v conté
+-----------------------------------------------------------------------------------+
|                                  COS CELESTE                                      |
|                             Classe: CCosCeleste                                   |
|  - Nom, radi físic, radi visual                                                   |
|  - Paràmetres keplerians (semieix major a, excentricitat e, període T)            |
|  - Inclinació axial, velocitat de rotació pròpia                                  |
|  - Textura ID (OpenGL)                                                            |
|  - Vector de satèl·lits fills (vector<CCosCeleste*>)                              |
|  - Mètodes: Actualitzar(temps), Renderitzar(shaderID, escala)                     |
+-----------------------------------------+-----------------------------------------+
                                          |
        +---------------------------------+---------------------------------+
        |                                 |                                 |
        v utilitza                        v utilitza                        v utilitza
+-----------------------+       +-----------------------+       +-----------------------+
|      EFEMÈRIDES       |       |        ÒRBITA         |       |    CAMERA MANAGER     |
|  Classe: CEfemerides  |       |    Classe: COrbita    |       | Classe: CCameraManager|
|  - Càlcul posicions   |       |  - VAO/VBO de línia   |       |  - Càmera Sol / Cos   |
|    JPL DE440 / SOFA   |       |    GL_LINE_LOOP       |       |  - Transició suau     |
|  - Temps julià (JD)   |       |  - Renderitzat de     |       |  - Mode Nau (reserva) |
|  - Data/Hora exacta   |       |    trajectòries       |       |  - Vistes ISS/Voyager |
+-----------------------+       +-----------------------+       +-----------------------+
```

### 5.1. Classe `CCosCeleste` (`CosCeleste.h` / `CosCeleste.cpp`)
Encapsula l'estat i el comportament individual de cada astre:
* **Atributs:**
  * Dades físiques: `nom`, `radiReal`, `radiVisual`, `distanciaSolReal`, `distanciaSolVisual`.
  * Paràmetres orbitals keplerians: semieix major ($a$), excentricitat ($e$), inclinació orbital ($i$), longitud del node ascendent ($\Omega$), argument del periheli ($\omega$), anomalia mitjana a l'època ($M_0$).
  * Dades de rotació: període de rotació pròpia, inclinació de l'eix de rotació (*axial tilt*).
  * Gràfics: textura principal `texDiffuseID`, textura nocturna opcional `texNightID`, anell opcional (`hasRings`, dimensions de l'anell).
  * Jerarquia: `std::vector<CCosCeleste*> satelits;` (permetent que la Terra tingui la Lluna, Júpiter tingui els satèl·lits galileans, etc.).
* **Mètodes clau:**
  * `void Actualitzar(double tempsSimulat);`: Calcula la nova anomalia excèntrica, l'anomalia veritable i la posició espacial $(x, y, z)$.
  * `void Dibuixar(GLuint shaderID, bool escalaReal);`: Configura la matriu `modelMatrix` amb GLM i crida a `draw_TriEBO_Object(GLU_SPHERE)`.

### 5.2. Classe `CEscenari` (`Escenari.h` / `Escenari.cpp`)
Nucli principal del sistema que engloba els planetes, satèl·lits, càmeres i elements de la imatge:
* Inicialitza l'arbre de tots els cossos celestes: Sol, Mercuri, Venus, Terra (i Lluna), Mart (i Fobos/Deimos), Júpiter (i satèl·lits galileans), Saturn (amb anells i Tità), Urà, Neptú i Plutó.
* Administra el mode d'escala: `ESCALA_INTUITIVA` (prioritària per al desenvolupament inicial) vs. `ESCALA_REAL`.
* Propaga les crides d'actualització temporal i renderitzat per tot l'arbre jeràrquic.
* Centralitza la gestió d'interacció i selecció d'astres (per menú o per clic).

### 5.3. Mòdul de Càlcul d'Efemèrides (`Efemerides.h` / `Efemerides.cpp`) – Cas d'ús 1
* Converteix la data i hora gregoriana (dia, mes, any, hora, minut, segon) a **Data Juliana (*Julian Date* - JD)**.
* Utilitza els models i dades d'efemèrides de la NASA (models simplificats JPL DE440 i SOFA) per obtenir les posicions i velocitats heliocèntriques dels planetes amb gran precisió física.

### 5.4. Classe de Traçat d'Òrbites (`Orbita.h` / `Orbita.cpp`) – Cas d'ús 4
* Genera dinàmicament un `VAO` i `VBO` que conté la trajectòria el·líptica mostrejada en 360 segments lineals:
  $$r(\theta) = \frac{a(1 - e^2)}{1 + e \cos(\theta)}$$
* Es renderitza mitjançant `glDrawArrays(GL_LINE_LOOP, 0, punts)` utilitzant un color tènue diferenciat per a cada planeta, amb opció d'activar o desactivar la línia per a una visualització més natural estil SpaceEngine.

### 5.5. Gestor de Càmeres Avançat (`CameraManager.h` / `CameraManager.cpp`) – Cas d'ús 3
* Centralitza el control de càmeres amb transicions suaus inspirades en OpenSpace i SpaceEngine:
  1. **Càmera Orbital Global (Focus al Sol):** Vista inicial que permet observar el Sistema Solar complet i les òrbites girant al voltant del Sol.
  2. **Càmera Orbital Centrada en Planeta/Satèl·lit:** En fer clic sobre qualsevol planeta o triar-lo a la interfície, la càmera canvia el focus per orbitar al seu voltant, desplaçant-se conjuntament amb el planeta al llarg de la seva òrbita.
  3. **Càmeres de Satèl·lits Artificials:** Punts de vista específics com la sonda Voyager o l'Estació Espacial Internacional (ISS).
  4. **Càmera Nau Espacial (Mode en Reserva):** Model de vol lliure 6-DOF (*Six Degrees of Freedom*) controlat per teclat/gamepad en primera i tercera persona, reservat per a una fase posterior un cop consolidades les càmeres orbitals.

### 5.6. Mòdul del Cinturó d'Asteroides (`CinturoAsteroides.h` / `CinturoAsteroides.cpp`) – Cas d'ús 5
* Genera milers d'asteroides amb coordenades semi-aleatòries distribuïdes entre les òrbites de Mart i Júpiter.
* **Optimització OpenGL 4.6:** Per aconseguir una alta taxa de fotogrames per segon (60+ FPS) sense saturar la CPU, s'ha d'implementar renderitzat instanciat mitjançant `glDrawElementsInstanced`, enviant un únic buffer de matrius de transformació a la GPU.

### 5.7. Mòdul de Punts de Lagrange (`PuntsLagrange.h` / `PuntsLagrange.cpp`) – Cas d'ús 5
* Càlcul geomètric dels punts d'equilibri gravitatori $L_1, L_2, L_3, L_4, L_5$ del sistema Sol-Terra o Sol-Júpiter.
* Dibuix d'indicadors visuals (creus o marcadors lluminosos) per a cadascun dels punts.

### 5.8. Conjunt de Textures Planetàries en Alta Resolució (`textures/`)
Cal incorporar les imatges esfèriques 2:1 (projecció equirectangular) de resolució adequada (2K o 4K):
* `sun.jpg`, `mercury.jpg`, `venus_surface.jpg`, `venus_atmosphere.jpg`, `earth_day.jpg`, `earth_night.jpg`, `earth_clouds.jpg`, `moon.jpg`, `mars.jpg`, `jupiter.jpg`, `saturn.jpg`, `saturn_rings.png` (amb canal alfa de transparència), `uranus.jpg`, `neptune.jpg`, `pluto.jpg`.
* Textura cúbica del fons d'estrelles per al Skybox (`space_skybox/`).

---

## 6. Matriu de Traçabilitat: Casos d'Ús vs. Fitxers i Components

Aquesta taula permet verificar ràpidament quin codi cal tocar per a cadascun dels casos d'ús establerts a [`Casos d'ús_ Simulador del Sistema Solar.md`](./Casos%20d'%C3%BAs_%20Simulador%20del%20Sistema%20Solar.md):

| Cas d'Ús | Fitxers Existents a Modificar | Nous Components a Crear | Funcionalitat Concreta |
|---|---|---|---|
| **Cas 1: Consulta Data/Hora Específica** | [`main.cpp`](../Entorn-GLFW-GL4.6-ImGui/EntornVGI/main.cpp) (ImGui), [`escena.cpp`](../Entorn-GLFW-GL4.6-ImGui/EntornVGI/escena.cpp) | `Efemerides.h/.cpp`, `CosCeleste.h/.cpp` | Formularis d'any/mes/dia/hora a ImGui, càlcul de Data Juliana amb dades NASA JPL/SOFA i posicionament instantani de tots els cossos. |
| **Cas 2: Control de Velocitat (Timelapse)** | [`main.cpp`](../Entorn-GLFW-GL4.6-ImGui/EntornVGI/main.cpp) (`OnTimer`, ImGui) | `Escenari.h/.cpp` | Factors multiplicadors de temps, pausa, reproducció inversa i actualització contínua d'anomalia orbital. |
| **Cas 3: Càmeres i Perspectives** | [`visualitzacio.cpp`](../Entorn-GLFW-GL4.6-ImGui/EntornVGI/visualitzacio.cpp), [`main.cpp`](../Entorn-GLFW-GL4.6-ImGui/EntornVGI/main.cpp) | `CameraManager.h/.cpp` | Càmera orbital global al Sol, transició de focus orbital a planeta/satèl·lit per clic directe o menú, i càmera nau en reserva. |
| **Cas 4: Escales, Etiquetes i Òrbites** | [`escena.cpp`](../Entorn-GLFW-GL4.6-ImGui/EntornVGI/escena.cpp), [`main.cpp`](../Entorn-GLFW-GL4.6-ImGui/EntornVGI/main.cpp) (ImGui) | `Orbita.h/.cpp`, `CosCeleste.h/.cpp` | Commutador Escala Intuïtiva vs. Real, generació de `GL_LINE_LOOP` per a les trajectòries orbitals estil SpaceEngine, indicadors visuals per a planetes petits. |
| **Cas 5: Cossos Menors i Lagrange** | [`escena.cpp`](../Entorn-GLFW-GL4.6-ImGui/EntornVGI/escena.cpp) | `CinturoAsteroides.h/.cpp`, `PuntsLagrange.h/.cpp` | Renderitzat instanciat de milers d'asteroides entre Mart i Júpiter, anells de Saturn i marcadors $L_1\dots L_5$. |
| **Cas 6: Interaccions Físiques i Col·lisions** | [`main.cpp`](../Entorn-GLFW-GL4.6-ImGui/EntornVGI/main.cpp) (`OnTimer`) | `MotorFisiques.h/.cpp` | Simulador gravitatori opcional n-cossos o detecció de col·lisions de partícules als anells de Saturn. |

---

## 7. Pla de Treball en Fases (Roadmap)

Per avançar ordenadament sense trencar la compilació del projecte, es recomana seguir el següent calendari de fases:

```
+-----------------------------------------------------------------------------------+
|  FASE 1: Neteja de l'Entorn i Base Visual                                         |
|  - Netejar l'entorn d'objectes de laboratori (Tie, tetera, corbes docents)        |
|  - Activar Skybox espacial amb mapa de cubs d'estrelles 4K/2K                     |
|  - Ubicar la Llum 0 a l'origen com a font solar central omnidireccional           |
+-----------------------------------------+-----------------------------------------+
                                          |
                                          v
+-----------------------------------------------------------------------------------+
|  FASE 2: Escenari Intuïtiu Funcional (Objectiu Prioritari)                        |
|  - Implementar arquitectura de classes CEscenari i CCosCeleste                    |
|  - Carregar i renderitzar el Sol i els 9 planetes amb textures esfèriques         |
|  - Model d'escala intuïtiva (no a escala estricta) per a fàcil comprensió         |
|  - Anells bàsics de Saturn i traçat de línies d'òrbita amb COrbita                |
|  - Càmera orbital al Sol i canvi de focus per clic o selecció de planeta          |
+-----------------------------------------+-----------------------------------------+
                                          |
                                          v
+-----------------------------------------------------------------------------------+
|  FASE 3: Escena a Escala Real i Efemèrides de Precisió                            |
|  - Implementar el mode d'Escala Real amb marcadors visuals per a planetes menors  |
|  - Implementar el mòdul CEfemerides basat en dades NASA (JPL DE440 / SOFA)        |
|  - Suport de consulta temporal exacta per data (any/mes/dia) i hora (h/min/s)     |
+-----------------------------------------+-----------------------------------------+
                                          |
                                          v
+-----------------------------------------------------------------------------------+
|  FASE 4: Cossos Menors, Satèl·lits i Interactivitat Avançada                      |
|  - Incorporar satèl·lits naturals (Lluna, satèl·lits galileans, Tità)             |
|  - Cinturó d'asteroides entre Mart i Júpiter amb Instanced Rendering              |
|  - Càlcul i marcadors dels punts de Lagrange L1-L5                                |
|  - Controls complets de timelapse (ràpid, lent, pausa, temps real)                |
+-----------------------------------------+-----------------------------------------+
                                          |
                                          v
+-----------------------------------------------------------------------------------+
|  FASE 5: Mòduls Addicionals en Reserva                                            |
|  - Càmera lliure tipus nau espacial (1a i 3a persona per teclat o comandament)    |
|  - Punts de vista de satèl·lits artificials (ISS i Voyager) i observatori UAB     |
|  - Simulació física/gravitatòria dinàmica o col·lisions als anells                |
+-----------------------------------------+-----------------------------------------+
```
