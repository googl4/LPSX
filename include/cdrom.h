#ifndef _CDROM_H
#define _CDROM_H

#include "types.h"

typedef struct {
	u8 index:2;
	u8 adpcmBusy:1;
	u8 parameterEmpty:1;
	u8 parameterReady:1;
	u8 responseReady:1;
	u8 dataReady:1;
	u8 busy:1;
} cdStatus_t;

typedef struct {
	u8 intr:3;
	u8 pad:1;
	u8 cmdStart:1;
} cdIntrFlags_t;

typedef enum {
	TEST = 0x19,
} cdCommands_t;

typedef enum {
	DATE = 0x20,
} cdTestCommands_t;

void setupCd( void );
void cdUpdate( void );

void cdWrite( u32 addr, u8 n );
u8 cdRead( u32 addr );

#endif
