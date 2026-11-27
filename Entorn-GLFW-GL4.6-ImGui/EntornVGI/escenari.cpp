#include "escenari.h"
#include <algorithm>
#include <cmath>
#include <vector>


// Activa el shader objectiu nomes si es diferent del shader actual per evitar glUseProgram redundants
static void activarShaderSiCal(ContextRenderEscenari& ctx, GLuint shaderObjectiu)
{
	if (shaderObjectiu != 0 && ctx.shaderActualID != shaderObjectiu)
	{
		glUseProgram(shaderObjectiu);
		ctx.shaderActualID = shaderObjectiu;
	}
}

void EsferaInt::drawObject(ContextRenderEscenari& ctx, const glm::mat4& matriuPare)
{
	// 1. DIBUIXAR L'ÒRBITA (nomes si mostrarOrbites esta actiu a la UI i el cos te orbita)
	if (ctx.mostrarOrbites && m_radiOrbita > 0.0)
	{
		activarShaderSiCal(ctx, ctx.shaderPhongID);

		// Pla orbital inclinat respecte al sistema de referencia del pare
		glm::mat4 matriuPlaOrbita = glm::rotate(matriuPare, (float)glm::radians(m_inclinacioOrbita), vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 matriuOrb = glm::scale(matriuPlaOrbita, vec3((float)m_radiOrbita));

		glUniform1i(glGetUniformLocation(ctx.shaderActualID, "textur"), GL_FALSE);
		glUniform1i(glGetUniformLocation(ctx.shaderActualID, "sw_material"), GL_FALSE);
		glUniform4i(glGetUniformLocation(ctx.shaderActualID, "sw_intensity"), GL_TRUE, GL_FALSE, GL_FALSE, GL_FALSE);
		glUniformMatrix4fv(glGetUniformLocation(ctx.shaderActualID, "modelMatrix"), 1, GL_FALSE, &matriuOrb[0][0]);

		CVAO orbitVAO = Get_VAOList(GLUT_TORUS);
		if (orbitVAO.vaoId != 0)
		{
			glBindVertexArray(orbitVAO.vaoId);
			glDrawArrays(GL_LINE_LOOP, 0, orbitVAO.nVertexs);
			glBindVertexArray(0);
		}
	}

	// 2. MATRIU DE MODEL DEL PROPI COS (usa m_trMatrix ja calculada a update() abans de la camera)
	glm::mat4 ModelMatrix = glm::rotate(m_trMatrix, (float)glm::radians(m_inclinacioEix), vec3(1.0f, 0.0f, 0.0f));
	ModelMatrix = glm::rotate(ModelMatrix, (float)glm::radians(m_angleRotacio), vec3(0.0f, 0.0f, 1.0f));
	ModelMatrix = glm::scale(ModelMatrix, vec3((float)m_radiCos));

	// 3. SELECCIÓ DE SHADER (normal mapping si disposa de mapa normal valid)
	const bool usaNormalMap = (m_normalMapID != 0 && ctx.shaderNormalMapID != 0);
	activarShaderSiCal(ctx, usaNormalMap ? ctx.shaderNormalMapID : ctx.shaderPhongID);

	// 4. CONFIGURACIÓ DE MATRIU, TEXTURA DIFUSA (GL_TEXTURE0) I MAPA NORMAL (GL_TEXTURE1)
	glUniformMatrix4fv(glGetUniformLocation(ctx.shaderActualID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);

	if (m_textureID != 0)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_textureID);
		glUniform1i(glGetUniformLocation(ctx.shaderActualID, "textur"), GL_TRUE);
		glUniform1i(glGetUniformLocation(ctx.shaderActualID, "texture0"), 0);
	}
	else
	{
		glUniform1i(glGetUniformLocation(ctx.shaderActualID, "textur"), GL_FALSE);
	}

	if (usaNormalMap)
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_normalMapID);
		glUniform1i(glGetUniformLocation(ctx.shaderActualID, "normalMap"), 1);
	}

	// Combinar m_sw_mat de l'objecte amb les opcions globals del menu Visualitzacio
	bool swMatCombinat[5] = {
		m_sw_mat[0] && ctx.iluminacioSolar,
		ctx.vistaClara,
		ctx.sw_mat[2],
		ctx.sw_mat[3],
		m_sw_mat[4]
	};
	SeleccionaColorMaterial(ctx.shaderActualID, { 1.0, 1.0, 1.0, 1.0 }, swMatCombinat);

	// 5. DIBUIXAR L'ESFERA
	draw_TriEBO_Object(GLU_SPHERE);

	// Restaurar estat de textures per seguretat
	if (usaNormalMap)
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	if (m_textureID != 0)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glUniform1i(glGetUniformLocation(ctx.shaderActualID, "textur"), GL_FALSE);
	}

	// 6. DIBUIXAR ELS FILLS (satèl·lits, anells, etc.) AMB LA POSICIÓ DEL PARE
	for (Object* o : m_sons)
		if (o)
			o->drawObject(ctx, m_trMatrix);
}

