#ifndef _DMA_H
#define _DMA_H

#include "types.h"

typedef struct {
	struct {
		u32 addr;
		u16 words;
		u16 blocks;
		struct {
			u32 dir:1;
			u32 step:1;
			u32 pad0:6;
			u32 chop:1;
			u32 mode:2;
			u32 pad1:5;
			u32 chopDMAWin:3;
			u32 pad2:1;
			u32 chhopCPUWin:3;
			u32 pad3:1;
			u32 enable:1;
			u32 pad4:3;
			u32 trigger:1;
			u32 pad5:3;
		};
		u32 pad6;
	} c[7];
	u32 ctrl;
	u32 intr;
	u32 pad7[2];
} dmaRegisters_t;

typedef enum {
	DMA_MDEC_IN,
	DMA_MDEC_OUT,
	DMA_GPU,
	DMA_CD,
	DMA_SPU,
	DMA_EXT,
	DMA_OTC
} dmaChannels;

extern dmaRegisters_t dmaRegs;

void processDMA( void );

#endif
