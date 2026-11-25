#include "escenari.h"
#include <cmath>
#include <vector>

// NOTA: no s'ha d'incloure "main.cpp" ni "main.h" aqui (definirien totes les variables
// globals dues vegades -> error de linker LNK2005). El shader es rep com a parametre.

// Copia matrius i llums del shader Phong al shader de normal mapping (perque tots dos il.luminin igual)
static void copiaUniformsShader(GLuint src, GLuint dst)
{
	auto locs = [&](const char* nom, GLint& ls, GLint& ld) {
		ls = glGetUniformLocation(src, nom);
		ld = glGetUniformLocation(dst, nom);
		return ls >= 0 && ld >= 0;
		};
	auto copiaMat4 = [&](const char* nom) {
		GLint ls, ld; GLfloat m[16];
		if (!locs(nom, ls, ld)) return;
		glGetUniformfv(src, ls, m);
		glProgramUniformMatrix4fv(dst, ld, 1, GL_FALSE, m);
		};
	auto copiaF = [&](const char* nom, int n) {	// vecN de floats (n = 1..4)
		GLint ls, ld; GLfloat v[4];
		if (!locs(nom, ls, ld)) return;
		glGetUniformfv(src, ls, v);
		if (n == 1) glProgramUniform1fv(dst, ld, 1, v);
		else if (n == 3) glProgramUniform3fv(dst, ld, 1, v);
		else glProgramUniform4fv(dst, ld, 1, v);
		};
	auto copiaI = [&](const char* nom) {		// int / bool
		GLint ls, ld, v;
		if (!locs(nom, ls, ld)) return;
		glGetUniformiv(src, ls, &v);
		glProgramUniform1i(dst, ld, v);
		};

	copiaMat4("projectionMatrix");
	copiaMat4("viewMatrix");

	// normalMatrix (solo la usa el frag para spots): inversa-transpuesta de la vista
	GLint lsV = glGetUniformLocation(src, "viewMatrix");
	GLint ldN = glGetUniformLocation(dst, "normalMatrix");
	if (lsV >= 0 && ldN >= 0) {
		glm::mat4 V; glGetUniformfv(src, lsV, &V[0][0]);
		glm::mat4 NM = glm::transpose(glm::inverse(V));
		glProgramUniformMatrix4fv(dst, ldN, 1, GL_FALSE, &NM[0][0]);
	}

	copiaF("LightModelAmbient", 4);
	copiaI("fixedLight");

	char n[64];
	for (int i = 0; i < 8; i++) {
		sprintf_s(n, "LightSource[%d].sw_light", i);		copiaI(n);
		sprintf_s(n, "LightSource[%d].position", i);		copiaF(n, 4);
		sprintf_s(n, "LightSource[%d].ambient", i);		copiaF(n, 4);
		sprintf_s(n, "LightSource[%d].diffuse", i);		copiaF(n, 4);
		sprintf_s(n, "LightSource[%d].specular", i);		copiaF(n, 4);
		sprintf_s(n, "LightSource[%d].attenuation", i);	copiaF(n, 3);
		sprintf_s(n, "LightSource[%d].restricted", i);		copiaI(n);
		sprintf_s(n, "LightSource[%d].spotDirection", i);	copiaF(n, 3);
		sprintf_s(n, "LightSource[%d].spotCosCutoff", i);	copiaF(n, 1);
		sprintf_s(n, "LightSource[%d].spotExponent", i);	copiaF(n, 1);
	}
}

