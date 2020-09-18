#include "bus.h"
#include "util.h"
#include "mips.h"
#include "dma.h"
#include "gpu.h"
#include "cdrom.h"
#include "joypad.h"

u32 addr_mask = 0x1FFFFFFF;

u32 ram_start   = 0x00000000;
u32 ram_end     = 0x00200000;
u32 exp1_start  = 0x1F000000;
u32 exp1_end    = 0x1F800000;
u32 cache_start = 0x1F800000;
u32 cache_end   = 0x1F800400;
u32 io_start    = 0x1F801000;
u32 io_end      = 0x1F802000;
u32 exp2_start  = 0x1F802000;
u32 exp2_end    = 0x1F805000;
u32 bios_start  = 0x1FC00000;
u32 bios_end    = 0x1FC80000;

u32 kseg2_io  = 0xFFFE0000;

u8 cache[1024];
u8 mem[2048 * 1024];
u8* bios;

cacheLine icache[CACHE_LINES];

extern int logInstrs;

u32 read( u8* ptr, accessType type ) {
	switch( type ) {
		case BYTE:
			return *ptr;
		
		case HALFWORD:
			return *(u16*)ptr;
		
		case WORD:
		case INSTRUCTION:
			return *(u32*)ptr;
			
		case BYTE_SIGNED:
			return (s32)(*(s8*)ptr);
			
		case HALFWORD_SIGNED:
			return (s32)(*(s16*)ptr);
	}
}

u32 write( u8* ptr, accessType type, u32 n ) {
	switch( type ) {
		case BYTE:
		case BYTE_SIGNED:
			*ptr = n;
			break;
		
		case HALFWORD:
		case HALFWORD_SIGNED:
			*(u16*)ptr = n;
			break;
		
		case WORD:
		case INSTRUCTION:
			*(u32*)ptr = n;
			break;
	}
}

u32 load( u32 addr, accessType type ) {
	if( addr >= kseg2_io ) {
		INFO( "KSEG2 cache control read" );
		return 0;
	}
	
	if( type == WORD        && addr & 0x02 ||
		type == INSTRUCTION && addr & 0x02 ||
	    type == HALFWORD    && addr & 0x01
	) {
		except( 0x04 );
	}
	
	if( type == INSTRUCTION && ( addr >> 29 != 0x0A ) ) {
		cacheAddress a;
		a.raw = addr;
		
		if( icache[a.cache_line].tag == a.tag && !icache[a.cache_line].invalidated ) {
			return icache[a.cache_line].instructions[a.index];
			
		} else {
			icache[a.cache_line].invalidated = FALSE;
			icache[a.cache_line].tag = a.tag;
			u32 p = addr & 0xFFFFFFF0;
			//printf( "icache load at 0x%.8X\n", p );
			icache[a.cache_line].instructions[0] = load( p, WORD );
			icache[a.cache_line].instructions[1] = load( p + 4, WORD );
			icache[a.cache_line].instructions[2] = load( p + 8, WORD );
			icache[a.cache_line].instructions[3] = load( p + 12, WORD );
			return icache[a.cache_line].instructions[a.index];
		}
	}
	
	addr &= addr_mask;
	
	if( addr >= cache_start && addr < cache_end ) {
		return read( cache + ( addr - cache_start ), type );
		
	} else if( addr < ram_end * 4 ) {
		return read( mem + addr % ram_end, type );
		
	} else if( addr >= bios_start && addr < bios_end ) {
		return read( bios + ( addr - bios_start ), type );
		
	} else if( addr >= io_start && addr < io_end ) {
		if( addr >= 0x1F801000 && addr < 0x1F801040 ) {
			INFO( "memory control read" );
			return 0;
			
		} else if( addr >= 0x1F801050 && addr < 0x1F801060 ) {
			INFO( "serial port read" );
			return 0;
			
		} else if( addr >= 0x1F801060 && addr < 0x1F801064 ) {
			INFO( "ram size read" );
			return 0;
			
		} else if( addr >= 0x1F801C00 && addr < 0x1F802000 ) {
			//INFO( "spu control read" );
			return 0;
			
		} else if( addr == 0x1F801074 ) {
			//INFO( "interrupt mask read" );
			//return 0;
			return imask;
			
		} else if( addr == 0x1F801070 ) {
			//INFO( "interrupt status read" );
			//return 0;
			return istatus;
			
		} else if( addr >= 0x1F801100 && addr < 0x1F80112C ) {
			//INFO( "timer register read" );
			return 0;
			
		} else if( addr >= 0x1F801080 && addr < 0x1F801100 ) {
			return dmaRead( addr );
			
		} else if( addr >= 0x1F801800 && addr < 0x1F801810 ) {
			INFO( "CDROM read, addr: 0x%.8X", addr );
			//return 0;
			//return 0x08;
			//return 0xFF;
			return cdRead( addr );
		
		} else if( addr == 0x1F801810 ) {
			//INFO( "GPUREAD read" );
			return gpuRead();
			
		} else if( addr == 0x1F801814 ) {
			//INFO( "GPUSTAT read 0x%X", gpuStatus.raw );
			//return 0x1C000000;
			return gpuStatus.raw;
			
		} else {
			ERR( "Unkown IO read, 0x%.8X\n", addr );
		}
		
	} else if( addr >= exp1_start && addr < exp1_end ) {
		INFO( "expansion 1 read" );
		return 0xFFFFFFFF;
		
	} else if( addr >= exp2_start && addr < exp2_end ) {
		INFO( "expansion 2 read" );
		return 0;
	
	} else {
		ERR( "Unkown memory read, 0x%.8X\n", addr );
	}
}

