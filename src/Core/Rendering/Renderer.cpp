//
// Created by addic on 2026/10/18.
//

#include "Renderer.h"

#include <cstring>

static void CopyBuffer_INTERNAL(uint32 count GC_UNUSED)
{
	if (g_pRenderer)
		g_pRenderer->CopyBuffers(count);
}

Renderer::Renderer()
{}

Renderer::~Renderer()
{}

void Renderer::RenderLoop()
{
	while (SYS_MainLoop())
	{

	}
}

void Renderer::GenerateBufferObjects()
{

}

bool Renderer::Initialize()
{
	VIDEO_Init();
	GXRMode = VIDEO_GetPreferredMode(NULL);

	PAD_Init();

	for (int i = 0; i < MaxFramebuffers; i++)
	{
		Framebuffer[i] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(GXRMode));
	}

	// Initialize Video
	VIDEO_Configure(GXRMode);
	VIDEO_SetNextFramebuffer(GetCurrentFramebuffer());
	// Note: This uses some actual bullshit to just call the Renderer's proper CopyBuffers.
	VIDEO_SetPostRetraceCallback(CopyBuffer_INTERNAL);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();

	// !!INITIALIZE FIFO!!
	GX_Buffer = MEM_K0_TO_K1(memalign(32, DEFAULT_FIFO_SIZE));
	memset(GX_Buffer, 0, DEFAULT_FIFO_SIZE);

	// Initialize GX
	GX_Init(GX_Buffer, DEFAULT_FIFO_SIZE);
	GX_SetCopyClear(BackBufferColor, 0x00ffffff);
	GX_SetViewport(0, 0, GXRMode->fbWidth, GXRMode->efbHeight, 0, 1);
	GX_SetDispCopyYScale((float) GXRMode->xfbHeight / (float) GXRMode->efbHeight);
	GX_SetScissor(0, 0, GXRMode->fbWidth, GXRMode->efbHeight);
	GX_SetDispCopySrc(0, 0, GXRMode->fbWidth, GXRMode->efbHeight);
	GX_SetDispCopyDst(GXRMode->fbWidth, GXRMode->xfbHeight);
	GX_SetCopyFilter(GXRMode->aa, GXRMode->sample_pattern, GX_TRUE, GXRMode->vfilter);
	GX_SetFieldMode(GXRMode->field_rendering,((GXRMode->viHeight == 2 * GXRMode->xfbHeight) ? GX_ENABLE : GX_DISABLE));
	GX_SetCullMode(GX_CULL_NONE);
	GX_CopyDisp(GetCurrentFramebuffer(), GX_TRUE);
	GX_SetDispCopyGamma(GX_GM_1_0);

	// Initialize Camera
	CameraView = new Camera({0.0f, 0.0f, 0.0f}, {0.0f,1.0f,0.0f}, {0.0f, 0.0f, -1.0f}, true);

	// Pre-assign the Vertex Attributes
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_S16, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_S8, 7);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_U16, 8);

	return true;
}

FORCE_INLINE void Renderer::UpdateScreen()
{
	//guMtxIdentity();
	//guMtxTransApply();
	//guMtxConcat();

}

FORCE_INLINE void Renderer::CopyBuffers(uint32 count GC_UNUSED)
{

}

Renderer* g_pRenderer{nullptr};
