#include "escenari.h"
#include <cmath>
#include <vector>

void EsferaInt::drawObject(GLuint& shaderActual, glm::mat4& matriuTG)
{
	glUniform4i(glGetUniformLocation(shaderActual, "sw_intensity"), m_sw_mat[0], m_sw_mat[1], m_sw_mat[2], m_sw_mat[3]);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	if (m_normalMapID != 0)
		glBindTexture(GL_TEXTURE_2D, m_normalMapID);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	if (m_textureID != 0)
		glBindTexture(GL_TEXTURE_2D, m_textureID);

	//
	double x = m_radiOrbita * cos(m_angleOrbita * PI / 180.0);
	double y = m_radiOrbita * sin(m_angleOrbita * PI / 180.0);

	m_trMatrix = glm::translate(matriuTG, vec3(x, y, 0.0f));

	glm::mat4 ModelMatrix = glm::rotate(m_trMatrix, (float)glm::radians(m_angleRotacio), vec3(0.0f, 0.0f, 1.0f));
	ModelMatrix = glm::scale(ModelMatrix, vec3(m_scale));
	glUniformMatrix4fv(glGetUniformLocation(shaderActual, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	//glUniform4i(glGetUniformLocation(shaderActual, "sw_intensity"), (i == 0 && sw_mat[0]), sw_mat[1], sw_mat[2], sw_mat[3]);
	//Shader i tal se assigna fuera antes de cada objeto segun queramos
	draw_TriEBO_Object(GLU_SPHERE);

	for (Object* son : m_sons)
		if (son) son->drawObject(shaderActual, m_trMatrix);
}
void EsferaInt::update(double T)
{
	if (m_radiOrbita > 0.0)
	{
		m_angleOrbita = std::fmod(m_angleOrbitaInicial + m_velAngular * T, 360.0);
		if (m_angleOrbita < 0.0) m_angleOrbita += 360.0;
	}

	m_angleRotacio = std::fmod(m_angleRotacioInicial + m_velRotacio * T, 360.0);
	if (m_angleRotacio < 0.0) m_angleRotacio += 360.0;

	for (Object* son : m_sons)
		if (son) son->update(T);
}



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

GLuint createDefaultNormalMap()
{
	GLuint texID = 0;
	unsigned char flatNormal[3] = { 128, 128, 255 }; // Vector (0, 0, 1)

	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, flatNormal);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	return texID;
}

CelestialBodyId mainBodies[10] = { CB_SUN, CB_MERCURY, CB_VENUS, CB_EARTH, CB_MARS, CB_JUPITER, CB_SATURN, CB_URANUS, CB_NEPTUNE, CB_MOON };

void Escenari::creaEscenari(GLuint& shader_programID, GLuint& shaderNormalMapID)
{	glUseProgram(shaderNormalMapID);
	glUniform1i(glGetUniformLocation(shaderNormalMapID, "texture0"), 0);
	glUniform1i(glGetUniformLocation(shaderNormalMapID, "normalMap"), 1);
	glUniform1i(glGetUniformLocation(shaderNormalMapID, "flag_invert_y"), GL_TRUE);

	glUseProgram(shader_programID);
	glUniform1i(glGetUniformLocation(shader_programID, "texture0"), GLint(0));
	glUniform1i(glGetUniformLocation(shader_programID, "flag_invert_y"), GL_TRUE);


	CColor colObj;
	colObj.r = 1.0; colObj.g = 1.0; colObj.b = 1.0; colObj.a = 1.0;
	deleteVAOList(GLU_SPHERE);
	deleteVAOList(GLU_DISK);
	deleteVAOList(GLUT_TORUS);
	SetColor4d(colObj.r, colObj.g, colObj.b, colObj.a);
	Set_VAOList(GLU_SPHERE, loadgluSphere_EBO(1.0, 40, 40));				// Esfera de radi 1 (cossos)
	Set_VAOList(GLU_DISK, loadgluDisk_EBO(8.0f, 12.0f, 80, 5)); //anell saturn
	Set_VAOList(GLUT_TORUS, loadglutSolidTorus_EBO(0.005, 1.0, 8, 120));	// Anell fi de radi 1 (orbites)

	const double x = 360.0 / 86400; // 360 graus / dies_orbita -> velocitat en graus/dia (1s simulat = 1 dia)
	const double factorRotacio = 1.0 / 86400;
	// Posicions i velocitats astronòmiques exactes a l'època J2000.0 (t = 0)
	m_objectList[CB_SUN] = new EsferaInt("Sol", 16.0, 0.0, 0.00, 0.0, 0.0, 14.184 * factorRotacio); m_objectList[CB_SUN]->m_sw_mat[0] = true;
	m_objectList[CB_MERCURY] = new EsferaInt("Mercuri", 2.0, 27.0, 252.25, x / 87.97, 0.0, 6.136 * factorRotacio);
	m_objectList[CB_VENUS] = new EsferaInt("Venus", 3.5, 40.0, 181.98, x / 224.70, 0.0, -1.481 * factorRotacio);
	m_objectList[CB_EARTH] = new EsferaInt("Terra", 3.7, 55.0, 100.46, x / 365.256, 0.0, 361.015 * factorRotacio);
	m_objectList[CB_MARS] = new EsferaInt("Mart", 2.8, 70.0, 355.45, x / 686.98, 0.0, 351.220 * factorRotacio);
	m_objectList[CB_JUPITER] = new EsferaInt("Júpiter", 9.0, 105.0, 34.40, x / 4332.59, 0.0, 871.249 * factorRotacio);
	m_objectList[CB_SATURN] = new EsferaInt("Saturn", 7.5, 140.0, 49.94, x / 10759.2, 0.0, 818.182 * factorRotacio);
	m_objectList[CB_URANUS] = new EsferaInt("Urà", 5.5, 175.0, 313.23, x / 30688.5, 0.0, -501.393 * factorRotacio);
	m_objectList[CB_NEPTUNE] = new EsferaInt("Neptú", 5.3, 210.0, 304.88, x / 60195.0, 0.0, 545.455 * factorRotacio);
	m_objectList[CB_MOON] = new EsferaInt("Lluna", 1.0, 7.0, 218.32, x / 27.322, 128.32, x / 27.322); // 218.32 - 90.0 per mantenir la cara visible cap a la Terra
	// m_objectList[CB_SATURN_RINGS] = new AnellsInt("Anells de Saturn", 1.0, 0.0, 0.0, 0.0, 0.0, 0.0); // radiOrbita i angleOrbita a 0.0 perquè hereten la posició de Saturn

	// Build scene graph hierarchy
	m_objectList[CB_SUN]->addSon(m_objectList[CB_MERCURY]);
	m_objectList[CB_SUN]->addSon(m_objectList[CB_VENUS]);
	m_objectList[CB_SUN]->addSon(m_objectList[CB_EARTH]);
	m_objectList[CB_SUN]->addSon(m_objectList[CB_MARS]);
	m_objectList[CB_SUN]->addSon(m_objectList[CB_JUPITER]);
	m_objectList[CB_SUN]->addSon(m_objectList[CB_SATURN]);
	m_objectList[CB_SUN]->addSon(m_objectList[CB_URANUS]);
	m_objectList[CB_SUN]->addSon(m_objectList[CB_NEPTUNE]);

	m_objectList[CB_EARTH]->addSon(m_objectList[CB_MOON]);
	// m_objectList[CB_SATURN]->addSon(m_objectList[CB_SATURN_RINGS]);

	// Càrrega de textures difuses
	m_objectList[CB_SUN]->loadTexture(".\\textures\\sun\\2k_sun.jpg");
	m_objectList[CB_MERCURY]->loadTexture(".\\textures\\planet\\Mercury\\2k_mercury.jpg");
	m_objectList[CB_VENUS]->loadTexture(".\\textures\\planet\\Venus\\2k_venus_surface.jpg");
	m_objectList[CB_EARTH]->loadTexture(".\\textures\\planet\\Earth\\earth2048.bmp");
	m_objectList[CB_MARS]->loadTexture(".\\textures\\planet\\Mars\\2k_mars.jpg");
	m_objectList[CB_JUPITER]->loadTexture(".\\textures\\planet\\Jupiter\\2k_jupiter.jpg");
	m_objectList[CB_SATURN]->loadTexture(".\\textures\\planet\\Saturn\\2k_saturn.jpg");
	m_objectList[CB_URANUS]->loadTexture(".\\textures\\planet\\Uranus\\2k_uranus.jpg");
	m_objectList[CB_NEPTUNE]->loadTexture(".\\textures\\planet\\Neptune\\2k_neptune.jpg");
	m_objectList[CB_MOON]->loadTexture(".\\textures\\planet\\Moon\\2k_moon.jpg");
	// m_objectList[CB_SATURN_RINGS]->loadTexture(".\\textures\\planet\\Saturn\\2k_saturn_ring_alpha.png");

	// Càrrega de mapes normals
	m_objectList[CB_SUN]->m_normalMapID = createDefaultNormalMap();
	m_objectList[CB_MERCURY]->loadNormMap(".\\textures\\planet\\NormalTextures\\NormalMapMercury.png");
	m_objectList[CB_VENUS]->loadNormMap(".\\textures\\planet\\NormalTextures\\NormalMapVenus.png");
	m_objectList[CB_EARTH]->loadNormMap(".\\textures\\planet\\NormalTextures\\NormalMapEarth.png");
	m_objectList[CB_MARS]->loadNormMap(".\\textures\\planet\\NormalTextures\\NormalMapMars.png");
	m_objectList[CB_JUPITER]->loadNormMap(".\\textures\\planet\\NormalTextures\\NormalMapJupiter.png");
	m_objectList[CB_SATURN]->loadNormMap(".\\textures\\planet\\NormalTextures\\NormalMapSaturn.png");
	m_objectList[CB_URANUS]->m_normalMapID = createDefaultNormalMap();
	m_objectList[CB_NEPTUNE]->loadNormMap(".\\textures\\planet\\NormalTextures\\NormalMapNeptune.png");
	m_objectList[CB_MOON]->m_normalMapID = createDefaultNormalMap();
	// m_objectList[CB_SATURN_RINGS]->m_normalMapID = baseNorm;

	glBindTexture(GL_TEXTURE_2D, 0);

	// Orbites: un unic cercle de linies de radi 1, compartit per les 8 orbites planetaries i la lunar
	creaOrbita();

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
static glm::mat4 matriuOrbitaLluna(const Object* terra)
{
	return glm::rotate(terra->m_trMatrix, glm::radians((float)INCLINACIO_ORBITA_LLUNA), glm::vec3(1.0f, 0.0f, 0.0f));
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
	if (false && mostrarOrbites) {
		glUniform1i(glGetUniformLocation(sh_programID, "textur"), GL_FALSE);
		SeleccionaColorMaterial(sh_programID, col_orbita, sw_mat_no_emisio);
		GLint locModelMatOrb = glGetUniformLocation(sh_programID, "modelMatrix");
		GLint locTwoSidedOrb = glGetUniformLocation(sh_programID, "sw_twoSided");
		if (m_orbitVAO.vaoId != 0) glBindVertexArray(m_orbitVAO.vaoId);
		for (i = 0; i < m_objectList.size(); i++)
		{
			// Les orbites planetaries es centren al Sol; la de la Lluna (i == 9) a la Terra
			glm::mat4 MatriuCentre = (i == 9) ? matriuOrbitaLluna(m_objectList[CB_EARTH]) : MatriuTG;
			//ModelMatrix = glm::scale(MatriuCentre, vec3(m_objectList[mainBodies[i]]->m_radiOrbita, m_objectList[mainBodies[i]]->m_radiOrbita, m_objectList[mainBodies[i]]->m_radiOrbita));
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
	// Preparamos el shader estándar (Phong)
	glUseProgram(sh_programID);
	glUniform1i(glGetUniformLocation(sh_programID, "textur"), GL_TRUE);
	glUniform1i(glGetUniformLocation(sh_programID, "modulate"), GL_TRUE);
	glUniform1i(glGetUniformLocation(sh_programID, "flag_invert_y"), GL_TRUE);
	SeleccionaColorMaterial(sh_programID, col_blanc, sw_mat);

	// Preparamos el shader de normal mapping una sola vez antes del bucle
	glUseProgram(shaderNormalMapID);
	copiaUniformsShader(sh_programID, shaderNormalMapID);
	glUniform1i(glGetUniformLocation(shaderNormalMapID, "textur"), GL_TRUE);
	glUniform1i(glGetUniformLocation(shaderNormalMapID, "modulate"), GL_TRUE);
	glUniform1i(glGetUniformLocation(shaderNormalMapID, "flag_invert_y"), GL_TRUE);
	SeleccionaColorMaterial(shaderNormalMapID, col_blanc, sw_mat);

	GLuint shaderActual = shaderNormalMapID;

	GLint locTwoSided = glGetUniformLocation(shaderActual, "sw_twoSided");
	//glUniform4i(glGetUniformLocation(shaderActual, "sw_intensity"), (true && sw_mat[0]), sw_mat[1], sw_mat[2], sw_mat[3]);

	for (auto& p : m_objectList)
	{
		CelestialBodyId id = p.first;
		Object* obj = p.second;

		if (!p.second) continue;
		if (p.second->m_parent == nullptr)
			obj->drawObject(shaderActual, MatriuTG);
	}

	for (i = 0; i < m_objectList.size(); i++)
	{
		break;
		bool esAnell = (i == 10);
		bool usaNormalMap = (m_objectList[mainBodies[i]]->m_normalMapID != 0 && shaderNormalMapID != 0);
		GLuint shaderObjetivo = usaNormalMap ? shaderNormalMapID : sh_programID;

		// Cambiar de shader solo cuando sea necesario
		if (shaderActual != shaderObjetivo) {
			glUseProgram(shaderObjetivo);
			shaderActual = shaderObjetivo;
		}

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_objectList[mainBodies[i]]->m_textureID);
		// Mapa normal (unidad 1)
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, usaNormalMap ? m_objectList[mainBodies[i]]->m_normalMapID : 0);

		int idxOrbita = esAnell ? 6 : i;
		//float x = m_objectList[mainBodies[idxOrbita]]->m_radiOrbita * (float)cos(m_objectList[mainBodies[idxOrbita]]->m_angleOrbita * PI / 180.0);
		//float y = m_objectList[mainBodies[idxOrbita]]->m_radiOrbita * (float)sin(m_objectList[mainBodies[idxOrbita]]->m_angleOrbita * PI / 180.0);

		//// Cálculo de posición y matriz de modelo (ahora sirve para todos, incluida la Luna)
		//glm::mat4 MatriuCentre = (i == 9) ? matriuOrbitaLluna(m_objectList[CB_EARTH]) : MatriuTG;
		//ModelMatrix = glm::translate(MatriuCentre, vec3(x, y, 0.0f));
		//if (esAnell)
		//	ModelMatrix = glm::rotate(ModelMatrix, glm::radians(20.0f), vec3(1.0f, 0.0f, 0.0f));
		//else
		//	ModelMatrix = glm::rotate(ModelMatrix, glm::radians(m_objectList[mainBodies[i]]->m_angleRotacio), vec3(0.0f, 0.0f, 1.0f));
		//ModelMatrix = glm::scale(ModelMatrix, vec3(m_objectList[mainBodies[i]]->m_radiCos, m_objectList[mainBodies[i]]->m_radiCos, m_objectList[mainBodies[i]]->m_radiCos));
		//m_objectList[mainBodies[i]]->m_mMatrix = ModelMatrix;
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
	glBindTexture(GL_TEXTURE_2D, 0);

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
void Escenari::updateVariables(double t, double dt, bool enPausa, int direccioTemps, bool escalaReal, double factorVelocitat)
{
	if (m_objectList.empty() || enPausa) return;

	// Limit de seguretat per a dt
	if (dt < 0.0) dt = 0.0;
	if (dt > 0.1) dt = 0.1;

	dt = dt * direccioTemps;
	// Factor de temps per al moviment orbital
	float factorTemps = 1.0f;
	//if (escalaReal) factorTemps = std::sqrt(factorVelocitat / 300.0);

	for (auto& p : m_objectList)
	{
		if (p.second && p.second->m_parent == nullptr)
			p.second->update(t);
	}

	//for (int i = 0; i < m_objectList.size(); i++)
	//{
	//	// Gir orbital al voltant del Sol (la Lluna, i == 9, al voltant de la Terra)
	//	if (i > 0)
	//	{
	//		double incrementOrbita = m_objectList[mainBodies[i]]->m_velAngular * dt * factorTemps;
	//		m_objectList[mainBodies[i]]->m_angleOrbita += incrementOrbita;
	//		while (m_objectList[mainBodies[i]]->m_angleOrbita >= 360.0f) m_objectList[mainBodies[i]]->m_angleOrbita -= 360.0f;
	//		while (m_objectList[mainBodies[i]]->m_angleOrbita < 0.0f) m_objectList[mainBodies[i]]->m_angleOrbita += 360.0f;
	//	}

	//	m_objectList[mainBodies[i]]->m_angleRotacio += m_objectList[mainBodies[i]]->m_velRotacio * dt;
	//	while (m_objectList[mainBodies[i]]->m_angleRotacio >= 360.0f) m_objectList[mainBodies[i]]->m_angleRotacio -= 360.0f;
	//	while (m_objectList[mainBodies[i]]->m_angleRotacio < 0.0f) m_objectList[mainBodies[i]]->m_angleRotacio += 360.0f;
	//}

	//// Lluna: rotacio sincrona (sempre mostra la mateixa cara a la Terra).
	//// El centre de la textura (cara visible, u = 0.5) queda a la direccio (0,-1,0) de l'esfera de
	//// glut_geometry. Girar-la (angleOrbita - 90) graus la fa apuntar sempre cap a la Terra.
	//m_objectList[CB_MOON]->m_angleRotacio = m_objectList[CB_MOON]->m_angleOrbita - 90.0f;
}


