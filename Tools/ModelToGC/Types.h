// Created by Coby Jennings on 2026/10/12.

#ifndef MODELTOGC_TYPES_H
#define MODELTOGC_TYPES_H

#if defined(WIN32)
#define alignas_gc(t) alignas(t)
#else
#define alignas_gc(t) __attribute__((aligned(t)))
#endif
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

// Note: This is taken from StaticMesh.h in GameCube Engine
struct StaticMesh
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

#endif //MODELTOGC_TYPES_H