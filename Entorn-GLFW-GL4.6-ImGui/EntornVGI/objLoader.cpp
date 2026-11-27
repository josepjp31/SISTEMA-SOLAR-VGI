//******** PRACTICA VISUALITZACIO GRAFICA INTERACTIVA (Escola Enginyeria - UAB)
//******** Entorn basic VS2026 MONOFINESTRA amb OpenGL 4.6, interficie GLFW 3.4, ImGui i llibreries GLM
//******** Enric Marti (Setembre 2026)
// objLoader.cpp: Implements the class COBJModel.
//
//	  Versio 3.0:	- Carregador rapid d'una passada en memoria, cache binaria .mbin,
//					  VBO GL_FLOAT intercalat + EBO i precarrega asincrona CPU->GPU.
//////////////////////////////////////////////////////////////////////////////////////

#include "framework.h"
#include "objLoader.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unordered_map>

namespace
{
	struct Vec3f
	{
		float x, y, z;
	};

	struct Vec2f
	{
		float u, v;
	};

	struct FaceCorner
	{
		int iv;
		int ivt;
		int ivn;
	};

	struct TriBucket
	{
		int materialIndex;
		std::vector<FaceCorner> corners; // 3 corners per triangle
	};

	struct TripletKey
	{
		int iv;
		int ivt;
		int ivn;
		bool operator==(const TripletKey& o) const
		{
			return iv == o.iv && ivt == o.ivt && ivn == o.ivn;
		}
	};

	struct TripletKeyHash
	{
		size_t operator()(const TripletKey& k) const
		{
			uint64_t h = 14695981039346656037ULL;
			uint64_t a = (uint64_t)(uint32_t)k.iv;
			uint64_t b = (uint64_t)(uint32_t)k.ivt;
			uint64_t c = (uint64_t)(uint32_t)k.ivn;
			h = (h ^ a) * 1099511628211ULL;
			h = (h ^ b) * 1099511628211ULL;
			h = (h ^ c) * 1099511628211ULL;
			return (size_t)h;
		}
	};

	struct MBinHeader
	{
		char magic[4];      // 'M','B','I','N'
		uint32_t version;   // 1
		int64_t srcSize;
		int64_t srcMtime;
		uint32_t numMaterials;
		uint32_t numSubmeshes;
	};

	struct MBinMaterial
	{
		char szName[1024];
		char szTexture[_MAX_PATH];
		float fDiffuse[3];
		float fAmbient[3];
		float fSpecular[3];
		float fEmmissive[3];
		float fShininess;
	};

	void ExtractBasePath(const char* filePath, char* outBase, size_t outBaseSize)
	{
		if (!filePath || outBaseSize == 0) return;
		strncpy_s(outBase, outBaseSize, filePath, _TRUNCATE);
		int len = (int)strlen(outBase);
		for (int i = len - 1; i >= 0; --i)
		{
			if (outBase[i] == '\\' || outBase[i] == '/')
			{
				outBase[i + 1] = '\0';
				return;
			}
		}
		outBase[0] = '\0';
	}

	bool IsAbsoluteOrDrivePath(const char* path)
	{
		if (!path || path[0] == '\0') return false;
		if (path[0] == '/' || path[0] == '\\') return true;
		if (path[1] == ':') return true;
		return false;
	}

	void BuildJoinPath(const char* basePath, const char* relPath, char* outPath, size_t outSize)
	{
		if (!relPath || relPath[0] == '\0')
		{
			if (outSize > 0) outPath[0] = '\0';
			return;
		}
		if (IsAbsoluteOrDrivePath(relPath) || !basePath || basePath[0] == '\0')
		{
			strncpy_s(outPath, outSize, relPath, _TRUNCATE);
			return;
		}
		size_t blen = strlen(basePath);
		if (strncmp(relPath, basePath, blen) == 0)
		{
			strncpy_s(outPath, outSize, relPath, _TRUNCATE);
			return;
		}
		strncpy_s(outPath, outSize, basePath, _TRUNCATE);
		strncat_s(outPath, outSize, relPath, _TRUNCATE);
	}

	void BuildMBinPath(const char* objPath, char* outMBinPath, size_t outSize)
	{
		strncpy_s(outMBinPath, outSize, objPath, _TRUNCATE);
		size_t len = strlen(outMBinPath);
		if (len >= 4)
		{
			const char* ext = outMBinPath + len - 4;
			if (_stricmp(ext, ".obj") == 0)
			{
				outMBinPath[len - 4] = '\0';
				strncat_s(outMBinPath, outSize, ".mbin", _TRUNCATE);
				return;
			}
		}
		strncat_s(outMBinPath, outSize, ".mbin", _TRUNCATE);
	}

	void InitDefaultMaterial(Material& m, const char* name)
	{
		memset(&m, 0, sizeof(Material));
		if (name)
			strncpy_s(m.szName, sizeof(m.szName), name, _TRUNCATE);
		m.fDiffuse[0] = 1.0f;
		m.fDiffuse[1] = 1.0f;
		m.fDiffuse[2] = 1.0f;
	}

	const char* SkipSpaces(const char* p, const char* end)
	{
		while (p < end && (*p == ' ' || *p == '\t')) ++p;
		return p;
	}

