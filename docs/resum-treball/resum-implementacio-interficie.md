# Resum d'Implementació: Subsistema d'Interfície d'Usuari (Dear ImGui)

Aquest document recull les tasques desenvolupades per a la creació i integració del subsistema d'interfície d'usuari del **Simulador Interactiu del Sistema Solar**, basat en la guia d'especificacions tècniques del projecte.

---

## 1. Objectiu del Treball
Desenvolupar una arquitectura modular d'interfície gràfica d'usuari (GUI) en mode immediat utilitzant **Dear ImGui (v1.89+)**, **C++**, **OpenGL 4.6** i **GLFW 3.4**, desacoblant la lògica de presentació del fitxer principal `main.cpp` i proporcionant un flux d'inici intuitiu basat en la selecció de modes de representació.

---

## 2. Fitxers Desenvolupats i Modificats

* **`EntornVGI/InterficieUI.h`** *(Nou)*: Declaració de les estructures d'estat de simulació, tipus enumerats, paràmetres de temps, càmera, visualització i mètodes estàtics de la classe `InterficieUI`.
* **`EntornVGI/InterficieUI.cpp`** *(Nou)*: Implementació procedural de tots els components gràfics: estils, selector inicial de modes escalable, menús dinàmics, panell de control temporal i widgets auxiliars.
* **`EntornVGI/main.cpp`** *(Modificat)*: Integració del mòdul al bucle de renderitzat, sincronització de càmera i temps, bloqueig d'events de fons i gestió del cicle de vida dels escenaris.
* **`EntornVGI/EntornVGI.vcxproj` & `EntornVGI.vcxproj.filters`** *(Modificats)*: Registre dels nous fitxers font i capçalera a la configuració del projecte Visual Studio.

---

## 3. Tasques Implementades

### 3.1. Arquitectura i Gestió de l'Estat Global
* Definició de les enumeracions de control:
  * `ModeSimulacio`: Distinció entre `INTUITIU` (mode didàctic) i `ESCALA_REAL` (mode científic).
  * `ModeCamera`: Modes `ORBITAL_CENTRADOR`, `FIXA_INCLINADA_25`, `GLOBAL_SISTEMA` i `COET_PRIMERA_PERSONA`.
