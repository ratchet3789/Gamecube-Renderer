// Created by Coby Jennings on 2026/10/08.

#ifndef GCCUBEENGINE_DEBUG_H
#define GCCUBEENGINE_DEBUG_H

#define DEBUG_EMULATOR_LOG(x, y) printf(x, y)
#include <ogc/ipc.h>
#include <stdio.h>

#include "../GenericTypes.h"

bool IsRunningDolphin()
{
	int fd = IOS_Open("/dev/dolphin", 1);
	if (fd >= 0)
	{
		IOS_Close(fd);
		return true;
	}
	return false;
}

// Flip Endianness
uint32 FlipUINT(const uint32 in)
{
	return ((0xFF000000 & in) >> 24 | // Move 3 > 0
			(0x00FF0000 & in) >> 8  | // Move 2 > 1
			(0x0000FF00 & in) << 8  | // Move 1 < 2
			(0x000000FF & in) << 24); // Move 0 < 3
}

float FlipFLOAT(const float in)
{
	uint32 bits;
	float out = 0.f;
	std::memcpy(&bits, &in, sizeof(float));
	bits = FlipUINT(bits);
	std::memcpy(&out, &bits, sizeof(float));
	return out;
}

#endif //GCCUBEENGINE_DEBUG_H