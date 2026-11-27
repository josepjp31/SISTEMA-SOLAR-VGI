// GestorRecursos.cpp : Implementacio del cache compartit i precarrega asincrona de recursos

#include "GestorRecursos.h"
#include "visualitzacio.h"
#include "glut_geometry.h"
#include <algorithm>
#include <atomic>
#include <cstdio>
#include <deque>
#include <mutex>
#include <thread>

// Declaracio de creaOrbita definida a escenari.cpp per construir el VAO compartit d'orbites
CVAO creaOrbita();

namespace
{
	// Llista completa de textures planetaries i mapes normals usats als dos escenaris
	const char* kTexturesPrecarrega2D[] = {
		"textures/sun/2k_sun.jpg",
		"textures/planet/Mercury/2k_mercury.jpg",
		"textures/planet/Venus/2k_venus_surface.jpg",
		"textures/planet/Earth/earth2048.bmp",
		"textures/planet/Mars/2k_mars.jpg",
		"textures/planet/Jupiter/2k_jupiter.jpg",
		"textures/planet/Saturn/2k_saturn.jpg",
		"textures/planet/Uranus/2k_uranus.jpg",
		"textures/planet/Neptune/2k_neptune.jpg",
		"textures/planet/Moon/2k_moon.jpg",
		"textures/planet/Saturn/2k_saturn_ring_alpha.png",
		"textures/planet/NormalTextures/NormalMapMercury.png",
		"textures/planet/NormalTextures/NormalMapVenus.png",
		"textures/planet/NormalTextures/NormalMapEarth.png",
		"textures/planet/NormalTextures/NormalMapMars.png",
		"textures/planet/NormalTextures/NormalMapJupiter.png",
		"textures/planet/NormalTextures/NormalMapSaturn.png",
		"textures/planet/NormalTextures/NormalMapNeptune.png"
	};

	// Ordre estandard de cares OpenGL (+X, -X, +Y, -Y, +Z, -Z) per al Skybox
	const char* kRutesSkybox[6] = {
		"textures/skybox/right.png",
		"textures/skybox/left.png",
		"textures/skybox/top.png",
		"textures/skybox/bottom.png",
		"textures/skybox/front.png",
		"textures/skybox/back.png"
	};

	std::unordered_map<std::string, GLuint> s_cacheTextures2D;
	GLuint s_defaultNormalMapID = 0;
	GLuint s_cubemapID = 0;

	std::mutex s_mutex;
	std::deque<ImatgeCPU> s_cuaTexturesCPU;
	ImatgeCPU s_skyboxCaresCPU[6];
	bool s_skyboxCPUReady = false;

	std::thread s_precarregaThread;
	std::atomic<bool> s_precarregaIniciada(false);
	std::atomic<bool> s_precarregaAcabada(false);
	std::atomic<bool> s_cancelarPrecarrega(false);

	// Unifica separadors (\ -> /) i suprimeix "./" inicial per evitar claus duplicades al cache
	std::string NormalitzarRuta(const char* ruta)
	{
		if (ruta == nullptr) return "";
		std::string s(ruta);
		std::replace(s.begin(), s.end(), '\\', '/');
		while (s.size() >= 2 && s[0] == '.' && s[1] == '/') {
			s.erase(0, 2);
		}
		return s;
	}

