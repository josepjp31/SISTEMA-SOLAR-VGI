# Informe tècnic: incorporació de la Lluna a l'escena intuïtiva

**Projecte:** Simulador del sistema solar a EntornVGI
**Autor:** Josep Montoro Pascual
**Data:** 16 de novembre de 2026
**Tasca:** acta 5 — «Afegir la Lluna a l'escena i vincular-ne el moviment amb la Terra»
**Base de codi:** `limpieza-codigo` (commit `bfdb736`)

## Objectiu

Afegir la Lluna com a satèl·lit de la Terra. Ha d'orbitar al voltant de la Terra mentre aquesta orbita el Sol, ha de rebre la llum del Sol com els planetes i ha de mostrar sempre la mateixa cara a la Terra.

## Resum dels canvis

Tot el canvi és dins de la classe `Escenari`. No s'ha modificat `main.cpp`, `InterficieUI`, `escena.cpp` ni els shaders. La Lluna s'ha afegit com **un cos més** de les llistes i bucles que ja existien per al Sol i els planetes, sense crear-hi un bloc a part.

| Fitxer | Canvi |
|---|---|
| `EntornVGI/escenari.h` | El constructor inicialitza `m_orbitVAO` a zero. (Després, amb la millora de les òrbites, `m_orbitVAO` ha passat a ser un únic cercle de línies compartit; vegeu `millora-orbites-antialiasing.md`.) |
| `EntornVGI/escenari.cpp` | La Lluna és l'element 9 de totes les llistes de `creaEscenari()`. Els bucles de `dibuixaEscenari()` i `updateVariables()` passen de 9 a 10 cossos. S'afegeix la funció auxiliar `matriuOrbitaLluna()`. |
| `EntornVGI/textures/planet/Moon/2k_moon.jpg` | Textura nova, còpia de `docs/planetes/textures/moon.jpg` (2048×1024, Solar System Scope, CC BY 4.0). |

## 1. Índexs

`Escenari` ja reservava `MAX_OBJECTES_ESCENARI = 10` objectes a `m_objectList`, però només en feia servir 9. La Lluna ocupa la posició lliure:

| Llista | Índex de la Lluna | Motiu |
|---|---|---|
| `m_objectList` | **9** | Posició lliure després de Neptú (8). |
| `m_fitxers` / `m_texturesID` | **10** | La 9 ja és l'anell de Saturn. |

La Lluna fa servir la mateixa classe `Object` que els planetes, sense camps nous. L'única diferència és com s'interpreten dos camps: **`m_radiOrbita` i `m_angleOrbita` són relatius a la Terra**, no al Sol.

Els bucles de 9 cossos de `main.cpp` i `InterficieUI` (menús, etiquetes, focus de càmera) no s'han tocat. Per tant, la Lluna no surt al menú de focus ni té etiqueta (vegeu «Pendent»).

## 2. Dades (`creaEscenari`)

S'afegeix un desè valor a cada llista existent:

```cpp
const float radiCos[10]     = { 16.0f, ..., 5.3f,   1.0f };  // Lluna
const float radiOrbita[10]  = { 0.0f,  ..., 210.0f, 7.0f };  // distància a la Terra
const float angleOrbita[10] = { 0.0f,  ..., 280.0f, 0.0f };

m_objectList[9].m_velAngular = x / 27.32f;  // mateixa fórmula que els planetes
m_objectList[9].m_velRotacio = 0.0f;        // rotació síncrona (vegeu §4)
```

L'òrbita lunar no necessita geometria pròpia: fa servir el mateix cercle de radi 1 que les òrbites planetàries (`m_orbitVAO`), escalat a `radiOrbita[9]`.

Els valors surten de `docs/planetes/planetes.json` (entrada `moon`), adaptats a l'escala intuïtiva:

| Propietat | Valor real | Valor a l'escena | Criteri |
|---|---|---|---|
| Radi | 1737 km (0,27 × Terra) | 1,0 | Proporció real respecte a la Terra (3,7 × 0,27). |
| Distància a la Terra | 384 400 km | 7,0 | Prou lluny per no tocar la Terra (radi 3,7) i lluny de les òrbites de Venus (40) i Mart (70); la Terra és a 55. |
| Període orbital | 27,32 dies | `x / 27.32` °/s | Mateixa fórmula que els planetes (`x = 3650`), així la proporció amb l'any terrestre és real: ~13,4 voltes per any. |
| Inclinació orbital | 5,145° | 5,145° | Valor real. |
| Rotació | 655,7 h (= període orbital) | Síncrona | Rotació capturada: sempre la mateixa cara cap a la Terra. |

Amb `x = 3650`, la Terra fa una volta al Sol en uns 36 s i la Lluna en uns 2,7 s.

## 3. Dibuix (`dibuixaEscenari`)

La Lluna es dibuixa dins dels mateixos bucles que els planetes: el d'òrbites (`for i = 1..9`) i el de cossos (`for i = 0..9`). Només canvien dues coses quan `i == 9`.