	void ExtractTrimmedSpan(const char* p, const char* lineEnd, char* outBuf, size_t outSize)
	{
		p = SkipSpaces(p, lineEnd);
		const char* q = lineEnd;
		while (q > p && (q[-1] == ' ' || q[-1] == '\t' || q[-1] == '\r' || q[-1] == '\n')) --q;
		size_t len = (size_t)(q - p);
		if (len >= outSize) len = outSize - 1;
		if (len > 0) memcpy(outBuf, p, len);
		outBuf[len] = '\0';
	}

	bool ReadEntireFile(const char* path, std::vector<char>& outBuf)
	{
		FILE* fp = NULL;
		if (fopen_s(&fp, path, "rb") != 0 || fp == NULL)
			return false;
		if (_fseeki64(fp, 0, SEEK_END) != 0)
		{
			fclose(fp);
			return false;
		}
		__int64 size64 = _ftelli64(fp);
		if (size64 < 0)
		{
			fclose(fp);
			return false;
		}
		_fseeki64(fp, 0, SEEK_SET);
		size_t fileSize = (size_t)size64;
		outBuf.resize(fileSize + 1);
		size_t readTotal = 0;
		if (fileSize > 0)
			readTotal = fread(outBuf.data(), 1, fileSize, fp);
		fclose(fp);
		outBuf[readTotal] = '\0';
		return true;
	}

	bool LoadMaterialLibCPU(const char* mtlPath, std::vector<Material>& outMaterials)
	{
		std::vector<char> buf;
		if (!ReadEntireFile(mtlPath, buf))
			return false;

		const char* p = buf.data();
		const char* end = buf.data() + (buf.size() > 0 ? buf.size() - 1 : 0);
		int curIdx = -1;

		while (p < end)
		{
			const char* nl = (const char*)memchr(p, '\n', (size_t)(end - p));
			const char* lineEnd = nl ? nl : end;
			const char* s = SkipSpaces(p, lineEnd);

			if (s < lineEnd && *s != '#')
			{
				size_t rem = (size_t)(lineEnd - s);
				if (rem >= 7 && strncmp(s, "newmtl", 6) == 0 && (s[6] == ' ' || s[6] == '\t'))
				{
					char matName[1024];
					ExtractTrimmedSpan(s + 7, lineEnd, matName, sizeof(matName));
					Material m;
					InitDefaultMaterial(m, matName);
					outMaterials.push_back(m);
					curIdx = (int)outMaterials.size() - 1;
				}
				else if (curIdx >= 0)
				{
					Material& m = outMaterials[curIdx];
					if (rem >= 3 && (s[2] == ' ' || s[2] == '\t'))
					{
						if (s[0] == 'K' && s[1] == 'a')
						{
							char* nx = NULL;
							m.fAmbient[0] = strtof(s + 3, &nx);
							m.fAmbient[1] = strtof(nx, &nx);
							m.fAmbient[2] = strtof(nx, &nx);
						}
						else if (s[0] == 'K' && s[1] == 'd')
						{
							char* nx = NULL;
							m.fDiffuse[0] = strtof(s + 3, &nx);
							m.fDiffuse[1] = strtof(nx, &nx);
							m.fDiffuse[2] = strtof(nx, &nx);
						}
						else if (s[0] == 'K' && s[1] == 's')
						{
							char* nx = NULL;
							m.fSpecular[0] = strtof(s + 3, &nx);
							m.fSpecular[1] = strtof(nx, &nx);
							m.fSpecular[2] = strtof(nx, &nx);
						}
						else if (s[0] == 'K' && s[1] == 'e')
						{
							char* nx = NULL;
							m.fEmmissive[0] = strtof(s + 3, &nx);
							m.fEmmissive[1] = strtof(nx, &nx);
							m.fEmmissive[2] = strtof(nx, &nx);
						}
						else if (s[0] == 'N' && s[1] == 's')
						{
							char* nx = NULL;
							float ns = strtof(s + 3, &nx);
							m.fShininess = (ns / 1000.0f) * 128.0f;
						}
					}
					else if (rem >= 7 && strncmp(s, "map_Kd", 6) == 0 && (s[6] == ' ' || s[6] == '\t'))
					{
						ExtractTrimmedSpan(s + 7, lineEnd, m.szTexture, sizeof(m.szTexture));
					}
				}
			}

			p = nl ? (nl + 1) : end;
		}
		return true;
	}

	int ResolveObjIndex(long rawIdx, int count)
	{
		if (rawIdx > 0)
		{
			int idx = (int)(rawIdx - 1);
			return (idx < count) ? idx : -1;
		}
		if (rawIdx < 0)
		{
			int idx = count + (int)rawIdx;
			return (idx >= 0 && idx < count) ? idx : -1;
		}
		return -1;
	}

