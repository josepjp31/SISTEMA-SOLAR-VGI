# Resumen de Trabajo: Fases 0 a 10 y Corrección de Codificación UTF-8

> **Proyecto:** Simulador del Sistema Solar (EntornVGI — OpenGL 4.6 / GLFW / Dear ImGui / NASA CSPICE)  
> **Alcance:** Fases 0 a 10 completadas + corrección integral de codificación UTF-8 *(Fases 11 y 12 no realizadas por decisión del usuario)*  
> **Fecha de cierre:** 16 de noviembre de 2026

---

## 1. Visión General de lo Realizado

Se ha transformado el proyecto original heredado del entorno docente `EntornVGI` en un simulador limpio, estable y modular con **dos modos de simulación independientes**:
- **Mode Didàctic / Intuïtiu (`EscenariIntuitiu`)**: conserva la vista clara didáctica con órbitas circulares coplanars y tamaños visuales ajustados.
- **Mode a Escala Real (`EscenariRealista`)**: simula el Sol, los 8 planetas, Plutón y la Luna a escala física real (`1 UA = 1.000 unitats OpenGL`) conectados a efemérides NASA JPL SPICE (`de440.bsp`, `plu060.bsp`, `pck00011.tpc`, `gm_de440.tpc`, `naif0012.tls`), con órbitas 3D reales, rotaciones sidéreas e inclinaciones axiales reales (`IAU_* -> J2000`), reloj UTC/TDB y control de timelapse.

| Fase | Objetivo Principal | Commit de Referencia | Estado |
| :--- | :--- | :---: | :---: |
| **Fase 0** | Correcciones base de compilación, memoria e iluminación | `77bd186` | Completada |
| **Fase 1** | Limpieza directa de código muerto (Nivel 1) | `46edff8` | Completada |
| **Fase 2** | Limpieza profunda de código legacy VGI y desacoplamiento | `bfdb736` | Completada |
| **Fase 3** | Abstracción 3D de cuerpos y parametrización de cámara/luz | *(integrada en `8bb9e08`)* | Completada |
| **Fase 4** | Separación polimórfica (`EscenariIntuitiu` vs `EscenariRealista`) | `8bb9e08` | Completada |
| **Fase 5** | Conexión del modo realista con NASA CSPICE y tiempo real | `6665357` | Completada |
| **Fase 6** | Cámara astronómica, iluminación, anillo de Saturno y halos 2D | `5c0df49` | Completada |
| **Fase 7** | Limpieza final de archivos residuales y variables VGI | `9fef578` | Completada |
| **Fase 8** | Corrección de ciclo de frame, sincronización, cámara libre y fechas | `607ae1b` | Completada |
| **Fase 9** | Catálogo astronómico único, proporciones exactas y textura lunar | `c388f86` | Completada |
| **Fase 10** | Documentación interna, unidades físicas y limpieza de comentarios | `edd9073` | Completada |
| **Ajuste UTF-8** | Unificación de fuentes a UTF-8 y `/utf-8` en `.vcxproj` | *(aplicado tras `edd9073`)* | Completada |
| **Fases 11–12** | Extensibilidad de capas/satélites y `GestorCameres` | — | No realizada (decisión de alcance) |

---

## 2. Detalle por Fases Ejecutadas

### Fase 0 — Correcciones Base y Configuración (`77bd186`)
- **Enlace SPICE en Release**: añadido `cspice.lib` en `Release|x64` dentro de `EntornVGI.vcxproj`.
- **Fuga de memoria en `main()`**: eliminada la instancia huérfana `new Escenari()` creada al arranque.
- **Iluminación y transparencia**: desactivada la emisión propia heredada (`sw_mat[0]`) en órbitas y anillos de Saturno, y activado `GL_BLEND` para la textura alfa del anillo.
- **Limpieza en `CelestialBodyData.cpp`**: eliminadas las líneas duplicadas en `getOrbitStates()`.

