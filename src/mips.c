#include <string.h>
#include <stdio.h>

#include "mips.h"
#include "util.h"
#include "bus.h"
#include "instructions.h"
#include "dma.h"
#include "dis.h"
#include "psx_exe.h"

u32 regs[32];
u32 hi, lo, pc;
cop0regs c0regs;

u32 currentpc;
u32 nextpc;

u32 loadreg;
u32 loadval;

u8 exception = FALSE;
u8 branchSlot = FALSE;
u8 branch = FALSE;

u32 istatus = 0;
u32 imask = 0;

int logInstrs = FALSE;
int logBranches = FALSE;

void except( u32 cause ) {
	//INFO( "exception: 0x%X", cause );
	
	c0regs.sr.raw = ( c0regs.sr.raw & ~0x3F ) | ( ( c0regs.sr.raw << 2 ) & 0x3F );
	c0regs.cause = cause << 2;
	c0regs.epc = currentpc;
	if( c0regs.sr.bev ) {
		pc = 0xBFC00180;
	} else {
		pc = 0x80000080;
	}
	exception = TRUE;
	if( branchSlot ) {
		c0regs.epc -= 4;
		c0regs.cause |= 1 << 31;
	}
}

char dmsg[256] = { 0 };

void execute( u32 opcode ) {
	u32 inregs[32] = { 0 };
	memcpy( &inregs[1], &regs[1], 31 * 4 );
	
	regs[loadreg] = loadval;
	loadreg = 0;
	
	regs[0] = 0;
	
	if( currentpc == 0xA0 ) {
		INFO( "bios call: 0xA0%.2X", inregs[9] );
	}
	if( currentpc == 0xB0 ) {
		if( inregs[9] == 0x3D ) {
			if( inregs[4] == '\n' ) {
				INFO( "bios: %s", dmsg );
				dmsg[0] = 0;
			} else {
				char tmp[2] = { inregs[4], 0 };
				strcat( dmsg, tmp );
			}
		} else {
			INFO( "bios call: 0xB0%.2X", inregs[9] );
		}
	}
	if( currentpc == 0xC0 ) {
		INFO( "bios call: 0xC0%.2X", inregs[9] );
	}
	
	u32 op = GET_OP( opcode );
	switch( op ) {
		case SPECIAL:
			switch( GET_FN( opcode ) ) {
				case SLL:
					regs[GET_RD( opcode )] = inregs[GET_RT( opcode )] << GET_SH( opcode );
					break;
					
				case ADDU:
					regs[GET_RD( opcode )] = (s32)inregs[GET_RS( opcode )] + (s32)inregs[GET_RT( opcode )];
					break;
					
				case ADD:
					if( __builtin_add_overflow( (s32)inregs[GET_RS( opcode )], (s32)inregs[GET_RT( opcode )], (s32*)&regs[GET_RD( opcode )] ) ) {
						except( 0x0C );
					}
					break;
					
				case OR:
					regs[GET_RD( opcode )] = inregs[GET_RS( opcode )] | inregs[GET_RT( opcode )];
					break;
					
				case AND:
					regs[GET_RD( opcode )] = inregs[GET_RS( opcode )] & inregs[GET_RT( opcode )];
					break;
					
				case SLTU:
					regs[GET_RD( opcode )] = inregs[GET_RS( opcode )] < inregs[GET_RT( opcode )];
					break;
					
				case SLT:
					regs[GET_RD( opcode )] = (s32)inregs[GET_RS( opcode )] < (s32)inregs[GET_RT( opcode )];
					break;
					
				case JR:
					pc = inregs[GET_RS( opcode )];
					branch = TRUE;
					if( logBranches ) {
						INFO( "JR to 0x%.8X(r%d)", inregs[GET_RS( opcode )], GET_RS( opcode ) );
					}
					break;
					
				case JALR:
					regs[GET_RD( opcode )] = pc;
					pc = inregs[GET_RS( opcode )];
					branch = TRUE;
					if( logBranches ) {
						INFO( "JALR to 0x%.8X(r%d)", inregs[GET_RS( opcode )], GET_RS( opcode ) );
					}
					break;
					
				case SUBU:
					regs[GET_RD( opcode )] = inregs[GET_RS( opcode )] - inregs[GET_RT( opcode )];
					break;
					
				case SUB:
					if( __builtin_sub_overflow( (s32)inregs[GET_RS( opcode )], (s32)inregs[GET_RT( opcode )], (s32*)&regs[GET_RD( opcode )] ) ) {
						except( 0x0C );
					}
					break;
					
				case SRA:
					regs[GET_RD( opcode )] = (s32)inregs[GET_RT( opcode )] >> GET_SH( opcode );
					break;
					
				case DIV:
					if( (s32)inregs[GET_RT( opcode )] == 0 ) {
						hi = inregs[GET_RS( opcode )];
						if( (s32)inregs[GET_RS( opcode )] >= 0 ) {
							lo = 0xFFFFFFFF;
						} else {
							lo = 0x00000001;
						}
						break;
					} else if( inregs[GET_RS( opcode )] == 0x80000000 && (s32)inregs[GET_RT( opcode )] == -1 ) {
						hi = 0;
						lo = 0x80000000;
						break;
					}
					lo = (s32)inregs[GET_RS( opcode )] / (s32)inregs[GET_RT( opcode )];
					hi = (s32)inregs[GET_RS( opcode )] % (s32)inregs[GET_RT( opcode )];
					break;
					
				case DIVU:
					if( inregs[GET_RT( opcode )] == 0 ) {
						hi = inregs[GET_RT( opcode )];
						lo = 0xFFFFFFFF;
					}
					lo = inregs[GET_RS( opcode )] / inregs[GET_RT( opcode )];
					hi = inregs[GET_RS( opcode )] % inregs[GET_RT( opcode )];
					break;
					
				case MFLO:
					regs[GET_RD( opcode )] = lo;
					break;
					
				case MFHI:
					regs[GET_RD( opcode )] = hi;
					break;
					
				case MTLO:
					lo = inregs[GET_RS( opcode )];
					break;
					
				case MTHI:
					hi = inregs[GET_RS( opcode )];
					break;
					
				case SRL:
					regs[GET_RD( opcode )] = inregs[GET_RT( opcode )] >> GET_SH( opcode );
					break;
					
				case SYSCALL:
					//pc -= 4;
					except( 0x08 );
					//INFO( "syscall %d, r4: 0x%.8X", GET_SYS( opcode ), regs[4] );
					//GET_SYS( opcode );
					break;
					
				case BREAK:
					except( 0x09 );
					break;
					
				case SLLV:
					regs[GET_RD( opcode )] = regs[GET_RT( opcode )] << ( regs[GET_RS( opcode )] & 0x1F );
					break;
					
				case NOR:
					regs[GET_RD( opcode )] = ~( inregs[GET_RS( opcode )] | inregs[GET_RT( opcode )] );
					break;
					
				case SRAV:
					regs[GET_RD( opcode )] = (s32)regs[GET_RT( opcode )] >> ( regs[GET_RS( opcode )] & 0x1F );
					break;
					
				case SRLV:
					regs[GET_RD( opcode )] = regs[GET_RT( opcode )] >> ( regs[GET_RS( opcode )] & 0x1F );
					break;
					
				case MULTU:
					{
					u64 tmp = (u64)inregs[GET_RS( opcode )] * inregs[GET_RT( opcode )];
					lo = tmp;
					hi = tmp >> 32;
					}
					break;
					
				case MULT:
					{
					s64 tmp = (s64)inregs[GET_RS( opcode )] * inregs[GET_RT( opcode )];
					lo = (u64)tmp & 0xFFFFFFFF;
					hi = (u64)tmp >> 32;
					}
					break;
					
				case XOR:
					regs[GET_RD( opcode )] = inregs[GET_RS( opcode )] ^ inregs[GET_RT( opcode )];
					break;
					
				default:
					printf( "pc: 0x%.8X\n", currentpc );
					ERR( "Unkown special function %s (%d), 0x%.8X\n", specialNames[GET_FN( opcode )], GET_FN( opcode ), opcode );
					break;
			}
			break;
			
		case BRANCH:
			switch( GET_RT( opcode ) ) {
				case BLTZ:
					if( (s32)inregs[GET_RS( opcode )] < 0 ) {
						pc = (s32)pc + ( GET_IMM( opcode ) << 2 ) - 4;
						branch = TRUE;
						if( logBranches ) {
							INFO( "BLTZ to 0x%.8X", pc );
						}
					}
					break;
					
				case BGEZ:
					if( (s32)inregs[GET_RS( opcode )] >= 0 ) {
						pc = (s32)pc + ( GET_IMM( opcode ) << 2 ) - 4;
						branch = TRUE;
						if( logBranches ) {
							INFO( "BGEZ to 0x%.8X", pc );
						}
					}
					break;
					
				case BLTZAL:
					if( (s32)inregs[GET_RS( opcode )] < 0 ) {
						regs[31] = pc;
						pc = (s32)pc + ( GET_IMM( opcode ) << 2 ) - 4;
						branch = TRUE;
						if( logBranches ) {
							INFO( "BLTZAL to 0x%.8X", pc );
						}
					}
					break;
					
				case BGEZAL:
					if( (s32)inregs[GET_RS( opcode )] >= 0 ) {
						regs[31] = pc;
						pc = (s32)pc + ( GET_IMM( opcode ) << 2 ) - 4;
						branch = TRUE;
						if( logBranches ) {
							INFO( "BGEZAL to 0x%.8X", pc );
						}
					}
					break;
					
				default:
					ERR( "Unknown branch type: %d", GET_RT( opcode ) );
					break;
			}
			break;
			
		case LUI:
			regs[GET_RT( opcode )] = GET_UIMM( opcode ) << 16;
			//printf( "lui: 0x%.8X (r%d)\n", GET_UIMM( opcode ) << 16, GET_RT( opcode ) );
			break;
			
		case ORI:
			regs[GET_RT( opcode )] = inregs[GET_RS( opcode )] | GET_UIMM( opcode );
			break;
			
		case XORI:
			regs[GET_RT( opcode )] = inregs[GET_RS( opcode )] ^ GET_UIMM( opcode );
			break;
			
		case ANDI:
			regs[GET_RT( opcode )] = inregs[GET_RS( opcode )] & GET_UIMM( opcode );
			break;
			
		case SW:
			store( (s32)inregs[GET_RS( opcode )] + GET_IMM( opcode ), WORD, inregs[GET_RT( opcode )] );
			//printf( "store 0x%.8X from r%d to 0x%.8X (r%d) offset %d\n", regs[GET_RT( opcode )], GET_RT( opcode ), regs[GET_RS( opcode )], GET_RS( opcode ), GET_IMM( opcode ) );
			break;
			
		case SH:
			store( (s32)inregs[GET_RS( opcode )] + GET_IMM( opcode ), HALFWORD, inregs[GET_RT( opcode )] );
			break;
			
		case SB:
			store( (s32)inregs[GET_RS( opcode )] + GET_IMM( opcode ), BYTE, inregs[GET_RT( opcode )] );
			break;
			
		case ADDIU:
			regs[GET_RT( opcode )] = (s32)inregs[GET_RS( opcode )] + GET_IMM( opcode );
			break;
			
		case J:
			pc = ( pc & 0xF0000000 ) | GET_ADDR( opcode );
			branch = TRUE;
			if( logBranches ) {
				INFO( "JMP to 0x%.8X", pc );
			}
			break;
			
		case JAL:
			regs[31] = pc;
			pc = ( pc & 0xF0000000 ) | GET_ADDR( opcode );
			branch = TRUE;
			if( logBranches ) {
				INFO( "JAL to 0x%.8X", pc );
			}
			break;
			
		case LWL:
			{
				u32 addr = (s32)inregs[GET_RS( opcode )] + GET_IMM( opcode );
				u32 word = load( addr & 0xFFFFFFFC, WORD );
				u32 align = addr & 0x03;
				u32 shift = align * 8;
				word <<= shift;
				u32 mask = 0x00FFFFFF >> ( 24 - shift );
				regs[GET_RT( opcode )] = ( regs[GET_RT( opcode )] & mask ) | word;
			}
			break;
			
		case LWR:
			{
				u32 addr = (s32)inregs[GET_RS( opcode )] + GET_IMM( opcode );
				u32 word = load( addr & 0xFFFFFFFC, WORD );
				u32 align = addr & 0x03;
				u32 shift = align * 8;
				word >>= shift;
				u32 mask = 0xFFFFFF00 << ( 24 - shift );
				regs[GET_RT( opcode )] = ( regs[GET_RT( opcode )] & mask ) | word;
			}
			break;
			
		case SWL:
			{
				u32 addr = (s32)inregs[GET_RS( opcode )] + GET_IMM( opcode );
				u32 val = regs[GET_RT( opcode )];
				u32 word = load( addr & 0xFFFFFFFC, WORD );
				u32 align = addr & 0x03;
				u32 shift = align * 8;
				val >>= 24 - shift;
				u32 mask = 0xFFFFFF00 << shift;
				store( addr & 0xFFFFFFFC, WORD, ( word & mask ) | val );
			}
			break;
			
		case SWR:
			{
				u32 addr = (s32)inregs[GET_RS( opcode )] + GET_IMM( opcode );
				u32 val = regs[GET_RT( opcode )];
				u32 word = load( addr & 0xFFFFFFFC, WORD );
				u32 align = addr & 0x03;
				u32 shift = align * 8;
				val <<= shift;
				u32 mask = 0x00FFFFFF >> ( 24 - shift );
				store( addr & 0xFFFFFFFC, WORD, ( word & mask ) | val );
			}
			break;
			
		case LW:
			loadreg = GET_RT( opcode );
			loadval = load( (s32)inregs[GET_RS( opcode )] + GET_IMM( opcode ), WORD );
			//regs[GET_RT( opcode )] = load( (s32)regs[GET_RS( opcode )] + GET_IMM( opcode ), WORD );
			//printf( "load 0x%.8X to r%d from 0x%.8X (r%d) offset %d\n", regs[GET_RT( opcode )], GET_RT( opcode ), regs[GET_RS( opcode )], GET_RS( opcode ), GET_IMM( opcode ) );
			break;
			
		case LH:
			loadreg = GET_RT( opcode );
			loadval = load( (s32)inregs[GET_RS( opcode )] + GET_IMM( opcode ), HALFWORD_SIGNED );
			break;
			
		case LHU:
			loadreg = GET_RT( opcode );
			loadval = load( (s32)inregs[GET_RS( opcode )] + GET_IMM( opcode ), HALFWORD );
			break;
			
		case LB:
			loadreg = GET_RT( opcode );
			loadval = load( (s32)inregs[GET_RS( opcode )] + GET_IMM( opcode ), BYTE_SIGNED );
			//regs[GET_RT( opcode )] = load( (s32)regs[GET_RS( opcode )] + GET_IMM( opcode ), BYTE_SIGNED );
			break;
			
		case LBU:
			loadreg = GET_RT( opcode );
			loadval = load( (s32)inregs[GET_RS( opcode )] + GET_IMM( opcode ), BYTE );
			break;
			
		case BNE:
			if( inregs[GET_RS( opcode )] != inregs[GET_RT( opcode )] ) {
				pc = (s32)pc + ( GET_IMM( opcode ) << 2 ) - 4;
				branch = TRUE;
				if( logBranches ) {
					INFO( "BNE to 0x%.8X", pc );
				}
			}
			break;
			
		case BEQ:
			if( inregs[GET_RS( opcode )] == inregs[GET_RT( opcode )] ) {
				pc = (s32)pc + ( GET_IMM( opcode ) << 2 ) - 4;
				branch = TRUE;
				if( logBranches ) {
					//printf( "0x%.8X: 0x%.8X (r%d) == 0x%.8X (r%d)\n", currentpc, inregs[GET_RS( opcode )], GET_RS( opcode ), inregs[GET_RT( opcode )], GET_RT( opcode ) );
					INFO( "BEQ to 0x%.8X", pc );
				}
			}
			break;
			
		case BGTZ:
			if( (s32)inregs[GET_RS( opcode )] > 0 ) {
				pc = (s32)pc + ( GET_IMM( opcode ) << 2 ) - 4;
				branch = TRUE;
				if( logBranches ) {
					INFO( "BGTZ to 0x%.8X", pc );
				}
			}
			break;
			
		case BLEZ:
			if( (s32)inregs[GET_RS( opcode )] <= 0 ) {
				pc = (s32)pc + ( GET_IMM( opcode ) << 2 ) - 4;
				branch = TRUE;
				if( logBranches ) {
					INFO( "BLEZ to 0x%.8X", pc );
				}
			}
			break;
			
		case COP0:
			switch( GET_RS( opcode ) ) {
				case MTC:
					c0regs.r[GET_RD( opcode )] = inregs[GET_RT( opcode )];
					break;
					
				case MFC:
					regs[GET_RT( opcode )] = c0regs.r[GET_RD( opcode )];
					break;
					
				case COP_SPECIAL:
					switch( GET_FN( opcode ) ) {
						case RFE:
							c0regs.sr.raw |= ( c0regs.sr.raw & 0x3F ) >> 2;
							break;
						
						default:
							printf( "pc: 0x%.8X\n", currentpc );
							ERR( "Unrecognised coprocessor special op: %d, 0x%.8X", GET_FN( opcode ), opcode );
					}
					break;
					
				default:
					printf( "pc: 0x%.8X\n", currentpc );
					ERR( "Unrecognised coprocessor op: %d, 0x%.8X", GET_RS( opcode ), opcode );
			}
			break;
			
		case COP1:
			except( 0x0B );
			break;
			
		case COP2:
			ERR( "GTE op\n" );
			break;
			
		case COP3:
			except( 0x0B );
			break;
			
		case ADDI:
			if( __builtin_add_overflow( (s32)inregs[GET_RS( opcode )], GET_IMM( opcode ), (s32*)&regs[GET_RT( opcode )] ) ) {
				except( 0x0C );
			}
			break;
			
		case SLTI:
			regs[GET_RT( opcode )] = (s32)inregs[GET_RS( opcode )] < GET_IMM( opcode );
			break;
			
		case SLTIU:
			regs[GET_RT( opcode )] = inregs[GET_RS( opcode )] < GET_IMM( opcode );
			break;
			
		default:
			// except( 0x0A );
			
			for( int i = 0; i < 32; i++ ) {
				printf( "r%-2d: 0x%.8X", i, inregs[i] );
				if( ( i + 1 ) % 4 == 0 ) {
					printf( "\n" );
				} else {
					printf( "  " );
				}
			}
			
			if( opcode == 0x00000000 ) {
				printf( "0x%.8X: NOP, 0x%.8X\n", currentpc, opcode );
			} else if( op == 0x00 ) {
				printf( "0x%.8X: %s, 0x%.8X\n", currentpc, specialNames[GET_FN( opcode )], opcode );
			} else {
				printf( "0x%.8X: %s, 0x%.8X\n", currentpc, opcodeNames[op], opcode );
			}
			
			ERR( "Unrecognised op: %s, 0x%.8X", opcodeNames[op], opcode );
			break;
	}
}

