// Created by Coby Jennings on 2026/10/08.

#ifndef GCCUBEENGINE_DEBUG_H
#define GCCUBEENGINE_DEBUG_H

#define DEBUG_EMULATOR_LOG(x, y) printf(x, y)
#include <ogc/ipc.h>
#include <stdio.h>

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

#endif //GCCUBEENGINE_DEBUG_H