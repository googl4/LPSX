#include <stdio.h>

#include "types.h"
#include "mips.h"
#include "instructions.h"
#include "util.h"

extern u32 currentpc;

void printInstr( char* buf, size_t bufLen, u32 addr, u32 opcode ) {
	u32 op = GET_OP( opcode );
	if( opcode == 0x00000000 ) {
		snprintf( buf, bufLen, "0x%.8X:0x%.8X: NOP", addr, opcode );
		
	} else if( op == 0x00 ) {
		u32 fn = GET_FN( opcode );
		switch( fn ) {
			case SLL:
			case ADDU:
			case ADD:
			case OR:
			case AND:
			case SLTU:
			case SLT:
			case SUBU:
			case SUB:
			case SRA:
			case SRL:
			case SLLV:
			case NOR:
			case SRAV:
			case SRLV:
			case XOR:
				snprintf( buf, bufLen, "0x%.8X:0x%.8X: %s r%d, r%d, r%d", addr, opcode, specialNames[fn], GET_RD( opcode ), GET_RT( opcode ), GET_RS( opcode ) );
				break;
				
			case JR:
				snprintf( buf, bufLen, "0x%.8X:0x%.8X: %s r%d", addr, opcode, specialNames[fn], GET_RS( opcode ) );
				break;
				
			case JALR:
				snprintf( buf, bufLen, "0x%.8X:0x%.8X: %s r%d, r%d", addr, opcode, specialNames[fn], GET_RD( opcode ), GET_RS( opcode ) );
				break;
				
			case DIV:
			case DIVU:
			case MULTU:
			case MULT:
				snprintf( buf, bufLen, "0x%.8X:0x%.8X: %s r%d, r%d", addr, opcode, specialNames[fn], GET_RS( opcode ), GET_RT( opcode ) );
				break;
				
			case MFLO:
			case MFHI:
				snprintf( buf, bufLen, "0x%.8X:0x%.8X: %s r%d", addr, opcode, specialNames[fn], GET_RD( opcode ) );
				break;
				
			case MTLO:
			case MTHI:
				snprintf( buf, bufLen, "0x%.8X:0x%.8X: %s r%d", addr, opcode, specialNames[fn], GET_RS( opcode ) );
				break;
				
			case SYSCALL:
				snprintf( buf, bufLen, "0x%.8X:0x%.8X: %s r%d", addr, opcode, specialNames[fn], GET_SYS( opcode ) );
				break;
					
			case BREAK:
				snprintf( buf, bufLen, "0x%.8X:0x%.8X: %s", addr, opcode, specialNames[fn] );
				break;
				
			default:
				ERR( "Unknown opcode %.2X", op );
				break;
		}
		
	} else if( op == 0x01 ) {
		u32 fn = GET_RT( opcode );
		snprintf( buf, bufLen, "0x%.8X:0x%.8X: %s r%d %+d", addr, opcode, branchNames[fn], GET_RS( opcode ), GET_IMM( opcode ) );
		
	} else {
		switch( op ) {
			case LUI:
				snprintf( buf, bufLen, "0x%.8X:0x%.8X: %s r%d, 0x%.4X", addr, opcode, opcodeNames[op], GET_RT( opcode ), GET_UIMM( opcode ) );
				break;
				
			case ORI:
			case XORI:
			case ANDI:
				snprintf( buf, bufLen, "0x%.8X:0x%.8X: %s r%d, r%d, 0x%.4X", addr, opcode, opcodeNames[op], GET_RT( opcode ), GET_RS( opcode ), GET_UIMM( opcode ) );
				break;
				
			case SW:
			case SH:
			case SB:
			case LWL:
			case LWR:
			case SWL:
			case SWR:
			case LW:
			case LH:
			case LHU:
			case LB:
			case LBU:
				snprintf( buf, bufLen, "0x%.8X:0x%.8X: %s r%d, [r%d, %+d]", addr, opcode, opcodeNames[op], GET_RT( opcode ), GET_RS( opcode ), GET_IMM( opcode ) );
				break;
				
			case ADDIU:
			case BNE:
			case BEQ:
			case ADDI:
			case SLTI:
			case SLTIU:
				snprintf( buf, bufLen, "0x%.8X:0x%.8X: %s r%d, r%d, %+d", addr, opcode, opcodeNames[op], GET_RT( opcode ), GET_RS( opcode ), GET_IMM( opcode ) );
				break;
				
			case J:
			case JAL:
				snprintf( buf, bufLen, "0x%.8X:0x%.8X: %s 0x.8X", addr, opcode, opcodeNames[op], GET_ADDR( opcode ) );
				break;
				
			case BGTZ:
			case BLEZ:
				snprintf( buf, bufLen, "0x%.8X:0x%.8X: %s r%d, %+d", addr, opcode, opcodeNames[op], GET_RS( opcode ), GET_IMM( opcode ) );
				break;
				
			case COP0:
			case COP1:
			case COP2:
			case COP3:
				switch( GET_RS( opcode ) ) {
					case MTC:
						snprintf( buf, bufLen, "0x%.8X:0x%.8X: MTC c%dr%d, r%d", addr, opcode, ( opcode >> 26 ) & 0x03, GET_RD( opcode ), GET_RT( opcode ) );
						break;
						
					case MFC:
						snprintf( buf, bufLen, "0x%.8X:0x%.8X: MFC r%d, c%dr%d", addr, opcode, GET_RT( opcode ), ( opcode >> 26 ) & 0x03, GET_RD( opcode ) );
						break;
						
					case COP_SPECIAL:
						snprintf( buf, bufLen, "0x%.8X:0x%.8X: RFE", addr, opcode );
						break;
						
					default:
						ERR( "Unknown opcode %.2X", op );
						break;
				}
				break;
				
			default:
				ERR( "Unknown opcode %.2X", op );
				break;
		}
	}
}
