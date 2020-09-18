#ifndef _HLEBIOS_H
#define _HLEBIOS_H

#include "types.h"

void bioscall( u32 vector, u32 func, u32* r );

#endif
