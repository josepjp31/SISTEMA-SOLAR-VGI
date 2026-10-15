# Catàleg de cossos i textures

El catàleg estructurat és [`planetes.json`](./planetes.json). Inclou el Sol, els vuit planetes, Plutó i la Lluna. Les propietats físiques/orbitals i les unitats s’expliquen al mateix JSON; `sourceIds` enllaça cada cos amb les fonts identificades al catàleg.

## Textures disponibles

Les textures font del projecte es troben a `docs/planetes/textures/`. El camp `textureRoot` del JSON apunta a aquesta carpeta; els noms dels mapes de cada cos són relatius a ella. Quan l’aplicació les carregui, caldrà configurar el carregador per llegir aquesta ruta o copiar els recursos a la carpeta runtime corresponent.

| Cos o ús | Fitxers presents |
|---|---|
| Sol | `sun.jpg` |
| Mercuri | `mercury_albedo.jpg` |
| Venus | `venus_albedo.jpg`, `venus_clouds.jpg` |
| Terra | `earth_albedo.jpg`, `earth_night.jpg`, `earth_clouds.jpg` |
| Lluna | `moon.jpg` |
| Mart | `mars_albedo.jpg` |
| Júpiter | `jupiter_albedo.jpg` |
| Saturn | `saturn_albedo.jpg`, `saturn_rings.png` |
| Urà | `uranus_albedo.jpg` |
| Neptú | `neptune_albedo.jpg` |
| Fons panoràmic | `stars_milky_way.jpg` |

**Plutó encara no té cap textura al directori.** Els camps de textura corresponents continuen a `null` al catàleg perquè no hi ha un fitxer associat.

## Formats i dimensions revisats

- Les 14 imatges JPG són JPEG RGB de **2048 × 1024** píxels. Tret de `stars_milky_way.jpg`, que és un panorama de fons, són mapes planetaris 2:1.
- `saturn_rings.png` és PNG RGBA de **2048 × 125** píxels i conserva canal alfa per a la transparència dels anells.
- Els mapes de núvols de Venus i la Terra són JPG RGB, sense canal alfa. Si el shader necessita transparència, caldrà generar-ne una màscara o obtenir mapes amb alfa.
- No hi ha mapes normals al directori actual; per això els camps `normal` són `null`.

> Recompte actual: 14 JPG i 1 PNG (15 fitxers).

## Procedència i llicència de les textures

Les textures del Sol, els planetes, la Lluna, els anells de Saturn i el panorama d’estrelles provenen del paquet [Solar System Scope — Textures](https://www.solarsystemscope.com/textures/). La pàgina indica que estan basades en dades d’elevació i imatges de NASA i que es distribueixen sota la llicència [Creative Commons Attribution 4.0 International (CC BY 4.0)](https://creativecommons.org/licenses/by/4.0/). La procedència i llicència també consten a `sources` al JSON i s’associen als cossos que utilitzen aquests recursos.

Crèdit recomanat per al portfolio i la distribució del projecte: **Textures by Solar System Scope (INOVE), based on NASA elevation and imagery data, licensed under CC BY 4.0.** En cas d’haver modificat una textura, indiqueu que s’ha modificat i descriviu el canvi.

## Fons d’estrelles i skybox

`stars_milky_way.jpg` és un únic panorama equirectangular; el JSON el registra a `environment.starPanorama`, no com a textura d’un cos. A més, l’entorn EntornVGI conté sis JPG (`right`, `left`, `top`, `bottom`, `front`, `back`) a `Entorn-GLFW-GL4.6-ImGui/EntornVGI/textures/skybox/`; aquests formen el cubemap de l’entorn base i són fitxers separats del panorama.

## Dades físiques i orbitals

El catàleg inclou massa, densitat mitjana, gravetat superficial, velocitat d’escapament i albedo geomètric quan la font els publica. La massa pot servir per a càlculs gravitatoris; gravetat i velocitat d’escapament són també dades explicatives. L’albedo geomètric és una magnitud fotomètrica, no el color de la textura. El període de rotació és sideri (respecte a les estrelles), no la durada del dia solar. Per al Sol s’inclou la lluminositat.

En Júpiter, Saturn, Urà i Neptú, les dades de gravetat i escapament es refereixen al nivell de pressió indicat per la font, no a una superfície sòlida. `orbit: null` indica el Sol com a cos central; les òrbites dels satèl·lits es mesuren respecte al seu `parentId`.

## Afegir un recurs o cos

La font i la llicència del paquet de textures queden registrades al JSON i en aquesta guia. Si algun fitxer s’ha retallat, redimensionat, convertit o modificat d’una altra manera, anotar-ho i indicar-ho en la distribució. No reanomenar un fitxer sense actualitzar-ne la ruta a `planetes.json`.

Cada cos nou necessita un `id` estable en minúscules ASCII, nom visible, classificació, propietats amb unitats, font i mapes disponibles. Per als satèl·lits, afegir `parentId` i especificar que els seus elements orbitals són relatius al pare. Si encara no s’ha recopilat una propietat, conservar-la com a `null` en lloc d’inventar-ne el valor.

`meanDistanceFromSunKm` és una distància mitjana de referència, no una posició instantània. Període, excentricitat i inclinació resumits no són suficients per a efemèrides precises. Per a posicions en una data concreta, utilitzar el mòdul d’efemèrides amb època, escala temporal i marc de referència explícits. La llista actual de satèl·lits és parcial.
