#ifndef _BUS_H
#define _BUS_H

#include "types.h"

#define CACHE_LINES 256
#define CACHE_LINE_SIZE 4

typedef union  {
	u32 raw;
	struct {
		u32 word_align:2;
		u32 index:2;
		u32 cache_line:8;
		u32 tag:20;
	};
} cacheAddress;

typedef struct {
	u32 tag;
	u8 invalidated;
	u32 instructions[CACHE_LINE_SIZE];
} cacheLine;

typedef enum {
	BYTE,
	HALFWORD,
	WORD,
	BYTE_SIGNED,
	HALFWORD_SIGNED,
	INSTRUCTION
} accessType;

extern u8* bios;

u32 load( u32 addr, accessType type );
void store( u32 addr, accessType type, u32 n );

#endif
