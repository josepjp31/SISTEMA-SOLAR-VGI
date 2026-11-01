# Integració de la Branca `interficies-entorn` amb `developTy` i Millores del Simulador

## 1. Objectiu del Treball
Aquest document detalla la unió i integració arquitectural entre el sistema de càrrega, renderitzat i textures centralitzat a la classe `Escenari` (branca `developTy`) i el subsistema d'interfície gràfica d'usuari desenvolupat amb Dear ImGui (`InterficieUI`, branca `interficies-entorn`). A més, s'han consolidat el control de càmeres, el model d'il·luminació solar, la representació de la Skybox i el control de velocitat de la simulació.

---

## 2. Fases d'Integració i Millores Tècniques

### 2.1. Adaptació de `Escenari::creaEscenari(GLuint& shader_programID)`
- S'ha actualitzat la signatura del mètode `creaEscenari` perquè rebi per referència el programa de shader actiu (`GLuint& shader_programID`), garantint que els VAO (`GLU_SPHERE`, `GLUT_TORUS`) i les 9 textures dels astres es vinculin al shader Phong en ús.
- Dins dels mètodes de la interfície (`InterficieUI::DibuixarSelectorInicialModes` i `InterficieUI::DibuixarBarraMenuSuperior`), en activar o commutar el mode de simulació es fa la crida a `currentEscenari->creaEscenari(shader_programID)` i s'activa el flag de sincronització `configuracioInicialPendent`.

### 2.2. Aïllament del Fons en el Selector Inicial
- En iniciar l'aplicació, el selector modal de dos modes es presenta sobre un fons espacial fosc sense carregar ni dibuixar el sistema solar ni la Skybox abans que l'usuari triï un mode de funcionament.
- A `OnPaint()` (`main.cpp`), si `g_EstatSimulador.mostrarSelectorModes == true`, es realitza únicament la neteja de buffers (`glClear`) amb el color de fons `#080A12`.
- Es mantenen bloquejades les entrades d'usuari a `OnKeyDown`, `OnMouseButton`, `OnMouseMove` i `OnMouseWheel` mentre el selector d'escenari està actiu per evitar desplaçaments accidentals de la vista.

### 2.3. Sistema de Càmeres i Enfocament d'Astres
- **Sincronització dels modes de càmera:**
  - `ModeCamera::ORBITAL_CENTRADOR`: Activa la càmera esfèrica (`CAM_ESFERICA`) amb rotació lliure tant en l'angle horitzontal (`OPV.beta`) com en el vertical (`OPV.alfa`) mitjançant el ratolí, permetent ajustar la distància amb el rodet del ratolí.
  - `ModeCamera::FIXA_INCLINADA_25`: Configura una inclinació vertical constant de 25 graus (`OPV.alfa = 25.0`), permetent exclusivament la rotació azimutal al voltant de l'eix de referència.
  - `ModeCamera::GLOBAL_SISTEMA`: Càmera esfèrica zenital d'alta elevació (`OPV.R = 550.0`, `OPV.alfa = 65.0`, `OPV.beta = 0.0`) orientada al centre del sistema solar.
  - `ModeCamera::COET_PRIMERA_PERSONA` (Càmera Lliure): Connectada directament amb el mode de navegació en primera persona (`CAM_NAVEGA`). S'han corregit les equacions d'orientació trigonomètrica a `OnMouseMove` i `Teclat_Navega`, afegint suport per a navegació contínua amb les tecles `W`, `A`, `S`, `D`, `Q`, `E` i les fletxes de teclat.
- **Centrat dinàmic sobre els planetes (`cosEnfocatId`):**
  - Quan es selecciona qualsevol dels 9 cossos astronòmics (`0: Sol` a `8: Neptú`), es calcula la posició orbital $(x, y, 0)$ en coordenades de món de l'astre actiu i s'aplica una translació oposada a la matriu de vista (`glm::translate(MatriuVista, glm::vec3(-px, -py, 0.0f))`), fent que la càmera orbiti de manera concèntrica al voltant del planeta elegit. Els límits de zoom (`minR`, `maxR`) s'adapten automàticament al radi del cos celeste.

### 2.4. Model d'Il·luminació Solar Directa i Correcció de la Skybox
- **Configuració del sistema d'il·luminació:**
  - Shader Phong amb ombrejat suau (`ilumina = SUAU`).
  - Llum puntual fixa (`ifixe = true`) situada a l'origen de coordenades `(0, 0, 0, 1)`, corresponent al centre del Sol, amb atenuació quadràtica $a = 0.00004$. La resta de fonts de llum (`llumGL[1..7]`) romanen desactivades.
  - Paràmetres de material configurats amb emissió activa (`sw_material[0] = true`) i reflexió difusa (`sw_material[2] = true`).
  - A `Escenari::dibuixaEscenari()`, s'envia l'uniform `sw_intensity` (`bvec4`) per activar l'emissió pròpia exclusivament per a la malla del Sol (`i == 0`), mentre que els planetes (`i > 0`) reben la il·luminació difusa procedent de l'estrella central generant el contrast de cara il·luminada i cara d'ombra.
- **Correcció del renderitzat de la Skybox:**
  - A `InitGL()` (`main.cpp`), s'ha reordenat la inicialització executant `initVAOList()` abans de carregar el VAO de la Skybox (`loadCubeSkybox_VAO()`), evitant que el seu identificador quedi esborrat a zero.
  - A `dibuixa_Skybox()` (`escena.cpp`), s'elimina la component de translació de la matriu de vista (`glm::mat4(glm::mat3(MatriuVista))`) i s'associa explícitament l'uniform `"skybox"` a la unitat de textura `GL_TEXTURE0`, assegurant que el fons estrellat envolti l'observador a distància infinita.

### 2.5. Control Temporal (Timelapse) i Ajust de Velocitats
- A `Escenari::updateVariables()`, l'actualització orbital es calcula en funció del temps diferencial (`dt`), la velocitat angular de cada astre, el sentit temporal (`direccioTemps`) i l'estat de pausa (`enPausa`).
- En el mode didàctic s'han establert velocitats angulars orbitals suaus i proporcionals a la distància al Sol, facilitant la visualització simultània dels planetes interiors i exteriors.
