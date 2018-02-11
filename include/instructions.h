#ifndef _INSTRUCTIONS_H
#define _INSTRUCTIONS_H

#include "types.h"

#define GET_OP( n ) ( n >> 26 )
#define GET_RS( n ) ( ( n >> 21 ) & 0x1F )
#define GET_RT( n ) ( ( n >> 16 ) & 0x1F )
#define GET_RD( n ) ( ( n >> 11 ) & 0x1F )
#define GET_SH( n ) ( ( n >> 6 ) & 0x1F )
#define GET_FN( n ) ( n & 0x3F )
#define GET_IMM( n ) ( (s32)( *(s16*)&n ) )
#define GET_UIMM( n ) ( n & 0xFFFF )
#define GET_ADDR( n ) ( ( n & 0x03FFFFFF ) << 2 )
#define GET_SYS( n ) ( ( n >> 6 ) & 0x0FFFFF )

typedef enum {
	SPECIAL = 0,
	BRANCH,
	J,
	JAL,
	BEQ,
	BNE,
	BLEZ,
	BGTZ,
	ADDI,
	ADDIU,
	SLTI,
	SLTIU,
	ANDI,
	ORI,
	XORI,
	LUI,
	COP0,
	COP1,
	COP2,
	COP3,
	
	LB = 32,
	LH,
	LWL,
	LW,
	LBU,
	LHU,
	LWR,
	
	SB = 40,
	SH,
	SWL,
	SW,
	
	SWR = 46,
	
	LWC0 = 48,
	LWC1,
	LWC2,
	LWC3,
	
	SWC0 = 56,
	SWC1,
	SWC2,
	SWC3
} opcodes;

static const char* opcodeNames[] = {
	"SPECIAL",
	"BRANCH",
	"J",
	"JAL",
	"BEQ",
	"BNE",
	"BLEZ",
	"BGTZ",
	"ADDI",
	"ADDIU",
	"SLTI",
	"SLTIU",
	"ANDI",
	"ORI",
	"XORI",
	"LUI",
	"COP0",
	"COP1",
	"COP2",
	"COP3",
	"", "", "", "", "", "", "", "", "", "", "", "",
	"LB",
	"LH",
	"LWL",
	"LW",
	"LBU",
	"LHU",
	"LWR",
	"",
	"SB",
	"SH",
	"SWL",
	"SW",
	"", "",
	"SWR",
	"",
	"LWC0",
	"LWC1",
	"LWC2",
	"LWC3",
	"", "", "", "",
	"SWC0",
	"SWC1",
	"SWC2",
	"SWC3"
};

typedef enum {
	SLL = 0,
	
	SRL = 2,
	SRA,
	SLLV,
	
	SRLV = 6,
	SRAV,
	JR,
	JALR,
	
	SYSCALL = 12,
	BREAK,
	
	MFHI = 16,
	MTHI,
	MFLO,
	MTLO,
	
	MULT = 24,
	MULTU,
	DIV,
	DIVU,
	
	ADD = 32,
	ADDU,
	SUB,
	SUBU,
	AND,
	OR,
	XOR,
	NOR,
	
	SLT = 42,
	SLTU
} specialFn;

static const char* specialNames[] = {
	"SLL",
	"",
	"SRL",
	"SRA",
	"SLLV",
	"",
	"SRLV",
	"SRAV",
	"JR",
	"JALR",
	"", "",
	"SYSCALL",
	"BREAK",
	"", "",
	"MFHI",
	"MTHI",
	"MFLO",
	"MTLO",
	"", "", "", "",
	"MULT",
	"MULTU",
	"DIV",
	"DIVU",
	"", "", "", "",
	"ADD",
	"ADDU",
	"SUB",
	"SUBU",
	"AND",
	"OR",
	"XOR",
	"NOR",
	"", "",
	"SLT",
	"SLTU"
};

typedef enum {
	BLTZ = 0,
	BGEZ,
	
	BLTZAL = 16,
	BGEZAL
} branchFn;

static const char* branchNames[] = {
	"BLTZ",
	"BGEZ",
	"", "", "", ""
	"", "", "", ""
	"", "", "", ""
	"", "",
	"BLTZAL",
	"BGEZAL"
};

typedef enum {
	MFC,
	CFC = 2,
	MTC = 4,
	CTC = 6,
	COP_SPECIAL = 16
} copFn;

typedef enum {
	RFE = 16
} copSpecialFn;

#endif
