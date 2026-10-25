//
// Created by addic on 2026/10/18.
//

#include <cstring>
#include <cstdlib>

#include "Renderer.h"
#include "RenderObjects/StaticMesh.h"

static void CopyBuffer_INTERNAL(uint32 count GC_UNUSED)
{
	if (g_pRenderer)
		g_pRenderer->CopyBuffers(count);
}

void Renderer::RenderLoop()
{
	const int MeshObjectCount = 512;
	StaticMeshObject* MeshObjects[MeshObjectCount];
	for (int i = 0; i < MeshObjectCount; i++)
	{
		//srand(time(NULL));
		MeshObjects[i] = new StaticMeshObject("Data/Meshes/brick_climbable.gcsm");
		MeshObjects[i]->TranslateModel(Vec3f((float)(rand() % 500)-250.0f,(float)(rand() % 500)-250.0f,(float)(rand() % 500)-250.0f));
	}

	while (SYS_MainLoop())
	{
		printf("[Gamecube] LOOP");
		PAD_ScanPads();

		GX_SetViewport(0, 0, GXRMode->fbWidth, GXRMode->efbHeight, 0, 1);

		// Set the Z-Buffer Compare Mode
		GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
		// Toggles the Color Buffer Updates
		GX_SetColorUpdate(GX_TRUE);

		// Draw
		CameraView->AddPosition({0,0,1.5f});
		printf("[Gamecube] Add Position");

		for (int i = 0; i < MeshObjectCount; i++)
		{
			MeshObjects[i]->RotateModel(Vec3f(1,1,0));
			MeshObjects[i]->RenderModel(CameraView->GetViewNonConst());
		}

		// Copies the embedded framebuffer (EFB) to the external framebuffer (XFB) in Main Memory
		GX_CopyDisp(Framebuffer[CurrentFramebuffer], GX_TRUE);
		GX_DrawDone();

		VIDEO_SetNextFramebuffer(Framebuffer[CurrentFramebuffer]);
		// Flush the Shader Registers to the drivers video registers
		VIDEO_Flush();
		// Wait for the next Vertical Retrace!
		VIDEO_WaitVSync();

		// Flip our buffer
		CurrentFramebuffer ^= 1;
	}
}

bool Renderer::Initialize()
{
	VIDEO_Init();
	printf("[Gamecube] Video Init");

	GXRMode = VIDEO_GetPreferredMode(NULL);
	printf("[Gamecube] GXR Assigned");

	PAD_Init();
	printf("[Gamecube] PAD Init");

	for (int i = 0; i < MaxFramebuffers; i++)
	{
		Framebuffer[i] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(GXRMode));
		printf("[Gamecube] Initialized Framebuffer %d", i);
	}

	// Initialize Video
	VIDEO_Configure(GXRMode);
	printf("[Gamecube] Configure Video GXRMode");
	VIDEO_SetNextFramebuffer(GetCurrentFramebuffer());
	printf("[Gamecube] Set Next Framebuffer");
	// Note: This uses some actual bullshit to just call the Renderer's proper CopyBuffers.
	VIDEO_SetPostRetraceCallback(CopyBuffer_INTERNAL);
	printf("[Gamecube] Set Post Retrace Callback");
	VIDEO_SetBlack(FALSE);
	printf("[Gamecube] Set Black");
	VIDEO_Flush();
	printf("[Gamecube] Flush Video");

	// !!INITIALIZE FIFO!!
	GX_Buffer = MEM_K0_TO_K1(memalign(32, DEFAULT_FIFO_SIZE));
	printf("[Gamecube] Initialize GX Buffer FIFO");
	memset(GX_Buffer, 0, DEFAULT_FIFO_SIZE);
	printf("[Gamecube] memset Buffer");

	// Initialize GX
	GX_Init(GX_Buffer, DEFAULT_FIFO_SIZE);
	printf("[Gamecube] Init GX");
	GX_SetCopyClear(BackBufferColor, 0x00ffffff);
	printf("[Gamecube] Copy Clear Backbuffer");
	GX_SetViewport(0, 0, GXRMode->fbWidth, GXRMode->efbHeight, 0, 1);
	printf("[Gamecube] Set Viewport");
	GX_SetDispCopyYScale((float) GXRMode->xfbHeight / (float) GXRMode->efbHeight);
	printf("[Gamecube] Copy Y Scale");
	GX_SetScissor(0, 0, GXRMode->fbWidth, GXRMode->efbHeight);
	printf("[Gamecube] Set Scissor");
	GX_SetDispCopySrc(0, 0, GXRMode->fbWidth, GXRMode->efbHeight);
	printf("[Gamecube] Set Disp Copy Src");
	GX_SetDispCopyDst(GXRMode->fbWidth, GXRMode->xfbHeight);
	printf("[Gamecube] Set Disp Copy Dst");
	GX_SetCopyFilter(GXRMode->aa, GXRMode->sample_pattern, GX_TRUE, GXRMode->vfilter);
	printf("[Gamecube] Set Copy Filter");
	GX_SetFieldMode(GXRMode->field_rendering, ((GXRMode->viHeight == 2 * GXRMode->xfbHeight) ? GX_ENABLE : GX_DISABLE));
	printf("[Gamecube] Set Field Mode");
	GX_SetCullMode(GX_CULL_NONE);
	printf("[Gamecube] Set Cull Mode");
	GX_CopyDisp(GetCurrentFramebuffer(), GX_TRUE);
	printf("[Gamecube] Copy Disp");
	GX_SetDispCopyGamma(GX_GM_1_0);
	printf("[Gamecube] Set Disp Copy Gamma");

	GX_SetCullMode(GX_CULL_NONE);

	GX_SetNumChans(1);
	GX_SetNumTexGens(0);
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLOR0A0);
	GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);

	// Initialize Camera
	CameraView = new Camera({0.0f, 0.0f, 100.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, true);
	printf("[Gamecube] Initialize Camera");

	// Pre-assign the Vertex Attributes
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	printf("[Gamecube] Assign VA Pos");
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
	printf("[Gamecube] Assign VA NRM");
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
	printf("[Gamecube] Assign VA CLR0");
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
	printf("[Gamecube] Assign VA TEX0");

	return true;
}

FORCE_INLINE void Renderer::CopyBuffers(uint32 count GC_UNUSED)
{
}

Renderer *g_pRenderer{nullptr};
