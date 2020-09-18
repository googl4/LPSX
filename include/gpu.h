#ifndef _GPU_H
#define _GPU_H

#include "types.h"

typedef struct {
	int pixels, hz, clocks;
} hmode_t;

static hmode_t hmodes[] = {
	{ 320, 50, 423 },
	{ 640, 50, 851 },
	{ 256, 50, 340 },
	{ 512, 50, 681 },
	{ 368, 50, 486 },
	{ 320, 60, 426 },
	{ 640, 60, 853 },
	{ 256, 60, 341 },
	{ 512, 60, 682 },
	{ 368, 60, 487 }
};

typedef union {
	u32 raw;
	struct {
		u32 texPageX:4;
		u32 texPageY:1;
		u32 transparency:2;
		u32 colours:2;
		u32 dither:1;
		u32 drawToDisplayArea:1;
		u32 forceWriteMask:1;
		u32 checkMask:1;
		u32 interlaceField:1;
		u32 reverse:1;
		u32 texDisable:1;
		u32 hres2:1;
		u32 hres:2;
		u32 vres:1;
		u32 pal:1;
		u32 outDepth:1;
		u32 interlace:1;
		u32 displayDisable:1;
		u32 irq:1;
		u32 DMARequest:1;
		u32 cmdReady:1;
		u32 vramReady:1;
		u32 DMAReady:1;
		u32 DMADir:2;
		u32 field:1;
	};
} gpuStatus_t;

typedef enum {
	OFF,
	FIFO,
	GP0,
	VRAM_READ
} DMADir_t;

typedef struct {
	u8 texFlipX;
	u8 texFlipY;
	u8 texWinXMask;
	u8 texWinYMask;
	u8 texWinX;
	u8 texWinY;
	u16 clipX1;
	u16 clipY1;
	u16 clipX2;
	u16 clipY2;
	s16 drawX;
	s16 drawY;
	u16 displayX;
	u16 displayY;
	u16 pixelStart;
	u16 pixelEnd;
	u16 lineStart;
	u16 lineEnd;
} gpuState_t;

typedef enum {
	NOP = 0x00,
	CLEAR_TEXCACHE = 0x01,
	FILL_RECT = 0x02,
	FLAT_QUAD = 0x28,
	FLAT_TEXTURED_QUAD = 0x2C,
	SHADED_TRI = 0x30,
	SHADED_QUAD = 0x38,
	DRAW_RECT = 0x64,
	TRANSFER_TO_VRAM = 0xA0,
	TRANSFER_FROM_VRAM = 0xC0,
	DRAW_MODE = 0xE1,
	SET_TEX_WIN = 0xE2,
	SET_CLIP1 = 0xE3,
	SET_CLIP2 = 0xE4,
	SET_DRAW = 0xE5,
	SET_MASK = 0xE6
} gp0Cmd_t;

typedef enum {
	RESET = 0x00,
	RESET_CMD_BUFFER = 0x01,
	IRQ_ACK = 0x02,
	DISPLAY_ENABLE = 0x03,
	DMA_DIR = 0x04,
	SET_DISPLAY = 0x05,
	HRANGE = 0x06,
	VRANGE = 0x07,
	DISPLAY_MODE = 0x08
} gp1Cmd_t;

extern gpuStatus_t gpuStatus;
extern gpuState_t gpuState;

void gp0( u32 cmd );
void gp1( u32 cmd );
u32 gpuRead( void );

void saveVram( void );

#endif
