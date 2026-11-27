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

// Aplica el material blanc (1,1,1,1) i flags d'intensitat usant el cache d'uniforms (zero glGetUniformLocation)
static void aplicarMaterialCache(const CacheUniformsShader& cache, const bool sw_mater[5])
{
	if (cache.locSwIntensity != -1)
		glUniform4i(cache.locSwIntensity, sw_mater[0], sw_mater[1], sw_mater[2], sw_mater[3]);
	if (cache.locSwMaterial != -1)
		glUniform1i(cache.locSwMaterial, sw_mater[4]);

	const float em = sw_mater[0] ? 1.0f : 0.0f;
	const float am = sw_mater[1] ? 1.0f : 0.0f;
	const float di = sw_mater[2] ? 1.0f : 0.0f;
	const float sp = sw_mater[3] ? 1.0f : 0.0f;
	const float sh = sw_mater[3] ? 32.0f : 0.0f;

	if (cache.locMatEmission != -1)  glUniform4f(cache.locMatEmission, em, em, em, 1.0f);
	if (cache.locMatAmbient != -1)   glUniform4f(cache.locMatAmbient, am, am, am, 1.0f);
	if (cache.locMatDiffuse != -1)   glUniform4f(cache.locMatDiffuse, di, di, di, 1.0f);
	if (cache.locMatSpecular != -1)  glUniform4f(cache.locMatSpecular, sp, sp, sp, 1.0f);
	if (cache.locMatShininess != -1) glUniform1f(cache.locMatShininess, sh);
}

// Sincronitza matrius i llums al shader de normal mapping directament des de CPU (zero glGetUniform*)
static void SincronitzarShaderNormalMap(const CacheUniformsShader& cache, const glm::mat4& matriuVista, const ParametresIluminacioEscenari& ilum)
{
	if (cache.locProjectionMatrix != -1)
		glUniformMatrix4fv(cache.locProjectionMatrix, 1, GL_FALSE, &ilum.matriuProjeccio[0][0]);
	if (cache.locViewMatrix != -1)
		glUniformMatrix4fv(cache.locViewMatrix, 1, GL_FALSE, &matriuVista[0][0]);
	if (cache.locNormalMatrix != -1) {
		glm::mat4 nm = glm::transpose(glm::inverse(matriuVista));
		glUniformMatrix4fv(cache.locNormalMatrix, 1, GL_FALSE, &nm[0][0]);
	}
	if (cache.locLightModelAmbient != -1) {
		if (ilum.llumAmbient) glUniform4f(cache.locLightModelAmbient, 0.5f, 0.5f, 0.5f, 1.0f);
		else glUniform4f(cache.locLightModelAmbient, 0.0f, 0.0f, 0.0f, 1.0f);
	}
	if (cache.locFixedLight != -1)
		glUniform1i(cache.locFixedLight, ilum.llumFixa ? GL_TRUE : GL_FALSE);

	if (ilum.llums != nullptr) {
		const int count = (ilum.numLlums < 8) ? ilum.numLlums : 8;
		for (int i = 0; i < count; ++i) {
			const LLUM& l = ilum.llums[i];
			const CacheUniformsShader::LocLlum& loc = cache.locLlums[i];
			if (loc.swLight != -1)       glUniform1i(loc.swLight, l.encesa ? GL_TRUE : GL_FALSE);
			if (loc.position != -1)      glUniform4f(loc.position, (GLfloat)l.posicio.x, (GLfloat)l.posicio.y, (GLfloat)l.posicio.z, (GLfloat)l.posicio.w);
			if (loc.diffuse != -1)       glUniform4f(loc.diffuse, (GLfloat)l.difusa.r, (GLfloat)l.difusa.g, (GLfloat)l.difusa.b, (GLfloat)l.difusa.a);
			if (loc.specular != -1)      glUniform4f(loc.specular, (GLfloat)l.especular.r, (GLfloat)l.especular.g, (GLfloat)l.especular.b, (GLfloat)l.especular.a);
			if (loc.attenuation != -1)   glUniform3f(loc.attenuation, (GLfloat)l.atenuacio.a, (GLfloat)l.atenuacio.b, (GLfloat)l.atenuacio.c);
			if (loc.restricted != -1)    glUniform1i(loc.restricted, l.restringida ? GL_TRUE : GL_FALSE);
			if (loc.spotDirection != -1) glUniform3f(loc.spotDirection, (GLfloat)l.spotdirection.x, (GLfloat)l.spotdirection.y, (GLfloat)l.spotdirection.z);
			if (loc.spotCosCutoff != -1) glUniform1f(loc.spotCosCutoff, l.spotcoscutoff);
			if (loc.spotExponent != -1)  glUniform1f(loc.spotExponent, l.spotexponent);
		}
	}
}