void EsferaInt::update(double tempsJ2000, const glm::mat4& matriuPare)
{
	if (m_radiOrbita > 0.0)
	{
		m_angleOrbita = std::fmod(m_angleOrbitaInicial + m_velAngular * tempsJ2000, 360.0);
		if (m_angleOrbita < 0.0) m_angleOrbita += 360.0;
	}

	m_angleRotacio = std::fmod(m_angleRotacioInicial + m_velRotacio * tempsJ2000, 360.0);
	if (m_angleRotacio < 0.0) m_angleRotacio += 360.0;

	// Calcular la posicio orbital global respecte a matriuPare abans de dibuixar (0 frames de lag)
	glm::mat4 matriuPlaOrbita = glm::rotate(matriuPare, (float)glm::radians(m_inclinacioOrbita), vec3(1.0f, 0.0f, 0.0f));
	float xOrb = (float)(m_radiOrbita * cos(glm::radians(m_angleOrbita)));
	float yOrb = (float)(m_radiOrbita * sin(glm::radians(m_angleOrbita)));
	glm::mat4 matriuPosicio = glm::translate(matriuPlaOrbita, vec3(xOrb, yOrb, 0.0f));

	// m_trMatrix passa nomes la posicio als fills sense inclinar els seus eixos
	m_trMatrix = matriuPare;
	m_trMatrix[3] = matriuPosicio[3];

	for (Object* son : m_sons)
		if (son) son->update(tempsJ2000, m_trMatrix);
}

void AnellInt::update(double tempsJ2000, const glm::mat4& matriuPare)
{
	m_trMatrix = matriuPare;
	for (Object* son : m_sons)
		if (son) son->update(tempsJ2000, m_trMatrix);
}

void AnellInt::drawObject(ContextRenderEscenari& ctx, const glm::mat4& matriuPare)
{
	// 1. Dibuixar amb el shader Phong estandard (suporta sw_twoSided per a superficies primes)
	activarShaderSiCal(ctx, ctx.shaderPhongID);

	// 2. Hereta la posició del planeta pare i aplica inclinació axial (els radis ja estan al CVAO)
	m_trMatrix = matriuPare;
	glm::mat4 ModelMatrix = glm::rotate(m_trMatrix, (float)glm::radians(m_inclinacioEix), vec3(1.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(ctx.shaderActualID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);

	// 3. Configuració de textura difusa
	if (m_textureID != 0)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_textureID);
		glUniform1i(glGetUniformLocation(ctx.shaderActualID, "textur"), GL_TRUE);
		glUniform1i(glGetUniformLocation(ctx.shaderActualID, "texture0"), 0);
	}
	else
	{
		glUniform1i(glGetUniformLocation(ctx.shaderActualID, "textur"), GL_FALSE);
	}

	// 4. Combinar material amb opcions de la UI i activar il·luminació bilateral + transparència
	bool swMatAnell[5] = {
		m_sw_mat[0] && ctx.iluminacioSolar,
		ctx.vistaClara,
		ctx.sw_mat[2],
		false,
		true
	};
	SeleccionaColorMaterial(ctx.shaderActualID, { 1.0, 1.0, 1.0, 1.0 }, swMatAnell);

	GLint locTwoSided = glGetUniformLocation(ctx.shaderActualID, "sw_twoSided");
	if (locTwoSided != -1) glUniform1i(locTwoSided, GL_TRUE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_FALSE);

	// 5. Dibuixar el CVAO propi (idèntic a draw_TriEBO_Object)
	if (m_vao.vaoId != 0)
	{
		glBindVertexArray(m_vao.vaoId);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vao.eboId);
		glDrawElements(GL_TRIANGLES, m_vao.nIndices, GL_UNSIGNED_INT, nullptr);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	// 6. Restaurar estats d'OpenGL i uniforms
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
	if (locTwoSided != -1) glUniform1i(locTwoSided, GL_FALSE);

	if (m_textureID != 0)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glUniform1i(glGetUniformLocation(ctx.shaderActualID, "textur"), GL_FALSE);
	}

	// 7. Propagar als fills
	for (Object* o : m_sons)
		if (o)
			o->drawObject(ctx, m_trMatrix);
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

