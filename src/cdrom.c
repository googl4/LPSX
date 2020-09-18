#include "cdrom.h"
#include "mips.h"
#include "util.h"

// 0x8005AA14

union {
	u8 byte;
	struct {
		u8 index:2;
		u8 adpcmBusy:1;
		u8 parameterEmpty:1;
		u8 parameterReady:1;
		u8 responseReady:1;
		u8 dataReady:1;
		u8 commandBusy:1;
	};
} cdStat = { 0 };

union {
	u8 byte;
	struct {
		u8 play:1;
		u8 seek:1;
		u8 read:1;
		u8 open:1;
		u8 idErr:1;
		u8 seekErr:1;
		u8 motor:1;
		u8 Error:1;
	};
} cdHWStat = { 0 };

u8 cdif = 0;
u8 cdie = 0;

u8 sndmap = 0;

u8 rfifo[16];
int rfifoHead = 0;
int rfifoTail = 0;
int rfifoLen = 0;

void pushResponse( u8 data ) {
	rfifo[rfifoHead] = data;
	rfifoHead = ( rfifoHead + 1 ) % 16;
	rfifoLen++;
	cdStat.responseReady = 1;
}

u8 popResponse( void ) {
	if( rfifoLen <= 1 ) {
		cdStat.responseReady = 0;
	}
	
	if( rfifoLen > 0 ) {
		u8 data = rfifo[rfifoTail];
		rfifoTail = ( rfifoTail + 1 ) % 16;
		rfifoLen--;
		
		return data;
		
	} else {
		return 0;
	}
}

u8 pfifo[16];
int pfifoHead = 0;
int pfifoTail = 0;
int pfifoLen = 0;

void pushParam( u8 data ) {
	pfifo[pfifoHead] = data;
	pfifoHead = ( pfifoHead + 1 ) % 16;
	pfifoLen++;
	cdStat.parameterEmpty = 0;
	cdStat.parameterReady = pfifoLen < 16;
}

u8 popParam( void ) {
	u8 data = 0;
	
	if( pfifoLen > 0 ) {
		data = pfifo[pfifoTail];
		pfifoTail = ( pfifoTail + 1 ) % 16;
		pfifoLen--;
	}
	
	cdStat.parameterEmpty = pfifoLen == 0;
	cdStat.parameterReady = pfifoLen < 16;
	
	return data;
}

void setupCd( void ) {
	cdStat.responseReady = 1;
	cdStat.parameterReady = 1;
	cdStat.parameterEmpty = 1;
	pfifoHead = 0;
	pfifoTail = 0;
	pfifoLen = 0;
}

int queuedInterrupt2 = -1;
int queuedInterrupt = -1;

void cdUpdate( void ) {
	if( queuedInterrupt >= 0 ) {
		istatus |= 0x04;
		cdif = queuedInterrupt;
		except( 0x00 );
		queuedInterrupt = -1;
	}
}