	bool ParseFaceCornerToken(const char*& cur, const char* lineEnd, int nPos, int nUV, int nNorm, FaceCorner& outCorner)
	{
		cur = SkipSpaces(cur, lineEnd);
		if (cur >= lineEnd || *cur == '\r' || *cur == '\n') return false;

		char* next = NULL;
		long rawV = strtol(cur, &next, 10);
		if (next == cur) return false;

		long rawVT = 0;
		long rawVN = 0;
		const char* p = next;
		if (p < lineEnd && *p == '/')
		{
			++p;
			if (p < lineEnd && *p != '/' && *p != ' ' && *p != '\t' && *p != '\r' && *p != '\n')
			{
				rawVT = strtol(p, &next, 10);
				p = next;
			}
			if (p < lineEnd && *p == '/')
			{
				++p;
				if (p < lineEnd && *p != ' ' && *p != '\t' && *p != '\r' && *p != '\n')
				{
					rawVN = strtol(p, &next, 10);
					p = next;
				}
			}
		}

		while (p < lineEnd && *p != ' ' && *p != '\t' && *p != '\r' && *p != '\n') ++p;
		cur = p;

		outCorner.iv = ResolveObjIndex(rawV, nPos);
		outCorner.ivt = (rawVT != 0) ? ResolveObjIndex(rawVT, nUV) : -1;
		outCorner.ivn = (rawVN != 0) ? ResolveObjIndex(rawVN, nNorm) : -1;
		return (outCorner.iv >= 0);
	}

	Vec3f ComputeFaceNormal(const Vec3f& a, const Vec3f& b, const Vec3f& c)
	{
		float ux = a.x - b.x, uy = a.y - b.y, uz = a.z - b.z;
		float vx = b.x - c.x, vy = b.y - c.y, vz = b.z - c.z;
		float nx = uy * vz - uz * vy;
		float ny = uz * vx - ux * vz;
		float nz = ux * vy - uy * vx;
		float len = sqrtf(nx * nx + ny * ny + nz * nz);
		if (len > 1e-8f)
		{
			nx /= len; ny /= len; nz /= len;
		}
		else
		{
			nx = 0.0f; ny = 0.0f; nz = 1.0f;
		}
		return { nx, ny, nz };
	}

	void BuildIndexedSubMesh(
		const TriBucket& bucket,
		bool isSingleBucket,
		const std::vector<Vec3f>& positions,
		const std::vector<Vec3f>& normals,
		const std::vector<Vec2f>& texcoords,
		OBJSubMeshCPU& outSubMesh)
	{
		outSubMesh.materialIndex = bucket.materialIndex;
		outSubMesh.vertices.clear();
		outSubMesh.indices.clear();

		const size_t nCorners = bucket.corners.size();
		if (nCorners == 0) return;

		// Cami rapid per a malles pre-indexades 1:1 (v == vt == vn), com milano_gotg.obj
		bool canUseDirectIndex = isSingleBucket
			&& !positions.empty()
			&& positions.size() == normals.size()
			&& positions.size() == texcoords.size();

		if (canUseDirectIndex)
		{
			for (size_t i = 0; i < nCorners; ++i)
			{
				const FaceCorner& fc = bucket.corners[i];
				if (fc.iv != fc.ivt || fc.iv != fc.ivn || fc.iv < 0)
				{
					canUseDirectIndex = false;
					break;
				}
			}
		}

		if (canUseDirectIndex)
		{
			const size_t nVerts = positions.size();
			outSubMesh.vertices.resize(nVerts);
			for (size_t i = 0; i < nVerts; ++i)
			{
				VertexOBJ& v = outSubMesh.vertices[i];
				v.px = positions[i].x; v.py = positions[i].y; v.pz = positions[i].z;
				v.nx = normals[i].x;   v.ny = normals[i].y;   v.nz = normals[i].z;
				v.u  = texcoords[i].u; v.v  = texcoords[i].v;
			}
			outSubMesh.indices.resize(nCorners);
			for (size_t i = 0; i < nCorners; ++i)
			{
				outSubMesh.indices[i] = (unsigned int)bucket.corners[i].iv;
			}
			return;
		}

		// Cami general amb deduplicacio per triplet (iv, ivt, ivn)
		outSubMesh.indices.reserve(nCorners);
		outSubMesh.vertices.reserve(nCorners / 2);
		std::unordered_map<TripletKey, unsigned int, TripletKeyHash> vertMap;
		vertMap.reserve(nCorners / 2);

		for (size_t i = 0; i + 2 < nCorners; i += 3)
		{
			Vec3f fallbackN = { 0.0f, 0.0f, 1.0f };
			bool needFallbackN = (bucket.corners[i].ivn < 0) ||
				(bucket.corners[i + 1].ivn < 0) ||
				(bucket.corners[i + 2].ivn < 0);
			if (needFallbackN)
			{
				fallbackN = ComputeFaceNormal(
					positions[bucket.corners[i].iv],
					positions[bucket.corners[i + 1].iv],
					positions[bucket.corners[i + 2].iv]);
			}

			for (size_t k = 0; k < 3; ++k)
			{
				const FaceCorner& fc = bucket.corners[i + k];
				if (fc.ivn >= 0)
				{
					TripletKey key = { fc.iv, fc.ivt, fc.ivn };
					auto it = vertMap.find(key);
					if (it != vertMap.end())
					{
						outSubMesh.indices.push_back(it->second);
						continue;
					}
					unsigned int newIdx = (unsigned int)outSubMesh.vertices.size();
					VertexOBJ v;
					v.px = positions[fc.iv].x; v.py = positions[fc.iv].y; v.pz = positions[fc.iv].z;
					v.nx = normals[fc.ivn].x;  v.ny = normals[fc.ivn].y;  v.nz = normals[fc.ivn].z;
					if (fc.ivt >= 0) { v.u = texcoords[fc.ivt].u; v.v = texcoords[fc.ivt].v; }
					else { v.u = 0.0f; v.v = 0.0f; }
					outSubMesh.vertices.push_back(v);
					vertMap.emplace(key, newIdx);
					outSubMesh.indices.push_back(newIdx);
				}
				else
				{
					unsigned int newIdx = (unsigned int)outSubMesh.vertices.size();
					VertexOBJ v;
					v.px = positions[fc.iv].x; v.py = positions[fc.iv].y; v.pz = positions[fc.iv].z;
					v.nx = fallbackN.x;        v.ny = fallbackN.y;        v.nz = fallbackN.z;
					if (fc.ivt >= 0) { v.u = texcoords[fc.ivt].u; v.v = texcoords[fc.ivt].v; }
					else { v.u = 0.0f; v.v = 0.0f; }
					outSubMesh.vertices.push_back(v);
					outSubMesh.indices.push_back(newIdx);
				}
			}
		}
	}