* Disseny d'estructures modulars:
  * `DataHoraSimulacio`: Gestió d'anys, mesos, dies i hores amb suport per a anys de traspàs i validació de rangs.
  * `EstatTempsSimulacio`: Control de timelapse (rang d'1 min/s a 20 dies/s), sentit de simulació (+1 endavant / -1 enrere) i acumulació temporal.
  * `OpcionsVisualitzacio`: Commutació de capes gràfiques (òrbites, etiquetes, halos, skybox, anells de Saturn, eixos).
  * `EstatSimulacio`: Estructura central que unifica l'estat complet del simulador.

### 3.2. Selector Inicial de Modes (Modal Central amb Escalat Adaptatiu, Fons Fosc i Text Sense Talls)
* Implementació d'un quadre de diàleg modal centrat en iniciar l'aplicació (`g_EstatSimulador.mostrarSelectorModes = true`).
* **Aïllament visual inicial:** Durant la fase de selecció inicial, la barra de menú superior i els panells romanen completament ocults; exclusivament es mostren les dues targetes de selecció de mode sobre el fons enfosquit (*backdrop*). Un cop triat el mode, el selector es tanca i es despleguen la barra de menú superior i els panells corresponents.
* **Preservació de l'estil fosc i contrast visual òptim:**
  * S'ha eliminat la commutació automàtica a `ImGui::StyleColorsLight()` present a `OnPaint`, garantint la preservació del tema fosc espacial.
  * Fons del modal enfosquit sòlidament (`IM_COL32(14, 20, 32, 252)`) amb marc i vora arrodonida.
  * Títols i subtítols en blanc pur i blau suau d'alta visibilitat sobre el fons fosc.
  * Textos descriptius i vinyetes interiors configurats explícitament amb colors clars d'alt contrast (`IM_COL32(235, 245, 255, 255)` i `IM_COL32(255, 245, 230, 255)`), eliminant el text fosc il·legible.
  * Text dels botons d'inici en blanc brillant d'alta llegibilitat sobre fons blau o taronja.
* **Disseny responsive / escalat adaptatiu:**
  * Càlcul dinàmic del factor d'escala a partir de la resolució activa de la finestra (`WorkSize.x`, `WorkSize.y`).
  * En activar la pantalla completa o resolucions altes (1080p, 1440p, 4K), el panell s'engrandeix proporcionalment.
  * Escalat de text i elements mitjançant `ImGui::SetWindowFontScale(factorEscala)` per garantir llegibilitat i comoditat de selecció.
* **Salt de línia automàtic ("enter") i prevenció de talls de text:**
  * Implementació de la funció auxiliar `DibuixarPuntLlista(text)` utilitzant `ImGui::TextWrapped` amb vinyeta (`ImGui::Bullet`), fent que qualsevol text que superi l'amplada disponible de la targeta salti automàticament a la següent línia ("faci un enter") sense truncar ni perdre cap paraula.
  * Amplada de botons dimensionada amb un marge generós (`ampleBoto`) basat en el text més llarg (`CalcTextSize("INICIAR MODE A ESCALA REAL")`), impedint desbordaments en qualsevol mida de finestra.
  * Posicionament resilient del botó de selecció (fixat a la part inferior si hi ha alçada suficient, o a continuació del contingut si l'espai vertical és just), evitant solapaments amb el text superior.
* **Enfosquiment de fons (*Backdrop*):**
  * Superposició translúcida sobre el viewport 3D per centrar l'atenció i evidenciar l'estat modal.
* **Targeta 1 - Mode Didàctic / Intuïtiu:**
  * Targeta visual en tons blaus i vora cian amb botó `INICIAR MODE DIDÀCTIC` (activa `ModeSimulacio::INTUITIU`, estableix la càmera orbital a 25° i assigna `escenaris[0]`).
* **Targeta 2 - Mode a Escala Real (Científic):**
  * Targeta visual en tons càlids i vora ambre amb botó `INICIAR MODE A ESCALA REAL` (activa `ModeSimulacio::ESCALA_REAL`, desplega el panell de timelapse inferior dret i assigna `escenaris[1]`).

### 3.3. Barra de Menú Superior Dinàmica (Escalat Adaptatiu en Pantalla Completa i Mida Inicial Àmplia)
* **Ampliació de la mida inicial de la finestra:**
  * Configuració de la resolució inicial de la finestra a **1280x720** (abans 640x480) tant a les variables de control (`w`, `h`, `w_old`, `h_old`) com a `glfwCreateWindow`, evitant que els ítems de la barra superior quedin comprimits o tallats en iniciar l'entorn.
* **Escalat segons el tamany de finestra:**
  * En finestra estàndard: manté dimensions equilibrades i confortables.
  * En pantalla completa / resolucions amples (>= 1920px): l'alçada i separació de la barra es dupliquen progressivament (padding vertical de 5px a 14px, horitzontal de 10px a 25px) amb escala de font de fins a 1.44x.
  * Text de la barra amb color d'alta visibilitat (`IM_COL32(235, 245, 255, 255)`).
  * Els desplegables (`BeginMenu`) hereten l'escala de font, fent que totes les opcions siguin clarament visibles i fàcils de prémer.
* **Filtratge per mode actiu:**
  * **En Mode Didàctic:** Menús adaptats (`Escenari`, `Planetes & Focus`, `Càmera`, `Visualització`, `Ajuda`).
  * **En Mode Escala Real:** Menús tècnics (`Escenari`, `Efemèrides & Temps`, `Origen & Càmeres`, `Visualització Realista`, `Dades Científiques`).
* **Banda dreta de la barra sense solapaments:**
  * Càlcul precís de l'espai per a l'indicador visual (`[MODE DIDÀCTIC / INTUÏTIU]` / `[MODE A ESCALA REAL]`), el botó d'accés ràpid `Canviar` per reobrir el selector de modes i el comptador de FPS, impedint que el text es talli o es desplaci fora de pantalla.

### 3.4. Panell de Control Temporal i Efemèrides (Cantonada Inferior Dreta)
* Finestra fixa a la cantonada inferior dreta visible exclusivament en mode a escala real.
* Controls de calendari UTC (Any, Mes, Dia i popup interactiu de calendari mensual).
* Controls horaris (Hora, Minut, Segon) i botons directes ("Ara", "J2000").
* Controls de reproducció: Pausa/Reprendre, sentit del temps (<< Enrere, Endavant >>), passos pas a pas (- / +).
* Lliscador logarítmic de velocitat simulat entre 1 min/s (60 s/s) i 20 dies/s (1.728.000 s/s) amb indicadors adaptatius d'unitats.

### 3.5. Integració amb el Bucle Principal i Gestió d'Events (`main.cpp`)
* **Inicialització d'estil:** Configuració del tema fosc (`InterficieUI::InicialitzarEstil()`).
* **Renderitzat ordenat:** Execució d'ImGui després de `glClear` i abans del swap de buffers (`glfwSwapBuffers`), garantint la correcta superposició sobre l'escena 3D.
* **Bloqueig d'interaccions d'escena:** Mentre el selector de modes està obert, es bloquegen les accions de ratolí, rotacions de càmera orbital i zoom per evitar modificacions no desitjades a la vista de fons.
* **Sincronització temporal:** Actualització de l'acumulador temporal a `OnTimer()` segons el delta de temps (`dt`) i la velocitat de timelapse.
* **Gestió del cicle de vida:** Assignació dinàmica i alliberament net de la memòria i VAOs de la llista d'escenaris abans de la destrucció del context OpenGL.

---

## 4. Estat de Verificació
* Compilació satisfactòria en Visual Studio 2026 (Plataforma `x64`, Configuració `Debug`), amb 0 advertències i 0 errors.
* Execució validada: obertura correcta de la finestra, renderitzat del selector inicial amb escalat adaptatiu, aïllament visual inicial, escalat del menú superior i persistència dels estats de la simulació.
