#include "dma.h"
#include "util.h"
#include "bus.h"
#include "gpu.h"

struct {
	union {
		struct {
			u32 addr;
			u16 words;
			u16 blocks;
			struct {
				u32 dir:1;
				u32 step:1;
				u32 :6;
				u32 chop:1;
				u32 mode:2;
				u32 :5;
				u32 chopDMAWin:3;
				u32 :1;
				u32 chopCPUWin:3;
				u32 :1;
				u32 enable:1;
				u32 :3;
				u32 trigger:1;
				u32 :3;
			};
		};
		u32 r[4];
	} c[7];
	u32 ctrl;
	u32 intr;
	u32 pad2[2];
} dma;

void dmaSetup( void ) {
	dma.ctrl = 0x07654321;
	dma.intr = 0;
}

void dmaWrite( u32 addr, u32 n ) {
	//INFO( "DMA write 0x%X 0x%X", addr, n );
	
	if( addr < 0x1F8010F0 ) {
		int channel = ( addr - 0x1F801080 ) >> 4;
		int reg = ( addr & 0x0F ) / 4;
		
		// zero unused bits
		if( reg == 0 ) {
			n &= 0x001FFFFC;
			
		} else if( reg == 2 ) {
			n &= 0x71770703;
		}
		
		dma.c[channel].r[reg] = n;
		//ERR( "dma write, channel %d, reg %d, 0x%X", channel, reg, n );
		
	} else {
		switch( addr ) {
			case 0x1F8010F0:
				dma.ctrl = n;
				break;
				
			case 0x1F8010F4:
				//dma.intr = ( dma.intr & 0xFF800000 ) | ( n & 0x7F803F );
				dma.intr = n & 0x3F; // unused writable bits
				dma.intr |= n & 0xFF8000; // interrupt enable bits
				dma.intr = ( dma.intr & 0x7FFFFFFF ) | ( ( ( dma.intr >> 15 ) & 1 ) << 31 ); // copy bit 15 to bit 31
				dma.intr &= 0x80FFFFFF | ~( n & 0x7F000000 ); // clear interrupt flags
				break;
			
			default:
				ERR( "DMA write 0x%X : 0x%X", addr, n );
				break;
		}
	}
}

u32 dmaRead( u32 addr ) {
	//INFO( "DMA read 0x%X", addr );
	
	if( addr < 0x1F8010F0 ) {
		int channel = ( addr - 0x1F801080 ) >> 4;
		int reg = ( addr & 0x0F ) / 4;
		
		return dma.c[channel].r[reg];
		
	} else {
		switch( addr ) {
			case 0x1F8010F0:
				return dma.ctrl;
				break;
				
			case 0x1F8010F4:
				return dma.intr;
				break;
			
			default:
				ERR( "dma read 0x%X", addr );
				break;
		}
	}
}

void dmaUpdate( void ) {
	for( int i = 0; i < 7; i++ ) {
		if( dma.c[i].enable && ( dma.c[i].trigger || dma.c[i].mode ) ) {
			s32 inc = 4 - ( dma.c[i].step << 3 ); // 0: +4, 1: -4
			u32 addr = dma.c[i].addr;
			
			//INFO( "DMA channel %d mode %d triggered", i, dma.c[i].mode );
			
			switch( dma.c[i].mode ) {
				case 0: // block transfer
				case 1:; // sync to peripheral
					u32 words = 0;
					
					if( dma.c[i].mode == 0 ) {
						words = dma.c[i].words;
					} else {
						words = dma.c[i].words * dma.c[i].blocks;
					}
					
					if( dma.c[i].dir == 0 ) { // 0: to ram, 1: from ram
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
									
								case DMA_GPU:
									n = load( 0x1F801810, WORD );
									break;
								
								default:
									ERR( "Unkown DMA source %d", i );
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
									ERR( "Unkown DMA target %d", i );
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
					ERR( "dma channel %d enabled, mode %d", i, dma.c[i].mode );
					break;
			}
			
			dma.c[i].enable = FALSE;
			dma.c[i].trigger = FALSE;
		}
	}
}

/*
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
*/
