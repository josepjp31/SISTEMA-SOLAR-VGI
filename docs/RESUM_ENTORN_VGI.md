# Resum i Esquema Tècnic de l'Entorn Base (EntornVGI)

> **Assignatura:** Visualització Gràfica Interactiva (VGI) – Grau en Enginyeria Informàtica (Escola d'Enginyeria, UAB)  
> **Tecnologies:** C++, OpenGL 4.6 (Core Profile), GLFW 3.4, GLEW, Dear ImGui, GLM, SOIL2, NFD  
> **Autors originals del marc docent:** Ferran Poveda, Marc Vivet, Carme Julià, Débora Gil, Enric Martí (Setembre 2026)  
> **Data d'anàlisi:** Octubre 2026  

---

## 1. Visió General de l'Arquitectura

L'entorn base **EntornVGI** és una aplicació d'escriptori monofinestra dissenyada per a l'aprenentatge i desenvolupament d'aplicacions de gràfics 3D interactius moderns sobre **OpenGL 4.6 Core Profile**.

A diferència dels entorns clàssics basats en el pipeline de funció fixa (`glBegin`/`glEnd`, matrius internes de la pila d'OpenGL), aquest entorn utilitza:
* **Objectes de memòria a la GPU:** `VAO` (*Vertex Array Objects*), `VBO` (*Vertex Buffer Objects*) i `EBO` (*Element Buffer Objects* / indexació de triangles i línies).
* **Shaders programables en GLSL 4.6:** Per al processament de geometria, il·luminació per fragment i mostreig de textures.
* **Àlgebra lineal amb GLM:** Totes les matrius de projecció, vista i model es calculen en CPU mitjançant la llibreria GLM i s'envien com a variables *uniform* als shaders.
* **Finestres i esdeveniments amb GLFW 3.4:** Gestió multiplataforma de finestra, context OpenGL modern i gestió de crides de retorn (*callbacks*) d'entrada (teclat, ratolí, redimensionament).
* **Interfície Gràfica Immediata amb Dear ImGui:** Menús contextuals, barres d'estat i controls de paràmetres interactius renderitzats directament sobre el buffer de color d'OpenGL.

```
+---------------------------------------------------------------------------------------+
|                                  MAIN LOOP                                            |
|  glfwPollEvents() -> OnTimer() -> draw_Menu_ImGui() -> OnPaint() -> glfwSwapBuffers() |
+-----------------------------------------+---------------------------------------------+
                                          |
        +---------------------------------+---------------------------------+
        |                                                                   |
        v                                                                   v
+-----------------------+                                       +-----------------------+
|  VISUALITZACIÓ & CAM  |                                       |   ESCENA & GEOMETRIA  |
|  visualitzacio.cpp    |                                       |   escena.cpp          |
|  - Projecció (P, O)   |                                       |   - dibuixa_EscenaGL  |
|  - Càmeres (Esfèrica, |                                       |   - dibuixa_Skybox    |
|    Navega, Geode)     |                                       |   - dibuixa_Eixos     |
|  - Llums (uniforms)   |                                       |   - Primitives        |
|  - Textures (SOIL2)   |                                       |                       |
+-----------+-----------+                                       +-----------+-----------+
            |                                                               |
            +-------------------------------+-------------------------------+
                                            |
                                            v
                                +-----------------------+
                                |      GPU / SHADERS    |
                                |  shader.cpp / GLSL    |
                                |  - phong_shdrML       |
                                |  - gouraud_shdrML     |
                                |  - skybox             |
                                |  - eixos              |
                                +-----------------------+
```

---

## 2. Inventari i Esquema Estructural de Fitxers

A continuació es detalla cada fitxer font, capçalera i directori inclòs a l'entorn base:

| Fitxer / Mòdul | Responsabilitat Principal | Components i Funcions Clau |
|---|---|---|
| [`main.cpp`](../Entorn-GLFW-GL4.6-ImGui/EntornVGI/main.cpp) | Punt d'entrada (`main()`), bucle d'esdeveniments, crides de retorn de GLFW, gestió d'estats globals i menús Dear ImGui. | `main()`, `InitGL()`, `OnPaint()`, `configura_Escena()`, `dibuixa_Escena()`, `OnTimer()`, `draw_Menu_ImGui()`, `ShowEntornVGIWindow()`, crides de retorn (`OnKeyDown`, `OnMouseMove`, `OnMouseButton`, etc.). |
| [`main.h`](../Entorn-GLFW-GL4.6-ImGui/EntornVGI/main.h) | Declaració de totes les variables globals d'estat de l'aplicació, prototips de funcions i accions de menú. | Variables d'estat de càmera (`OPV`, `opvN`, `camera`), projecció (`projeccio`), llums (`llumGL[]`), selecció d'objecte (`objecte`), matrius globals (`ProjectionMatrix`, `ViewMatrix`, `GTMatrix`). |
| [`escena.cpp`](../Entorn-GLFW-GL4.6-ImGui/EntornVGI/escena.cpp) | Renderitzat concret dels objectes 3D a la GPU i enviament de matrius Model als shaders. | `dibuixa_EscenaGL()`, `dibuixa()`, `dibuixa_Skybox()`, `dibuixa_Eixos()`, primitives de prova (`arc()`, `tie()`, `Alas()`, `Motor()`, etc.). |
| [`escena.h`](../Entorn-GLFW-GL4.6-ImGui/EntornVGI/escena.h) | Interfície pública de funcions de dibuix de l'escena. | Declaracions de `dibuixa_EscenaGL`, `dibuixa_Skybox`, `dibuixa_Eixos`, `loadSea_VAO`. |
| [`visualitzacio.cpp`](../Entorn-GLFW-GL4.6-ImGui/EntornVGI/visualitzacio.cpp) | Càlcul de càmeres (*LookAt*), projeccions (*Perspective* / *Ortho*), gestió de llums a shaders, càrrega de textures i auxiliars gràfics (eixos, graelles). | `Projeccio_Perspectiva()`, `Projeccio_Orto()`, `Vista_Esferica()`, `Vista_Navega()`, `Vista_Geode()`, `Iluminacio()`, `loadIMA_SOIL()`, `loadCubemap()`, `draw_Eixos()`, `draw_Grid()`. |
| [`visualitzacio.h`](../Entorn-GLFW-GL4.6-ImGui/EntornVGI/visualitzacio.h) | Capçalera de les funcions de visualització i càmera. | Declaració de les funcions esmentades i paràmetres de configuració de càmera i projecció. |
| [`shader.cpp`](../Entorn-GLFW-GL4.6-ImGui/EntornVGI/shader.cpp) / [`shader.h`](../Entorn-GLFW-GL4.6-ImGui/EntornVGI/shader.h) | Classe d'abstracció C++ per compilar vèrtex i fragment shaders, enllaçar programes GPU i passar valors *uniform*. | Classe `Shader`, mètodes `loadFileShaders()`, `use()`, `setMatrix4fv()`, `setFloat4()`, `setInt()`, `setBool()`, `releaseAllShaders()`. |
| [`glut_geometry.cpp`](../Entorn-GLFW-GL4.6-ImGui/EntornVGI/glut_geometry.cpp) / [`glut_geometry.h`](../Entorn-GLFW-GL4.6-ImGui/EntornVGI/glut_geometry.h) | Biblioteca de geometria procedimental modernitzada a OpenGL 4.6 (`VAO`, `VBO`, `EBO`). Conté generadors d'esferes, cubs, cons, cilindres, toroides, corbes i el cub Skybox. | `draw_TriEBO_Object(GLU_SPHERE)`, `loadgluSphere_EBO()`, `drawCubeSkybox()`, `loadCubeSkybox_VAO()`, `draw_TriEBO_Object(GLUT_TORUS)`, gestor `VAOList`. |
| [`material.cpp`](../Entorn-GLFW-GL4.6-ImGui/EntornVGI/material.cpp) / [`material.h`](../Entorn-GLFW-GL4.6-ImGui/EntornVGI/material.h) | Taula de propietats òptiques dels materials predefinits (ambient, difusa, especular, emissió, brillantor/*shininess*) i assignació de color/material als shaders. | Taula `materials[MAX_MATERIALS]`, `SeleccionaMaterial()`, `SeleccionaColorMaterial()`. |
| [`quatern.cpp`](../Entorn-GLFW-GL4.6-ImGui/EntornVGI/quatern.cpp) / [`quatern.h`](../Entorn-GLFW-GL4.6-ImGui/EntornVGI/quatern.h) | Àlgebra de quaternions per a rotacions 3D contínues sense bloqueig de cardan (*Gimbal Lock*), interpolacions lineals (`QuatLerp`) i esfèriques (`QuatSlerp`). | Estructura `GL_Quat`, `EixAngleToQuat()`, `QuatToMatrix()`, `EulerToQuat()`, `QuatSlerp()`, `QuatNormalize()`. |
| [`objLoader.cpp`](../Entorn-GLFW-GL4.6-ImGui/EntornVGI/objLoader.cpp) / [`objLoader.h`](../Entorn-GLFW-GL4.6-ImGui/EntornVGI/objLoader.h) | Carregador de fitxers de malla en format Wavefront `.obj` amb fitxer associat de materials `.mtl` i generació automàtica de buffers `VAO` independents per material. | Classe `COBJModel`, `LoadModel()`, `draw_TriVAO_OBJ()`, `LoadMaterialLib()`. |
| [`constants.h`](../Entorn-GLFW-GL4.6-ImGui/EntornVGI/constants.h) | Fitxer global de constants, enumeracions i estructures bàsiques compartides. | Constants `CAM_ESFERICA`, `CAM_NAVEGA`, `PERSPECT`, `ORTO`, estructures `CPunt3D`, `CEsfe3D`, `CColor`, `CVAO`, `INSTANCIA`, `LLUM`, `MATERIAL`. |
| [`framework.h`](../Entorn-GLFW-GL4.6-ImGui/EntornVGI/framework.h) | Capçalera mestra de compilació: inclou la crida a GLEW, GLFW, GLM, SOIL2 i llibreries estàndard de C++. | Inclusió de `<gl/glew.h>`, `<GLFW/glfw3.h>`, `<glm/glm.hpp>`, `"glut_geometry.h"`, `"constants.h"`. |
| [`shaders/`](../Entorn-GLFW-GL4.6-ImGui/EntornVGI/shaders) | Carpeta de codis font GLSL per a la GPU. | `phong_shdrML.*`, `gouraud_shdrML.*`, `flat_shdrML.*`, `skybox.*`, `eixos.*`. |
| [`textures/`](../Entorn-GLFW-GL4.6-ImGui/EntornVGI/textures) | Imatges i mapes de textura de prova (incloent `skybox/`: `right.jpg`, `left.jpg`, `top.jpg`, `bottom.jpg`, `front.jpg`, `back.jpg`). | Imatges per al mapa de cubs (*cubemap*) del cel i textures generals de prova. |

---

## 3. Cicle de Vida i Flux d'Execució (`main.cpp`)

1. **Inicialització del Subsistema (`main()`):**
   * Inicialització de la llibreria GLFW mitjançant `glfwInit()`.
   * Creació de la finestra principal i el context gràfic OpenGL 4.6 Core (`glfwCreateWindow()`, `glfwMakeContextCurrent()`).
   * Inicialització de GLEW per resoldre els punters de funció de les extensions d'OpenGL 4.6 moderns (`glewInit()`).
   * Activació del context de depuració d'OpenGL (`glDebugMessageCallback`) per capturar errors de shaders o pipeline a temps d'execució.
   * Inicialització de les variables d'estat a `InitGL()`: càmera esfèrica per defecte, 8 llums OpenGL (Llum 0 encesa), càrrega de shaders bàsics (`gouraud_shdrML`, `eixos`, `skybox`), càrrega del cub Skybox a la memòria de vídeo i les seves 6 textures cubemap.
   * Registre de les crides de retorn d'esdeveniments de GLFW: redimensionament (`OnSize`), ratolí (`OnMouseButton`, `OnMouseMove`, `OnMouseWheel`), teclat (`OnKeyDown`), i refresc (`OnPaint`).
   * Inicialització del context de Dear ImGui (`ImGui::CreateContext()`, `ImGui_ImplGlfw_InitForOpenGL()`, `ImGui_ImplOpenGL3_Init()`).

2. **Bucle d'Animació i Renderitzat (`while (!glfwWindowShouldClose(window))`):**
   * Càlcul del temps delta per fotograma: `delta = now - previous`.
   * Gestió del temporitzador d'animació: si `time <= 0.0` i `(satelit || anima)`, es crida a la funció `OnTimer()`.
   * Processament d'esdeveniments d'entrada mitjançant `glfwPollEvents()`.
   * Construcció de la interfície d'usuari Dear ImGui a `draw_Menu_ImGui()`.
   * Execució de la funció principal de dibuix `OnPaint(window)`.
   * Renderitzat de la capa ImGui mitjançant `ImGui_ImplOpenGL3_RenderDrawData()`.
   * Intercanvi dels buffers frontal i posterior: `glfwSwapBuffers(window)`.

3. **Flux de Renderitzat (`OnPaint()`):**
   * Selecció del tipus de projecció: Perspectiva (`PERSPECT`), Ortogràfica (`ORTO`) o Axonomètrica (`AXONOM`).
   * En perspectiva:
     * Càlcul de la matriu de projecció: `ProjectionMatrix = Projeccio_Perspectiva(...)`.
     * Càlcul de la matriu de vista segons la càmera activa: `Vista_Esferica(...)`, `Vista_Navega(...)` o `Vista_Geode(...)`.
     * Crida a `configura_Escena()` per actualitzar les transformacions d'instanciació (`GTMatrix`).
     * Crida a `dibuixa_Escena()`:
       1. Si `SkyBoxCube` està actiu, executa `dibuixa_Skybox()` amb el seu shader i cubemap dedicat.
       2. Executa `dibuixa_Eixos()` per dibuixar els eixos X, Y, Z i la graella.
       3. Crida a `dibuixa_EscenaGL()` per pintar els objectes de la geometria amb el shader actiu.

---

## 4. El Pipeline Gràfic i Shaders (`shaders/`)

L'entorn incorpora diversos parells de Vertex Shaders (`.vert`) i Fragment Shaders (`.frag`):

### 4.1. Phong Shader Multi-Llum (`phong_shdrML.vert` / `phong_shdrML.frag`)
És el shader més avançat de l'entorn i el candidat principal per a un renderitzat realista:
* **Matrius uniform:** `projectionMatrix`, `viewMatrix`, `modelMatrix`, `normalMatrix` (transposada de la inversa de la matriu model-vista per a la transformació correcta de les normals).
* **Atributs per vèrtex:**
  * `layout(location = 0) in vec3 in_Vertex;`
  * `layout(location = 1) in vec4 in_Color;`
  * `layout(location = 2) in vec3 in_Normal;`
  * `layout(location = 3) in vec2 in_TexCoord;`
* **Càlcul lumínic per fragment:**
  * Suporta fins a 8 fonts de llum (`NUM_MAX_LLUMS = 8`) mitjançant l'estructura `Light`: posició, colors ambient, difús i especular, coeficients d'atenuació per distància ($f_{att} = \frac{1}{a\cdot d^2 + b\cdot d + c}$), i configuració de focus direccionals (*spotlights* amb `spotcoscutoff` i `spotexponent`).
  * Suporta llum fixa en coordenades món o lligada a la càmera (`fixedLight`).
  * Suporta modulació de textura 2D amb el color de la llum (`modulate`, `textur`).
  * Suporta paràmetres de materials: coeficients ambient, difús, especular, emissiu i l'exponent d'especularitat (*shininess*).

### 4.2. Shader d'Eixos (`eixos.vert` / `eixos.frag`)
* Dibuixa línies dels eixos utilitzant la matriu de vista i projecció directament, sense càlculs d'il·luminació, aplicant els colors purs de vèrtex (Vermell=X, Verd=Y, Blau=Z).

### 4.3. Shader de Skybox (`skybox.VERT` / `skybox.FRAG`)
* Mostreja una textura cúbica (`samplerCube`) utilitzant les coordenades de posició del vèrtex com a vector de direcció 3D.
* Ajusta `glDepthFunc(GL_LEQUAL)` perquè el cub es dibuixi sempre al fons infinit respecte als objectes de l'escena.

---

## 5. El Sistema de Càmeres (`visualitzacio.cpp`)

L'entorn implementa tres models de càmera que calculen una matriu `glm::mat4 ViewMatrix`:

1. **Càmera Esfèrica (`Vista_Esferica`):**
   * Model orbital al voltant de l'origen de coordenades $(0,0,0)$.
   * Controlada per coordenades esfèriques de l'observador: distància $R$, angle azimutal $\alpha$ i angle d'elevació $\beta$ (emmagatzemats a l'estructura `CEsfe3D OPV`).
   * Permet configurar l'eix polar superior (`Vis_Polar`: eix Z, Y o X).
   * Genera la matriu mitjançant `glm::lookAt(posicioOcular, centre, vectorUp)`.

2. **Càmera Navega (`Vista_Navega`):**
   * Model de primera persona o vol espacial lliure.
   * L'observador té una posició lliure cartesiana `CPunt3D opvN` i vectors de direcció $n$ (vector frontal de mirada) i $v$ (vector vertical o *up*).
   * La posició canvia en funció de les tecles de desplaçament (avançar, retrocedir, girar).

3. **Càmera Geode (`Vista_Geode`):**
   * Variant esfèrica on l'observador mira cap a l'exterior o manté un punt de vista relatiu fixat.

---

## 6. Primitives Geomètriques Disponibles (`glut_geometry.cpp`)

El fitxer `glut_geometry.cpp` (i la seva capçalera `glut_geometry.h`) ofereix un conjunt complet de primitives clàssiques convertides a buffers moderns (`VAO`, `VBO`, `EBO`):

* **Esfera (`GLU_SPHERE`):**
  * `loadgluSphere_EBO(radius, slices, stacks)`: Genera els vèrtexs, coordenades de textura esfèriques $(u,v)$, vectors normals a la superfície i índexs triangulars.
  * `draw_TriEBO_Object(GLU_SPHERE)`: Renderitza l'esfera a través d'un únic `glDrawElements`.
* **Cub Skybox (`CUBE_SKYBOX`):**
  * `loadCubeSkybox_VAO()` i `drawCubeSkybox()`: Cub preparat per al mapa de cubs de l'entorn espacial.
* **Torus / Toroide (`GLUT_TORUS`):**
  * `glutSolidTorus_VAO(innerRadius, outerRadius, nsides, rings)`: Geometria ideal per a la generació d'anells planetaris (com els de Saturn).
* **Cilindres i Discs (`GLU_CYLINDER`, `GLU_DISK`):**
  * Geometries auxiliars per a tubs, bases o representació d'anells plans.
* **Corbes i Línies (`draw_LinEBO_Object`, `draw_LinVAO_Object`):**
  * Suport per pintar línies i polilínies en 3D (Bézier, B-Spline, etc.), base directa per dibuixar les línies de les trajectòries orbitals dels planetes.

---

## 7. Controls Interactius i Interfície Dear ImGui

L'entorn disposa d'un sistema d'interacció doble:
1. **Dreceres de teclat i ratolí GLFW:**
   * Botó esquerre arrossegant: Rotació de càmera orbital ($\alpha, \beta$).
   * Roda del ratolí / tecles `+` i `-`: Zoom (variació del radi $R$).
   * Tecles de cursor: Desplaçament i navegació.
   * Tecla `S`: Activa/Desactiva el mode satèl·lit (rotació automàtica de la càmera).
2. **Finestres ImGui:**
   * **Status Menu:** Mostra en temps real la posició de la càmera en coordenades esfèriques i cartesianes, colors de fons i d'objecte, transformacions de model aplicades, i ràtio de fotogrames per segon (FPS).
   * **EntornVGI Menu:** Menú complet amb desplegables (*collapsing headers*) per a càmeres, vistes, projeccions, transformacions, llums, materials, shaders i càrrega d'arxius OBJ/textures.