	bool TryLoadMBin(const char* mbinPath, int64_t srcSize, int64_t srcMtime, OBJPreloadData& outData)
	{
		FILE* fp = NULL;
		if (fopen_s(&fp, mbinPath, "rb") != 0 || fp == NULL)
			return false;

		MBinHeader hdr;
		if (fread(&hdr, sizeof(MBinHeader), 1, fp) != 1)
		{
			fclose(fp);
			return false;
		}
		if (hdr.magic[0] != 'M' || hdr.magic[1] != 'B' || hdr.magic[2] != 'I' || hdr.magic[3] != 'N' ||
			hdr.version != 1 || hdr.srcSize != srcSize || hdr.srcMtime != srcMtime ||
			hdr.numMaterials > MAX_SIZE_VAOLIST || hdr.numSubmeshes == 0 || hdr.numSubmeshes > MAX_SIZE_VAOLIST)
		{
			fclose(fp);
			return false;
		}

		outData.materials.resize(hdr.numMaterials);
		for (uint32_t i = 0; i < hdr.numMaterials; ++i)
		{
			MBinMaterial bm;
			if (fread(&bm, sizeof(MBinMaterial), 1, fp) != 1)
			{
				fclose(fp);
				outData.materials.clear();
				return false;
			}
			Material& m = outData.materials[i];
			memset(&m, 0, sizeof(Material));
			memcpy(m.szName, bm.szName, sizeof(m.szName));
			memcpy(m.szTexture, bm.szTexture, sizeof(m.szTexture));
			m.szName[sizeof(m.szName) - 1] = '\0';
			m.szTexture[sizeof(m.szTexture) - 1] = '\0';
			memcpy(m.fDiffuse, bm.fDiffuse, sizeof(m.fDiffuse));
			memcpy(m.fAmbient, bm.fAmbient, sizeof(m.fAmbient));
			memcpy(m.fSpecular, bm.fSpecular, sizeof(m.fSpecular));
			memcpy(m.fEmmissive, bm.fEmmissive, sizeof(m.fEmmissive));
			m.fShininess = bm.fShininess;
			m.iTextureID = 0;
		}

		outData.submeshes.resize(hdr.numSubmeshes);
		for (uint32_t s = 0; s < hdr.numSubmeshes; ++s)
		{
			int32_t matIdx = -1;
			uint32_t nVerts = 0;
			uint32_t nIndices = 0;
			if (fread(&matIdx, sizeof(int32_t), 1, fp) != 1 ||
				fread(&nVerts, sizeof(uint32_t), 1, fp) != 1 ||
				fread(&nIndices, sizeof(uint32_t), 1, fp) != 1 ||
				nVerts == 0 || nIndices == 0)
			{
				fclose(fp);
				outData.materials.clear();
				outData.submeshes.clear();
				return false;
			}

			OBJSubMeshCPU& sm = outData.submeshes[s];
			sm.materialIndex = (int)matIdx;
			sm.vertices.resize(nVerts);
			sm.indices.resize(nIndices);

			if (fread(sm.vertices.data(), sizeof(VertexOBJ), nVerts, fp) != nVerts ||
				fread(sm.indices.data(), sizeof(unsigned int), nIndices, fp) != nIndices)
			{
				fclose(fp);
				outData.materials.clear();
				outData.submeshes.clear();
				return false;
			}
		}

		fclose(fp);
		return true;
	}

