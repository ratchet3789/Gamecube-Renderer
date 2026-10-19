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

// Data extracted directly from the GCCubeEngine_tar.s
extern "C"
{
	extern const u8 GCCubeEngine_tar[];
	extern const u32 GCCubeEngine_tar_size;
}

class FileSystem
{
private:
	// Tarball fules are a fixed 512 byte block that precede every single file or directory entry inside the .tar archive.
	// Octal is also Base-8
	struct alignas_gc(32) Header
	{
		char FileName[100]; 			// File Path and Name
		char FileMode[8];				// File Mode (Octal)
		char OwnerNumID[8];				// Owners Numeric User ID (Octal)
		char GroupNumID[8];				// Group's Numeric User ID (Octal)
		char FileSize[12];				// File Size in Bytes (Octal)
		char LastModifiedTime[12];		// Last Modification Time in Numeric Unix
		char Checksum[8];				// Checksum for Header Record(Octal)
		char TypeFlag;					// Type Flag
		char LinkedFileName[100];		// Name of Linked File
		char UStarMagicIndicator[6];	// UStar Indicator "ustar"
		char UStarVersion[2];			// UStar Version
		char OwnerUserName[32];			// Owner User Name
		char OwnerGroupName[32];		// Owner Group Name
		char DeviceNumMajor[8];			// Device Major Number
		char DeviceNumMinor[8];			// Device Minor Number
		char FilenamePrefix[155];		// Filename Prefx (GNU/POSIX TAR)
		char Padding[12];				// Padding
	};

	// Helper to convert Octal ASCII string from header to integer size
	static u32 OctalToUint(const char *str, size_t max_len)
	{
		u32 result = 0;
		for (size_t i = 0; i < max_len && str[i] >= '0' && str[i] <= '7'; ++i)
		{
			result = (result << 3) + (str[i] - '0');
		}
		return result;
	}

public:
	struct FileEntry
	{
		const u8* Data;
		u32 Size;
	};

	static bool GetFile(const char* FileName, FileEntry& OutFile)
	{
		const u8* Cur = DataTar;
		const u8* End = DataTar + DataTarSize;

		while (current + sizeof(Header) < End)
		{
			const Header* Hdr = reinterpret_cast<const Header*>(Cur);

			// End of Archive
			if (Hdr->FileName[0] == '\0')
				break;

			u32 FileSize = OctalToUint(Hdr->FileSize, sizeof(Hdr->FileSize));

			// Reconstruct full path if POSIX Unix Standard TAR (USTAR) is used
			char FullPath[256]={0};
			if (Hdr->FilenamePrefix[0] != '\0')
			{
				strncpy(FullPath, Hdr->FilenamePrefix, sizeof(Hdr->FilenamePrefix));
				strncpy(FullPath, "/");
			}

			strncat(FullPath, Hdr->FileName, sizeof(Hdr->FileName));

			// Check if the Filename matches the query
			if (strcmp(FullPath, FileName) == 0)
			{
				// Data sits directly after the header block
				OutFile.Data = Cur + 512;
				OutFile.Size = FileSize;
				return true;
			}

			// Move Ptr 512 Bytes + 512 bytes
			u32 AlignedSize = (FileSize + 511) & ~511;
			Cur += 512 + AlignedSize;
		}

		return false;
	}
};

#endif //GCCUBEENGINE_FILESYSTEM_H