	// Puja una imatge descodificada a GPU seleccionant format optimal (RGB vs RGBA vs RED)
	GLuint PujarImatge2DAGPU(const ImatgeCPU& img)
	{
		if (img.pixels == nullptr || img.width <= 0 || img.height <= 0) return 0;

		GLenum format = GL_RGBA;
		GLint internalFormat = GL_RGBA8;
		if (img.channels == 3) {
			format = GL_RGB;
			internalFormat = GL_RGB8;
		} else if (img.channels == 1) {
			format = GL_RED;
			internalFormat = GL_R8;
		}

		GLint prevUnpack = 4;
		glGetIntegerv(GL_UNPACK_ALIGNMENT, &prevUnpack);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		GLuint texID = 0;
		glGenTextures(1, &texID);
		glBindTexture(GL_TEXTURE_2D, texID);
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, img.width, img.height, 0, format, GL_UNSIGNED_BYTE, img.pixels);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		if (GLEW_VERSION_4_6 || GLEW_EXT_texture_filter_anisotropic) {
			GLfloat maxAniso = 1.0f;
			glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
			GLfloat aniso = (maxAniso > 8.0f) ? 8.0f : maxAniso;
			if (aniso > 1.0f) {
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);
			}
		}

		glBindTexture(GL_TEXTURE_2D, 0);
		glPixelStorei(GL_UNPACK_ALIGNMENT, prevUnpack);
		return texID;
	}

	// Puja les 6 cares del Skybox descodificades a CPU cap a un cubemap OpenGL
	GLuint PujarSkyboxAGPU(ImatgeCPU cares[6])
	{
		for (int i = 0; i < 6; ++i) {
			if (cares[i].pixels == nullptr || cares[i].width <= 0 || cares[i].height <= 0) {
				return 0;
			}
		}

		GLint prevUnpack = 4;
		glGetIntegerv(GL_UNPACK_ALIGNMENT, &prevUnpack);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		GLuint texID = 0;
		glGenTextures(1, &texID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, texID);

		for (int i = 0; i < 6; ++i) {
			GLenum format = (cares[i].channels == 4) ? GL_RGBA : GL_RGB;
			GLint internalFmt = (cares[i].channels == 4) ? GL_RGBA8 : GL_RGB8;
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFmt,
				cares[i].width, cares[i].height, 0, format, GL_UNSIGNED_BYTE, cares[i].pixels);
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		glPixelStorei(GL_UNPACK_ALIGNMENT, prevUnpack);
		return texID;
	}

	void AlliberarImatgeCPU(ImatgeCPU& img)
	{
		if (img.pixels != nullptr) {
			SOIL_free_image_data(img.pixels);
			img.pixels = nullptr;
		}
		img.width = 0;
		img.height = 0;
		img.channels = 0;
		img.rutaNormalitzada.clear();
	}

	void FonsPrecarregaEscena()
	{
		// 1. Primer descodificar el Skybox per mostrar el fons d'estrelles a l'instant en entrar
		ImatgeCPU caresLocal[6];
		bool skyboxOk = true;
		for (int i = 0; i < 6; ++i) {
			if (s_cancelarPrecarrega.load()) {
				skyboxOk = false;
				break;
			}
			int w = 0, h = 0, ch = 0;
			unsigned char* px = SOIL_load_image(kRutesSkybox[i], &w, &h, &ch, SOIL_LOAD_RGB);
			if (px == nullptr) {
				fprintf(stderr, "No s'ha pogut precarregar cara skybox: %s\n", kRutesSkybox[i]);
				skyboxOk = false;
				break;
			}
			caresLocal[i].pixels = px;
			caresLocal[i].width = w;
			caresLocal[i].height = h;
			caresLocal[i].channels = 3;
			caresLocal[i].rutaNormalitzada = kRutesSkybox[i];
		}

		if (skyboxOk && !s_cancelarPrecarrega.load()) {
			std::lock_guard<std::mutex> lock(s_mutex);
			for (int i = 0; i < 6; ++i) {
				s_skyboxCaresCPU[i] = caresLocal[i];
			}
			s_skyboxCPUReady = true;
		} else {
			for (int i = 0; i < 6; ++i) {
				AlliberarImatgeCPU(caresLocal[i]);
			}
		}

		// 2. Descodificar totes les textures difuses i mapes normals planetaris
		const size_t numTex = sizeof(kTexturesPrecarrega2D) / sizeof(kTexturesPrecarrega2D[0]);
		for (size_t i = 0; i < numTex; ++i) {
			if (s_cancelarPrecarrega.load()) break;

			std::string rutaNorm = NormalitzarRuta(kTexturesPrecarrega2D[i]);
			int w = 0, h = 0, ch = 0;
			unsigned char* px = SOIL_load_image(rutaNorm.c_str(), &w, &h, &ch, SOIL_LOAD_AUTO);
			if (px == nullptr) {
				fprintf(stderr, "No s'ha pogut precarregar textura: %s\n", rutaNorm.c_str());
				continue;
			}

			ImatgeCPU img;
			img.pixels = px;
			img.width = w;
			img.height = h;
			img.channels = ch;
			img.rutaNormalitzada = rutaNorm;

			std::lock_guard<std::mutex> lock(s_mutex);
			s_cuaTexturesCPU.push_back(img);
		}

		s_precarregaAcabada.store(true);
	}
}

