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

	void operator+=(Vec3f B)
	{
		x += B.x;
		y += B.y;
		z += B.z;
	}

	void operator-=(Vec3f B)
	{
		x -= B.x;
		y -= B.y;
		z -= B.z;
	}
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
public:
	StaticMeshObject(const char *FilePath)
	{
		// Todo Gamecube File IO
		MeshData = GAMECUBE_StaticMesh();
		Path = FilePath;

		guMtxIdentity(ModelMatrix);
		LoadStaticMesh();
	}

	const char *Path;

	GAMECUBE_StaticMesh MeshData;
	Vec3i *Indices;
	Vec3f *Vertices;
	Vec3f *Textures;
	Vec3f *Normals;
	Vec4f *Colors;
	int DataSize;

	void *DisplayListBuffer{NULL};
	uint32_t FinalDListSize{0};

	Mtx ModelMatrix;
	Vec3f YPR;
	Vec3f Pos;

	void LoadStaticMesh()
	{
		FileSystem::FileEntry File;
		FileSystem::GetFile(Path, File);

		uint32 FileLen = File.Size;
		uint8_t *Data = const_cast<uint8_t *>(File.Data);

		// Copy Header
		memcpy(&MeshData, Data, sizeof(GAMECUBE_StaticMesh));
		// Validate
		assert(MeshData.IndexCount > 0 && MeshData.VertexCount > 0);

		Indices = (Vec3i *) (Data + MeshData.IndexArrayOffset);
		Vertices = (Vec3f *) (Data + MeshData.VertexArrayOffset);
		Textures = (Vec3f *) (Data + MeshData.UVOffset);
		Normals = (Vec3f *) (Data + MeshData.NormalOffset);
		Colors = (Vec4f *) (Data + MeshData.ColorOffset);

		DataSize = FileLen;

		// Flush to Main Memory
		DCFlushRange(Data, FileLen);

		CreateDisplayList();
	}

	void RotateModel(Vec3f Rot)
	{
		YPR = Rot;
		// X = Pitch
		// Y = Yaw
		// Z = Roll
		Mtx Yaw, Pitch, Roll;

		guVector XAxis{1.0f,0.0f,0.0f};
		guVector YAxis{0.0f,1.0f,0.0f};
		guVector ZAxis{0.0f,0.0f,1.0f};

		guMtxRotAxisDeg(Pitch, &XAxis, YPR.x);
		guMtxRotAxisDeg(Yaw, &YAxis, YPR.y);
		guMtxRotAxisDeg(Roll, &ZAxis, YPR.z);

		Mtx RotationMatrix, TempRot;
		guMtxConcat(Yaw, Pitch, TempRot);
		guMtxConcat(TempRot, Roll, RotationMatrix);

		guMtxConcat(ModelMatrix, RotationMatrix, ModelMatrix);
	}

	void TranslateModel(Vec3f NewPos)
	{
		Pos += NewPos;
		guMtxTrans(ModelMatrix, Pos.x, Pos.y, Pos.z);
	}

	void RenderModel(Mtx CameraMatrix)
	{
		// Tell GX we're supplying 16 bit indices for Pos, Nrm,, TexCoord
		GX_ClearVtxDesc();
		GX_SetVtxDesc(GX_VA_POS, GX_INDEX16);
		GX_SetVtxDesc(GX_VA_NRM, GX_INDEX16);
		GX_SetVtxDesc(GX_VA_TEX0, GX_INDEX16);

		// Setup Data Formats
		GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
		GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
		GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

		GX_SetArray(GX_VA_POS, Vertices, sizeof(Vec3f));
		GX_SetArray(GX_VA_NRM, Normals, sizeof(Vec3f));
		GX_SetArray(GX_VA_TEX0, Textures, sizeof(Vec2f));

		// Apply Loc/Rot/Scale
		Mtx CameraOffsetMatrix;
		guMtxConcat(CameraMatrix, ModelMatrix, CameraOffsetMatrix);
		GX_LoadPosMtxImm(CameraOffsetMatrix, GX_PNMTX0);

		printf("Draw");
		GX_CallDispList(DisplayListBuffer, FinalDListSize);
	}

	void CreateDisplayList()
	{
		// Calculate buffer size (32 byte headers + 2 bytes per attribute per vertex)
		// Assuming 3 Attribs (Pos, Nrm, TexCoord) with 16 bit indices
		uint32_t rawSize = 32 + (MeshData.IndexCount * 18);
		// Align size to 32 byte boundary
		uint32_t DisplayListSize = (rawSize + 31) & ~31;

		// Alloc 32 byte aligned memory
		DisplayListBuffer = memalign(32, DisplayListSize);
		// Invalidate range before CPU write via GX
		DCInvalidateRange(DisplayListBuffer, DisplayListSize);

		GX_ClearVtxDesc();
		GX_SetVtxDesc(GX_VA_POS, GX_INDEX16);
		GX_SetVtxDesc(GX_VA_NRM, GX_INDEX16);
		GX_SetVtxDesc(GX_VA_TEX0, GX_INDEX16);

		GX_BeginDispList(DisplayListBuffer, DisplayListSize);
		GX_Begin(GX_TRIANGLES, GX_VTXFMT0, MeshData.IndexCount * 3);

		for (uint32_t i = 0; i < MeshData.IndexCount; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				uint32_t *offset = &Indices[i].x;
				offset += j;
				GX_Position1x16(*offset);
				GX_Normal1x16(*offset);
				GX_TexCoord1x16(*offset);
				printf("Indice Data: %d", *offset);
			}
		}

		GX_End();
		FinalDListSize = GX_EndDispList();
		FinalDListSize = (FinalDListSize + 31) & ~31;
		printf("ListSize: %d", FinalDListSize);
		DCFlushRange(DisplayListBuffer, FinalDListSize);
	}
};

#endif //GCCUBEENGINE_STATICMESH_H
