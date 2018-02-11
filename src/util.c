#include <stdio.h>
#include <stdlib.h>

#include "util.h"

void* loadFile( const char* filename ) {
	FILE* f = fopen( filename, "rb" );
	fseek( f, 0, SEEK_END );
	size_t fsize = ftell( f );
	rewind( f );
	void* data = malloc( fsize );
	fread( data, fsize, 1, f );
	return data;
}
