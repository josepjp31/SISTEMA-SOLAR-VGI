# Informe tècnic: millora de les òrbites i antialiàsing

**Projecte:** Simulador del sistema solar a EntornVGI
**Autor:** Josep Montoro Pascual
**Data:** 16 de novembre de 2026
**Tasca:** acta 5 — «Millorar la representació de les òrbites (es veuen discontínues) i estudiar l'antialiàsing»

## Problema

Les òrbites es veien com a línies de punts (discontínues). Hi havia dues causes:

1. **Geometria massa fina.** Cada òrbita era un tor 3D (`loadglutSolidTorus_EBO`) de gruix 0,03 unitats. Des de la càmera global (distància ~550), aquest gruix ocupa molt menys d'un píxel. El rasteritzador només pinta un píxel si el seu centre cau dins d'un triangle, així que només alguns trams del tor generaven píxels i l'òrbita apareixia trencada.
2. **L'antialiàsing no estava actiu.** A `main.cpp`, la línia `glfwWindowHint(GLFW_SAMPLES, 4)` estava comentada i, a més, situada **després** de `glfwCreateWindow`. Els *hints* de GLFW només afecten les finestres que es creen després, així que no hauria tingut cap efecte.

## Solució

### 1. Òrbites com a línies (`GL_LINE_LOOP`)

Una primitiva de línia sempre genera com a mínim un píxel per columna o fila que travessa. Per tant, és contínua independentment de la distància. Els tors s'han substituït per **un únic cercle de radi 1** al pla XY:

- `Escenari::creaOrbita()` (`escenari.cpp`) genera 512 vèrtexs `(cos a, sin a, 0)` i en crea el VAO amb `load_TRIANGLES_VAO` de `glut_geometry`, que és el VAO genèric de l'entorn: posició, color, normal i textura, les quatre entrades que espera el shader Phong.
- Totes les òrbites (8 planetes + Lluna) reutilitzen aquest VAO. La `modelMatrix` l'escala al radi de cada òrbita, igual que es feia amb els tors. Com a conseqüència, `m_orbitVAO` passa de ser un vector de 9 tors a un sol `CVAO`.
- Es dibuixa amb `glDrawArrays(GL_LINE_LOOP, 0, nVertexs)`.
- 512 segments són suficients perquè no es vegin trams rectes ni a l'òrbita de Neptú (radi 210): la fletxa de cada segment és d'unes 0,008 unitats.

### 2. Il·luminació de les línies

El shader Phong calcula la llum difusa amb la normal. Una línia no té superfície, així que s'ha triat la normal que reprodueix el que es veia amb el tor:

- **Normal cap al centre de l'òrbita** `(−cos a, −sin a, 0)`. Per a les òrbites planetàries, el centre és el Sol: la normal coincideix amb la direcció de la llum i l'òrbita queda il·luminada uniformement en tot el recorregut (amb l'atenuació per distància de sempre).
- **Òrbita lunar.** Està centrada a la Terra, de manera que la normal no sempre apunta al Sol. Es dibuixa amb `sw_twoSided = true`, el mateix uniform que es fa servir per a l'anell de Saturn, perquè cap tram quedi totalment fosc.
- Es manté el material sense emissió (`sw_mat_no_emisio`) que ja feien servir les òrbites: només el Sol emet llum.

### 3. Antialiàsing MSAA ×4 (`main.cpp`)

```cpp
glfwWindowHint(GLFW_SAMPLES, 4);   // abans de glfwCreateWindow
...
glEnable(GL_MULTISAMPLE);          // després de crear el context
```

El framebuffer guarda 4 mostres per píxel. El color final de cada píxel és la mitjana de les mostres cobertes. Així, les línies de les òrbites i les vores de les esferes (planetes, Sol, anell) es veuen suavitzades en comptes d'esglaonades. Afecta tota l'escena i no requereix canvis als shaders.

### 4. Gestió de memòria

- `alliberaOrbita()` (privada a `Escenari`) esborra el VAO i el VBO de l'òrbita. La crida `alliberaEscenari()` i també `creaOrbita()` abans de crear-ne un de nou. Abans, cada vegada que es canviava de mode es tornava a cridar `creaEscenari()` i es perdien els 8 tors anteriors a la GPU.
- `load_TRIANGLES_VAO` no inicialitza `eboId`. Es posa a 0 explícitament perquè l'alliberament no esborri un identificador brossa.

## Fitxers modificats

| Fitxer | Canvi |
|---|---|
| `EntornVGI/escenari.h` | `CVAO m_orbitVAO` (únic), mètodes privats `creaOrbita()` i `alliberaOrbita()`. |
| `EntornVGI/escenari.cpp` | `creaOrbita()`. Bucle d'òrbites de `dibuixaEscenari()` amb `GL_LINE_LOOP` i `sw_twoSided` per a la Lluna. |
| `EntornVGI/main.cpp` | `GLFW_SAMPLES = 4` abans de crear la finestra i `glEnable(GL_MULTISAMPLE)`. |

## Verificació

- Compilació en Visual Studio 2026, `Debug | x64`: 0 errors, 0 avisos.
- Comprovacions visuals pendents en executar:
  1. les òrbites es veuen contínues des de la vista global i des de prop;
  2. les vores del Sol i dels planetes es veuen suaus;
  3. l'opció «Òrbites» continua amagant-les;
  4. canviant diverses vegades de mode (Didàctic ↔ Escala) no augmenta la memòria de vídeo.

## Observacions

- **Gruix de línia.** Les línies fan 1 píxel. En un context *core* amb *forward compatible*, `glLineWidth` > 1 no està garantit, i per això no s'ha fet servir. Si es volguessin òrbites més gruixudes, caldria generar-les com a cintes de triangles orientades a càmera (per exemple, amb un *geometry shader*).
- **Hints de context.** A `main.cpp`, els altres *hints* (versió 4.6, perfil *core*, *debug*) també estan després de `glfwCreateWindow` i, per tant, no s'apliquen. No s'han mogut perquè canviar el tipus de context podria afectar codi de tot l'equip; convé revisar-ho conjuntament.
