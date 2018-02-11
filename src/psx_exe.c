#include "psx_exe.h"
#include "util.h"
#include "bus.h"

extern u32 pc;
extern u32 regs[32];

void loadEXE( char* filename ) {
	EXEheader_t* exe = loadFile( filename );
	printf( "exe id: %.*s\n", 8, exe->id );
	printf( "scei string: %s\n", exe->SCEI );
	printf( "pc: 0x%.8X\n", exe->pc );
	printf( "load address:0x%.8X\n", exe->dst );
	printf( "load size:0x%.8X\n", exe->size );
	
	pc = exe->pc;
	regs[28] = exe->gp;
	regs[29] = exe->sp + exe->spOffset;
	regs[30] = regs[29];
	
	for( int i = 0; i < exe->size; i++ ) {
		store( exe->dst + i, BYTE, exe->data[i] );
	}
}
