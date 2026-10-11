// Created by Coby Jennings on 2026/10/11.

#ifndef GCCUBEENGINE_STATICMESH_H
#define GCCUBEENGINE_STATICMESH_H
#include <debug.h>

#include "generic_types.h"

// .gcstm
// Byte packed to 64 to support 2 32 byte cache
// lines in the GC.
struct GAMECUBE_StaticMesh
{
	char TypeIdentifier[4] {'G','C','S','M'};
	uint32 VersionNumber = 1;

	// Counts
	uint32 IndexCount;
	uint32 VertexCount;

	// Offsets
	uint32 IndexArrayOffset;
	uint32 VertexArrayOffset;
	uint32 UVOffset;
	uint32 NormalOffset;
	uint32 ColorOffset;

	// Because the cache lines of the Gamecube are 32 bytes and it expects exact values, we just throw some padding
	// into this object to ensure its going to be 2 cache lines long!
	// The logic is either 32 or 64 bytes. If we have 36 (as we currently do without padding) then we need to pad out
	// those extra bytes so it matches those exact 2 lines.
	char padding[28];
};

class StaticMeshObject
{
	StaticMeshObject(const char* FilePath)
	{
		// Todo Gamecube File IO
		StaticMeshObject = GAMECUBE_StaticMesh();
	}

	GAMECUBE_StaticMesh StaticMeshObject;
	EObjectRenderType RenderType;

	GAMECUBE_StaticMesh AsGamecubeMesh()

	{ return reinterpret_cast<GAMECUBE_StaticMesh>(StaticMeshObject); }

	uint16* indices;
	float VertexArray;
	uint32 UVArray;
};

StaticMeshObject g_SMO("");

#endif //GCCUBEENGINE_STATICMESH_H