void store( u32 addr, accessType type, u32 n ) {
	if( addr >= kseg2_io ) {
		INFO( "KSEG2 cache control write 0x%.8X", n );
		return;
	}
	
	if( type == WORD && addr & 0x02 ||
	    type == HALFWORD && addr & 0x01
	) {
		except( 0x05 );
	}
	
	cacheAddress a;
	a.raw = addr;
	
	if( c0regs.sr.isc ) {
		icache[a.cache_line].instructions[a.index] = n;
		icache[a.cache_line].invalidated = TRUE; // TODO: emulate tag/inv/lock cache modes
		                                         //       L64360 datasheet, page 191
		return;
	} else if( icache[a.cache_line].tag == a.tag ) {
		icache[a.cache_line].invalidated = TRUE; // might need per word invalidation
	}
	
	addr &= addr_mask;
	
	if( addr >= cache_start && addr < cache_end ) {
		write( cache + ( addr - cache_start ), type, n );
		
	} else if( addr < ram_end ) {
		write( mem + addr, type, n );
		
	} else if( addr >= bios_start && addr < bios_end ) {
		ERR( "BIOS write\n" );
		
	} else if( addr >= io_start && addr < io_end ) {
		
		if( addr >= 0x1F801000 && addr < 0x1F801040 ) {
			INFO( "memory control write" );
			return;
			
		} else if( addr >= 0x1F801050 && addr < 0x1F801060 ) {
			INFO( "serial port write" );
			return;
			
		} else if( addr >= 0x1F801060 && addr < 0x1F801064 ) {
			INFO( "ram size write" );
			return;
			
		} else if( addr >= 0x1F801C00 && addr < 0x1F802000 ) {
			//INFO( "spu control write" );
			return;
			
		} else if( addr == 0x1F801074 ) {
			//INFO( "interrupt mask write 0x%.4X", n );
			imask = n;
			return;
			
		} else if( addr == 0x1F801070 ) {
			//INFO( "interrupt status write 0x%X", n );
			istatus &= n;
			return;
			
		} else if( addr >= 0x1F801100 && addr < 0x1F80112C ) {
			INFO( "timer register write" );
			return;
			
		} else if( addr >= 0x1F801080 && addr < 0x1F801100 ) {
			dmaWrite( addr, n );
			return;
			
		} else if( addr >= 0x1F801800 && addr < 0x1F801810 ) {
			INFO( "CDROM write, addr: 0x%.8X value: 0x%.8X", addr, n );
			cdWrite( addr, n );
			return;
			
		} else if( addr == 0x1F801810 ) {
			gp0( n );
			return;
			
		} else if( addr == 0x1F801814 ) {
			gp1( n );
			return;
			
		} else if( addr >= 0x1F801040 && addr < 0x1F801050 ) {
			joyPadWrite( addr, n );
			return;
			
		} else {
			ERR( "Unkown IO write, 0x%.8X\n", addr );
		}
		
	} else if( addr >= exp1_start && addr < exp1_end ) {
		INFO( "expansion 1 write" );
		return;
		
	} else if( addr >= exp2_start && addr < exp2_end ) {
		INFO( "expansion 2 write" );
		if( addr == 0x1F802041 ) {
			INFO( "POST: 0x%X", n );
		}
		return;
	
	} else {
		ERR( "Unkown memory write, 0x%.8X\n", addr );
	}
}
