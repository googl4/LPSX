#include "dma.h"
#include "util.h"
#include "bus.h"
#include "gpu.h"

dmaRegisters_t dmaRegs;

void processDMA( void ) {
	for( int i = 0; i < 7; i++ ) {
		if( dmaRegs.c[i].enable && ( dmaRegs.c[i].trigger || dmaRegs.c[i].mode ) ) {
			s32 inc = 4 - ( dmaRegs.c[i].step << 3 ); // 0: +4, 1: -4
			u32 addr = dmaRegs.c[i].addr & 0x001FFFFC;
			
			switch( dmaRegs.c[i].mode ) {
				case 0: // block transfer
				case 1:; // sync to peripheral
					u32 words = 0;
					
					if( dmaRegs.c[i].mode == 0 ) {
						words = dmaRegs.c[i].words;
					} else {
						words = dmaRegs.c[i].words * dmaRegs.c[i].blocks;
					}
					
					if( dmaRegs.c[i].dir == 0 ) { // 0: to ram, 1: from ram
						while( words > 0 ) {
							u32 n = 0;
							
							switch( i ) {
								case DMA_OTC:
									if( words == 1 ) {
										n = 0xFFFFFFFF;
									} else {
										n = ( addr - 4 ) & 0x001FFFFF;
									}
									break;
								
								default:
									ERR( "Unkown DMA source" );
									break;
							}
							
							store( addr, WORD, n );
							
							addr += inc;
							words--;
						}
					} else {
						while( words > 0 ) {
							u32 n = load( addr, WORD );
							
							switch( i ) {
								case DMA_GPU:
									gp0( n );
									break;
								
								default:
									ERR( "Unkown DMA target" );
									break;
							}
							
							addr += inc;
							words--;
						}
					}
					break;
				
				case 2: // linked list
					while( TRUE ) {
						u32 header = load( addr, WORD );
						u32 words = header >> 24;
						//INFO( "GPU packet %d words", words );
						while( words > 0 ) {
							addr = ( addr + 4 ) & 0x001FFFFC;
							
							u32 cmd = load( addr, WORD );
							//INFO( "GPU command 0x%.8X", cmd );
							gp0( cmd );
							
							words--;
						}
						
						if( header & 0x00800000 ) {
							break;
						}
						
						addr = header & 0x001FFFFC;
					}
					break;
				
				default:
					ERR( "Unknown DMA mode, channel %d, mode %d", i, dmaRegs.c[i].mode );
					break;
			}
			
			dmaRegs.c[i].enable = FALSE;
			dmaRegs.c[i].trigger = FALSE;
		}
	}
}
