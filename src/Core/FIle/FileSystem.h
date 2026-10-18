//
// Created by Coby Jennings on 2026/10/19.
//

#ifndef GCCUBEENGINE_FILESYSTEM_H
#define GCCUBEENGINE_FILESYSTEM_H

#include <cstdio>
#include <gccore.h>
#include <fat.h>
#include <malloc.h>

#include "../GenericTypes.h"

#define FILE_LENGTH(file, out) \
	fseek(file, 0, SEEK_END); \
	FileSize = ftell(file); \
	fseek(file, 0, SEEK_SET);

class FileSystem
{
public:
	static bool Initialize()
	{
		if (!isInitialized)
		{
			if (!fatInitDefault())
			{
				printf("[Gamecube][FileSystem] Failed to Initialize Filesystem!");
				return false;
			}
			isInitialized = true;
		}
		return isInitialized;
	}

	static bool isInitialized{false};

	static void* LoadRawBianry(const char* FilePath, uint32& iLength)
	{
		FILE* File = fopen(FilePath);
		if (!File)
			return nullptr;

		size_t FileSize;
		FILE_LENGTH(File, FileLen);

		// Calculate aligned size
		iLength = (FileSize+31) & ~31;

		// Align to guarantee 32 byte boundary
		void* buffer = memalign(32, iLength);
		if (!buffer)
		{
			fclose(File);
			return nullptr;
		}

		// Set our new memory to empty
		memset(buffer, 0, iLength);

		// Read out file into memory
		fread(buffer, 0, FileSize, File);
		fclose(File);

		DCStoreRange(buffer, iLength);
		return buffer;
	}
};

#endif //GCCUBEENGINE_FILESYSTEM_H