### Fases 1 y 2 — Limpieza de Código Muerto y Legacy VGI (`46edff8`, `bfdb736`)
- **Módulos de escena y material**: eliminados modelos de prácticas anteriores (`tie`, `arc`, `loadSea_VAO`, `dibuixa`, prototipo `sistema_solar`) de `escena.h/.cpp` y tablas no usadas de `material.h/.cpp`.
- **Simplificación masiva de `main.cpp` y `visualitzacio.cpp`**: retiradas más de 5.500 líneas de código legacy (`ShowEntornVGIWindow`, `Barra_Estat`, `shortCut_*`, `Teclat_Shift/Ctrl/Trans*`, `OnObjecte*`, `CarregaPlaneta`, `OnSistemaTots`, `Projeccio_Orto`, `Vista_Geode`, `draw_Grid`, `glShadeModel` obsoleto), corrigiendo además el bug tipográfico en la luz #1 (`lumin[1].difusa.r`) y simplificando `dibuixa_EscenaGL` / `dibuixaEscenari`.

### Fases 3 y 4 — Abstracción 3D y Separación Polimórfica de Modos (`8bb9e08`)
- **`Escenari` como clase base polimórfica** (`escenari.h/.cpp`): encapsula `ConfigCamera` (límites de zoom, distancia inicial, velocidad libre, atenuación), posiciones 3D `m_posicio` y getters comunes (`getPosicioCos`, `getPosicioVista`, `getRadiCos`, `getNomCos`), eliminando los cálculos `cos`/`sin` externos en `main.cpp`.
- **Separación en dos clases especializadas**:
  - `EscenariIntuitiu`: encapsula las 8 órbitas toroidales y el movimiento circular didáctico en `z = 0`.
  - `EscenariRealista`: implementa la estructura científica para 11 cuerpos (Sol, 8 planetas, Plutón y Luna) con gestión propia de kernels SPICE, escala en UA y VAOs de órbitas `GL_LINE_STRIP`.

### Fases 5 y 6 — Efemérides NASA SPICE, Cámara Astronómica, Iluminación y Halos (`6665357`, `5c0df49`)
- **Puente SPICE (`CelestialBodyData.h/.cpp`)**: implementado `utcToET()` (UTC → segundos TDB desde J2000), `getCBstateKm()` (en `km` y `km/s` para `oscltx_c` + `GM`), *fallback* automático de Plutón (`999` → `9`) fuera de `1800–2199`, y matriz de rotación 3D `IAU_* -> J2000`.
- **Simulación física y órbitas 3D**: cálculo de posiciones reales en `double` (`1 UA = 1.000 u`), sincronización del reloj `DataHoraSimulacio` con `m_ephemerisTimeET`, generación de órbitas 3D reales `GL_LINE_STRIP` y anillo de Saturno con rotación real (~26,73°).
- **Cámara e iluminación a escala real**: planos `p_near` y `p_far` dinámicos en `main.cpp`, eliminación del límite `R >= 1.0` en `Vista_Esferica`, zoom logarítmico en rueda/ratón/teclado, origen relativo al cuerpo enfocado en doble precisión (sin *jitter* en `float32`), atenuación `1.0e-10f` para mantener iluminados Neptuno y Plutón, y marcadores/halos 2D (`DibuixarMarcadorPlaneta`) cuando los planetas tienen tamaño aparente sub-píxel.

### Fase 7 — Limpieza Final de Archivos y Variables Residuales (`9fef578`)
- Eliminados del proyecto y del disco `quatern.h/.cpp`, `shaders/flat_shdrML.*`, `shaders/gouraud_shdrML.*` y `verify_spice.obj` *(conservando `objLoader.*` para uso futuro)*.
- Limpieza adicional de variables globales y ramas muertas en `main.h`, `main.cpp`, `visualitzacio.h/.cpp` y `constants.h`, además de ajustar dependencias y PCH en `EntornVGI.vcxproj`.