void Escenari::creaEscenari(GLuint& shader_programID, GLuint& shaderNormalMapID)
{
	std::vector<char*> texturePaths;
	texturePaths.push_back((char*)".\\textures\\sun\\\\2k_sun.jpg");
	texturePaths.push_back((char*)".\\textures\\planet\\Mercury\\2k_mercury.jpg");	// 1
	texturePaths.push_back((char*)".\\textures\\planet\\Venus\\2k_venus_surface.jpg");		// 2
	texturePaths.push_back((char*)".\\textures\\planet\\Earth\\earth2048.bmp");		// 3
	texturePaths.push_back((char*)".\\textures\\planet\\Mars\\2k_mars.jpg");		// 4
	texturePaths.push_back((char*)".\\textures\\planet\\Jupiter\\2k_jupiter.jpg");	// 5
	texturePaths.push_back((char*)".\\textures\\planet\\Saturn\\2k_saturn.jpg");	// 6
	texturePaths.push_back((char*)".\\textures\\planet\\Uranus\\2k_uranus.jpg");	// 7
	texturePaths.push_back((char*)".\\textures\\planet\\Neptune\\2k_neptune.jpg");		// 8
	texturePaths.push_back((char*)".\\textures\\planet\\Moon\\2k_moon.jpg");	// Lluna (cos 9)
	texturePaths.push_back((char*)".\\textures\\planet\\Saturn\\2k_saturn_ring_alpha.png");	// 6

	// Mapas normales de los planetas
	std::vector<char*> normalMapPaths;
	normalMapPaths.push_back(nullptr); // Sol
	normalMapPaths.push_back((char*)".\\textures\\planet\\NormalTextures\\NormalMapMercury.png");
	normalMapPaths.push_back((char*)".\\textures\\planet\\NormalTextures\\NormalMapVenus.png");
	normalMapPaths.push_back((char*)".\\textures\\planet\\NormalTextures\\NormalMapEarth.png");
	normalMapPaths.push_back((char*)".\\textures\\planet\\NormalTextures\\NormalMapMars.png");
	normalMapPaths.push_back((char*)".\\textures\\planet\\NormalTextures\\NormalMapJupiter.png");
	normalMapPaths.push_back((char*)".\\textures\\planet\\NormalTextures\\NormalMapSaturn.png");
	normalMapPaths.push_back(nullptr); // Urano: sin mapa normal
	normalMapPaths.push_back((char*)".\\textures\\planet\\NormalTextures\\NormalMapNeptune.png");
	normalMapPaths.push_back(nullptr);
	normalMapPaths.push_back(nullptr);

	m_objectList.resize(11);

	CColor colObj;
	colObj.r = 1.0; colObj.g = 1.0; colObj.b = 1.0; colObj.a = 1.0;


	glUseProgram(shader_programID);

	deleteVAOList(GLU_SPHERE);
	deleteVAOList(GLU_DISK);
	deleteVAOList(GLUT_TORUS);
	SetColor4d(colObj.r, colObj.g, colObj.b, colObj.a);
	Set_VAOList(GLU_SPHERE, loadgluSphere_EBO(1.0, 40, 40));				// Esfera de radi 1 (cossos)
	Set_VAOList(GLU_DISK, loadgluDisk_EBO(8.0f, 12.0f, 80, 5)); //anell saturn
	Set_VAOList(GLUT_TORUS, loadglutSolidTorus_EBO(0.005, 1.0, 8, 120));	// Anell fi de radi 1 (orbites)


	for (int i = 0; i < m_objectList.size(); i++)
		if (m_objectList[i].m_textureID) { glDeleteTextures(1, &m_objectList[i].m_textureID); m_objectList[i].m_textureID = 0; }

	// Carregar textures i configurar parametres de filtratge i repeticio un sol cop
	for (int i = 0; i < m_objectList.size(); i++)
	{
		if (texturePaths[i] == nullptr) continue;
		m_objectList[i].m_textureID = loadIMA_SOIL(texturePaths[i]);
		if (m_objectList[i].m_textureID == 0) {
			fprintf(stderr, "No s'ha pogut carregar: %s\n", texturePaths[i]);
		}
		else {
			glBindTexture(GL_TEXTURE_2D, m_objectList[i].m_textureID);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, 8.0f);
		}
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	//carrega mapas normales
	glActiveTexture(GL_TEXTURE0);
	for (int i = 0; i < m_objectList.size(); i++)
		if (m_objectList[i].m_normalMapID != 0){ glDeleteTextures(1, &m_objectList[i].m_normalMapID); m_objectList[i].m_normalMapID = 0; }

	//Load normal maps
	for (int i = 0; i < m_objectList.size(); i++){
		if (normalMapPaths[i] == nullptr) continue;

		m_objectList[i].m_normalMapID = loadIMA_SOIL(normalMapPaths[i]);

		if (m_objectList[i].m_normalMapID == 0) {
			fprintf(stderr, "No s'ha pogut carregar el mapa normal: %s\n",
				normalMapPaths[i]);
		}
		else {
			glBindTexture(GL_TEXTURE_2D, m_objectList[i].m_normalMapID);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, 8.0f);
		}
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	glUseProgram(shaderNormalMapID);
	glUniform1i( glGetUniformLocation(shaderNormalMapID, "texture0"), 0);
	glUniform1i(glGetUniformLocation(shaderNormalMapID, "normalMap"), 1);
	glUniform1i(glGetUniformLocation(shaderNormalMapID, "flag_invert_y"), GL_TRUE);

	glUseProgram(shader_programID);
	glUniform1i(glGetUniformLocation(shader_programID, "texture0"), GLint(0));
	glUniform1i(glGetUniformLocation(shader_programID, "flag_invert_y"), GL_TRUE);

	//Initialize class variables
	// NOTA: la Lluna (index 9) orbita la Terra, no el Sol: el seu radi i angle d'orbita son relatius a la Terra
	const float radiCos[11] = {
		16.0f,  // Sol
		2.0f,   // Mercurio
		3.5f,   // Venus
		3.7f,   // Tierra
		2.8f,   // Marte
		9.0f,   // Júpiter
		7.5f,   // Saturno
		5.5f,   // Urano
		5.3f,   // Neptuno
		1.0f,    // Luna (0.27 x Tierra, proporcio real 1737 km / 6371 km)
		1.0f   // anell
	};

	const float radiOrbita[11] = {
		0.0f,   // Sol
		27.0f,  // Mercurio
		40.0f,  // Venus
		55.0f,  // Tierra
		70.0f,  // Marte
		105.0f, // Júpiter
		140.0f, // Saturno
		175.0f, // Urano
		210.0f, // Neptuno
		7.0f,    // Luna (distancia a la Tierra)
		140.0f // anell
	};

	const float angleOrbita[11] = { 0.0f,  0.0f, 40.0f, 80.0f,120.0f,160.0f,200.0f,240.0f,280.0f, 0.0f, 200.0f };

	for (int i = 0; i < m_objectList.size(); i++)
	{
		m_objectList[i].m_radiCos = radiCos[i];
		m_objectList[i].m_radiOrbita = radiOrbita[i];
		m_objectList[i].m_angleOrbita = angleOrbita[i];
	}

	// Orbites: un unic cercle de linies de radi 1, compartit per les 8 orbites planetaries i la lunar
	creaOrbita();

	for (int i = 0; i < m_objectList.size(); i++)
	{
		m_objectList[i].m_angleRotacio = 0.0f;
	}

	//velocitats al voltant de sol
	float x = 3650;
	// Velocitats angulars orbitals en graus per segon (molt mes suaus i proporcionals a la distancia al Sol)
	m_objectList[0].m_velAngular = 0.0f; // Sol (estatic al centre)
	m_objectList[1].m_velAngular = x / 88.0f; // Mercuri
	m_objectList[2].m_velAngular = x / 225.0f; // Venus
	m_objectList[3].m_velAngular = x / 365.0f; // Terra
	m_objectList[4].m_velAngular = x / 687.0f; // Mart
	m_objectList[5].m_velAngular = x / 4333.0f; // Jupiter
	m_objectList[6].m_velAngular = x / 10756.0f; // Saturn
	m_objectList[7].m_velAngular = x / 30687.0f; // Ura
	m_objectList[8].m_velAngular = x / 60190.0f; // Neptu
	m_objectList[9].m_velAngular = x / 27.32f; // Lluna (al voltant de la Terra)

	float factorRotacio = 1.0f;
	// Velocitats de rotació sobre el propi eix
	m_objectList[0].m_velRotacio = 14.184f * factorRotacio;    // Sol (360 / 25.38 dies, periode sideri a l'equador)
	m_objectList[1].m_velRotacio = 6.136f * factorRotacio;     // Mercuri
	m_objectList[2].m_velRotacio = -1.481f * factorRotacio;    // Venus
	m_objectList[3].m_velRotacio = 361.015f * factorRotacio;   // Terra
	m_objectList[4].m_velRotacio = 351.220f * factorRotacio;   // Mart
	m_objectList[5].m_velRotacio = 871.249f * factorRotacio;   // Jupiter
	m_objectList[6].m_velRotacio = 818.182f * factorRotacio;   // Saturn
	m_objectList[7].m_velRotacio = -501.393f * factorRotacio;  // Urà
	m_objectList[8].m_velRotacio = 545.455f * factorRotacio;   // Neptú
	m_objectList[9].m_velRotacio = 0.0f;   // Lluna: rotacio sincrona, es calcula a updateVariables()

}

