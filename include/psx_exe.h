#ifndef _PSX_EXE_H
#define _PSX_EXE_H

#include "types.h"

static const char* sceID = "SCE EXE";
static const char* psxID = "PS-X EXE";

typedef struct {
	u8  id[8];
	u32 textOffset;
	u32 dataOffset;
	u32 pc;
	u32 gp;
	u32 textAddr;
	u32 textSize;
	u32 dataAddr;
	u32 dataSize;
	u32 bssAddr;
	u32 bssSize;
	u32 stackAddr;
	u32 stackSize;
	u32 savedSP;
	u32 savedFP;
	u32 savedGP;
	u32 savedRA;
	u32 savedS0;
} SCEheader_t;

typedef struct {
	u8  id[8];
	u32 textOffset;
	u32 pc;
	u32 textAddr;
	u32 textSize;
	u32 stackAddr;
	u32 stackSize;
	u32 savedSP;
	u32 savedFP;
	u32 savedGP;
	u32 savedRA;
	u32 savedS0;
} PSXheader_t;

typedef struct {
	u8 id[8];
	u8 pad1[8];
	u32 pc;
	u32 gp;
	u32 dst;
	u32 size;
	u32 pad2[2];
	u32 memfillAddr;
	u32 memfillSize;
	u32 sp;
	u32 spOffset;
	u8 pad3[20];
	u8 SCEI[1972];
	u8 data[];
} EXEheader_t;

void loadEXE( char* filename );

#endif
