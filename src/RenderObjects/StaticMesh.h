// Created by Coby Jennings on 2026/10/11.

#ifndef GCCUBEENGINE_STATICMESH_H
#define GCCUBEENGINE_STATICMESH_H
#include <cassert>
#include <cstring>
#include <debug.h>
#include <ogc/gx.h>
#include <malloc.h>
#include <ogc/cache.h>

#include "../Core/GenericTypes.h"
#include "Core/FIle/FileSystem.h"

struct alignas_gc(16) Vec4f
{
	float x;
	float y;
	float z;
	float w;
};

struct alignas_gc(16) Vec3f
{
	float x;
	float y;
	float z;
};

struct alignas_gc(16) Vec3i
{
	uint32_t x;
	uint32_t y;
	uint32_t z;
};

struct alignas_gc(16) Vec2f
{
	float x;
	float y;
};

// .gcstm
// Byte packed to 64 to support 2 32 byte cache
// SHOULD be 48 bytes wide
// lines in the GC.
struct alignas_gc(32) GAMECUBE_StaticMesh
{
	char TypeIdentifier[4]{'G', 'C', 'S', 'M'};
	unsigned int VersionNumber = 1;

	// Counts
	unsigned int IndexCount;
	unsigned int VertexCount;
	unsigned int UVCount;
	unsigned int NormalCount;
	unsigned int ColorCount;

	// Offsets
	unsigned int IndexArrayOffset;
	unsigned int VertexArrayOffset;
	unsigned int UVOffset;
	unsigned int NormalOffset;
	unsigned int ColorOffset;

	char padding[16];
};

class alignas_gc(32) StaticMeshObject
{
	StaticMeshObject(const char *FilePath)
	{
		// Todo Gamecube File IO
		StaticMeshObject = GAMECUBE_StaticMesh();
	}

	GAMECUBE_StaticMesh MeshData;
	Vec3i *Indices;
	Vec3f *Vertices;
	Vec3f *Textures;
	Vec3f *Normals;
	Vec4f *Colors;

	void *DisplayListBuffer{NULL};
	u32 DisplayListSize{0};

	void LoadStaticMesh()
	{
		FileSystem::Initialize();
		uint32 FileLen = 0;
		uint8_t *Data = FileSystem::LoadRawBianry("Data/Meshes/SimpleCube.gcsm", FileLen);
		memcpy(&MeshData, Data, 64);
		Data += 64;

		if (MeshData.IndexCount == 0)
			assert(false);

		size_t IndexSize = sizeof(Vec3i) * MeshData.IndexCount;
		Indices = (Vec3i*)malloc(IndexSize);
		memcpy(Indices, Data, IndexSize);
		Data += IndexSize;

		if (MeshData.VertexCount == 0)
			assert(false);

		size_t VertSize = sizeof(Vec3f) * MeshData.VertexCount;
		Vertices = (Vec3f*)malloc(VertSize);
		memcpy(Vertices, Data, VertSize);
		Data += VertSize;

		if (MeshData.UVCount == 0)
			assert(false);

		size_t TexSize = sizeof(Vec3f) * MeshData.UVCount;
		Textures = (Vec3f*)malloc(TexSize);
		memcpy(Textures, Data, TexSize);
		Data += TexSize;

		if (MeshData.NormalCount == 0)
			assert(false);

		size_t NrmSize = sizeof(Vec3f) * MeshData.NormalCount;
		Normals = (Vec3f*)malloc(NrmSize);
		memcpy(Normals, Data, NrmSize);
		Data += NrmSize;

		if (MeshData.ColorCount == 0)
			assert(false);

		size_t ColSize = sizeof(Vec4f) * MeshData.ColorCount;
		Colors = (Vec4f*)malloc(ColSize);
		memcpy(Colors, Data, ColSize);
		Data += ColSize;
	}

	void InitModelDisplayList()
	{
		// Allocate FIFO Buffer
		DisplayListBuffer = memalign(32, 256 * 1024);
		memset(DisplayListBuffer, 0, 256 * 1024);

		// Setup Array ptr for indexed drawing
		// Tell GX that GX_VA_POS (Vtx Position) is going to get a INT16
		GX_SetVtxDesc(GX_VA_POS, GX_INDEX16);
		// Tell GX that
		GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_S16, 0);
		GX_SetArray(GX_VA_POS, RawVerts, 3 * sizeof(s16));

		DCStoreRange(DisplayListBuffer, 256 * 1024);

		GX_Begin(GX_TRIANGLES, GX_VTXFMT0, numIndices);
		for (u32 i = 0; i < numIndices; i++)
		{
			GX_Position1x16(RawIndices[i]);
		}
		GX_End();

		DisplayListSize = GX_EndDispList();
		DCStoreRange(DisplayListBuffer, DisplayListSize);
	}

	void RenderModel(Mtx ModelMatrix)
	{
		// Apply Loc/Rot/Scale
		GX_LoadPosMtxImm(ModelMatrix, GX_PNMTX0);

		GX_CallDispList(DisplayListBuffer, DisplayListSize);
	}

	void CreateDisplayList()
	{
		//uint32 *Indices =
	}
};

#endif //GCCUBEENGINE_STATICMESH_H