void cdWrite( u32 addr, u8 n ) {
	switch( addr & 3 ) {
		case 0:
			cdStat.index = n;
			break;
			
		case 1:
			switch( cdStat.index ) {
				case 0:
					switch( n ) {
						case 0x01: // GetStat
							pushResponse( cdHWStat.byte );
							/*
							istatus |= 0x04;
							cdif = 3;
							except( 0x00 );
							*/
							queuedInterrupt = 3;
							break;
						
						case 0x0A: // Init
							/*
							istatus |= 0x04;
							//cdStat.responseReady = 1;
							//cdStat.parameterReady = 1;
							//cdStat.parameterEmpty = 1;
							//pfifoHead = 0;
							//pfifoTail = 0;
							//pfifoLen = 0;
							cdif = 3;
							*/
							cdHWStat.byte = 0;
							cdHWStat.motor = 1;
							pushResponse( cdHWStat.byte );
							pushResponse( cdHWStat.byte );
							queuedInterrupt2 = 2;
							//except( 0x00 );
							queuedInterrupt = 3;
							break;
							
						case 0x19: { // Test
							u8 subfn = popParam();
							switch( subfn ) {
								case 0x20: // Version
									/*
									// report HC05 firmware version C2 (b) 14 Aug 1997 (board version PU-20)
									pushResponse( 0xC2 );
									pushResponse( 0x14 );
									pushResponse( 0x08 );
									pushResponse( 0x97 );
									*/
									// report HC05 firmware version C0 (a) 19 Sep 1994 (board version PU-7)
									pushResponse( 0xC0 );
									pushResponse( 0x19 );
									pushResponse( 0x09 );
									pushResponse( 0x94 );
									/*
									istatus |= 0x04;
									cdif = 3;
									except( 0x00 );
									*/
									queuedInterrupt = 3;
									break;
								
								default:
									ERR( "cd test command 0x%X", subfn );
									break;
							}
							break;
						}
						
						case 0x1A: // GetID
							// no disk
							//INT5(08h,40h, 00h,00h, 00h,00h,00h,00h)
							// SCEA (ntsc), SCEE (pal), SCEI (ntsc-j)
							//INT2(02h,00h, 20h,00h, 53h,43h,45h,4xh)
							// audio disk
							//INT5(0Ah,90h, 00h,00h, 00h,00h,00h,00h)
							
							
							// no disk
							pushResponse( 0x00 );
							pushResponse( 0x00 );
							pushResponse( 0x00 );
							pushResponse( 0x00 );
							pushResponse( 0x00 );
							pushResponse( 0x00 );
							pushResponse( 0x40 );
							pushResponse( 0x08 );
							
							queuedInterrupt2 = 5;
							
							/*
							// audio disk
							pushResponse( 0x00 );
							pushResponse( 0x00 );
							pushResponse( 0x00 );
							pushResponse( 0x00 );
							pushResponse( 0x00 );
							pushResponse( 0x00 );
							pushResponse( 0x90 );
							pushResponse( 0x0A );
							
							queuedInterrupt2 = 5;
							*/
							/*
							// game disk (SCEE)
							pushResponse( 0x45 );
							pushResponse( 0x45 );
							pushResponse( 0x43 );
							pushResponse( 0x53 );
							pushResponse( 0x00 );
							pushResponse( 0x20 );
							pushResponse( 0x00 );
							pushResponse( 0x02 );
							
							queuedInterrupt2 = 2;
							*/
							pushResponse( cdHWStat.byte );
							queuedInterrupt = 3;
						break;
						
						default:
							ERR( "cd command 0x%X", n );
							break;
					}
					break;
					
				case 1:
					sndmap = n;
					break;
					
				default:
					ERR( "cd write 0x%.8X : %d : 0x%.2X", addr, cdStat.index, n );
					break;
			}
			break;
			
		case 2:
			switch( cdStat.index ) {
				case 0:
					pushParam( n );
					break;
				
				case 1:
					cdie = n;
					break;
					
				default:
					ERR( "cd write 0x%.8X : %d : 0x%.2X", addr, cdStat.index, n );
					break;
			}
			break;
			
		case 3:
			if( cdStat.index & 1 ) {
				// TODO: clear response fifo
				cdif &= ~n;
				// hack: push all responses in advance
				if( queuedInterrupt2 >= 0 ) {
					/*
					istatus |= 0x04;
					cdif = queuedInterrupt2;
					queuedInterrupt2 = -1;
					except( 0x00 );
					*/
					queuedInterrupt = queuedInterrupt2;
					queuedInterrupt2 = -1;
				}
				
			} else {
				ERR( "cd write 0x%.8X : 0x%.2X", addr, n );
			}
			break;
			
		default:
			ERR( "cd write 0x%.8X : 0x%.2X", addr, n );
			break;
	}
}

u8 cdRead( u32 addr ) {
	switch( addr & 3 ) {
		case 0:
			//cdStat.parameterEmpty = 1;
			//cdStat.parameterReady = 1;
			return cdStat.byte;
			
		case 1:
			//WARN( "CD FIFO read" );
			return popResponse();
			
		case 3:
			if( addr & 1 ) {
				return cdif | 0xE0;
				
			} else {
				ERR( "cd read 0x%.8X", addr );
			}
			break;
			
		default:
			ERR( "cd read 0x%.8X", addr );
			break;
	}
	
	return 0;
}