// creaOrbita: genera un cercle de radi 1 al pla XY com a GL_LINE_LOOP.
// Abans les orbites eren tors 3D de gruix 0.03: vistes de lluny feien menys d'un pixel d'amplada
// i el rasteritzador les dibuixava discontinues. Una linia sempre ocupa com a minim 1 pixel continu,
// i amb el multisampling (MSAA, activat a main.cpp) les seves vores queden suavitzades.
void Escenari::creaOrbita()
{
	const int nSegments = 512;	// Prou punts perque no es vegin trams rectes ni a l'orbita de Neptu
	std::vector<double> vertices, normals, colors, textures;
	vertices.reserve(nSegments * 3);	normals.reserve(nSegments * 3);
	colors.reserve(nSegments * 4);		textures.reserve(nSegments * 2);

	for (int k = 0; k < nSegments; k++)
	{
		double a = 2.0 * PI * k / nSegments;
		vertices.push_back(cos(a));		vertices.push_back(sin(a));		vertices.push_back(0.0);
		// Normal cap al centre de l'orbita (el Sol): la cara de la linia que "mira" el Sol rep la llum difusa
		normals.push_back(-cos(a));		normals.push_back(-sin(a));		normals.push_back(0.0);
		colors.push_back(0.7);	colors.push_back(0.7);	colors.push_back(0.7);	colors.push_back(1.0);
		textures.push_back(0.0);	textures.push_back(0.0);
	}

	alliberaOrbita();	// Si l'escenari es torna a crear (canvi de mode), no deixar el VAO anterior a la GPU
	m_orbitVAO = load_TRIANGLES_VAO(vertices, normals, colors, textures);	// VAO generic: posicio, color, normal, textura
	m_orbitVAO.eboId = 0;	// load_TRIANGLES_VAO no crea EBO ni inicialitza aquest camp
}

