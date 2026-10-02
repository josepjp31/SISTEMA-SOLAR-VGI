# Simulador Gràfic Interactiu del Sistema Solar

Projecte desenvolupat per a l'assignatura de **Visualització Gràfica Interactiva (VGI)** a l'Escola d'Enginyeria de la **Universitat Autònoma de Barcelona (UAB)** mitjançant Aprenentatge Basat en Projectes (ABP).

---

## Descripció del Projecte

Eina de visualització gràfica interactiva d'alt realisme encarregada per l'**IEEC (Institut d'Estudis Espaials de Catalunya)**. L'aplicació permet simular i consultar amb precisió temporal (dia, mes, any, hora, minut, segon) la posició i el moviment dels principals cossos celestes del Sistema Solar (Sol, planetes, satèl·lits naturals, cinturó d'asteroides i Plutó), oferint a més navegació espacial interactiva i càmeres d'inspecció.

El projecte està implementat en **C++** sota el framework docent **EntornVGI** amb **GLFW** i **OpenGL 4.6** modern (VAO, VBO, shaders GLSL i GLM).

---

## Característiques Principals

* **Càlcul d'Efemèrides i Temps:**
  * Posicionament orbital basat en data i hora concretes mitjançant paràmetres keplerians.
  * Control del pas del temps: reproducció en temps real, *timelapse* regulable (ràpid/lent) i pausa.
* **Doble Sistema d'Escala:**
  * **Escala Visual/Intuïtiva:** Relació adaptada per a l'exploració còmoda dels cossos celestes i les seves òrbites.
  * **Escala Real:** Simulació a mida física amb indicadors visuals de localització per a planetes i satèl·lits menors.
* **Cossos Celestes i Efectes Gràfics:**
  * Sol com a font de llum dinàmica principal amb shaders específics.
  * Planetes amb les seves rotacions pròpies i de translació.
  * Satèl·lits naturals (Lluna, satèl·lits galileans, etc.).
  * Cinturó d'asteroides i anells de Saturn tractats modularment.
  * Traçat actiu/desactiu d'òrbites celestes i punts de Lagrange.
* **Càmeres i Controls:**
  * **Càmera Orbital/Fixa:** Enfocament i seguiment relatiu a qualsevol planeta o satèl·lit.
  * **Càmera Nau/Coet:** Navegació espacial en primera i tercera persona controlable per teclat o comandament.
  * **Càmeres Especials:** Sonda Voyager, Estació Espacial Internacional (ISS) i vista des de l'observatori UAB.

---

##  Membres de l'Equip (Grup 05)

| **Lucas Luna Rodriguez** | 
| **Abel Paiz Ilias** | 
| **Ty Devia Ballesteros** | 
| **Carles Peiró** |
| **Noa Gangolells Alcázar** | 
| **Josep Montoro Pascual** | 

---

## Requisits i Compilació

1. **Entorn:** Windows 10/11 amb **Visual Studio 2022 o 2026**.
2. **Dependències:**
   * Suport C++ per a GLFW (instal·lable des de l'instal·lador de Visual Studio).
   * OpenGL 4.6 (compatibilitat de drivers de la GPU).
   * Llibreries incloses a la base de l'entorn: `GLEW`, `GLM`, `SOIL`.
3. **Passos per executar:**
   * Obrir la solució `EntornVGI.sln` (o `.slnx`).
   * Configurar l'arquitectura en **`x64`** i perfil **`Debug`** o **`Release`**.
   * Compilar amb `Ctrl + Shift + B` (o *Compilar Solució*).
   * Executar amb `F5`.

---
