#ifndef _UTIL_H
#define _UTIL_H

#include <stdio.h>
#include <stdlib.h>

#define LOG_WARN
#define LOG_INFO

#define ERR( ... ) { fprintf( stderr, "\e[31mError: " ); fprintf( stderr, __VA_ARGS__ ); fprintf( stderr, "\e[0m\n" ); exit( -1 ); }

#ifdef LOG_WARN
#define WARN( ... ) { printf( "\e[33mWarn: " ); printf( __VA_ARGS__ ); printf( "\e[0m\n" ); fflush( stdout ); }
#else
#define WARN( ... )
#endif

#ifdef LOG_INFO
#define INFO( ... ) { printf( "Info: " ); printf( __VA_ARGS__ ); printf( "\n" ); fflush( stdout ); }
#else
#define INFO( ... )
#endif

#define GET_BIT( n, b ) ( ( n >> b ) & 0x01 )
#define SET_BIT( n, b ) ( n |= 1 << b )

#define GET_BITS( n, s, l ) ( ( n >> s ) & ( ( 1 << l ) - 1 ) )

#define SWAP( a, b ) { typeof(a) t = a; a = b; b = t; }

void* loadFile( const char* filename );

#endif
