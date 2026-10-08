#include <gccore.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

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

#include "debug.h"
#include "input_manager.h"
#include "memory.h"
#include "observer.h"
#include "generic_types.h"

#define DEFAULT_FIFO_SIZE (256*1024) // See GX_FIFO_MINSIZE

// Vertex Data
int16a_32(cube) = {
	// x y z
	-30,  30, -30, 	// 0
	 30,  30, -30, 	// 1
	 30,  30,  30, 	// 2
	-30,  30,  30, 	// 3
	 30, -30, -30, 	// 4
	 30, -30,  30, 	// 5
	-30, -30,  30,  // 6
	-30, -30, -30,  // 7
};

uint8a_32(colors) = {
	// r, g, b, a
	100,  10, 100, 255, // 0 purple
	240,   0,   0, 255,	// 1 red
	255, 180,   0, 255,	// 2 orange
	255, 255,   0, 255, // 3 yellow
	 10, 120,  40, 255, // 4 green
	  0,  20, 100, 255  // 5 blue
};

// Double Buffering
static uint32 g_FramebufferCurrent = 0;
static uint32 g_FramebufferFirstFrame = 1;
static void* g_FrameBuffers[2] = {NULL,NULL};
static GXRModeObj* g_GXRMode = NULL;

GXColor g_BackBuffer = {107,92,177};

void* Initialize();

int main()
{
	printf("[GAMECUBE RENDERING] Initialized\n");
	Initialize();

	printf("[GAMECUBE RENDERING] SYS LOOP\n");
	while (SYS_MainLoop())
	{
		VIDEO_WaitVSync();
		PAD_ScanPads();
		int buttonsDown = PAD_ButtonsDown(0);

		if (buttonsDown & PAD_BUTTON_A)
		{
			printf("[GAMECUBE RENDERING] A Pressed\n");
			printf("Button A Pressed.\n");
		}

		if (buttonsDown & PAD_BUTTON_START)
		{
			printf("[GAMECUBE RENDERING] Start Pressed\n");
			exit(0);
		}
	}

	return 0;
}

void* InitializeFramebuffers()
{
	float yScale;
	uint32 framebufferHeight;

	// Init the Video, Setup Framebuffer and setup retrace callback to copy EFB to XFB
	VIDEO_Init();
	PAD_Init();

	g_GXRMode = VIDEO_GetPreferredMode(NULL);

	g_FramebufferCurrent = g_FramebufferFirstFrame = 0;

	void* g_pFIFO = nullptr;
	g_pFIFO = memalign(32, DEFAULT_FIFO_SIZE);
	memset(g_pFIFO, 0, DEFAULT_FIFO_SIZE);

	g_FrameBuffers[0] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(g_GXRMode));
	g_FrameBuffers[1] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(g_GXRMode));

	VIDEO_Configure(g_GXRMode);
	VIDEO_SetNextFramebuffer(g_FrameBuffers[g_FramebufferCurrent]);

	if (!g_FramebufferFirstFrame)
		VIDEO_SetBlack(FALSE);

	VIDEO_Flush();
	VIDEO_WaitVSync();

	if (g_GXRMode)
		VIDEO_WaitVSync();

	console_init(g_FrameBuffers[g_FramebufferCurrent], 20,20,g_GXRMode->fbWidth, g_GXRMode->xfbHeight, g_GXRMode->fbWidth * VI_DISPLAY_PIX_SZ);

	GX_Init(g_pFIFO, DEFAULT_FIFO_SIZE);

	GX_SetCopyClear(g_BackBuffer, 0x00ffffff);

	// Other GX Setup
	GX_SetViewport(0,0,g_GXRMode->fbWidth, g_GXRMode->efbHeight, 0,1);
	yScale = GX_GetYScaleFactor(g_GXRMode->efbHeight, g_GXRMode->xfbHeight);
	framebufferHeight = GX_SetDispCopyYScale(yScale);
	GX_SetScissor(0,0,g_GXRMode->fbWidth, g_GXRMode->efbHeight);
	GX_SetDispCopyDst(g_GXRMode->fbWidth, framebufferHeight);
	GX_SetCopyFilter(g_GXRMode->aa, g_GXRMode->sample_pattern, GX_TRUE, g_GXRMode->vfilter);
	GX_SetFieldMode(g_GXRMode->field_rendering, ((g_GXRMode->viHeight == 2 * g_GXRMode->xfbHeight) ? GX_ENABLE : GX_DISABLE));

	GX_SetCullMode(GX_CULL_BACK);
	GX_CopyDisp(g_FrameBuffers[g_FramebufferCurrent], GX_TRUE);


	VIDEO_Configure(g_GXRMode);
	VIDEO_SetNextFramebuffer(fb);
	VIDEO_SetBlack(false);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if (g_GXRMode->viTVMode & VI_NON_INTERLACE)
		VIDEO_WaitVSync();

	return fb;
}