void EsferaInt::drawObject(ContextRenderEscenari& ctx, const glm::mat4& matriuPare)
{
	// 1. DIBUIXAR L'ÒRBITA (nomes si mostrarOrbites esta actiu a la UI i el cos te orbita)
	if (ctx.mostrarOrbites && m_radiOrbita > 0.0)
	{
		activarShaderSiCal(ctx, ctx.shaderPhongID);
		const CacheUniformsShader* cacheOrb = ctx.cacheActual();

		// Pla orbital inclinat respecte al sistema de referencia del pare
		glm::mat4 matriuPlaOrbita = glm::rotate(matriuPare, (float)glm::radians(m_inclinacioOrbita), vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 matriuOrb = glm::scale(matriuPlaOrbita, vec3((float)m_radiOrbita));

		if (cacheOrb) {
			if (cacheOrb->locTextur != -1)            glUniform1i(cacheOrb->locTextur, GL_FALSE);
			if (cacheOrb->locSwMaterial != -1)        glUniform1i(cacheOrb->locSwMaterial, GL_FALSE);
			if (cacheOrb->locSwIntensity != -1)       glUniform4i(cacheOrb->locSwIntensity, GL_TRUE, GL_FALSE, GL_FALSE, GL_FALSE);
			if (cacheOrb->locUseNormalMatrixPV != -1) glUniform1i(cacheOrb->locUseNormalMatrixPV, GL_FALSE);
			if (cacheOrb->locModelMatrix != -1)       glUniformMatrix4fv(cacheOrb->locModelMatrix, 1, GL_FALSE, &matriuOrb[0][0]);
		}

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
	const CacheUniformsShader* cache = ctx.cacheActual();

	// 4. CONFIGURACIÓ DE MATRIUS (incloent normalMatrixPV precalculada a CPU), TEXTURA DIFUSA I MAPA NORMAL
	if (cache) {
		if (cache->locModelMatrix != -1)
			glUniformMatrix4fv(cache->locModelMatrix, 1, GL_FALSE, &ModelMatrix[0][0]);

		if (cache->locUseNormalMatrixPV != -1 && cache->locUNormalMatrixPV != -1) {
			glm::mat3 normalMatPV = glm::mat3(glm::transpose(glm::inverse(ctx.matriuVista * ModelMatrix)));
			glUniform1i(cache->locUseNormalMatrixPV, GL_TRUE);
			glUniformMatrix3fv(cache->locUNormalMatrixPV, 1, GL_FALSE, &normalMatPV[0][0]);
		}

		if (m_textureID != 0)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_textureID);
			if (cache->locTextur != -1)  glUniform1i(cache->locTextur, GL_TRUE);
			if (cache->locTexture0 != -1) glUniform1i(cache->locTexture0, 0);
		}
		else
		{
			if (cache->locTextur != -1) glUniform1i(cache->locTextur, GL_FALSE);
		}

		if (usaNormalMap)
		{
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, m_normalMapID);
			if (cache->locNormalMap != -1) glUniform1i(cache->locNormalMap, 1);
		}

		// Combinar m_sw_mat de l'objecte amb les opcions globals del menu Visualitzacio
		bool swMatCombinat[5] = {
			m_sw_mat[0] && ctx.iluminacioSolar,
			ctx.vistaClara,
			ctx.sw_mat[2],
			ctx.sw_mat[3],
			m_sw_mat[4]
		};
		aplicarMaterialCache(*cache, swMatCombinat);
	}

	// 5. DIBUIXAR L'ESFERA
	draw_TriEBO_Object(GLU_SPHERE);

	// Restaurar estat de textures i normal matrix per seguretat
	if (cache && cache->locUseNormalMatrixPV != -1)
		glUniform1i(cache->locUseNormalMatrixPV, GL_FALSE);

	if (usaNormalMap)
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	if (m_textureID != 0)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		if (cache && cache->locTextur != -1) glUniform1i(cache->locTextur, GL_FALSE);
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
	const CacheUniformsShader* cache = ctx.cacheActual();

	// 2. Hereta la posició del planeta pare i aplica inclinació axial (els radis ja estan al CVAO)
	m_trMatrix = matriuPare;
	glm::mat4 ModelMatrix = glm::rotate(m_trMatrix, (float)glm::radians(m_inclinacioEix), vec3(1.0f, 0.0f, 0.0f));
	if (cache) {
		if (cache->locModelMatrix != -1)
			glUniformMatrix4fv(cache->locModelMatrix, 1, GL_FALSE, &ModelMatrix[0][0]);

		if (cache->locUseNormalMatrixPV != -1 && cache->locUNormalMatrixPV != -1) {
			glm::mat3 normalMatPV = glm::mat3(glm::transpose(glm::inverse(ctx.matriuVista * ModelMatrix)));
			glUniform1i(cache->locUseNormalMatrixPV, GL_TRUE);
			glUniformMatrix3fv(cache->locUNormalMatrixPV, 1, GL_FALSE, &normalMatPV[0][0]);
		}

		// 3. Configuració de textura difusa
		if (m_textureID != 0)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_textureID);
			if (cache->locTextur != -1)  glUniform1i(cache->locTextur, GL_TRUE);
			if (cache->locTexture0 != -1) glUniform1i(cache->locTexture0, 0);
		}
		else
		{
			if (cache->locTextur != -1) glUniform1i(cache->locTextur, GL_FALSE);
		}

		// 4. Combinar material amb opcions de la UI i activar il·luminació bilateral + transparència
		bool swMatAnell[5] = {
			m_sw_mat[0] && ctx.iluminacioSolar,
			ctx.vistaClara,
			ctx.sw_mat[2],
			false,
			true
		};
		aplicarMaterialCache(*cache, swMatAnell);

		if (cache->locSwTwoSided != -1) glUniform1i(cache->locSwTwoSided, GL_TRUE);
	}

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
	if (cache) {
		if (cache->locSwTwoSided != -1) glUniform1i(cache->locSwTwoSided, GL_FALSE);
		if (cache->locUseNormalMatrixPV != -1) glUniform1i(cache->locUseNormalMatrixPV, GL_FALSE);
	}

	if (m_textureID != 0)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		if (cache && cache->locTextur != -1) glUniform1i(cache->locTextur, GL_FALSE);
	}

	// 7. Propagar als fills
	for (Object* o : m_sons)
		if (o)
			o->drawObject(ctx, m_trMatrix);
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

	// VAOs compartits (creats un sol cop entre escenaris)
	GestorRecursos::AssegurarVAOsEscenari();

	const double x = 360.0 / 86400; // 360 graus / dies_orbita -> velocitat en graus/dia (1s simulat = 1 dia)
	const double factorRotacio = 1.0 / 86400;
	// Posicions i velocitats astronòmiques exactes a l'època J2000.0 (t = 0)
	m_objectList[CB_SUN] = new EsferaInt("Sol", 16.0, 0.0, 0.00, 0.0, 0.0, 14.184 * factorRotacio, 0.00, 0.00); m_objectList[CB_SUN]->m_sw_mat[0] = true;
	m_objectList[CB_MERCURY] = new EsferaInt("Mercuri", 2.0, 27.0, 252.25, x / 87.97, 0.0, 6.136 * factorRotacio, 0.03, 7.00);
	m_objectList[CB_VENUS] = new EsferaInt("Venus", 3.5, 40.0, 181.98, x / 224.70, 0.0, 1.481 * factorRotacio, 177.36, 3.39);
	m_objectList[CB_EARTH] = new EsferaInt("Terra", 3.7, 55.0, 100.46, x / 365.256, 100.46, 361.015 * factorRotacio, -23.44, 0.00);
	m_objectList[CB_MARS] = new EsferaInt("Mart", 2.8, 70.0, 355.45, x / 686.98, 0.0, 351.220 * factorRotacio, 25.19, 1.85);
	m_objectList[CB_JUPITER] = new EsferaInt("Júpiter", 9.0, 105.0, 34.40, x / 4332.59, 0.0, 871.249 * factorRotacio, 3.13, 1.30);
	m_objectList[CB_SATURN] = new EsferaInt("Saturn", 7.5, 140.0, 49.94, x / 10759.2, 0.0, 818.182 * factorRotacio, 26.73, 2.49);
	m_objectList[CB_URANUS] = new EsferaInt("Urà", 5.5, 175.0, 313.23, x / 30688.5, 0.0, 501.393 * factorRotacio, 97.77, 0.77);
	m_objectList[CB_NEPTUNE] = new EsferaInt("Neptú", 5.3, 210.0, 304.88, x / 60195.0, 0.0, 545.455 * factorRotacio, 28.32, 1.77);
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

	// Càrrega de textures difuses compartides via GestorRecursos (zero duplicacio en VRAM entre escenaris)
	m_objectList[CB_SUN]->m_textureID     = GestorRecursos::ObtenirTextura2D("textures/sun/2k_sun.jpg");
	m_objectList[CB_MERCURY]->m_textureID = GestorRecursos::ObtenirTextura2D("textures/planet/Mercury/2k_mercury.jpg");
	m_objectList[CB_VENUS]->m_textureID   = GestorRecursos::ObtenirTextura2D("textures/planet/Venus/2k_venus_surface.jpg");
	m_objectList[CB_EARTH]->m_textureID   = GestorRecursos::ObtenirTextura2D("textures/planet/Earth/earth2048.bmp");
	m_objectList[CB_MARS]->m_textureID    = GestorRecursos::ObtenirTextura2D("textures/planet/Mars/2k_mars.jpg");
	m_objectList[CB_JUPITER]->m_textureID = GestorRecursos::ObtenirTextura2D("textures/planet/Jupiter/2k_jupiter.jpg");
	m_objectList[CB_SATURN]->m_textureID  = GestorRecursos::ObtenirTextura2D("textures/planet/Saturn/2k_saturn.jpg");
	m_objectList[CB_URANUS]->m_textureID  = GestorRecursos::ObtenirTextura2D("textures/planet/Uranus/2k_uranus.jpg");
	m_objectList[CB_NEPTUNE]->m_textureID = GestorRecursos::ObtenirTextura2D("textures/planet/Neptune/2k_neptune.jpg");
	m_objectList[CB_MOON]->m_textureID    = GestorRecursos::ObtenirTextura2D("textures/planet/Moon/2k_moon.jpg");
	anellSat->m_textureID                 = GestorRecursos::ObtenirTextura2D("textures/planet/Saturn/2k_saturn_ring_alpha.png");

	// Càrrega de mapes normals compartits (i textura plana 1x1 unica per defecte)
	GLuint normalDefecte = GestorRecursos::ObtenirNormalMapPerDefecte();
	m_objectList[CB_SUN]->m_normalMapID     = normalDefecte;
	m_objectList[CB_MERCURY]->m_normalMapID = GestorRecursos::ObtenirTextura2D("textures/planet/NormalTextures/NormalMapMercury.png");
	m_objectList[CB_VENUS]->m_normalMapID   = GestorRecursos::ObtenirTextura2D("textures/planet/NormalTextures/NormalMapVenus.png");
	m_objectList[CB_EARTH]->m_normalMapID   = GestorRecursos::ObtenirTextura2D("textures/planet/NormalTextures/NormalMapEarth.png");
	m_objectList[CB_MARS]->m_normalMapID    = GestorRecursos::ObtenirTextura2D("textures/planet/NormalTextures/NormalMapMars.png");
	m_objectList[CB_JUPITER]->m_normalMapID = GestorRecursos::ObtenirTextura2D("textures/planet/NormalTextures/NormalMapJupiter.png");
	m_objectList[CB_SATURN]->m_normalMapID  = GestorRecursos::ObtenirTextura2D("textures/planet/NormalTextures/NormalMapSaturn.png");
	m_objectList[CB_URANUS]->m_normalMapID  = normalDefecte;
	m_objectList[CB_NEPTUNE]->m_normalMapID = GestorRecursos::ObtenirTextura2D("textures/planet/NormalTextures/NormalMapNeptune.png");
	m_objectList[CB_MOON]->m_normalMapID    = normalDefecte;
	anellSat->m_normalMapID                 = normalDefecte;

	glBindTexture(GL_TEXTURE_2D, 0);
	m_inicialitzat = true;
}