// creaOrbita: genera un cercle de radi 1 al pla XY com a GL_LINE_LOOP.
// Abans les orbites eren tors 3D de gruix 0.03: vistes de lluny feien menys d'un pixel d'amplada
// i el rasteritzador les dibuixava discontinues. Una linia sempre ocupa com a minim 1 pixel continu,
// i amb el multisampling (MSAA, activat a main.cpp) les seves vores queden suavitzades.
CVAO creaOrbita()
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

	CVAO orbitVAO = load_TRIANGLES_VAO(vertices, normals, colors, textures);	// VAO generic: posicio, color, normal, textura
	orbitVAO.eboId = 0;	// load_TRIANGLES_VAO no crea EBO ni inicialitza aquest camp
	return orbitVAO;
}


void Escenari::creaEscenari(GLuint& shader_programID, GLuint& shaderNormalMapID)
{
	if (m_inicialitzat) return;

	if (shaderNormalMapID != 0) {
		glUseProgram(shaderNormalMapID);
		glUniform1i(glGetUniformLocation(shaderNormalMapID, "texture0"), 0);
		glUniform1i(glGetUniformLocation(shaderNormalMapID, "normalMap"), 1);
		glUniform1i(glGetUniformLocation(shaderNormalMapID, "flag_invert_y"), GL_TRUE);
	}

	glUseProgram(shader_programID);
	glUniform1i(glGetUniformLocation(shader_programID, "texture0"), GLint(0));
	glUniform1i(glGetUniformLocation(shader_programID, "flag_invert_y"), GL_TRUE);

	CColor colObj;
	colObj.r = 1.0; colObj.g = 1.0; colObj.b = 1.0; colObj.a = 1.0;
	SetColor4d(colObj.r, colObj.g, colObj.b, colObj.a);
	if (Get_VAOList(GLU_SPHERE).vaoId == 0) {
		Set_VAOList(GLU_SPHERE, loadgluSphere_EBO(1.0, 40, 40)); // Esfera de radi 1 (cossos)
	}
	if (Get_VAOList(GLUT_TORUS).vaoId == 0) {
		Set_VAOList(GLUT_TORUS, creaOrbita()); // Cercle de linies de radi 1 (orbites)
	}

	const double x = 360.0 / 86400; // 360 graus / dies_orbita -> velocitat en graus/dia (1s simulat = 1 dia)
	const double factorRotacio = 1.0 / 86400;
	// Posicions i velocitats astronòmiques exactes a l'època J2000.0 (t = 0)
	m_objectList[CB_SUN] = new EsferaInt("Sol", 16.0, 0.0, 0.00, 0.0, 0.0, 14.184 * factorRotacio, 0.00, 0.00); m_objectList[CB_SUN]->m_sw_mat[0] = true;
	m_objectList[CB_MERCURY] = new EsferaInt("Mercuri", 2.0, 27.0, 252.25, x / 87.97, 0.0, 6.136 * factorRotacio, 0.03, 7.00);
	m_objectList[CB_VENUS] = new EsferaInt("Venus", 3.5, 40.0, 181.98, x / 224.70, 0.0, 1.481 * factorRotacio, 177.36, 3.39);
	m_objectList[CB_EARTH] = new EsferaInt("Terra", 3.7, 55.0, 100.46, x / 365.256, 100.46, 361.015 * factorRotacio, -23.44, 0.00);
	m_objectList[CB_MARS] = new EsferaInt("Mart", 2.8, 70.0, 355.45, x / 686.98, 0.0, 351.220 * factorRotacio, 25.19, 1.85);
	m_objectList[CB_JUPITER] = new EsferaInt("J\xFApiter", 9.0, 105.0, 34.40, x / 4332.59, 0.0, 871.249 * factorRotacio, 3.13, 1.30);
	m_objectList[CB_SATURN] = new EsferaInt("Saturn", 7.5, 140.0, 49.94, x / 10759.2, 0.0, 818.182 * factorRotacio, 26.73, 2.49);
	m_objectList[CB_URANUS] = new EsferaInt("Ur\xE0", 5.5, 175.0, 313.23, x / 30688.5, 0.0, 501.393 * factorRotacio, 97.77, 0.77);
	m_objectList[CB_NEPTUNE] = new EsferaInt("Nept\xFA", 5.3, 210.0, 304.88, x / 60195.0, 0.0, 545.455 * factorRotacio, 28.32, 1.77);
	m_objectList[CB_MOON] = new EsferaInt("Lluna", 1.0, 7.0, 218.32, x / 27.322, 128.32, x / 27.322, 6.68, 5.14);
	Object* anellSat = new AnellInt("Anells de Saturn", 9.0, 17.5, 26.73);

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
	m_objectList[CB_SATURN]->addSon(anellSat);

	// Assignacio d'identificadors, categories i ordre determinista als menus
	auto configurarMetadades = [&](CelestialBodyId id, CategoriaObjecte cat, int ordre) {
		Object* obj = obtenirObjecte(id);
		if (obj != nullptr) {
			obj->m_id = id;
			obj->m_categoria = cat;
			obj->m_ordreMenu = ordre;
		}
	};
	configurarMetadades(CB_SUN,     CategoriaObjecte::ESTRELLA,         0);
	configurarMetadades(CB_MERCURY, CategoriaObjecte::PLANETA,          10);
	configurarMetadades(CB_VENUS,   CategoriaObjecte::PLANETA,          20);
	configurarMetadades(CB_EARTH,   CategoriaObjecte::PLANETA,          30);
	configurarMetadades(CB_MOON,    CategoriaObjecte::SATELLIT_NATURAL, 31);
	configurarMetadades(CB_MARS,    CategoriaObjecte::PLANETA,          40);
	configurarMetadades(CB_JUPITER, CategoriaObjecte::PLANETA,          50);
	configurarMetadades(CB_SATURN,  CategoriaObjecte::PLANETA,          60);
	configurarMetadades(CB_URANUS,  CategoriaObjecte::PLANETA,          70);
	configurarMetadades(CB_NEPTUNE, CategoriaObjecte::PLANETA,          80);

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
	anellSat->loadTexture(".\\textures\\planet\\Saturn\\2k_saturn_ring_alpha.png");

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
	anellSat->m_normalMapID = createDefaultNormalMap();

	glBindTexture(GL_TEXTURE_2D, 0);
	m_inicialitzat = true;
}

