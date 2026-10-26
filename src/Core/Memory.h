// Created by Coby Jennings on 2026/10/08.

#ifndef GAMECUBEENGINE_MEMORY_H
#define GAMECUBEENGINE_MEMORY_H

#include <assert.h>
#include <cstdint>

#include "Debugging/Debugging.h"

// PERFORMANCE BUDGETS
#define ONE_MB_BYTES 1048576

// Single draw call budget (before thrashing)
#define GX_TEXTURE_CACHE_LIMIT ONE_MB_BYTES

// We store Audio Memory in ARAM so we don't need an Arena for Audio :D
// >>>>> MEM1 (System Memory) <<<<<
// Memory you cannot touch (SDK Reserved)
#define MEM1_PREALLOCATED_SYSTEM_SDK_RESERVED_MEMORY	(ONE_MB_BYTES * 3)

// These are tightly managed to be at most 20MB of the 24MB of System Memory across the Gamecube
#define MEM1_TEXTURE_BUDGET								(ONE_MB_BYTES * 8) // Textures (Oops! All Textures)
#define MEM1_MESH_BUDGET 								(ONE_MB_BYTES * 3) // Mesh and Display List (GX_Begin/End Blobs)
#define MEM1_ANIMATION_BUDGET 							(ONE_MB_BYTES * 3) // Quantized 16-bit float Rotation/Translations
#define MEM1_COLLISION_BUDGET 							(ONE_MB_BYTES * 1) // BVH/Octrees & AABB/Sphere arrays
#define MEM1_ENTITIES_BUDGET 							(ONE_MB_BYTES * 3) // ECS Component Arrays, Particle Pools, Transform Matrices
#define MEM1_UI_FONT_BUDGET 							(ONE_MB_BYTES * 1) // UI Textures, Fonts
#define MEM1_FRAME_SCRATCHPAD_BUDGET 					(ONE_MB_BYTES * 1) // Matrix Stacks, Physics Results. Resets every frame @ O(1)

class MemoryArena
{
public:
	MemoryArena(size_t MaxCapacity) : m_Capacity(MaxCapacity)
	{
#if DEBUG
		if (IsRunningDolphin()) { DEBUG_EMULATOR_LOG("Allocated new arena with %d memory.\n", MaxCapacity); }
#endif
	}

	void* Allocate(size_t Size, size_t Alignment = 32)
	{
		size_t currAddr = reinterpret_cast<size_t>(m_Buffer + m_Offset);
		/*
		 * Pow 2 has a distinct bit pattern (0010000), alignment -1 is (0001111)
		 * curAddr + alignment-1 adds 31 to the current address.
		 * If its aligned adding the value will keep it within range of the 32 byte block, otherwise it bumps it to the next one
		 * ~ flips all bits (0001111 to 1110000) and & masks remainders, snapping the value to the nearest multiple of 32
		 * eg, assume 36 (00100100).
		 * currAddress (36 & 0010 0100) + (32-1) (67 & 0100 0011) ~31 (-32 or 1110 0000).
		 * 67 & ~31 (0100 0011 & 1110 0000) is 64 (0100 0000
		*/
		size_t alignedAddr = (currAddr + (Alignment-1)) & ~(Alignment-1);
		size_t newOffset = (alignedAddr - reinterpret_cast<size_t>(m_Buffer)) + Size;

		if (newOffset > m_Capacity)
		{
			assert(false); // Tried to allocate memory outside of arena
			return nullptr;
		}

		m_Offset = newOffset;
		return reinterpret_cast<void*>(alignedAddr);
	}

	void Clear()
	{
		m_Offset = 0;
	}

	size_t GetCapacity() { return m_Capacity; }
	size_t GetUsedMemory() { return m_Offset; }

private:
	uint8_t* m_Buffer;
	size_t m_Capacity;
	size_t m_Offset;
};

// Todo: Specific per-system allocators to ensure no fragmentation
// Todo: Move to specific subfolders. Eg TextureManager.h
MemoryArena *g_TextureArena = nullptr;
MemoryArena *GetTextureArena() { return (g_TextureArena == nullptr ? new MemoryArena(MEM1_TEXTURE_BUDGET) : g_TextureArena);}

MemoryArena *g_MeshArena = nullptr;
MemoryArena *GetMeshArena() { return (g_MeshArena == nullptr ? new MemoryArena(MEM1_MESH_BUDGET) : g_MeshArena);}

MemoryArena *g_AnimationArena = nullptr;
MemoryArena *GetAnimationArena() { return (g_AnimationArena == nullptr ? new MemoryArena(MEM1_ANIMATION_BUDGET) : g_AnimationArena);}

MemoryArena *g_CollisionArena = nullptr;
MemoryArena *GetCollisionArena() { return (g_CollisionArena == nullptr ? new MemoryArena(MEM1_COLLISION_BUDGET) : g_CollisionArena);}

MemoryArena *g_EntityArena = nullptr;
MemoryArena *GetEntityArena() { return (g_EntityArena == nullptr ? new MemoryArena(MEM1_ENTITIES_BUDGET) : g_EntityArena);}

MemoryArena *g_UIArena = nullptr;
MemoryArena *GetUIArena() { return (g_UIArena == nullptr ? new MemoryArena(MEM1_UI_FONT_BUDGET) : g_UIArena);}

MemoryArena *g_FrameScratchpadArena = nullptr;
MemoryArena *GetFrameScratchpadArena() { return (g_FrameScratchpadArena == nullptr ? new MemoryArena(MEM1_FRAME_SCRATCHPAD_BUDGET) : g_FrameScratchpadArena);}

// TEMPORARY: The entire RAM space of the Gamecube for Gamespace
MemoryArena *g_GeneralArena = nullptr;
MemoryArena *GetGeneralArena() { return (g_GeneralArena == nullptr ? new MemoryArena(ONE_MB_BYTES*21) : g_GeneralArena); }

void* operator new(size_t size)
{
	return g_GeneralArena->Allocate(size);
}

void operator delete(void* ptr) noexcept
{
	// No!
	// The Arena reclaims memory at once on Arena::Clear()
	assert(false);
}

#endif // GAMECUBEENGINE_MEMORY_H