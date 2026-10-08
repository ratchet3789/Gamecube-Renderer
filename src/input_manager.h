// Created by Coby Jennings on 2026/10/08.

#ifndef GCCUBEENGINE_INPUT_MANAGER_H
#define GCCUBEENGINE_INPUT_MANAGER_H

#include <ogc/pad.h>
#include "debug.h"
#include "observer.h"

typedef void(*CallbackFunc)(void* data);

DEFINE_OBSERVER(OBS_PAD_BUTTON_LEFT, 4, void*)
DEFINE_OBSERVER(OBS_PAD_BUTTON_RIGHT, 4, void*)
DEFINE_OBSERVER(OBS_PAD_BUTTON_DOWN, 4, void*)
DEFINE_OBSERVER(OBS_PAD_BUTTON_UP, 4, void*)
DEFINE_OBSERVER(OBS_PAD_TRIGGER_Z, 4, void*)
DEFINE_OBSERVER(OBS_PAD_TRIGGER_R, 4, void*)
DEFINE_OBSERVER(OBS_PAD_TRIGGER_L, 4, void*)
DEFINE_OBSERVER(OBS_PAD_BUTTON_A, 4, void*)
DEFINE_OBSERVER(OBS_PAD_BUTTON_B, 4, void*)
DEFINE_OBSERVER(OBS_PAD_BUTTON_X, 4, void*)
DEFINE_OBSERVER(OBS_PAD_BUTTON_Y, 4, void*)
DEFINE_OBSERVER(OBS_PAD_BUTTON_MENU, 4, void*)
DEFINE_OBSERVER(OBS_PAD_BUTTON_START, 4, void*)

class InputManager
{
public:
	InputManager() {}
	~InputManager() = default;

	void ProcessInputs()
	{}

	void LoadInputs()
	{
#if DEBUG
		if (IsRunningDolphin())
		{
			// Load from relative INI file
		}
#endif
		// Load from Stored InputConfigs in memory.
	}
};

#endif //GCCUBEENGINE_INPUT_MANAGER_H