// dibuixaEscenari: renderitza orbites (si mostrarOrbites es true des del menu Visualitzacio),
// anells de Saturn i cossos celestes amb Phong i normal mapping.
void Escenari::dibuixaEscenari(GLuint sh_programID, GLuint shaderNormalMapID, glm::mat4 MatriuVista, glm::mat4 MatriuTG, bool sw_mat[5], bool mostrarOrbites)
{
	(void)MatriuVista;
	CColor col_blanc;
	col_blanc.r = 1.0;	col_blanc.g = 1.0;	col_blanc.b = 1.0;	col_blanc.a = 1.0;

	// Preparamos el shader estandard (Phong)
	glUseProgram(sh_programID);
	glUniform1i(glGetUniformLocation(sh_programID, "textur"), GL_TRUE);
	glUniform1i(glGetUniformLocation(sh_programID, "modulate"), GL_TRUE);
	glUniform1i(glGetUniformLocation(sh_programID, "flag_invert_y"), GL_TRUE);
	SeleccionaColorMaterial(sh_programID, col_blanc, sw_mat);

	// Preparamos el shader de normal mapping una sola vez antes del bucle
	GLuint shaderInicialActiu = sh_programID;
	if (shaderNormalMapID != 0)
	{
		glUseProgram(shaderNormalMapID);
		copiaUniformsShader(sh_programID, shaderNormalMapID);
		glUniform1i(glGetUniformLocation(shaderNormalMapID, "textur"), GL_TRUE);
		glUniform1i(glGetUniformLocation(shaderNormalMapID, "modulate"), GL_TRUE);
		glUniform1i(glGetUniformLocation(shaderNormalMapID, "flag_invert_y"), GL_TRUE);
		SeleccionaColorMaterial(shaderNormalMapID, col_blanc, sw_mat);
		shaderInicialActiu = shaderNormalMapID;
	}

	ContextRenderEscenari ctx;
	ctx.shaderPhongID = sh_programID;
	ctx.shaderNormalMapID = shaderNormalMapID;
	ctx.shaderActualID = shaderInicialActiu;
	ctx.mostrarOrbites = mostrarOrbites;
	ctx.iluminacioSolar = sw_mat[0];
	ctx.vistaClara = sw_mat[1];
	for (int k = 0; k < 5; ++k) ctx.sw_mat[k] = sw_mat[k];

	// Recorregut recursiu des dels nodes arrel del graf d'escena
	for (auto& p : m_objectList)
	{
		Object* obj = p.second;
		if (!obj) continue;
		if (obj->m_parent == nullptr)
			obj->drawObject(ctx, MatriuTG);
	}

	// Restaurar estat de textures i tornar al shader Phong principal
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	activarShaderSiCal(ctx, sh_programID);
	glUniform4i(glGetUniformLocation(sh_programID, "sw_intensity"), sw_mat[0], sw_mat[1], sw_mat[2], sw_mat[3]);
	glUniform1i(glGetUniformLocation(sh_programID, "sw_material"), sw_mat[4]);
	glUniform1i(glGetUniformLocation(sh_programID, "textur"), GL_FALSE);
}