// Inclinacio del pla orbital de la Lluna respecte l'ecliptica (graus). Font: docs/planetes/planetes.json
static const float INCLINACIO_ORBITA_LLUNA = 5.145f;

// Matriu del sistema de referencia de l'orbita lunar: centrat a la Terra i inclinat respecte l'ecliptica.
// La fan servir tant l'orbita lunar com la Lluna, perque quedin sempre alineades.
static glm::mat4 matriuOrbitaLluna(const glm::mat4& MatriuTG, const Object& terra)
{
	float xT = terra.m_radiOrbita * (float)cos(terra.m_angleOrbita * PI / 180.0);
	float yT = terra.m_radiOrbita * (float)sin(terra.m_angleOrbita * PI / 180.0);
	glm::mat4 M = glm::translate(MatriuTG, vec3(xT, yT, 0.0f));
	return glm::rotate(M, glm::radians(INCLINACIO_ORBITA_LLUNA), vec3(1.0f, 0.0f, 0.0f));
}



// dibuixaEscenari: renderitza orbites (si mostrarOrbites es true des del menu Visualitzacio),
// anells de Saturn i cossos celestes amb Phong i normal mapping.
void Escenari::dibuixaEscenari(GLuint sh_programID, GLuint shaderNormalMapID, glm::mat4 MatriuVista, glm::mat4 MatriuTG, bool sw_mat[5], bool mostrarOrbites)
{
	(void)MatriuVista;
	CColor col_blanc, col_orbita;
	col_blanc.r = 1.0;	col_blanc.g = 1.0;	col_blanc.b = 1.0;	col_blanc.a = 1.0;
	col_orbita.r = 0.7;	col_orbita.g = 0.7;	col_orbita.b = 0.7;	col_orbita.a = 1.0;

	glm::mat4 ModelMatrix(1.0);
	int i;
	// Desactivar emissio per a orbites i anell de Saturn (nomes el Sol emet llum propia)
	bool sw_mat_no_emisio[5] = { false, sw_mat[1], sw_mat[2], sw_mat[3], sw_mat[4] };

	// 1. Orbites: anell fi sense textura, escalat al radi de cada orbita (condicionat a mostrarOrbites)
	if (mostrarOrbites) {
		glUniform1i(glGetUniformLocation(sh_programID, "textur"), GL_FALSE);
		SeleccionaColorMaterial(sh_programID, col_orbita, sw_mat_no_emisio);
		GLint locModelMatOrb = glGetUniformLocation(sh_programID, "modelMatrix");
		GLint locTwoSidedOrb = glGetUniformLocation(sh_programID, "sw_twoSided");
		if (m_orbitVAO.vaoId != 0) glBindVertexArray(m_orbitVAO.vaoId);
		for (i = 0; i < m_objectList.size(); i++)
		{
			// Les orbites planetaries es centren al Sol; la de la Lluna (i == 9) a la Terra
			glm::mat4 MatriuCentre = (i == 9) ? matriuOrbitaLluna(MatriuTG, m_objectList[3]) : MatriuTG;
			ModelMatrix = glm::scale(MatriuCentre, vec3(m_objectList[i].m_radiOrbita, m_objectList[i].m_radiOrbita, m_objectList[i].m_radiOrbita));
			glUniformMatrix4fv(locModelMatOrb, 1, GL_FALSE, &ModelMatrix[0][0]);

			// L'orbita lunar no esta centrada al Sol: la seva normal no sempre mira cap a la llum.
			// Amb il.luminacio bilateral (mateix uniform que l'anell de Saturn) mai queda totalment fosca.
			if (locTwoSidedOrb != -1) glUniform1i(locTwoSidedOrb, (i == 9));

			if (m_orbitVAO.vaoId != 0)
				glDrawArrays(GL_LINE_LOOP, 0, m_orbitVAO.nVertexs);
		}
		if (locTwoSidedOrb != -1) glUniform1i(locTwoSidedOrb, GL_FALSE);
		glBindVertexArray(0);
	}

	// 3. Cossos: esfera amb la seva textura, situada sobre la seva òrbita
	// Preparamos el shader de normal mapping una sola vez antes del bucle
	if (shaderNormalMapID != 0) {
		glUseProgram(shaderNormalMapID);
		copiaUniformsShader(sh_programID, shaderNormalMapID);
		glUniform1i(glGetUniformLocation(shaderNormalMapID, "textur"), GL_TRUE);
		glUniform1i(glGetUniformLocation(shaderNormalMapID, "modulate"), GL_TRUE);
		glUniform1i(glGetUniformLocation(shaderNormalMapID, "flag_invert_y"), GL_TRUE);
		SeleccionaColorMaterial(shaderNormalMapID, col_blanc, sw_mat);
	}

	// Preparamos el shader estándar (Phong)
	glUseProgram(sh_programID);
	glUniform1i(glGetUniformLocation(sh_programID, "textur"), GL_TRUE);
	glUniform1i(glGetUniformLocation(sh_programID, "modulate"), GL_TRUE);
	glUniform1i(glGetUniformLocation(sh_programID, "flag_invert_y"), GL_TRUE);
	SeleccionaColorMaterial(sh_programID, col_blanc, sw_mat);

	GLuint shaderActual = sh_programID;

	GLint locTwoSided = glGetUniformLocation(sh_programID, "sw_twoSided");
	for (i = 0; i < m_objectList.size(); i++)
	{
		bool esAnell = (i == 10);
		bool usaNormalMap = (m_objectList[i].m_normalMapID != 0 && shaderNormalMapID != 0);
		GLuint shaderObjetivo = usaNormalMap ? shaderNormalMapID : sh_programID;

		// Cambiar de shader solo cuando sea necesario
		if (shaderActual != shaderObjetivo) {
			glUseProgram(shaderObjetivo);
			shaderActual = shaderObjetivo;
		}

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_objectList[i].m_textureID);
		// Mapa normal (unidad 1)
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, usaNormalMap ? m_objectList[i].m_normalMapID : 0);

		int idxOrbita = esAnell ? 6 : i;
		float x = m_objectList[idxOrbita].m_radiOrbita * (float)cos(m_objectList[idxOrbita].m_angleOrbita * PI / 180.0);
		float y = m_objectList[idxOrbita].m_radiOrbita * (float)sin(m_objectList[idxOrbita].m_angleOrbita * PI / 180.0);

		// Cálculo de posición y matriz de modelo (ahora sirve para todos, incluida la Luna)
		glm::mat4 MatriuCentre = (i == 9) ? matriuOrbitaLluna(MatriuTG, m_objectList[3]) : MatriuTG;
		ModelMatrix = glm::translate(MatriuCentre, vec3(x, y, 0.0f));
		if (esAnell)
			ModelMatrix = glm::rotate(ModelMatrix, glm::radians(20.0f), vec3(1.0f, 0.0f, 0.0f));
		else
			ModelMatrix = glm::rotate(ModelMatrix, glm::radians(m_objectList[i].m_angleRotacio), vec3(0.0f, 0.0f, 1.0f));
		ModelMatrix = glm::scale(ModelMatrix, vec3(m_objectList[i].m_radiCos, m_objectList[i].m_radiCos, m_objectList[i].m_radiCos));

		glUniformMatrix4fv(glGetUniformLocation(shaderActual, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniform4i(glGetUniformLocation(shaderActual, "sw_intensity"), (i == 0 && sw_mat[0]), sw_mat[1], sw_mat[2], sw_mat[3]);

		if (esAnell) {
			if (locTwoSided != -1) glUniform1i(locTwoSided, GL_TRUE);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			draw_TriEBO_Object(GLU_DISK);

			glDisable(GL_BLEND);
			if (locTwoSided != -1) glUniform1i(locTwoSided, GL_FALSE);
		}
		else {
			draw_TriEBO_Object(GLU_SPHERE);
		}
	}

	// Restaurar estado de texturas y volver al shader principal
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);

	if (shaderActual != sh_programID) {
		glUseProgram(sh_programID);
	}

	glUniform4i(glGetUniformLocation(sh_programID, "sw_intensity"), sw_mat[0], sw_mat[1], sw_mat[2], sw_mat[3]);
	glUniform1i(glGetUniformLocation(sh_programID, "textur"), GL_FALSE);
}