// dibuixaEscenari: renderitza orbites (si mostrarOrbites es true des del menu Visualitzacio),
// anells de Saturn i cossos celestes amb Phong i normal mapping sense consultes glGetUniform* per frame.
void Escenari::dibuixaEscenari(GLuint sh_programID, GLuint shaderNormalMapID, glm::mat4 MatriuVista, glm::mat4 MatriuTG, bool sw_mat[5], bool mostrarOrbites, const ParametresIluminacioEscenari* ilum)
{
	static CacheUniformsShader s_cachePhong;
	static CacheUniformsShader s_cacheNormalMap;
	s_cachePhong.ActualitzarSiCal(sh_programID);
	if (shaderNormalMapID != 0) {
		s_cacheNormalMap.ActualitzarSiCal(shaderNormalMapID);
	}

	// Preparamos el shader estandard (Phong) amb cache d'uniforms
	glUseProgram(sh_programID);
	if (s_cachePhong.locTextur != -1)     glUniform1i(s_cachePhong.locTextur, GL_TRUE);
	if (s_cachePhong.locModulate != -1)   glUniform1i(s_cachePhong.locModulate, GL_TRUE);
	if (s_cachePhong.locFlagInvertY != -1) glUniform1i(s_cachePhong.locFlagInvertY, GL_TRUE);
	aplicarMaterialCache(s_cachePhong, sw_mat);

	// Preparamos el shader de normal mapping una sola vegada des de CPU (zero glGetUniform*)
	GLuint shaderInicialActiu = sh_programID;
	if (shaderNormalMapID != 0)
	{
		glUseProgram(shaderNormalMapID);
		if (ilum != nullptr) {
			SincronitzarShaderNormalMap(s_cacheNormalMap, MatriuVista, *ilum);
		}
		if (s_cacheNormalMap.locTextur != -1)     glUniform1i(s_cacheNormalMap.locTextur, GL_TRUE);
		if (s_cacheNormalMap.locModulate != -1)   glUniform1i(s_cacheNormalMap.locModulate, GL_TRUE);
		if (s_cacheNormalMap.locFlagInvertY != -1) glUniform1i(s_cacheNormalMap.locFlagInvertY, GL_TRUE);
		aplicarMaterialCache(s_cacheNormalMap, sw_mat);
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
	ctx.matriuVista = MatriuVista;
	ctx.cachePhong = &s_cachePhong;
	ctx.cacheNormalMap = &s_cacheNormalMap;

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
	if (s_cachePhong.locSwIntensity != -1)
		glUniform4i(s_cachePhong.locSwIntensity, sw_mat[0], sw_mat[1], sw_mat[2], sw_mat[3]);
	if (s_cachePhong.locSwMaterial != -1)
		glUniform1i(s_cachePhong.locSwMaterial, sw_mat[4]);
	if (s_cachePhong.locTextur != -1)
		glUniform1i(s_cachePhong.locTextur, GL_FALSE);
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


