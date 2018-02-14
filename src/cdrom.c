#include "cdrom.h"
#include "mips.h"
#include "util.h"

extern int halt;

cdStatus_t status;
u8 cdie = 0;
cdIntrFlags_t cdif = { 0 };

u8 parameterFifo[16];
int parameterHead = 0;
int parameterTail = 0;

void parameterPush( u8 n ) {
	parameterFifo[parameterHead] = n;
	parameterHead = ( parameterHead + 1 ) & 0x0F;
}

u8 parameterPop( void ) {
	int p = parameterTail;
	parameterTail = ( parameterTail + 1 ) & 0x0F;
	return parameterFifo[p];
}

u8 responseFifo[16];
int responseHead = 0;
int responseTail = 0;

void responsePush( u8 n ) {
	responseFifo[responseHead] = n;
	responseHead = ( responseHead + 1 ) & 0x0F;
}

u8 responsePop( void ) {
	int p = responseTail;
	responseTail = ( responseTail + 1 ) & 0x0F;
	return responseFifo[p];
}

void cdCommand( u8 cmd ) {
	switch( cmd ) {
		case TEST:;
			u8 fn = parameterPop();
			switch( fn ) {
				case DATE:
					responsePush( 0x95 );
					responsePush( 0x05 );
					responsePush( 0x16 );
					responsePush( 0xC1 );
					cdif.intr = 3;
					except( 0x00 );
					break;
				
				default:
					ERR( "Unknown cd test command: 0x%.2X", fn );
					break;
			}
			break;
		
		default:
			ERR( "Unknown cd command: 0x%.2X", cmd );
			break;
	}
}

void cdWrite( u32 addr, u8 n ) {
	halt = TRUE;
	//INFO( "cd write, register: %d, index: %d, value: 0x%X", addr & 0x0F, status.index, n );
	switch( addr & 0x0F ) {
		case 0:
			status.index = n;
			break;
			
		case 1:
			switch( status.index ) {
				case 0:
					cdCommand( n );
					break;
					
				default:
					INFO( "cd write, register: %d, index: %d", addr & 0x0F, status.index );
					break;
			}
			break;
			
		case 2:
			switch( status.index ) {
				case 0:
					parameterPush( n );
					break;
					
				case 1:
					cdie = n;
					break;
					
				default:
					INFO( "cd write, register: %d, index: %d", addr & 0x0F, status.index );
					break;
			}
			break;
			
		case 3:
			switch( status.index ) {
				case 1:
					responseTail = responseHead;
					*(u8*)&cdif &= ~( n & 0x1F );
					break;
					
				default:
					INFO( "cd write, register: %d, index: %d", addr & 0x0F, status.index );
					break;
			}
			break;
	}
}

// 0x80058484
// 0x800584E8
// 0x800585C0

u8 cdRead( u32 addr ) {
	halt = TRUE;
	//INFO( "cd read, register: %d, index: %d", addr & 0x0F, status.index );
	switch( addr & 0x0F ) {
		case 0:
			if( responseHead != responseTail ) {
				status.responseReady = TRUE;
			}
			if( parameterHead == parameterTail ) {
				status.parameterEmpty = TRUE;
			}
			status.parameterReady = TRUE;
			return *(u8*)&status;
			
		case 1:
			return responsePop();
			
		case 2:
			// data fifo
			break;
			
		case 3:
			switch( status.index ) {
				case 0:
				case 2:
					return 0xE0 | cdie;
				
				case 1:
				case 3:
					return 0xE0 | *(u8*)&cdif;
			}
			break;
	}
}
