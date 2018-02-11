#ifndef _UTIL_H
#define _UTIL_H

#include <stdio.h>
#include <stdlib.h>

#define ERR( ... ) { fprintf( stderr, "Error: " ); fprintf( stderr, __VA_ARGS__ ); fprintf( stderr, "\n" ); exit( -1 ); }
#define INFO( ... ) { printf( "Info: " ); printf( __VA_ARGS__ ); printf( "\n" ); fflush( stdout ); }

#define GET_BIT( n, b ) ( ( n >> b ) & 0x01 )

void* loadFile( const char* filename );

#endif
