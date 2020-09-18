#ifndef _GTE_H
#define _GTE_H

#include "types.h"

typedef enum {
	RTPS  = 0x01,
	NCLIP = 0x06,
	OP    = 0x0C,
	DPCS  = 0x10,
	INTPL = 0x11,
	MVMA  = 0x12,
	NCDS  = 0x13,
	CDP   = 0x14,
	NCDT  = 0x16,
	NCCS  = 0x1B,
	CC    = 0x1C,
	NCS   = 0x1E,
	NCT   = 0x20,
	SQR   = 0x28,
	DCPL  = 0x29,
	DPCT  = 0x2A,
	AVSZ3 = 0x2D,
	AVSZ4 = 0x2E,
	RTPT  = 0x30,
	GPF   = 0x3D,
	GPL   = 0x3E,
	NCCT  = 0x3F
} gteOps;

union gteRegs {
	u32 r[64];
	struct {
		// data registers
		u32 VXY0, VZ0;
		u32 VXY1, VZ1;
		u32 VXY2, VZ2;
		u32 RGBC;
		u32 OTZ;
		u32 IR0, IR1, IR2, IR3;
		u32 SXY0, SXY1, SXY2, SXYP;
		u32 SZ0, SZ1, SZ2, SZ3;
		u32 RGB0, RGB1, RGB2;
		u32 RES1;
		u32 MAC0, MAC1, MAC2, MAC3;
		u32 IRGB, ORGB;
		u32 LZCS, LZCR;
		// control registers
		u32 RT1, RT2, RT3, RT4, RT5;
		u32 TRX, TRY, TRZ;
		u32 L1, L2, L3, L4, L5;
		u32 RBK, GBK, BBK;
		u32 LC1, LC2, LC3, LC4, LC5;
		u32 RFC, GFC, BFC;
		u32 OFX, OFY;
		u32 H;
		u32 DQA, DQB;
		u32 ZSF3, ZSF4;
		u32 FLAG;
	};
};

void gteExec( u32 opcode );

#endif
