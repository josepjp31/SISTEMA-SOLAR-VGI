# Requisits principals del simulador del Sistema Solar

**Projecte:** Simulador Gràfic Interactiu del Sistema Solar  
**Assignatura:** Visualització Gràfica Interactiva (VGI), UAB — ABP Grup 05  
**Versió:** 1.0 — 9 d’octubre de 2026

## 1. Propòsit i abast

L’aplicació ha de permetre explorar visualment el Sol, els vuit planetes i Plutó en diferents moments, amb una vista didàctica com a prioritat inicial i una vista a escala física com a ampliació. Plutó es manté al projecte perquè així s’acorda a les actes; científicament es classifica com a planeta nan, no com un dels vuit planetes.

El producte es construeix sobre l’entorn docent EntornVGI: C++ i OpenGL 4.6, GLFW, GLM, Dear ImGui i les eines de càrrega de textures disponibles. La primera fita funcional acordada és una escena intuïtiva amb Sol i planetes texturats, càmera orbital i selecció de cos.

## 2. Usuaris i objectius

L’usuari és una persona que vol comprendre la configuració i el moviment del Sistema Solar mitjançant visualització interactiva. Ha de poder:

- fixar una data i hora de simulació i observar la configuració corresponent;
- accelerar, alentir, pausar i reprendre el temps simulat;
- explorar el sistema des d’una càmera orbital i centrar-la en un cos;
- alternar entre una escala didàctica i una escala física;
- activar o desactivar òrbites, etiquetes i indicadors de cossos petits;
- activar la visualització de cossos menors i punts d’interès quan aquests mòduls estiguin disponibles.

## 3. Requisits funcionals

| ID | Requisit | Prioritat | Criteri d’acceptació |
|---|---|---|---|
| RF-01 | Mostrar el Sol, Mercuri, Venus, Terra, Mart, Júpiter, Saturn, Urà, Neptú i Plutó. | Essencial | Cada cos es renderitza amb nom, geometria esfèrica i textura assignada quan l’actiu existeix. |
| RF-02 | Mostrar una escena didàctica amb òrbites i mides/distàncies ajustades per facilitar l’exploració. | Essencial | L’usuari pot distingir i inspeccionar els cossos en la vista general. |
| RF-03 | Permetre seleccionar un cos i centrar-hi la càmera orbital; tornar a la vista general del Sol. | Essencial | El canvi de focus es pot fer des del menú; la selecció directa és desitjable si el temps ho permet. |
| RF-04 | Permetre consultar i establir data i hora de simulació (dia, mes, any, hora, minut i segon). | Essencial | En confirmar una data vàlida, l’escena recalcula l’estat temporal dels cossos. |
| RF-05 | Permetre pausa, reproducció, temps real i velocitats de timelapse configurables. | Essencial | En pausa la data no avança; en reproducció avança segons el multiplicador seleccionat. |
| RF-06 | Mostrar una vista amb distàncies i radis físics, amb indicadors per trobar els cossos petits. | Important | El canvi d’escala modifica posicions i mides segons els valors físics del catàleg; els indicadors són commutables. |
| RF-07 | Commutar la representació d’òrbites i etiquetes. | Important | Cada opció actualitza la visualització sense reiniciar l’aplicació. |
| RF-08 | Representar rotació axial i translació orbital dels cossos. | Important | L’orientació i posició canvien amb el temps simulat, utilitzant dades i convencions documentades. |
| RF-09 | Incorporar llunes naturals, començant per la Lluna i ampliable a llunes seleccionades dels gegants. | Ampliació | Les llunes són cossos de primer nivell de dades, vinculats al cos pare i amb òrbita pròpia. |
| RF-10 | Mostrar cinturó d’asteroides i punts de Lagrange. | Ampliació | Es poden activar i desactivar; la implementació ha de conservar interactivitat fluida. |
| RF-11 | Oferir càmera lliure, punts de vista ISS/Voyager i observatori UAB. | Reserva | Es consideren després de consolidar l’escena intuïtiva i la vista a escala. |
| RF-12 | Oferir simulació de forces i col·lisions. | Reserva | Només s’aborda si el temps ho permet; no és necessària per a la primera fita. |

