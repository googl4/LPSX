#include <string.h>

#include "types.h"
#include "util.h"
#include "mips.h"
#include "bus.h"

extern int cpuBreak;
extern int halt;
extern u32 pc;

// args r4,5,6,7, sp+10,14 ...
// return r2

char dmsg[256] = { 0 };

void bioscall( u32 vector, u32 func, u32* r ) {
	switch( vector ) {
		case 0xA0:
			switch( func ) {
				case 0x13:
					// savestate
					break;
				
				case 0x2B:
					// memset, r4 addr, r5 fill, r6 len
					for( int i = 0; i < r[6]; i++ ) {
						store( r[4] + i, BYTE, r[5] );
					}
					break;
				
				case 0x39:
					// init heap, r4 address, r5 size
					break;
					
				case 0x44:
					// flush cache
					break;
					
				default:
					INFO( "bios call: 0xA0%.2X", func );
					cpuBreak = TRUE;
					halt = TRUE;
					break;
			}
			break;
			
		case 0xB0:
			switch( func ) {
				case 0x12:
					// initpad
					break;
					
				case 0x13:
					// startpad
					break;
					
				case 0x17:
					// returnFromException
					break;
				
				case 0x39:
					// fileGetDeviceFlag, r4 fd
					r[2] = 0;
					break;
					
				case 0x44:
					// file rename, r4 oldname, r5 newname
					break;
				
				case 0x3D:
					if( r[4] == '\n' ) {
						INFO( "bios: %s", dmsg );
						dmsg[0] = 0;
					} else {
						char tmp[2] = { r[4], 0 };
						strcat( dmsg, tmp );
					}
					break;
					
				default:
					INFO( "bios call: 0xB0%.2X", func );
					cpuBreak = TRUE;
					halt = TRUE;
					break;
			}
			break;
			
		case 0xC0:
			INFO( "bios call: 0xC0%.2X", func );
			cpuBreak = TRUE;
			halt = TRUE;
			break;
	}
	pc = r[31];
}
