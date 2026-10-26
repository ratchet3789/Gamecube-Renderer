//
// Created by Coby Jennings on 2026/10/08.
//

#ifndef GAMECUBEENGINE_GENERIC_TYPES_H
#define GAMECUBEENGINE_GENERIC_TYPES_H

// This is just to ensure type parity
typedef unsigned char uint8;
typedef signed char int8;
typedef unsigned short uint16;
typedef signed short int16;
typedef unsigned int uint32;
typedef signed int int32;

// The Examples (such as acube.c) use s16/u8 etc with ATTRIBUTE_ALIGN(32)
// So we TLDR'd them into Macro's that we can just define locally.
// Eg int8a_32(cube) = { 1, 2, 3, 4 };
// Unsigned 8 bit Array, 32 byte aligned
#define uint8a_32(Type)		uint8 		Type[] 	ATTRIBUTE_ALIGN(32)
// Signed 8 bit Array, 32 byte aligned
#define int8a_32(Type)		int8 		Type[] 	ATTRIBUTE_ALIGN(32)
// Unsigned 16 bit Array, 32 byte aligned
#define uint16a_32(Type)	uint16 		Type[] 	ATTRIBUTE_ALIGN(32)
// Signed 16 bit Array, 32 byte aligned
#define int16a_32(Type)		int16 		Type[] 	ATTRIBUTE_ALIGN(32)
// Unsigned 32 bit Array, 32 byte aligned
#define uint32a_32(Type)	uint32 		Type[] 	ATTRIBUTE_ALIGN(32)
// Signed 32 bit Array, 32 byte aligned
#define int32a_32(Type)		int32 		Type[] 	ATTRIBUTE_ALIGN(32)

#if defined(_WIN32)
#define FORCE_INLINE __forceinline
#else
#define FORCE_INLINE inline __attribute__((always_inline))
#endif

// This is inspired by Amazons (and its probably everywhere else) AZ_UNUSED
// because its so fucking odd to me that you need to define a var as unused
#define GC_UNUSED __attribute__((unused))

#define DEFAULT_FIFO_SIZE (256*1024)

#define alignas_gc(t) __attribute__((aligned(t)))

inline uint32 FNV1_Str(const char* Str)
{
	uint32 Hash = 2166136261;
	const uint32 FNVPrime = 16777619;

	while (*Str != '\0')
	{
		Hash ^= static_cast<uint8>(*Str);
		Hash *= FNVPrime;
		Str++;
	}
	return Hash;
}

template <typename T>
// Ensure floats are positive!
uint32 FNV1_Numeric(const T& Value)
{
	uint32 Hash = 2166136261;
	const uint32 FNVPrime = 16777619;

	const uint8* bytes = reinterpret_cast<const uint8*>(&Value);
	for (uint32 i = 0; i < sizeof(T); i++)
	{
		Hash ^= bytes[i];
		Hash *= FNVPrime;
	}
	return Hash;
}


#endif // GAMECUBEENGINE_GENERIC_TYPES_H