#ifndef _MIPS_H
#define _MIPS_H

#include "types.h"

typedef union {
	u32 raw;
	struct {
		u32 iec:1;
		u32 kuc:1;
		u32 iep:1;
		u32 kup:1;
		u32 ieo:1;
		u32 kuo:1;
		u32 pad1:2;
		u32 sw:2;
		u32 intr:6;
		u32 isc:1;
		u32 pad2:1;
		u32 pz:1;;
		u32 pad3:1;
		u32 pe:1;
		u32 ts:1;
		u32 bev:1;
		u32 pad4:5;
		u32 cu:4;
	};
} cop0status;

typedef union {
	u32 r[32];
	struct {
		u32 r0;
		u32 r1;
		u32 r2;
		u32 bpc;
		u32 r4;
		u32 bda;
		u32 tar;
		u32 dcic;
		u32 bada;
		u32 bdam;
		u32 r10;
		u32 bpcm;
		cop0status sr;
		u32 cause;
		u32 epc;
		u32 prid;
	};
} cop0regs;

extern cop0regs c0regs;
extern u32 istatus;
extern u32 imask;

void setupCPU( void );
void step( int clocks );
void except( u32 cause );

#endif