void CacheUniformsShader::ActualitzarSiCal(GLuint id)
{
	if (id == 0) return;
	if (programID == id) return;

	programID = id;
	locProjectionMatrix  = glGetUniformLocation(id, "projectionMatrix");
	locViewMatrix        = glGetUniformLocation(id, "viewMatrix");
	locModelMatrix       = glGetUniformLocation(id, "modelMatrix");
	locNormalMatrix      = glGetUniformLocation(id, "normalMatrix");
	locUseNormalMatrixPV = glGetUniformLocation(id, "useNormalMatrixPV");
	locUNormalMatrixPV   = glGetUniformLocation(id, "uNormalMatrixPV");
	locTexture0          = glGetUniformLocation(id, "texture0");
	locNormalMap         = glGetUniformLocation(id, "normalMap");
	locTextur            = glGetUniformLocation(id, "textur");
	locModulate          = glGetUniformLocation(id, "modulate");
	locFlagInvertY       = glGetUniformLocation(id, "flag_invert_y");
	locSwMaterial        = glGetUniformLocation(id, "sw_material");
	locSwIntensity       = glGetUniformLocation(id, "sw_intensity");
	locSwTwoSided        = glGetUniformLocation(id, "sw_twoSided");
	locLightModelAmbient = glGetUniformLocation(id, "LightModelAmbient");
	locFixedLight        = glGetUniformLocation(id, "fixedLight");
	locMatEmission       = glGetUniformLocation(id, "material.emission");
	locMatAmbient        = glGetUniformLocation(id, "material.ambient");
	locMatDiffuse        = glGetUniformLocation(id, "material.diffuse");
	locMatSpecular       = glGetUniformLocation(id, "material.specular");
	locMatShininess      = glGetUniformLocation(id, "material.shininess");

	char nom[64];
	for (int i = 0; i < 8; ++i) {
		sprintf_s(nom, "LightSource[%d].sw_light", i);      locLlums[i].swLight = glGetUniformLocation(id, nom);
		sprintf_s(nom, "LightSource[%d].position", i);      locLlums[i].position = glGetUniformLocation(id, nom);
		sprintf_s(nom, "LightSource[%d].ambient", i);       locLlums[i].ambient = glGetUniformLocation(id, nom);
		sprintf_s(nom, "LightSource[%d].diffuse", i);       locLlums[i].diffuse = glGetUniformLocation(id, nom);
		sprintf_s(nom, "LightSource[%d].specular", i);      locLlums[i].specular = glGetUniformLocation(id, nom);
		sprintf_s(nom, "LightSource[%d].attenuation", i);   locLlums[i].attenuation = glGetUniformLocation(id, nom);
		sprintf_s(nom, "LightSource[%d].restricted", i);    locLlums[i].restricted = glGetUniformLocation(id, nom);
		sprintf_s(nom, "LightSource[%d].spotDirection", i); locLlums[i].spotDirection = glGetUniformLocation(id, nom);
		sprintf_s(nom, "LightSource[%d].spotCosCutoff", i); locLlums[i].spotCosCutoff = glGetUniformLocation(id, nom);
		sprintf_s(nom, "LightSource[%d].spotExponent", i);  locLlums[i].spotExponent = glGetUniformLocation(id, nom);
	}
}