## 4. Requisits no funcionals

- **Plataforma:** Windows 10/11 i Visual Studio; arquitectura x64.
- **Gràfics:** OpenGL 4.6 Core Profile amb shaders i buffers moderns (VAO/VBO/EBO); evitar el pipeline fix.
- **Interfície:** Dear ImGui, amb controls comprensibles per a temps, focus de càmera i visualització.
- **Dades:** magnituds físiques en unitats SI o unitats declarades; cada valor ha de tenir font i convenció. El catàleg ha d'admetre mida, massa, densitat, gravetat, velocitat d'escapament, rotació, òrbita i recursos visuals. Els valors visuals ajustats no han de sobreescriure els físics.
- **Recursos:** les textures han de tenir atribució/llicència registrada i un nom estable; el programa ha de poder informar quan falta un recurs.
- **Mantenibilitat:** separar escenari, dades de cossos, efemèrides, càmera i renderització en mòduls integrables.
- **Rendiment:** mantenir una interacció fluida; utilitzar renderitzat instanciat si el nombre d’asteroides ho requereix.

## 5. Dades astronòmiques i límits de precisió

El catàleg inicial és `docs/planetes/planetes.json`. Inclou paràmetres mitjans orientatius i la ruta prevista de les textures. Aquests valors permeten preparar l’escena i no substitueixen una efemèride calculada per a una data concreta. Per a posicions precises, el mòdul d’efemèrides haurà d’obtenir estats de JPL Horizons o d’una efemèride JPL apropiada i documentar època, escala temporal, origen i marc de referència.

Els elements orbitals depenen de l’època i del marc de referència. No s’han d’interpretar els valors arrodonits d’aquest catàleg com a condicions inicials d’alta precisió. Per a la primera escena, es poden fer servir com a guia visual i reemplaçar-los per dades d’efemèrides quan s’integri aquest mòdul.

## 6. Fora de la primera fita

La càmera nau 6-DOF, els modes ISS/Voyager/UAB, els satèl·lits nombrosos, els punts de Lagrange dinàmics i les col·lisions gravitacionals són extensions. Cal prioritzar una escena intuïtiva funcional i una arquitectura que permeti ampliar-la sense bloquejar la primera entrega.

## 7. Traçabilitat

Els requisits deriven dels sis casos d’ús del projecte i de les decisions registrades a les actes ABP 1 i 2. Les prioritats reflecteixen especialment l’acord de l’acta 2: escena intuïtiva primer, escena a escala després i nau espacial com a reserva. Si el grup canvia l’abast, cal registrar el motiu i actualitzar aquest document per al control ABP.

## 8. Fonts

- NASA/JPL, [Planetary Physical Parameters](https://ssd.jpl.nasa.gov/planets/phys_par.html) — radis mitjans, radis equatorials i períodes de rotació/orbitals.
- NASA Goddard, [Planetary Fact Sheet — Metric](https://nssdc.gsfc.nasa.gov/planetary/factsheet/) — distàncies mitjanes, períodes i elements orbitals arrodonits.
- NASA/JPL, [Orbits & Ephemerides](https://ssd.jpl.nasa.gov/orbits.html) — Horizons com a font d’efemèrides i distinció entre elements i trajectòries temporals.
- NASA/JPL, [Planetary classification](https://ssd.jpl.nasa.gov/planets/index.html) — vuit planetes i reclassificació de Plutó com a planeta menor/ nan.
- NASA Goddard, [Sun Fact Sheet](https://nssdc.gsfc.nasa.gov/planetary/factsheet/sunfact.html) i [Moon Fact Sheet](https://nssdc.gsfc.nasa.gov/planetary/factsheet/moonfact.html) — propietats del Sol i la Lluna.
- NASA/JPL, [Planetary Satellite Physical Parameters](https://ssd.jpl.nasa.gov/sats/phys_par/) — dades físiques dels satèl·lits.

Les fonts s’han consultat el 11 d’octubre de 2026. Les xifres incloses al catàleg indiquen valors resumits i les pàgines originals poden actualitzar-se.
