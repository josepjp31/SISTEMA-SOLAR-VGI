# Sondas espaciales

Sondas de la NASA (ISS, Hubble, JWST, Juno, Cassini, Parker, Voyager, Pioneer) añadidas a la **escena intuitiva**.

- Rama: `models-sondes`
- Todo el código lleva el comentario `//SONDES` (búscalo con Ctrl+Shift+F).
- Las rutas son relativas a `Entorn-GLFW-GL4.6-ImGui/EntornVGI/`. Los números de línea pueden moverse un poco si se edita el fichero.

---

## Qué se ha hecho

- **Modelos:** sacados de [NASA 3D Resources](https://github.com/nasa/NASA-3D-Resources), pasados de `.glb` a `.obj` con Blender.
- **Dónde están:** `textures/sondes/`, una carpeta por sonda (`.obj` + `.mtl` + `textures/`).
- **Código** (mismo sistema que la Milano):

| Fichero | Líneas | Qué tiene |
|---|---|---|
| `escena.h` | 26 | Parámetro `sondes[]` de `dibuixa_EscenaGL` |
| `escena.h` | 33-35 | `NUM_SONDES` y la lista de nombres (`SONDA_ISS`, `SONDA_HUBBLE`…) |
| `main.h` | 123-124 | El array `ObSondes` con los modelos |
| `main.cpp` | 550-572 | `carregaSondes()`: carga los `.obj` (rutas en 553-562) |
| `main.cpp` | 726-728 | Llamada a `carregaSondes()` la primera vez |
| `main.cpp` | 734 | Se pasa `ObSondes` a `dibuixa_EscenaGL` |
| `escena.cpp` | 169-232 | Bloque `//SONDES`: calcula la posición de cada sonda y la dibuja |

---

## Modificar una sonda

Todo en el bloque `//SONDES` de **`escena.cpp`**.

### Tamaño — `escena.cpp`, línea 183

```cpp
const float escalaSonda[NUM_SONDES] = { 0.06f, 0.003f, 0.07f, 0.08f, 0.07f, 0.25f, 0.10f, 0.20f };
//                                      ISS    Hubble  JWST   Juno   Cassini Parker Voyager Pioneer
```

Número más alto = más grande.

### Orientación — `escena.cpp`, línea 218

```cpp
ModelMatrix = glm::rotate(ModelMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
```

Pone todas las sondas de pie. Si salen boca abajo, cambia `90.0f` por `-90.0f`.

### Posición y movimiento — `escena.cpp`, líneas 197-204

Hay una línea por sonda. `puntOrbita(radio, ángulo)` da un punto a esa distancia y con ese ángulo en grados.

Distancias de los planetas al Sol, como referencia:

| 1 Mercurio | 2 Venus | 3 Tierra | 4 Marte | 5 Júpiter | 6 Saturno | 7 Urano | 8 Neptuno |
|---|---|---|---|---|---|---|---|
| 27 | 40 | 55 | 70 | 105 | 140 | 175 | 210 |

**Quietas** — Voyager (línea 203) y Pioneer (línea 204):

```cpp
posSonda[SONDA_VOYAGER] = puntOrbita(90.0f, 35.0f);    // a 90 del Sol, ángulo 35°
posSonda[SONDA_PIONEER] = puntOrbita(235.0f, 200.0f);
```

**Orbitando un planeta** — ISS (197), Hubble (198), Juno (200) y Cassini (201):

```cpp
posSonda[SONDA_ISS] = terra + puntOrbita(5.5f, obj[3].m_angleOrbita * 40.0f);
//                    ^^^^^              ^^^^                         ^^^^^
//                    planeta            radio                        velocidad
```

- **Planeta:** `terra`, `jupiter` o `saturn` (calculados en las líneas 191-193).
- **Radio:** distancia al planeta.
- **Velocidad:** número más alto = más rápido. Sigue la pausa y la velocidad del tiempo.
- El Hubble (línea 198) suma `+ 180.0f` al final para empezar en el lado opuesto a la ISS.

**Orbitando el Sol** — Parker (línea 202):

```cpp
posSonda[SONDA_PARKER] = puntOrbita(21.0f, obj[1].m_angleOrbita * 1.5f);
```

**Siempre detrás de la Tierra** — JWST (línea 199):

```cpp
posSonda[SONDA_JWST] = terra + glm::normalize(terra) * 9.0f;   // 9 = distancia a la Tierra
```

---

## Añadir una sonda nueva

1. **Blender:** importa el `.glb` → guarda el `.blend` → `File → External Data → Unpack Resources` → exporta `.obj` con *Triangulated Mesh*, *Materials* y *Path Mode: Copy*.
2. En el `.mtl`, cambia las líneas `Ka ...` por `Ka 0.15 0.15 0.15`.
3. Copia la carpeta (sin el `.blend`) a `textures/sondes/`.
4. En el código, añade la sonda **al final** de cada lista, siempre en el mismo orden:

| Fichero | Línea | Qué añadir |
|---|---|---|
| `escena.h` | 34 | `NUM_SONDES` +1 |
| `escena.h` | 35 | El nombre al final del `enum` (ej. `SONDA_NOVA`) |
| `main.cpp` | 561 | La ruta del `.obj` al final de `fitxers` (con coma en la línea anterior) |
| `escena.cpp` | 173-182 | Su centro al final de `centreSonda` |
| `escena.cpp` | 183 | Su escala al final de `escalaSonda` |
| `escena.cpp` | 204 | Una línea nueva `posSonda[SONDA_NOVA] = ...;` |

---

## Pendiente

- Reexportar el **Hubble** con texturas.
- La **Voyager** está en una posición provisional (`escena.cpp`, línea 203) para que se vea.