void GestorRecursos::IniciarPrecarregaEscena()
{
	if (!s_precarregaIniciada.exchange(true)) {
		s_cancelarPrecarrega.store(false);
		s_precarregaAcabada.store(false);
		s_precarregaThread = std::thread(FonsPrecarregaEscena);
	}
}

void GestorRecursos::ProcessarCuaPujadaGPU(int maxTexturesPerFrame)
{
	// Pujar primer el cubemap del Skybox si ja esta descodificat a RAM
	if (s_cubemapID == 0) {
		ImatgeCPU caresLocal[6];
		bool teSkybox = false;
		{
			std::lock_guard<std::mutex> lock(s_mutex);
			if (s_skyboxCPUReady) {
				for (int i = 0; i < 6; ++i) {
					caresLocal[i] = s_skyboxCaresCPU[i];
					s_skyboxCaresCPU[i] = ImatgeCPU();
				}
				s_skyboxCPUReady = false;
				teSkybox = true;
			}
		}
		if (teSkybox) {
			s_cubemapID = PujarSkyboxAGPU(caresLocal);
			for (int i = 0; i < 6; ++i) {
				AlliberarImatgeCPU(caresLocal[i]);
			}
		}
	}

	if (maxTexturesPerFrame <= 0) return;

	// Extreure un lot acotat d'imatges per no causar micro-congelacions en cap fotograma
	std::vector<ImatgeCPU> lot;
	{
		std::lock_guard<std::mutex> lock(s_mutex);
		while (!s_cuaTexturesCPU.empty() && (int)lot.size() < maxTexturesPerFrame) {
			lot.push_back(s_cuaTexturesCPU.front());
			s_cuaTexturesCPU.pop_front();
		}
	}

	for (ImatgeCPU& img : lot) {
		auto it = s_cacheTextures2D.find(img.rutaNormalitzada);
		if (it == s_cacheTextures2D.end() || it->second == 0) {
			GLuint texID = PujarImatge2DAGPU(img);
			if (texID != 0) {
				s_cacheTextures2D[img.rutaNormalitzada] = texID;
			}
		}
		AlliberarImatgeCPU(img);
	}
}

void GestorRecursos::FinalitzarPrecarregaGPU()
{
	if (!s_precarregaIniciada.load()) {
		IniciarPrecarregaEscena();
	}
	if (s_precarregaThread.joinable()) {
		s_precarregaThread.join();
	}
	ProcessarCuaPujadaGPU(1000);
}

GLuint GestorRecursos::ObtenirTextura2D(const char* ruta)
{
	std::string rutaNorm = NormalitzarRuta(ruta);
	if (rutaNorm.empty()) return 0;

	// 1. Consulta rapida al cache GPU
	auto it = s_cacheTextures2D.find(rutaNorm);
	if (it != s_cacheTextures2D.end() && it->second != 0) {
		return it->second;
	}

	// 2. Comprovar si ja es troba descodificada a la cua CPU
	ImatgeCPU imgTrobada;
	bool trobadaEnCua = false;
	{
		std::lock_guard<std::mutex> lock(s_mutex);
		for (auto qIt = s_cuaTexturesCPU.begin(); qIt != s_cuaTexturesCPU.end(); ++qIt) {
			if (qIt->rutaNormalitzada == rutaNorm) {
				imgTrobada = *qIt;
				s_cuaTexturesCPU.erase(qIt);
				trobadaEnCua = true;
				break;
			}
		}
	}

	if (trobadaEnCua) {
		GLuint texID = PujarImatge2DAGPU(imgTrobada);
		AlliberarImatgeCPU(imgTrobada);
		if (texID != 0) {
			s_cacheTextures2D[rutaNorm] = texID;
		}
		return texID;
	}

	// 3. Fallback sincron si l'usuari entra abans que el fil secundari l'hagi llegit
	ImatgeCPU imgDirecta;
	imgDirecta.pixels = SOIL_load_image(rutaNorm.c_str(), &imgDirecta.width, &imgDirecta.height, &imgDirecta.channels, SOIL_LOAD_AUTO);
	if (imgDirecta.pixels == nullptr && ruta != nullptr) {
		imgDirecta.pixels = SOIL_load_image(ruta, &imgDirecta.width, &imgDirecta.height, &imgDirecta.channels, SOIL_LOAD_AUTO);
	}
	if (imgDirecta.pixels == nullptr) {
		fprintf(stderr, "No s'ha pogut carregar textura: %s\n", rutaNorm.c_str());
		return 0;
	}

	GLuint texID = PujarImatge2DAGPU(imgDirecta);
	AlliberarImatgeCPU(imgDirecta);
	if (texID != 0) {
		s_cacheTextures2D[rutaNorm] = texID;
	}
	return texID;
}

