# Optimitzacions de Rendiment i Resolució de Col·lapses del Pipeline Gràfic

## 1. Context i Diagnòstic del Problema

Després dels darrers canvis introduïts en les branques de desenvolupament, es va detectar una caiguda dràstica i crítica en la taxa de fotogrames per segon (FPS), passant d'un rendiment molt elevat (>3000 FPS en maquinari dedicat) a xifres molt reduïdes i amb interrupcions en el renderitzat.

Una anàlisi exhaustiva del codi font i de l'estat d'OpenGL va revelar diversos problemes crítics de gestió de memòria, crides invàlides al controlador gràfic i colls d'ampolla a la CPU i la GPU.

---

## 2. Causes de la Pèrdua de Rendiment

### 2.1. Bug Crític d'Accés Fora de Rang i Dibuix Invàlid (`m_orbitVAO[-1]`)
* **Ubicació:** `Escenari::dibuixaEscenari()` (`escenari.cpp`)
* **Causa:** El bucle de renderitzat de les òrbites es va modificar per començar a `i = 0` en lloc de `i = 1`:
  ```cpp
  for (i = 0; i < 9; i++) {
      CVAO& orbita = m_orbitVAO[i - 1]; // Quan i = 0 -> m_orbitVAO[-1]!
      if (orbita.vaoId != 0) {
          glBindVertexArray(orbita.vaoId);
          glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, orbita.eboId);
          glDrawElements(GL_TRIANGLES, orbita.nIndices, GL_UNSIGNED_INT, nullptr);
      }
  }
  ```
* **Impacte:** L'array `m_orbitVAO` té mida 8 (ja que el Sol, índex 0, no orbita). En avaluar `i = 0`, s'accedia a memòria contigua prèvia (`m_colObj`, de tipus `CColor` amb valors `GLdouble` $1.0$). En interpretar aquests bits com a estructura `CVAO`, `orbita.vaoId` i `orbita.nIndices` prenien valors brossa d'aproximadament **$1.072.693.248$**. A cada fotograma, el driver d'OpenGL rebia una ordre de dibuixar més de **1.000 milions d'índexs** sobre un buffer inexistent, provocant parades crítiques del pipeline de la GPU (*pipeline stalls*) i errors continus de controlador.

### 2.2. Destrucció i Corrupció del VAO de la Skybox
* **Ubicació:** `Escenari::creaEscenari()` (`escenari.cpp`) i `main.cpp`
* **Causa:** `creaEscenari()` executava `netejaVAOList()`, el qual recorre la taula global de VAOs i elimina tots els identificadors de la memòria de vídeo, inclòs `VAOList[CUBE_SKYBOX]`. Com que la variable `skC_VAOID.vaoId` a `main.cpp` no es reiniciava a `0`, `OnVistaSkyBox()` no tornava a generar el cub i s'intentava dibuixar la Skybox amb un VAO destruït.
* **Impacte:** Crides contínues de dibuix amb estat invàlid d'OpenGL que afegien sobrecàrrega de gestió d'errors al controlador.

### 2.3. Reconfiguració Redundant de Textures per Fotograma
* **Ubicació:** `Escenari::dibuixaEscenari()` (`escenari.cpp`)
* **Causa:** A cada frame, per a cadascun dels 9 cossos celestes i l'anell de Saturn, es cridava a `SetTextureParameters(0, m_texturesID[i], ...)`.
* **Impacte:** S'executaven múltiples crides a `glTexParameteri` (`GL_TEXTURE_WRAP_S`, `GL_TEXTURE_WRAP_T`, `GL_TEXTURE_MIN_FILTER`, `GL_TEXTURE_MAG_FILTER`) desenes de vegades per frame de forma innecessària.

### 2.4. Consultes Repetitives d'Uniforms a la GPU (`glGetUniformLocation`)
* **Ubicació:** `Escenari::dibuixaEscenari()` (`escenari.cpp`)
* **Causa:** Dins dels bucles d'òrbites i de planetes es consultava `glGetUniformLocation(sh_programID, "modelMatrix")` i `"sw_intensity"` repetidament en lloc d'emmagatzemar l'identificador.

### 2.5. Actualització de Rotació Pròpia sense Diferencial Temporal (`dt`)
* **Ubicació:** `Escenari::updateVariables()` (`escenari.cpp`)
* **Causa:** L'increment de l'angle de rotació sobre l'eix propi s'executava sense factor `dt`:
  ```cpp
  m_objectList[i].m_angleRotacio += m_objectList[i].m_velRotacio * (float)direccioTemps;
  ```
* **Impacte:** Amb valors de velocitat de fins a $871.25^\circ/\text{s}$, a altes freqüències de refresc els astres giraven centenars de milers de graus per segon i requerien iteracions de bucles `while` de normalització contínues.

