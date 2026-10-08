# **Casos d'Ús per al Simulador del Sistema Solar**

## **Cas d'Ús 1: Consultar la configuració del sistema en una data i hora específiques**

* **Actor principal:** Usuari de l'aplicació.  
* **Descripció:** L'usuari vol visualitzar amb un alt grau de realisme la posició exacta dels planetes, els seus satèl·lits i les seves rotacions en un moment temporal determinat.  
* **Flux principal:**  
  1. L'usuari accedeix a l'índex o menú interactiu de l'aplicació.  
  2. L'usuari introdueix una data específica (dia, mes i any) i una hora exacta (hora, minut i segon).  
  3. El sistema calcula i renderitza la nova posició espacial i l'estat de rotació dels planetes, satèl·lits (incloent-hi Plutó i les llunes), i altres cossos celestes segons el moment sol·licitat.

## **Cas d'Ús 2: Control de la velocitat del temps (Timelapse)**

* **Actor principal:** Usuari de l'aplicació.  
* **Descripció:** L'usuari vol observar el moviment de translació i les dues rotacions pròpies dels cossos celestes a diferents velocitats per entendre'n la dinàmica al llarg del temps.  
* **Flux principal:**  
  1. Des del menú interactiu, l'usuari accedeix a les funcions de temps (timelapse).  
  2. L'usuari selecciona entre les opcions temporals disponibles: accelerar el temps (opció ràpida), alentir el temps (opció lenta) o aturar el temps completament.  
  3. El sistema ajusta la velocitat de moviment de les òrbites i les rotacions planetàries en temps real segons l'opció triada.

## **Cas d'Ús 3: Exploració de l'espai mitjançant canvi de càmeres i perspectives**

* **Actor principal:** Usuari de l'aplicació.  
* **Descripció:** L'usuari vol experimentar el Sistema Solar des de diferents punts de vista per tal d'explorar l'espai lliurement o fer seguiment d'elements concrets.  
* **Flux principal:**  
  1. L'usuari obre el menú principal per gestionar les perspectives.  
  2. L'usuari selecciona un dels tipus de càmera disponibles:  
     * **Càmera lliure (tipus coet):** Pren el control manual amb tecles o gamepad i pot alternar entre primera o tercera persona.  
     * **Càmera fixa:** Activa una vista de tipus orbital.  
     * **Càmera de satèl·lit artificial:** Situa la visió als punts de vista de l'Estació Espacial Internacional (ISS) o de la sonda Voyager.  
  3. El sistema reposiciona l'espectador i actualitza l'entorn visual.

## **Cas d'Ús 4: Configuració de la visualització gràfica (Escales, Etiquetes i Òrbites)**

* **Actor principal:** Usuari de l'aplicació.  
* **Descripció:** L'usuari personalitza l'aspecte de la simulació per obtenir una vista més didàctica o bé una visió completament escalada del Sistema Solar.  
* **Flux principal:**  
  1. L'usuari accedeix a les opcions de visualització de la interfície.  
  2. L'usuari pot commutar les següents funcionalitats:  
     * Canviar el sistema d'escales (entre escala real i escala intuïtiva/ortogràfica).  
     * Activar/desactivar un indicador visual per als planetes més petits.  
     * Mostrar/ocultar les etiquetes identificatives dels planetes.  
     * Mostrar/ocultar la representació visual de les òrbites de cada cos per a una vista més "natural".  
  3. El sistema actualitza immediatament la renderització aplicant els paràmetres seleccionats.

## **Cas d'Ús 5: Observació de cossos menors i punts d'interès (Asteroides i Lagrange)**

* **Actor principal:** Usuari de l'aplicació.  
* **Descripció:** L'usuari explora elements secundaris del Sistema Solar per observar fenòmens geomètrics i posicions estructurals especials.  
* **Flux principal:**  
  1. L'usuari navega cap a les zones d'interès determinades de l'espai o activa la seva visualització.  
  2. L'usuari observa el cinturó d'asteroides, analitzant les seves transformacions geomètriques en temps real.  
  3. L'usuari visualitza els punts de Lagrange al sistema per comprendre el comportament gravitacional en aquestes coordenades.

## **Cas d'Ús 6: Simulació d'interaccions físiques i col·lisions**

* **Actor principal:** Usuari de l'aplicació.  
* **Descripció:** L'usuari activa el motor de físiques avançades per observar com interactuen els cossos celestes de manera dinàmica més enllà de l'òrbita estàndard.  
* **Flux principal:**  
  1. L'usuari activa el mode de simulació física des del menú.  
  2. El sistema aplica diferents forces gravitacionals per simular la interacció realista entre els planetes.  
  3. L'usuari enfoca la càmera als asteroides de Saturn per avaluar què passa quan es produeix un xoc o col·lisió realista entre aquests elements.