	void SaveMBin(const char* mbinPath, int64_t srcSize, int64_t srcMtime, const OBJPreloadData& data)
	{
		if (data.materials.size() > MAX_SIZE_VAOLIST || data.submeshes.empty() || data.submeshes.size() > MAX_SIZE_VAOLIST)
			return;

		char tmpPath[_MAX_PATH];
		strncpy_s(tmpPath, sizeof(tmpPath), mbinPath, _TRUNCATE);
		strncat_s(tmpPath, sizeof(tmpPath), ".tmp", _TRUNCATE);

		FILE* fp = NULL;
		if (fopen_s(&fp, tmpPath, "wb") != 0 || fp == NULL)
			return;

		MBinHeader hdr;
		hdr.magic[0] = 'M'; hdr.magic[1] = 'B'; hdr.magic[2] = 'I'; hdr.magic[3] = 'N';
		hdr.version = 1;
		hdr.srcSize = srcSize;
		hdr.srcMtime = srcMtime;
		hdr.numMaterials = (uint32_t)data.materials.size();
		hdr.numSubmeshes = (uint32_t)data.submeshes.size();

		bool ok = (fwrite(&hdr, sizeof(MBinHeader), 1, fp) == 1);

		for (size_t i = 0; ok && i < data.materials.size(); ++i)
		{
			const Material& m = data.materials[i];
			MBinMaterial bm;
			memset(&bm, 0, sizeof(MBinMaterial));
			memcpy(bm.szName, m.szName, sizeof(bm.szName));
			memcpy(bm.szTexture, m.szTexture, sizeof(bm.szTexture));
			memcpy(bm.fDiffuse, m.fDiffuse, sizeof(bm.fDiffuse));
			memcpy(bm.fAmbient, m.fAmbient, sizeof(bm.fAmbient));
			memcpy(bm.fSpecular, m.fSpecular, sizeof(bm.fSpecular));
			memcpy(bm.fEmmissive, m.fEmmissive, sizeof(bm.fEmmissive));
			bm.fShininess = m.fShininess;
			ok = (fwrite(&bm, sizeof(MBinMaterial), 1, fp) == 1);
		}

		for (size_t s = 0; ok && s < data.submeshes.size(); ++s)
		{
			const OBJSubMeshCPU& sm = data.submeshes[s];
			int32_t matIdx = (int32_t)sm.materialIndex;
			uint32_t nVerts = (uint32_t)sm.vertices.size();
			uint32_t nIndices = (uint32_t)sm.indices.size();
			ok = (fwrite(&matIdx, sizeof(int32_t), 1, fp) == 1) &&
				 (fwrite(&nVerts, sizeof(uint32_t), 1, fp) == 1) &&
				 (fwrite(&nIndices, sizeof(uint32_t), 1, fp) == 1) &&
				 (fwrite(sm.vertices.data(), sizeof(VertexOBJ), nVerts, fp) == nVerts) &&
				 (fwrite(sm.indices.data(), sizeof(unsigned int), nIndices, fp) == nIndices);
		}

		fclose(fp);
		if (ok)
		{
			if (!MoveFileExA(tmpPath, mbinPath, MOVEFILE_REPLACE_EXISTING))
				DeleteFileA(tmpPath);
		}
		else
		{
			DeleteFileA(tmpPath);
		}
	}
}

//////////////////////////////////////////////////////////////////////
// Construction / Destruction
//////////////////////////////////////////////////////////////////////

_stdcall COBJModel::COBJModel()
{
	numMaterials = 0;
	numSubmeshes = 0;
	cachedProgramID = 0;
	loc_texture0 = -1;
	loc_textur = -1;
	loc_matAmbient = -1;
	loc_matDiffuse = -1;
	loc_matSpecular = -1;
	loc_matShininess = -1;
	memset(vector_Materials, 0, sizeof(vector_Materials));
	memset(vMaterials, 0, sizeof(vMaterials));
	initVAOList_OBJ();
}

_stdcall COBJModel::~COBJModel()
{
	netejaVAOList_OBJ();
	netejaTextures_OBJ();
}

void _stdcall COBJModel::FreePreloadData(OBJPreloadData& data)
{
	for (size_t i = 0; i < data.textures.size(); ++i)
	{
		if (data.textures[i].pixels != NULL)
		{
			SOIL_free_image_data(data.textures[i].pixels);
			data.textures[i].pixels = NULL;
		}
		data.textures[i].width = 0;
		data.textures[i].height = 0;
		data.textures[i].channels = 0;
	}
	data.textures.clear();
	data.submeshes.clear();
	data.materials.clear();
	data.valid = false;
}

