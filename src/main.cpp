#include <gccore.h>
#include <malloc.h>
#include <cmath>

/*
 * Gamecube Hardware:
 * The GameCube is...
 * 1. Big Endian
 * 2. PowerPC 750CXe (G3) or the IBM Gekko, RISC based CPU.
 * 2.1. It has a clock rate of 486 MHz
 * 2.2. It has 1 Core
 * 2.3. It has a 32kb L1 and a 256kb L2 cache
 * 2.4. It has a Floating Point Unit
 * 2.5. It has a Branch Processing Unit (Handles Branch Prediction))
 * 2.6. It has a System Register Unit (Executes instructions that manage the CPUs internal state)
 * 2.7. It has a Load/Store Unit (Moves data between physical memory/Caches and the CPU registers)
 * 2.8. It has Two Integer Units
 * 2.8.1 The IU1 executes all integer instructions
 * 2.8.2 The IU2 executes all except Multiply and Divide
 * 2.9. It has a 32-bit address Bus (Max number of memory addresses the CPU can point to. 2^32 distint locations or 4GB of total addressable space.
 * 2.10. It has a 64-bit Data Bus (How mucih actual data can be transferred in a single clock cycle. Moves 8 bytes of data per-transfer. Because Gekko has 32-byte cache lines, it takes 4 transfers (Memory Bus Clock Cycles) to fill a single line.)
 * 2.11. Has 32-bit Cache Lines
 *
 * The Gamecube has...
 * 1. 24MB System RAM
 * 2. 3MB Video RAM
 * 	2.1. 2MB Framebuffer
 * 	2.2. 1MB Texture Cache
 * 3. 16MB I/O Buffer RAM
 * 4. An ATI Flipper GPU @ 162MHz
 * 5. An IBM Gekko CPU @ 485 MHz
 * 6. A Clock Frequency of 162 MHz
 *
 * Texture Read Bandwidth: 10.4GB/s (MAX)
 * Pixel Depth: 24-bit Color, 24-bit Z-Buffer
 * A-Memory 24mb
 *
 * Src: https://datasheets.chipdb.org/IBM/PowerPC/Gekko/gekko_user_manual.pdf
 * Remember, Fuck AI.
 */

// TESTING TO MAKE SURE THINGS AREN'T BROKEN
#include <cstring>

#include "Core/Debugging/Debugging.h"
#include "Core/InputManager.h"
#include "Core/Memory.h"
#include "Core/Algorithms/Observer.h"
#include "Core/GenericTypes.h"

// Vertex Data
int16a_32(vertices) = {
	// x y z
	-15, 15, 0,		// Top Left
	-15, -15, 0,	// Bottom Left
	15, -15, 0,		// Bottom Right
	15, 15, 0		// Top Right
};

int8a_32(indices) = {
	0,1,2, // Tri 1 (Left)
	1,2,3 // Tri 2 (Right)
};

uint8a_32(colors) = {
	// r, g, b, a
	255, 0, 0, 255,
	0, 255, 0, 255,
	0, 0, 255, 255,
	255, 255, 0, 255
};

// Double Buffering
static uint32 g_FramebufferCurrent = 0;
static uint32 g_FramebufferFirstFrame = 1;
static void *g_FrameBuffer{NULL};
static GXRModeObj *g_GXRMode = NULL;
vu8 ReadyForCopy{false};
GXColor g_BackBuffer = {107, 92, 177};
Mtx ModelView;
guVector camOffset;
float lookSensitivity = 0.05f;


// Preprocessor Definitions
void UpdateScreen(Mtx viewMatrix);
static void CopyBuffers(uint32 unused);
PADStatus Controller[4];

