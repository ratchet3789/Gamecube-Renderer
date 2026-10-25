//
// Created by Coby Jennings on 2026/10/18.
//

#ifndef GCCUBEENGINE_RENDERER_H
#define GCCUBEENGINE_RENDERER_H
#include <gccore.h>
#include <malloc.h>

#include "../GenericTypes.h"
#include "Camera.h"

class alignas(32) Renderer
{
public:
	Renderer() = default;
	~Renderer() = default;
	// The loop... if I have to explain this you probably shouldn't be reading this
	void RenderLoop();
	// Sets up Gamecube's GX system
	bool Initialize();
	// Copy Display to the GPU and Flush to the Framebuffer
	FORCE_INLINE void CopyBuffers(uint32 count GC_UNUSED);

	// Getters & Setters
	void SetCamera(Camera* CameraIn) { CameraView = CameraIn; }

	// Not Const as is written to by CopyDisp
	FORCE_INLINE void* GetCurrentFramebuffer() { return Framebuffer[CurrentFramebuffer]; }

private:
	// Current targeted buffer
	uint32 CurrentFramebuffer=0;
	const uint32 MaxFramebuffers=2;
	// The pointers to each of our buffers
	void* Framebuffer[2]{nullptr, nullptr};
	// Our Graphics Model object
	GXRModeObj* GXRMode {nullptr};
	// Can we copy the buffers to the GPU?
	vu8 ReadyForCopy{false};
	// Our Backbuffer base color (Indigo Gamecube RGB)
	GXColor BackBufferColor{107,92,177};

	// FIFO Buffer, currently default size = 256 KiB buffer, initialized at the top of Initialize
	void* GX_Buffer;

	// We create a camera in our Initialize
	// Because we NEED one. However we __SHOULD__ expose Camera logic
	// to both the Editor (dev code) and the underlying system so we
	// can just swap cameras at runtime
	Camera* CameraView;
};

extern Renderer* g_pRenderer;

#endif //GCCUBEENGINE_RENDERER_H