// updateVariables: avanca les orbites i rotacions dels cossos segons els controls de la UI:
// - enPausa: atura completament el moviment quan l'usuari prem Pausa o Espai (tret de forcarActualitzacio).
// - direccioTemps: +1 (endavant) o -1 (rebobinar) des del panell temporal / Backspace.
// - escalaReal + factorVelocitat: ajusta el pas orbital segons el timelapse actiu en mode ESCALA_REAL.
void Escenari::updateVariables(double t, double dt, bool enPausa, int direccioTemps, bool escalaReal, double factorVelocitat, bool forcarActualitzacio)
{
	(void)dt;
	(void)direccioTemps;
	(void)escalaReal;
	(void)factorVelocitat;

	if (m_objectList.empty() || (enPausa && !forcarActualitzacio)) return;

	for (auto& p : m_objectList)
	{
		if (p.second && p.second->m_parent == nullptr)
			p.second->update(t, glm::mat4(1.0f));
	}
}

std::vector<const Object*> Escenari::obtenirObjectesEnfocables() const
{
	std::vector<const Object*> resultat;
	resultat.reserve(m_objectList.size());
	for (const auto& p : m_objectList)
	{
		const Object* obj = p.second;
		if (obj != nullptr && obj->m_enfocable)
			resultat.push_back(obj);
	}
	std::sort(resultat.begin(), resultat.end(), [](const Object* a, const Object* b) {
		if (a->m_ordreMenu != b->m_ordreMenu)
			return a->m_ordreMenu < b->m_ordreMenu;
		return a->m_nom < b->m_nom;
	});
	return resultat;
}