**a) La matriu de partida.** Els planetes es col·loquen a partir de `MatriuTG`, centrada al Sol. La Lluna es col·loca a partir del sistema de la seva òrbita, centrat a la Terra i inclinat 5,145°:

```cpp
glm::mat4 MatriuCentre = (i == 9) ? matriuOrbitaLluna(MatriuTG, m_objectList[3]) : MatriuTG;
ModelMatrix = glm::translate(MatriuCentre, vec3(x, y, 0.0f));   // a partir d'aquí, igual que els planetes
```

Això dona la jerarquia de transformacions de `GUIA_DESENVOLUPAMENT_SISTEMA_SOLAR.md` (§4.1):

```
M_Lluna = TG · T(posició Terra) · Rx(5,145°) · T(posició orbital lunar) · Rz(rotació) · S(radi)
          \_____________________________________/
                     matriuOrbitaLluna()
```

La funció `matriuOrbitaLluna()` calcula la posició de la Terra igual que el bucle dels planetes. La fan servir tant l'òrbita com el cos, de manera que la Lluna sempre cau exactament sobre la seva òrbita i segueix la Terra.

**b) La textura.** La textura 9 és l'anell de Saturn, així que el cos 9 fa servir la 10:

```cpp
int tex = (i == 9) ? 10 : i;
```

**Il·luminació.** No cal cap canvi: el bucle ja dona emissió només a `i == 0` (el Sol). La Lluna, com els planetes, només reflecteix la llum del Sol. Per això té cara diürna i cara nocturna, i respecta els controls «Il·luminació Solar Directa» i «Vista Clara». L'òrbita lunar fa servir el mateix color i material sense emissió que les altres, i l'opció «Òrbites» del menú també l'amaga. Com que no està centrada al Sol, es dibuixa amb il·luminació bilateral (`sw_twoSided`) perquè cap tram quedi totalment fosc.

## 4. Moviment (`updateVariables`)

El bucle passa de 9 a 10 cossos. Per tant, l'angle orbital de la Lluna s'actualitza amb la mateixa fórmula que els planetes: mateixos `dt`, `direccioTemps` (endavant/enrere), `factorTemps` (mode escala) i pausa.

L'única línia específica és la de la rotació síncrona, després del bucle:

```cpp
m_objectList[9].m_angleRotacio = m_objectList[9].m_angleOrbita - 90.0f;
```

- **Per què no es fa amb `m_velRotacio` com els planetes.** La rotació dels planetes no aplica `factorTemps`, però l'òrbita sí. Amb una velocitat de rotació pròpia, en mode escala la Lluna es desincronitzaria. Derivant la rotació de l'angle orbital, sempre coincideixen.
- **Per què `- 90°`.** A l'esfera de `loadgluSphere_EBO`, el vèrtex amb coordenada de textura `u = 0.5` (el centre del mapa, la cara visible de la Lluna) és a la direcció `(0, −1, 0)`. Una rotació `Rz(θ)` porta aquesta direcció a `(sin θ, −cos θ)`. Perquè apunti cap a la Terra, `−(cos α, sin α)` on `α` és l'angle orbital, cal `θ = α − 90°`.

## 5. Correcció addicional

`CVAO` és una `struct` sense constructor. Com que `escenaris = new Escenari[5]` i només se'n creen dos, els `m_orbitVAO` dels escenaris no utilitzats contenien valors brossa que `alliberaEscenari()` intentava esborrar. Ara el constructor d'`Escenari` els posa a zero.

## Verificació

- Compilació en Visual Studio 2026, configuració `Debug | x64`: 0 errors.
- Comprovacions visuals pendents en executar:
  1. la Lluna gira al voltant de la Terra i la segueix al llarg de l'òrbita solar;
  2. el costat de la Lluna que mira al Sol és il·luminat i l'altre és fosc;
  3. enfocant la Terra (*Planetes & Focus → Terra*), la Lluna mostra sempre la mateixa cara a la Terra;
  4. en desactivar «Òrbites», desapareix també l'òrbita lunar;
  5. pausa i inversió del temps afecten també la Lluna.

## Pendent / possibles millores

- **Etiqueta «Lluna».** El sistema d'etiquetes de `main.cpp` recorre els 9 cossos amb posicions heliocèntriques. Afegir-hi la Lluna implica calcular-ne la posició a través de la Terra. A més, des de la vista global se solaparia amb l'etiqueta «Terra»; es podria mostrar només quan la Terra està enfocada.
- **Focus de càmera a la Lluna.** Caldria ampliar els menús i `kCatalegCossos` d'`InterficieUI` (part d'en Carles).
- **Escena a escala.** Els valors actuals són intuïtius. A l'escena real s'hauran de substituir per la posició de `CB_MOON` (`CelestialBodyData`, CSPICE).
- **Inclinació axial.** Ni la Terra ni la Lluna tenen inclinació de l'eix de rotació (els planetes giren sobre Z), coherent amb la resta de l'escena intuïtiva.
