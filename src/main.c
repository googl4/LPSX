#include "gl.h"
#include "mips.h"
#include "ui.h"

int halt = FALSE;

int main( int argc, char* argv[] ) {
	setupGL();
	setupUI();
	setupCPU();
	
	while( TRUE ) {
		if( !halt ) {
			step( 591900 );
		}
		update();
	}
	
	return 0;
}
