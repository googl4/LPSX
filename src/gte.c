#include "types.h"
#include "util.h"

void gteExec( u32 opcode ) {
	u32 op = opcode & 0x3F;
	u32 lm = GET_BIT( opcode, 10 );
	u32 tv = ( opcode >> 13 ) & 0x03;
	u32 mv = ( opcode >> 15 ) & 0x03;
	u32 mtrx = ( opcode >> 17 ) & 0x03;
	u32 sf = GET_BIT( opcode, 19 );
	
	printf( "gte op: %X, %X, %X\n", opcode, op, ( opcode >> 20 ) & 0x1F );
}
