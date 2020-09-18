#ifndef _DMA_H
#define _DMA_H

#include "types.h"

typedef enum {
	DMA_MDEC_IN,
	DMA_MDEC_OUT,
	DMA_GPU,
	DMA_CD,
	DMA_SPU,
	DMA_EXT,
	DMA_OTC
} dmaChannels;

void dmaSetup( void );
void dmaWrite( u32 addr, u32 n );
u32 dmaRead( u32 addr );
void dmaUpdate( void );

#endif