GLuint GestorRecursos::ObtenirNormalMapPerDefecte()
{
	if (s_defaultNormalMapID != 0) return s_defaultNormalMapID;

	unsigned char flatNormal[3] = { 128, 128, 255 }; // Vector tangent pla (0, 0, 1)
	glGenTextures(1, &s_defaultNormalMapID);
	glBindTexture(GL_TEXTURE_2D, s_defaultNormalMapID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, flatNormal);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
	return s_defaultNormalMapID;
}

GLuint GestorRecursos::ObtenirTexturaSkybox()
{
	if (s_cubemapID != 0) return s_cubemapID;

	// Intentar pujar les cares precarregades a RAM si ja estan disponibles
	ProcessarCuaPujadaGPU(0);
	if (s_cubemapID != 0) return s_cubemapID;

	// Fallback sincron en cas d'acces immediat abans que acabi la descodificacio del Skybox
	std::vector<std::string> faces;
	faces.reserve(6);
	for (int i = 0; i < 6; ++i) faces.emplace_back(kRutesSkybox[i]);
	s_cubemapID = loadCubemap(faces);
	return s_cubemapID;
}

bool GestorRecursos::SkyboxLlestAGPU()
{
	return s_cubemapID != 0;
}

void GestorRecursos::AssegurarVAOsEscenari()
{
	CColor colObj;
	colObj.r = 1.0; colObj.g = 1.0; colObj.b = 1.0; colObj.a = 1.0;
	SetColor4d(colObj.r, colObj.g, colObj.b, colObj.a);

	if (Get_VAOList(GLU_SPHERE).vaoId == 0) {
		Set_VAOList(GLU_SPHERE, loadgluSphere_EBO(1.0, 40, 40)); // Esfera unitaria compartida
	}
	if (Get_VAOList(GLUT_TORUS).vaoId == 0) {
		Set_VAOList(GLUT_TORUS, creaOrbita()); // Cercle unitari d'orbites compartit
	}
}

void GestorRecursos::AlliberarTot()
{
	s_cancelarPrecarrega.store(true);
	if (s_precarregaThread.joinable()) {
		s_precarregaThread.join();
	}

	{
		std::lock_guard<std::mutex> lock(s_mutex);
		for (ImatgeCPU& img : s_cuaTexturesCPU) {
			AlliberarImatgeCPU(img);
		}
		s_cuaTexturesCPU.clear();
		for (int i = 0; i < 6; ++i) {
			AlliberarImatgeCPU(s_skyboxCaresCPU[i]);
		}
		s_skyboxCPUReady = false;
	}

	for (auto& kv : s_cacheTextures2D) {
		if (kv.second != 0) {
			glDeleteTextures(1, &kv.second);
		}
	}
	s_cacheTextures2D.clear();

	if (s_defaultNormalMapID != 0) {
		glDeleteTextures(1, &s_defaultNormalMapID);
		s_defaultNormalMapID = 0;
	}
	if (s_cubemapID != 0) {
		glDeleteTextures(1, &s_cubemapID);
		s_cubemapID = 0;
	}

	deleteVAOList(GLU_SPHERE);
	deleteVAOList(GLU_DISK);
	deleteVAOList(GLUT_TORUS);
	deleteVAOList(CUBE_SKYBOX);

	s_precarregaIniciada.store(false);
	s_precarregaAcabada.store(false);
	s_cancelarPrecarrega.store(false);
}