u32 opcode;
u32 nextOpcode;

void setupCPU( void ) {
	bios = loadFile( "bios/SCPH1001.BIN" );
	
	memset( regs, 0, sizeof( u32[32] ) );
	hi = 0;
	lo = 0;
	pc = 0xBFC00000;
	
	memset( &c0regs, 0, sizeof( u32[32] ) );
	c0regs.sr.bev = TRUE;
	c0regs.sr.ts = TRUE;
	c0regs.prid = 0x02;
	
	dmaRegs.ctrl = 0x07654321;
	
	opcode = 0x00;
	nextOpcode = 0x00;
	
	//loadEXE( "tests/PSXTEST.EXE" );
}

void step( int clocks ) {
	while( clocks > 0 ) {
		if( exception ) {
			opcode = load( pc, INSTRUCTION );
			currentpc = pc;
			pc += 4;
			nextOpcode = load( pc, INSTRUCTION );
			nextpc = pc;
			exception = FALSE;
		} else {
			opcode = nextOpcode;
			currentpc = nextpc;
			nextOpcode = load( pc, INSTRUCTION );
			nextpc = pc;
		}
		
		//printf( "fetch: 0x%.8X from 0x%.8X\n", nextOpcode, pc );
		pc += 4;
		
		branchSlot = branch;
		branch = FALSE;
		
		execute( opcode );
		
		processDMA();
		
		clocks--;
	}
}
