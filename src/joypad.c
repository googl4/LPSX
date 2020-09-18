#include "types.h"
#include "util.h"
#include "mips.h"

u16 joyCtrl;
u16 joyBaud;
u16 joyMode;

int beginTransfer = FALSE;

void joyPadUpdate( void ) {
	if( beginTransfer ) {
		istatus |= 0x40;
		except( 0x00 );
		beginTransfer = FALSE;
	}
}

extern int halt;
extern int cpuBreak;;

void joyPadWrite( u32 addr, u32 n ) {
	switch( addr & 0x0F ) {
		case 0x08:
			joyMode = n;
			INFO( "JOY_MODE 0x%X", n );
			break;
		
		case 0x0A:
			joyCtrl = n;
			INFO( "JOY_CTRL 0x%X", n );
			if( n & 0x0002 ) {
				beginTransfer = TRUE;
			}
			break;
			
		case 0x0E:
			joyBaud = n;
			INFO( "JOY_BAUD 0x%X", n );
			break;
		
		default:
			ERR( "joypad write 0x%X : %X", addr, n );
			break;
	}
	
	//joyPadUpdate();
	
	halt = TRUE;
	cpuBreak = TRUE;
}