bool _stdcall COBJModel::LoadToCPU(const char* szFileName, OBJPreloadData& outData)
{
	FreePreloadData(outData);
	if (!szFileName || szFileName[0] == '\0')
		return false;

	struct __stat64 st;
	if (_stat64(szFileName, &st) != 0)
		return false;

	int64_t srcSize = (int64_t)st.st_size;
	int64_t srcMtime = (int64_t)st.st_mtime;

	char szBasePath[_MAX_PATH];
	ExtractBasePath(szFileName, szBasePath, sizeof(szBasePath));

	char szMBinPath[_MAX_PATH];
	BuildMBinPath(szFileName, szMBinPath, sizeof(szMBinPath));

	bool loadedFromCache = TryLoadMBin(szMBinPath, srcSize, srcMtime, outData);
	if (!loadedFromCache)
	{
		std::vector<char> fileBuf;
		if (!ReadEntireFile(szFileName, fileBuf))
			return false;

		size_t estVerts = (size_t)(srcSize / 64);
		if (estVerts < 1024) estVerts = 1024;
		std::vector<Vec3f> positions;
		std::vector<Vec3f> normals;
		std::vector<Vec2f> texcoords;
		positions.reserve(estVerts);
		normals.reserve(estVerts);
		texcoords.reserve(estVerts);

		std::vector<TriBucket> buckets;
		std::unordered_map<int, int> matToBucket;
		int curMaterialIdx = -1;

		auto appendCornerToBucket = [&](int matIdx, const FaceCorner& c0, const FaceCorner& c1, const FaceCorner& c2)
		{
			int bIdx = -1;
			auto it = matToBucket.find(matIdx);
			if (it == matToBucket.end())
			{
				bIdx = (int)buckets.size();
				buckets.push_back(TriBucket());
				buckets.back().materialIndex = matIdx;
				buckets.back().corners.reserve(estVerts * 3);
				matToBucket[matIdx] = bIdx;
			}
			else
			{
				bIdx = it->second;
			}
			buckets[bIdx].corners.push_back(c0);
			buckets[bIdx].corners.push_back(c1);
			buckets[bIdx].corners.push_back(c2);
		};

		const char* p = fileBuf.data();
		const char* end = fileBuf.data() + (fileBuf.size() > 0 ? fileBuf.size() - 1 : 0);

		while (p < end)
		{
			const char* nl = (const char*)memchr(p, '\n', (size_t)(end - p));
			const char* lineEnd = nl ? nl : end;
			const char* s = SkipSpaces(p, lineEnd);

			if (s < lineEnd && *s != '#')
			{
				size_t rem = (size_t)(lineEnd - s);
				if (s[0] == 'v' && rem >= 2)
				{
					if (s[1] == ' ' || s[1] == '\t')
					{
						char* nx = NULL;
						float vx = strtof(s + 2, &nx);
						float vy = strtof(nx, &nx);
						float vz = strtof(nx, &nx);
						positions.push_back({ vx, vy, vz });
					}
					else if (s[1] == 't' && rem >= 3 && (s[2] == ' ' || s[2] == '\t'))
					{
						char* nx = NULL;
						float tu = strtof(s + 3, &nx);
						float tv = strtof(nx, &nx);
						texcoords.push_back({ tu, tv });
					}
					else if (s[1] == 'n' && rem >= 3 && (s[2] == ' ' || s[2] == '\t'))
					{
						char* nx = NULL;
						float vnx = strtof(s + 3, &nx);
						float vny = strtof(nx, &nx);
						float vnz = strtof(nx, &nx);
						normals.push_back({ vnx, vny, vnz });
					}
				}
				else if (s[0] == 'f' && rem >= 2 && (s[1] == ' ' || s[1] == '\t'))
				{
					FaceCorner poly[256];
					int nPoly = 0;
					const char* q = s + 2;
					int nPos = (int)positions.size();
					int nUV = (int)texcoords.size();
					int nNorm = (int)normals.size();
					while (nPoly < 256 && ParseFaceCornerToken(q, lineEnd, nPos, nUV, nNorm, poly[nPoly]))
					{
						nPoly++;
					}
					for (int k = 1; k + 1 < nPoly; ++k)
					{
						appendCornerToBucket(curMaterialIdx, poly[0], poly[k], poly[k + 1]);
					}
				}
				else if (rem >= 7 && strncmp(s, "usemtl", 6) == 0 && (s[6] == ' ' || s[6] == '\t'))
				{
					char matName[1024];
					ExtractTrimmedSpan(s + 7, lineEnd, matName, sizeof(matName));
					for (size_t m = 0; m < outData.materials.size(); ++m)
					{
						if (strcmp(outData.materials[m].szName, matName) == 0)
						{
							curMaterialIdx = (int)m;
							break;
						}
					}
				}
				else if (rem >= 7 && strncmp(s, "mtllib", 6) == 0 && (s[6] == ' ' || s[6] == '\t'))
				{
					char libName[_MAX_PATH];
					ExtractTrimmedSpan(s + 7, lineEnd, libName, sizeof(libName));
					char libFullPath[_MAX_PATH];
					BuildJoinPath(szBasePath, libName, libFullPath, sizeof(libFullPath));
					LoadMaterialLibCPU(libFullPath, outData.materials);
				}
			}

			p = nl ? (nl + 1) : end;
		}

		// Alliberar text cru de memoria abans de construir buffers
		std::vector<char>().swap(fileBuf);

		if (outData.materials.size() > MAX_SIZE_VAOLIST)
			outData.materials.resize(MAX_SIZE_VAOLIST);

		const bool isSingleBucket = (buckets.size() == 1);
		size_t maxBuckets = buckets.size();
		if (maxBuckets > MAX_SIZE_VAOLIST) maxBuckets = MAX_SIZE_VAOLIST;

		outData.submeshes.resize(maxBuckets);
		for (size_t b = 0; b < maxBuckets; ++b)
		{
			BuildIndexedSubMesh(buckets[b], isSingleBucket, positions, normals, texcoords, outData.submeshes[b]);
		}

		if (!outData.submeshes.empty())
			SaveMBin(szMBinPath, srcSize, srcMtime, outData);
	}

	// Descodificar textures a RAM (SOIL_load_image es thread-safe, sense crides OpenGL)
	outData.textures.resize(outData.materials.size());
	for (size_t i = 0; i < outData.materials.size(); ++i)
	{
		const Material& mat = outData.materials[i];
		if (mat.szTexture[0] == '\0') continue;

		char texPath[_MAX_PATH];
		BuildJoinPath(szBasePath, mat.szTexture, texPath, sizeof(texPath));

		OBJTextureCPU& tex = outData.textures[i];
		tex.pixels = SOIL_load_image(texPath, &tex.width, &tex.height, &tex.channels, SOIL_LOAD_AUTO);
		if (tex.pixels == NULL && strcmp(texPath, mat.szTexture) != 0)
		{
			tex.pixels = SOIL_load_image(mat.szTexture, &tex.width, &tex.height, &tex.channels, SOIL_LOAD_AUTO);
		}
	}

	outData.valid = !outData.submeshes.empty();
	return outData.valid;
}

