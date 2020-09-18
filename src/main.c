#include "gl.h"
#include "mips.h"
#include "ui.h"
#include "gpu.h"
#include "util.h"
#include "cdrom.h"
#include "joypad.h"
#include "dma.h"

int halt = FALSE;

int hz = 44100 * 0x300;
int ntscLines = 263;
int palLines = 314;

int main( int argc, char* argv[] ) {
	setupGL();
	setupUI();
	setupCPU();
	setupCd();
	
	while( TRUE ) {
		if( !halt ) {
			for( int i = 0; i < ntscLines; i++ ) {
				/* breaks halt/cpubreak
				for( int j = 0; j < 4; j++ ) {
					step( hz / 60 / ntscLines / 4 );
					
					cdUpdate();
					joyPadUpdate();
					//dmaUpdate();
				}
				*/
				step( hz / 60 / ntscLines );
				
				cdUpdate();
				joyPadUpdate();
				
				if( !gpuStatus.interlace ) {
					if( i < gpuState.lineStart || i > gpuState.lineEnd ) {
						gpuStatus.field = 0;
					} else {
						gpuStatus.field = !gpuStatus.field;
					}
					gpuStatus.interlaceField = 1;
				}
				
				if( i == gpuState.lineEnd ) {
					SET_BIT( istatus, 0 );
				}
				
				if( halt ) {
					break;
				}
			}
		}
		
		update();
		
		if( gpuStatus.interlace ) {
			gpuStatus.field = !gpuStatus.field;
		}
		
		//saveVram();
	}
	
	return 0;
}