// updateVariables: avanca les orbites i rotacions dels 10 cossos segons els controls de la UI:
// - enPausa: atura completament el moviment quan l'usuari prem Pausa o Espai.
// - direccioTemps: +1 (endavant) o -1 (rebobinar) des del panell temporal / Backspace.
// - escalaReal + factorVelocitat: ajusta el pas orbital segons el timelapse actiu en mode ESCALA_REAL.
void Escenari::updateVariables(double dt, bool enPausa, int direccioTemps, bool escalaReal, double factorVelocitat)
{
	if (m_objectList.empty() || enPausa) return;

	// Limit de seguretat per a dt
	if (dt < 0.0) dt = 0.0;
	if (dt > 0.1) dt = 0.1;

	// Factor de temps per al moviment orbital
	float factorTemps = 1.0f;
	if (escalaReal) factorTemps = (float)std::sqrt(factorVelocitat / 300.0);

	for (int i = 0; i < m_objectList.size(); i++)
	{
		// Gir orbital al voltant del Sol (la Lluna, i == 9, al voltant de la Terra)
		if (i > 0)
		{
			float incrementOrbita = m_objectList[i].m_velAngular * (float)dt * (float)direccioTemps * factorTemps;
			m_objectList[i].m_angleOrbita += incrementOrbita;
			while (m_objectList[i].m_angleOrbita >= 360.0f) m_objectList[i].m_angleOrbita -= 360.0f;
			while (m_objectList[i].m_angleOrbita < 0.0f) m_objectList[i].m_angleOrbita += 360.0f;
		}

		m_objectList[i].m_angleRotacio += m_objectList[i].m_velRotacio * (float)dt * (float)direccioTemps;
		while (m_objectList[i].m_angleRotacio >= 360.0f) m_objectList[i].m_angleRotacio -= 360.0f;
		while (m_objectList[i].m_angleRotacio < 0.0f) m_objectList[i].m_angleRotacio += 360.0f;
	}

	// Lluna: rotacio sincrona (sempre mostra la mateixa cara a la Terra).
	// El centre de la textura (cara visible, u = 0.5) queda a la direccio (0,-1,0) de l'esfera de
	// glut_geometry. Girar-la (angleOrbita - 90) graus la fa apuntar sempre cap a la Terra.
	m_objectList[9].m_angleRotacio = m_objectList[9].m_angleOrbita - 90.0f;
}