int _stdcall COBJModel::UploadFromCPU(OBJPreloadData& data)
{
	if (!data.valid || data.submeshes.empty())
	{
		FreePreloadData(data);
		return 1;
	}

	netejaVAOList_OBJ();
	netejaTextures_OBJ();
	initVAOList_OBJ();
	memset(vector_Materials, 0, sizeof(vector_Materials));
	memset(vMaterials, 0, sizeof(vMaterials));

	GLint matCount = (GLint)data.materials.size();
	if (matCount > MAX_SIZE_VAOLIST) matCount = MAX_SIZE_VAOLIST;
	numMaterials = matCount;

	GLint prevUnpack = 4;
	glGetIntegerv(GL_UNPACK_ALIGNMENT, &prevUnpack);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	for (GLint i = 0; i < matCount; ++i)
	{
		vMaterials[i] = data.materials[i];
		vMaterials[i].iTextureID = 0;

		if ((size_t)i < data.textures.size() && data.textures[i].pixels != NULL)
		{
			OBJTextureCPU& tex = data.textures[i];
			GLuint texId = SOIL_create_OGL_texture(
				tex.pixels,
				&tex.width,
				&tex.height,
				tex.channels,
				SOIL_CREATE_NEW_ID,
				SOIL_FLAG_GL_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_TEXTURE_REPEATS
			);

			if (texId != 0)
			{
				glBindTexture(GL_TEXTURE_2D, texId);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

				if (GLEW_EXT_texture_filter_anisotropic)
				{
					GLfloat maxAniso = 1.0f;
					glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
					GLfloat aniso = (maxAniso > 8.0f) ? 8.0f : maxAniso;
					if (aniso > 1.0f)
						glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);
				}

				glBindTexture(GL_TEXTURE_2D, 0);
				vMaterials[i].iTextureID = texId;
			}
		}
	}
	glPixelStorei(GL_UNPACK_ALIGNMENT, prevUnpack);

	GLint subCount = (GLint)data.submeshes.size();
	if (subCount > MAX_SIZE_VAOLIST) subCount = MAX_SIZE_VAOLIST;

	GLint uploaded = 0;
	for (GLint k = 0; k < subCount; ++k)
	{
		const OBJSubMeshCPU& sm = data.submeshes[k];
		if (sm.vertices.empty() || sm.indices.empty()) continue;

		CVAO auxVAO = { 0, 0, 0, 0, 0 };
		glGenVertexArrays(1, &auxVAO.vaoId);
		glGenBuffers(1, &auxVAO.vboId);
		glGenBuffers(1, &auxVAO.eboId);

		glBindVertexArray(auxVAO.vaoId);

		glBindBuffer(GL_ARRAY_BUFFER, auxVAO.vboId);
		glBufferData(GL_ARRAY_BUFFER,
			(GLsizeiptr)(sm.vertices.size() * sizeof(VertexOBJ)),
			sm.vertices.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, auxVAO.eboId);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,
			(GLsizeiptr)(sm.indices.size() * sizeof(unsigned int)),
			sm.indices.data(), GL_STATIC_DRAW);

		// location 0: in_Vertex
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexOBJ), (GLvoid*)0);

		// location 1: in_Color desactivat (subministrat per material amb glVertexAttrib4f)
		glDisableVertexAttribArray(1);

		// location 2: in_Normal
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(VertexOBJ), (GLvoid*)(3 * sizeof(float)));

		// location 3: in_TexCoord
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(VertexOBJ), (GLvoid*)(6 * sizeof(float)));

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		auxVAO.nVertexs = (GLint)sm.vertices.size();
		auxVAO.nIndices = (GLint)sm.indices.size();

		Set_VAOList_OBJ(uploaded, auxVAO);
		vector_Materials[uploaded] = sm.materialIndex;
		uploaded++;
	}

	numSubmeshes = uploaded;
	FreePreloadData(data);
	return (numSubmeshes > 0) ? 0 : 1;
}

int _stdcall COBJModel::LoadModel(char* szFileName)
{
	OBJPreloadData tmp;
	if (!LoadToCPU(szFileName, tmp))
		return 1;
	return UploadFromCPU(tmp);
}

void _stdcall COBJModel::updateUniformCache(GLuint sh_programID)
{
	if (cachedProgramID == sh_programID) return;
	cachedProgramID = sh_programID;
	loc_texture0 = glGetUniformLocation(sh_programID, "texture0");
	loc_textur = glGetUniformLocation(sh_programID, "textur");
	loc_matAmbient = glGetUniformLocation(sh_programID, "material.ambient");
	loc_matDiffuse = glGetUniformLocation(sh_programID, "material.diffuse");
	loc_matSpecular = glGetUniformLocation(sh_programID, "material.specular");
	loc_matShininess = glGetUniformLocation(sh_programID, "material.shininess");
}