---

## 3. Optimitzacions i Solucions Implementades

### 3.1. Correcció del Bucle d'Òrbites
S'ha restablert el rang d'iteració correcte de les òrbites astronòmiques (del planeta `1: Mercuri` al `8: Neptú`), associant correctament el color i el shader de material:
```cpp
// 1. Òrbites: anell fi sense textura, escalat al radi de cada òrbita
if (mostrarOrbites) {
    glUniform1i(glGetUniformLocation(sh_programID, "textur"), GL_FALSE);
    SeleccionaColorMaterial(sh_programID, col_orbita, sw_mat);
    GLint locModelMatOrb = glGetUniformLocation(sh_programID, "modelMatrix");
    for (i = 1; i < 9; i++)
    {
        ModelMatrix = glm::scale(MatriuTG, vec3(m_objectList[i].m_radiOrbita, ...));
        glUniformMatrix4fv(locModelMatOrb, 1, GL_FALSE, &ModelMatrix[0][0]);
        CVAO& orbita = m_orbitVAO[i - 1];
        if (orbita.vaoId != 0)
        {
            glBindVertexArray(orbita.vaoId);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, orbita.eboId);
            glDrawElements(GL_TRIANGLES, orbita.nIndices, GL_UNSIGNED_INT, nullptr);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
        }
    }
}
```

### 3.2. Optimització Integral de la Skybox
1. **Activació sota demanda:** A `main()` s'estableix `SkyBoxCube = false;` per defecte, evitant que es renderitzi innecessàriament durant la visualització del selector inicial de modes. S'activa a `SincronitzarEstatSimuladorAmbOpenGL()` (`main.cpp`) un cop l'usuari tria un mode de simulació.
2. **Validació d'identificador a OpenGL:** A `OnVistaSkyBox()` (`main.cpp`) es valida la vigència del VAO mitjançant `glIsVertexArray(skC_VAOID.vaoId)` per regenerar-lo si ha estat purgat.
3. **Optimització del Buffer de Profunditat (`glDepthMask`):** A `dibuixa_Skybox()` (`escena.cpp`):
   - S'afegeix comprovació prèvia de seguretat (`Get_VAOId(CUBE_SKYBOX) == 0`).
   - S'aplica `glDepthMask(GL_FALSE)` abans del dibuix i `glDepthMask(GL_TRUE)` en finalitzar. En no escriure al Z-buffer (ja que el fons està a distància infinita), s'estalvia un consum crític d'amplada de banda de memòria a la GPU.
   - S'ha retirat la crida runtime redundant a `glBindAttribLocation`.

### 3.3. Inicialització Única de Paràmetres de Textura i Enllaç Ràpid
A `creaEscenari()`, els paràmetres de repetició i filtratge s'assignen un sol cop després de carregar les imatges:
```cpp
for (int i = 0; i < 10; i++)
{
    m_texturesID[i] = loadIMA_SOIL(m_fitxers[i]);
    if (m_texturesID[i] != 0) {
        glBindTexture(GL_TEXTURE_2D, m_texturesID[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
}
glBindTexture(GL_TEXTURE_2D, 0);
```
Al bucle de dibuix de `dibuixaEscenari()`, s'activa directament la unitat de textura `GL_TEXTURE0` i s'enllaça el seu identificador amb `glBindTexture(GL_TEXTURE_2D, m_texturesID[i])`, reduint dràsticament el cost de crides per frame.

### 3.4. Cacheig de Localitzacions d'Uniforms
Les localitzacions dels uniforms freqüents (`modelMatrix`, `sw_intensity`) s'obtenen abans dels bucles de cossos i òrbites:
```cpp
GLint locModelMatCos = glGetUniformLocation(sh_programID, "modelMatrix");
GLint locSwInt = glGetUniformLocation(sh_programID, "sw_intensity");
```

### 3.5. Correcció Temporal de la Rotació Pròpia
A `Escenari::updateVariables()`, s'ha incorporat el pas de temps `dt`:
```cpp
m_objectList[i].m_angleRotacio += m_objectList[i].m_velRotacio * (float)dt * (float)direccioTemps;
```

### 3.6. Gestió Segura dels VAOs de l'Escenari
* A `creaEscenari()` s'elimina la crida a `netejaVAOList()` i se substitueix per l'esborrat específic dels recursos de l'escenari (`deleteVAOList(GLU_SPHERE)`, `deleteVAOList(GLU_DISK)`, `deleteVAOList(GLUT_TORUS)`), preservant intactes la resta d'elements (com la Skybox i els eixos).
* A `alliberaEscenari()` (`escenari.h`) s'afegeix l'alliberament explícit dels buffers VBO/EBO i VAO dels 8 anells d'òrbites (`m_orbitVAO[0..7]`) i de les textures carregades.