int main()
{
	Mtx View;
	Mtx44 Projection;
	GXColor BackgroundColor = {75, 0, 130, 255};
	void *FIFOBuffer = NULL;

	VIDEO_Init();
	g_GXRMode = VIDEO_GetPreferredMode(NULL);

	printf("[GAMECUBE] Video Initialized\n");

	PAD_Init();
	g_FrameBuffer = MEM_K0_TO_K1(SYS_AllocateFramebuffer(g_GXRMode));

	printf("[GAMECUBE] Gamepad Initialized\n");

	VIDEO_Configure(g_GXRMode);
	VIDEO_SetNextFramebuffer(g_FrameBuffer);
	VIDEO_SetPostRetraceCallback(CopyBuffers);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();

	printf("[GAMECUBE] Video Flush\n");

	FIFOBuffer = MEM_K0_TO_K1(memalign(32, DEFAULT_FIFO_SIZE));
	memset(FIFOBuffer, 0, DEFAULT_FIFO_SIZE);

	printf("[GAMECUBE] FIFO Allocated\n");

	GX_Init(FIFOBuffer, DEFAULT_FIFO_SIZE);
	GX_SetCopyClear(BackgroundColor, 0x00ffffff);
	GX_SetViewport(0, 0, g_GXRMode->fbWidth, g_GXRMode->efbHeight, 0, 1);
	GX_SetDispCopyYScale((float) g_GXRMode->xfbHeight / (float) g_GXRMode->efbHeight);
	GX_SetScissor(0, 0, g_GXRMode->fbWidth, g_GXRMode->efbHeight);
	GX_SetDispCopySrc(0, 0, g_GXRMode->fbWidth, g_GXRMode->efbHeight);
	GX_SetDispCopyDst(g_GXRMode->fbWidth, g_GXRMode->xfbHeight);
	GX_SetCopyFilter(g_GXRMode->aa, g_GXRMode->sample_pattern, GX_TRUE, g_GXRMode->vfilter);
	GX_SetFieldMode(g_GXRMode->field_rendering,((g_GXRMode->viHeight == 2 * g_GXRMode->xfbHeight) ? GX_ENABLE : GX_DISABLE));

	printf("[GAMECUBE] GX Initialized\n");

	GX_SetCullMode(GX_CULL_NONE);
	GX_CopyDisp(g_FrameBuffer, GX_TRUE);
	GX_SetDispCopyGamma(GX_GM_1_0);

	printf("[GAMECUBE] Culling Setup\n");

	guVector Camera = {0.0f, 0.0f, 0.0f};
	guVector Up = {0.0f, 1.0f, 0.0f};
	guVector Look = {0.0f, 0.0f, -1.0f};

	printf("[GAMECUBE] Directions Initialized\n");

	guPerspective(Projection, 60, 1.33f, 10.0f, 300.0f);
	GX_LoadProjectionMtx(Projection, GX_PERSPECTIVE);
	printf("[GAMECUBE] Projection Matrix Initialized\n");

	GX_ClearVtxDesc();
	printf("[GAMECUBE] GX_ClearVtxDesc\n");
	GX_SetVtxDesc(GX_VA_POS, GX_INDEX8);
	GX_SetVtxDesc(GX_VA_CLR0, GX_INDEX8);
	printf("[GAMECUBE] GX_SetVtxDesc\n");
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_S16, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
	printf("[GAMECUBE] GX_SetVtxAttrFmt\n");
	GX_SetArray(GX_VA_POS, vertices, 3 * sizeof(int16));
	GX_SetArray(GX_VA_CLR0, colors, 4 * sizeof(uint8));
	printf("[GAMECUBE] GX_SetArray\n");
	GX_SetNumChans(1);
	printf("[GAMECUBE] GX_SetNumChans\n");
	GX_SetNumTexGens(0);
	printf("[GAMECUBE] GX_SetNumTexGens\n");
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLOR0A0);
	printf("[GAMECUBE] GX_SetTevOrder\n");
	GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
	printf("[GAMECUBE] GX_SetTevOp\n");

	printf("[GAMECUBE] Pre Loop\n");

	while (SYS_MainLoop())
	{
		guLookAt(View, &Camera, &Up, &Look);
		GX_SetViewport(0, 0, g_GXRMode->fbWidth, g_GXRMode->efbHeight, 0, 1);
		GX_InvVtxCache();
		GX_InvalidateTexAll();
		UpdateScreen(View);
		PAD_Read(Controller);
		if (Controller[0].button & PAD_BUTTON_START)
		{
			void(*reload)() = (void(*)())0x80001800;
			reload();
		}

		camOffset.x -= Controller[0].substickX * lookSensitivity;
		//	camOffset.y -= Controller[0].substickY * lookSensitivity;
	}
	return 0;
}

void UpdateScreen(Mtx viewMatrix)
{
	guMtxIdentity(ModelView);
	guMtxTransApply(ModelView, ModelView, camOffset.x, camOffset.y, -50.0f);
	guMtxConcat(viewMatrix, ModelView, ModelView);

	GX_LoadPosMtxImm(ModelView, GX_PNMTX0);

	GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
		GX_Position1x8(0);
		GX_Color1x8(0);
		GX_Position1x8(1);
		GX_Color1x8(1);
		GX_Position1x8(2);
		GX_Color1x8(2);
		GX_Position1x8(3);
		GX_Color1x8(3);
	GX_End();

	GX_DrawDone();
	ReadyForCopy = GX_TRUE;

	VIDEO_WaitVSync();
	return; // If this fixes it, what?
}

static void CopyBuffers(uint32 count __attribute__((unused)))
{
	if (ReadyForCopy == GX_TRUE)
	{
		GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
		GX_SetColorUpdate(GX_TRUE);
		GX_CopyDisp(g_FrameBuffer, GX_TRUE);
		GX_Flush();
		ReadyForCopy = GX_FALSE;
	}
}