void _stdcall COBJModel::UseMaterial_ShaderID(GLuint sh_programID, const Material& pMaterial)
{
	glVertexAttrib4f(1, pMaterial.fDiffuse[0], pMaterial.fDiffuse[1], pMaterial.fDiffuse[2], 1.0f);

	if (pMaterial.iTextureID != 0)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, pMaterial.iTextureID);
		if (loc_texture0 != -1) glUniform1i(loc_texture0, 0);
		if (loc_textur != -1) glUniform1i(loc_textur, GL_TRUE);
	}
	else
	{
		if (loc_textur != -1) glUniform1i(loc_textur, GL_FALSE);
		if (loc_matAmbient != -1)
			glUniform4f(loc_matAmbient, pMaterial.fAmbient[0], pMaterial.fAmbient[1], pMaterial.fAmbient[2], 1.0f);
		if (loc_matDiffuse != -1)
			glUniform4f(loc_matDiffuse, pMaterial.fDiffuse[0], pMaterial.fDiffuse[1], pMaterial.fDiffuse[2], 1.0f);
		if (loc_matSpecular != -1)
			glUniform4f(loc_matSpecular, pMaterial.fSpecular[0], pMaterial.fSpecular[1], pMaterial.fSpecular[2], 1.0f);
		if (loc_matShininess != -1)
			glUniform1f(loc_matShininess, pMaterial.fShininess);
	}
}

void _stdcall COBJModel::DrawModel(int prim_Id)
{
	deleteVAOList(prim_Id);
}

OBJLOADER_CLASS_DECL COBJModel* _stdcall InitObject()
{
	return new COBJModel;
}

OBJLOADER_CLASS_DECL void _stdcall UnInitObject(COBJModel * obj)
{
	delete obj;
}

void _stdcall COBJModel::EliminaLlista(int prim_Id)
{
	deleteVAOList(prim_Id);
}

// ------------------------ GESTIO VAOLIST
void _stdcall COBJModel::initVAOList_OBJ()
{
	for (int i = 0; i < MAX_SIZE_VAOLIST; i++)
	{
		VAOList_OBJ[i].vaoId = 0;
		VAOList_OBJ[i].vboId = 0;
		VAOList_OBJ[i].eboId = 0;
		VAOList_OBJ[i].nVertexs = 0;
		VAOList_OBJ[i].nIndices = 0;
	}
}

void _stdcall COBJModel::Set_VAOList_OBJ(GLint k, CVAO auxVAO)
{
	if (k < 0 || k >= MAX_SIZE_VAOLIST) return;
	VAOList_OBJ[k] = auxVAO;
}

void _stdcall COBJModel::netejaVAOList_OBJ()
{
	for (GLint i = 0; i < MAX_SIZE_VAOLIST; i++)
	{
		if (VAOList_OBJ[i].vaoId != 0) deleteVAOList_OBJ(i);
	}
	numSubmeshes = 0;
}

void _stdcall COBJModel::netejaTextures_OBJ()
{
	for (int i = 0; i < MAX_SIZE_VAOLIST; i++)
	{
		if (vMaterials[i].iTextureID != 0)
		{
			glDeleteTextures(1, &vMaterials[i].iTextureID);
			vMaterials[i].iTextureID = 0;
		}
	}
	numMaterials = 0;
}

void _stdcall COBJModel::deleteVAOList_OBJ(GLint k)
{
	if (k < 0 || k >= MAX_SIZE_VAOLIST) return;
	if (VAOList_OBJ[k].vaoId != 0)
	{
		glBindVertexArray(VAOList_OBJ[k].vaoId);
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		glDisableVertexAttribArray(3);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		if (VAOList_OBJ[k].vboId != 0)
			glDeleteBuffers(1, &VAOList_OBJ[k].vboId);

		if (VAOList_OBJ[k].eboId != 0)
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			glDeleteBuffers(1, &VAOList_OBJ[k].eboId);
		}

		glBindVertexArray(0);
		glDeleteVertexArrays(1, &VAOList_OBJ[k].vaoId);

		VAOList_OBJ[k].vaoId = 0;
		VAOList_OBJ[k].vboId = 0;
		VAOList_OBJ[k].eboId = 0;
		VAOList_OBJ[k].nVertexs = 0;
		VAOList_OBJ[k].nIndices = 0;
	}
}

//--------------- DRAWS (DIBUIX DE VAO's)
void _stdcall COBJModel::draw_TriVAO_Object_OBJ(GLint k)
{
	if (k < 0 || k >= MAX_SIZE_VAOLIST) return;
	if (VAOList_OBJ[k].vaoId != 0)
	{
		glBindVertexArray(VAOList_OBJ[k].vaoId);
		if (VAOList_OBJ[k].eboId != 0 && VAOList_OBJ[k].nIndices > 0)
			glDrawElements(GL_TRIANGLES, VAOList_OBJ[k].nIndices, GL_UNSIGNED_INT, 0);
		else
			glDrawArrays(GL_TRIANGLES, 0, VAOList_OBJ[k].nVertexs);
		glBindVertexArray(0);
	}
}

void _stdcall COBJModel::draw_TriVAO_OBJ(GLuint sh_programID)
{
	updateUniformCache(sh_programID);

	Material defaultMat;
	InitDefaultMaterial(defaultMat, "default");

	for (int i = 0; i < numSubmeshes; i++)
	{
		int matIdx = vector_Materials[i];
		if (matIdx >= 0 && matIdx < numMaterials)
			UseMaterial_ShaderID(sh_programID, vMaterials[matIdx]);
		else
			UseMaterial_ShaderID(sh_programID, defaultMat);

		draw_TriVAO_Object_OBJ(i);
	}
}