### Fase 8 — Estabilidad del Ciclo de Frame, Sincronización y Cámara Libre (`607ae1b`)
- Eliminada la doble/triple ejecución de `creaEscenari()` y `recalcularOrbites3D()` al iniciar o cambiar de modo, centralizándola en `SincronitzarEstatSimuladorAmbOpenGL()`.
- Corregido el orden de renderizado para calcular las matrices del frame antes de proyectar etiquetas/halos 2D (sin salto de 1 frame al cambiar de astro).
- En `EscenariIntuitiu`, el anillo de Saturno pasa a dibujarse después de las esferas con `glDepthMask(GL_FALSE)`.
- En `EscenariRealista`, eliminada la llamada redundante a `getCBrotMat(CB_SATURN)` en dibujo; en `main()`, corregido el orden de liberación de shaders antes de `glfwTerminate()`.
- En `CAM_NAVEGA`, añadida protección frente a división por cero y escalado de la sensibilidad vertical del ratón para Escala Real.
- Acotado el selector de años en `DataHoraSimulacio::Validar()` al rango cubierto por `de440.bsp` (`1550–2649`).

### Fase 9 — Catálogo Único, Datos Reales y Proporciones Exactas (`c388f86`)
- Creado el módulo único `CatalegAstronomic.h/.cpp` alineado con `docs/planetes/planetes.json`, eliminando la duplicación de datos entre `EscenariRealista` e `InterficieUI`.
- Escalado el anillo de Saturno usando el **radio ecuatorial real** (`60.268 km`) en lugar del radio medio (`58.232 km`).
- Añadida la textura lunar `EntornVGI/textures/moon/2k_moon.jpg` para la Luna.
- Corregida la ficha HUD en `InterficieUI.cpp` para mostrar distancia geocéntrica en km y período alrededor de la Tierra para satélites, además del indicador `(retrògrad)` en rotaciones negativas.
- Implementada regeneración selectiva de órbitas 3D (`recalcularOrbitaCos`) cuando el timelapse avanza más de un umbral (> 1,5 días para la Luna o > 5% del período orbital), evitando que la Luna se salga de su trayectoria precesante.

### Fase 10 — Documentación Interna y Limpieza de Comentarios (`edd9073`)
- Añadidas cabeceras de arquitectura, contratos de métodos y unidades físicas explícitas (`[km]`, `[UA]`, `[s TDB]`, `[graus]`) en todos los `.h` activos.
- Añadidos comentarios explicativos del *porqué* matemático y OpenGL en los `.cpp` principales y eliminados bloques antiguos de OpenGL 1.1 comentados en `visualitzacio.cpp` y `material.cpp`.

### Ajuste Final de Codificación UTF-8 *(Posterior a Fase 10)*
- **Causa detectada**: ficheros creados sin BOM (`CatalegAstronomic.cpp`, `EscenariIntuitiu.cpp`, `EscenariRealista.cpp`, etc.) se compilaban con `/execution-charset:utf-8` sin `/source-charset:utf-8`, haciendo que MSVC los interpretara como Windows-1252 y provocara doble codificación (*mojibake*) en nombres con acentos (`Júpiter`, `Urà`, `Neptú`, `Plutó`, `Satèl·lit`, `Òrbita`, `Distància`). Además, 12 archivos heredados seguían guardados en Windows-1252.
- **Solución aplicada**:
  - Convertidos los 12 archivos en Windows-1252 (`glut_geometry.*`, `objLoader.*`, `shader.*`, `visualitzacio.*`, `constants.h`, `framework.h`, `main.h`, `material.h`) a **UTF-8**.
  - Sustituido `/execution-charset:utf-8` por `/utf-8` en las 4 configuraciones de `EntornVGI.vcxproj`.
  - Añadido el carácter `•` al rango de glifos en `InterficieUI::InicialitzarEstil()`.

---

## 3. Alcance Excluido (Pendiente para Ampliaciones Futuras)

Por decisión del usuario, no se han implementado en este ciclo las siguientes fases del plan de extensibilidad:
- **Fase 11**: jerarquía dinámica extensible de satélites, módulo Kepleriano analítico, capas de cinturón de asteroides instanciado, puntos de Lagrange, partículas de anillos y soporte `.obj` para sondas/cohetes.
- **Fase 12**: `GestorCameres` desacoplado con cámara 6-DOF, cámaras de sonda/superficie y selección directa de astros por clic (*ray-picking*).
