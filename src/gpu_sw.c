#include <string.h>

#include "types.h"
#include "gpu.h"
#include "util.h"

gpuStatus_t gpuStatus = { .raw = 0x1C000000 };
gpuState_t gpuState;

union {
	u8 b[512][2048];
	u16 h[512][1024];
	u32 w[512][512];
} vram;

u32 gpfifo[16];
int fifoLen = 0;

typedef struct {
	s16 x, y;
} s16x2;

typedef struct {
	s16 x, y, z;
} s16x3;

int min( int a, int b ) {
	return a < b ? a : b;
}

int max( int a, int b ) {
	return a > b ? a : b;
}

void rasterRect( u16 x, u16 y, u16 w, u16 h, s16x3 c );
void rasterTri( s16x2 v0, s16x3 c0, s16x2 v1, s16x3 c1, s16x2 v2, s16x3 c2 );

u16 rgb24to15( u32 col ) {
	return ( ( col & 0x0000FF ) >> 3 ) | ( ( col & 0x00F800 ) >> 6 ) | ( ( col & 0xF80000 ) >> 9 );
}

u16 packRGB8( u16 r, u16 g, u16 b ) {
	return ( r >> 3 ) | ( ( g & 0xF8 ) << 2 ) | ( ( b & 0xF8 ) << 7 );
}

s16x3 unpackRGB( u32 w ) {
	s16x3 c;
	c.x = w & 0xFF;
	c.y = ( w >> 8 ) & 0xFF;
	c.z = ( w >> 16 ) & 0xFF;
	return c;
}

int rx = 0;
int ry = 0;
int rw = 0;
int rh = 0;
int rcx = 0;
int rcy = 0;

u32 GPURD = 0;

u32 gpuRead( void ) {
	if( rcy != ry + rh ) {
		GPURD = vram.w[rcy][rcx >> 1];
		rcx += 2;
		if( rcx == rx + rw ) {
			rcx = rx;
			rcy++;
		}
	}
	
	return GPURD;
}

int tx = 0;
int ty = 0;
int tw = 0;
int th = 0;
int tcx = 0;
int tcy = 0;

void gp0( u32 cmd ) {
	if( tcy != ty + th ) {
		vram.w[tcy][tcx >> 1] = cmd;
		//INFO( "vram write %d,%d %.8X", tcx, tcy, cmd );
		tcx += 2;
		if( tcx == tx + tw ) {
			tcx = tx;
			tcy++;
		}
		return;
	}
	
	gpfifo[fifoLen++] = cmd;
	
	u32 op = gpfifo[0] >> 24;
	
	switch( op ) {
		case NOP:
			break;
			
		case CLEAR_TEXCACHE:
			break;
		
		case FILL_RECT: {
			if( fifoLen < 3 ) {
				return;
			}
			
			u32 x = gpfifo[1] & 0xFFF0;
			u32 y = gpfifo[1] >> 16;
			u32 w = gpfifo[2] & 0xFFF0;
			u32 h = gpfifo[2] >> 16;
			
			u16 col = rgb24to15( gpfifo[0] );
			
			//INFO( "vram fill %d,%d %dx%d %.4X", x, y, w, h, col );
			
			for( int cy = y; cy < y + h; cy++ ) {
				for( int cx = x; cx < x + w; cx++ ) {
					vram.h[cy][cx] = col;
				}
			}
			break;
		}
		
		case FLAT_QUAD: {
			if( fifoLen < 5 ) {
				return;
			}
			
			u32 x1 = gpfifo[1] & 0xFFFF;
			u32 y1 = gpfifo[1] >> 16;
			u32 x2 = gpfifo[2] & 0xFFFF;
			u32 y2 = gpfifo[2] >> 16;
			u32 x3 = gpfifo[3] & 0xFFFF;
			u32 y3 = gpfifo[3] >> 16;
			u32 x4 = gpfifo[4] & 0xFFFF;
			u32 y4 = gpfifo[4] >> 16;
			
			u32 col = gpfifo[0] & 0x00FFFFFF;
			s16x3 c = unpackRGB( col );
			
			//WARN( "TEXTURED_QUAD %d,%d %d,%d %d,%d %d,%d %.6X", x1, y1, x2, y2, x3, y3, x4, y4, col );
			
			rasterTri( (s16x2){ x1, y1 }, c, (s16x2){ x2, y2 }, c, (s16x2){ x3, y3 }, c );
			rasterTri( (s16x2){ x3, y3 }, c, (s16x2){ x2, y2 }, c, (s16x2){ x4, y4 }, c );
			
			break;
		}
		
		case FLAT_TEXTURED_QUAD: {
			if( fifoLen < 9 ) {
				return;
			}
			
			u32 x1 = gpfifo[1] & 0xFFFF;
			u32 y1 = gpfifo[1] >> 16;
			u32 x2 = gpfifo[3] & 0xFFFF;
			u32 y2 = gpfifo[3] >> 16;
			u32 x3 = gpfifo[5] & 0xFFFF;
			u32 y3 = gpfifo[5] >> 16;
			u32 x4 = gpfifo[7] & 0xFFFF;
			u32 y4 = gpfifo[7] >> 16;
			
			u32 col = gpfifo[0] & 0x00FFFFFF;
			s16x3 c = unpackRGB( col );
			
			//WARN( "TEXTURED_QUAD %d,%d %d,%d %d,%d %d,%d %.6X", x1, y1, x2, y2, x3, y3, x4, y4, col );
			
			rasterTri( (s16x2){ x1, y1 }, c, (s16x2){ x2, y2 }, c, (s16x2){ x3, y3 }, c );
			rasterTri( (s16x2){ x3, y3 }, c, (s16x2){ x2, y2 }, c, (s16x2){ x4, y4 }, c );
			
			break;
		}
		
		case SHADED_TRI: {
			if( fifoLen < 6 ) {
				return;
			}
			
			u32 x1 = gpfifo[1] & 0xFFFF;
			u32 y1 = gpfifo[1] >> 16;
			u32 x2 = gpfifo[3] & 0xFFFF;
			u32 y2 = gpfifo[3] >> 16;
			u32 x3 = gpfifo[5] & 0xFFFF;
			u32 y3 = gpfifo[5] >> 16;
			
			s16x3 c1 = unpackRGB( gpfifo[0] & 0x00FFFFFF );
			s16x3 c2 = unpackRGB( gpfifo[2] & 0x00FFFFFF );
			s16x3 c3 = unpackRGB( gpfifo[4] & 0x00FFFFFF );
			
			//WARN( "SHADED_TRI %.6X", c1 );
			
			rasterTri( (s16x2){ x1, y1 }, c1, (s16x2){ x2, y2 }, c2, (s16x2){ x3, y3 }, c3 );
			
			break;
		}
		
		case SHADED_QUAD: {
			if( fifoLen < 8 ) {
				return;
			}
			
			u32 x1 = gpfifo[1] & 0xFFFF;
			u32 y1 = gpfifo[1] >> 16;
			u32 x2 = gpfifo[3] & 0xFFFF;
			u32 y2 = gpfifo[3] >> 16;
			u32 x3 = gpfifo[5] & 0xFFFF;
			u32 y3 = gpfifo[5] >> 16;
			u32 x4 = gpfifo[7] & 0xFFFF;
			u32 y4 = gpfifo[7] >> 16;
			
			s16x3 c1 = unpackRGB( gpfifo[0] & 0x00FFFFFF );
			s16x3 c2 = unpackRGB( gpfifo[2] & 0x00FFFFFF );
			s16x3 c3 = unpackRGB( gpfifo[4] & 0x00FFFFFF );
			s16x3 c4 = unpackRGB( gpfifo[6] & 0x00FFFFFF );
			
			//WARN( "SHADED_QUAD %.6X", c1 );
			
			rasterTri( (s16x2){ x1, y1 }, c1, (s16x2){ x2, y2 }, c2, (s16x2){ x3, y3 }, c3 );
			rasterTri( (s16x2){ x3, y3 }, c3, (s16x2){ x2, y2 }, c2, (s16x2){ x4, y4 }, c4 );
			
			break;
		}
		
		case DRAW_RECT: {
			if( fifoLen < 4 ) {
				return;
			}
			
			u32 x = gpfifo[1] & 0xFFFF;
			u32 y = gpfifo[1] >> 16;
			u32 tx = gpfifo[2] & 0xFF;
			u32 ty = ( gpfifo[2] >> 8 ) & 0xFF;
			u32 clut = gpfifo[2] >> 16;
			u32 w = gpfifo[3] & 0xFFFF;
			u32 h = gpfifo[3] >> 16;
			
			s16x3 c = unpackRGB( gpfifo[0] & 0x00FFFFFF );
			
			rasterRect( x, y, w, h, c );
			
			break;
		}
		
		case TRANSFER_TO_VRAM: {
			if( fifoLen < 3 ) {
				return;
			}
			
			u32 x = gpfifo[1] & 0xFFFE;
			u32 y = gpfifo[1] >> 16;
			u32 w = gpfifo[2] & 0xFFFE;
			u32 h = gpfifo[2] >> 16;
			
			//INFO( "vram write %d,%d %dx%d", x, y, w, h );
			
			tx = x;
			ty = y;
			tw = w;
			th = h;
			tcx = x;
			tcy = y;
			break;
		}
		
		case TRANSFER_FROM_VRAM: {
			if( fifoLen < 3 ) {
				return;
			}
			
			u32 x = gpfifo[1] & 0xFFFE;
			u32 y = gpfifo[1] >> 16;
			u32 w = gpfifo[2] & 0xFFFE;
			u32 h = gpfifo[2] >> 16;
			
			rx = x;
			ry = y;
			rw = w;
			rh = h;
			rcx = x;
			rcy = y;
			
			//ERR( "vram read %d,%d %dx%d", x, y, w, h );
			break;
		}
		
		case DRAW_MODE:
			gpuStatus.raw = ( gpuStatus.raw & 0xFFFFFC00 ) | ( gpfifo[0] & 0x03FF );
			gpuStatus.texDisable = GET_BIT( gpfifo[0], 11 );
			gpuState.texFlipX = GET_BIT( gpfifo[0], 12 );
			gpuState.texFlipY = GET_BIT( gpfifo[0], 13 );
			break;
			
		case SET_TEX_WIN:
			gpuState.texWinXMask = GET_BITS( gpfifo[0], 0, 5 );
			gpuState.texWinYMask = GET_BITS( gpfifo[0], 5, 5 );
			gpuState.texWinX = GET_BITS( gpfifo[0], 10, 5 );
			gpuState.texWinY = GET_BITS( gpfifo[0], 15, 5 );
			break;
			
		case SET_CLIP1:
			gpuState.clipX1 = GET_BITS( gpfifo[0], 0, 10 );
			gpuState.clipY1 = GET_BITS( gpfifo[0], 10, 10 );
			//INFO( "set clip 1 %d,%d", gpuState.clipX1, gpuState.clipY1 );
			break;
			
		case SET_CLIP2:
			gpuState.clipX2 = GET_BITS( gpfifo[0], 0, 10 );
			gpuState.clipY2 = GET_BITS( gpfifo[0], 10, 10 );
			//INFO( "set clip 2 %d,%d", gpuState.clipX2, gpuState.clipY2 );
			break;
			
		case SET_DRAW: {
			u16 x = GET_BITS( gpfifo[0], 0, 11 );
			u16 y = GET_BITS( gpfifo[0], 11, 11 );
			gpuState.drawX = (s16)( x << 5 ) >> 5;
			gpuState.drawY = (s16)( y << 5 ) >> 5;
			//INFO( "set draw origin %d,%d", gpuState.drawX, gpuState.drawY );
			break;
		}
		
		case SET_MASK: {
			gpuStatus.forceWriteMask = GET_BIT( gpfifo[0], 0 );
			gpuStatus.checkMask = GET_BIT( gpfifo[0], 1 );
			break;
		}
		
		default:
			ERR( "Unknown gp0 command 0x%.8X\n", gpfifo[0] );
			break;
	}
	
	fifoLen = 0;
}

void gp1( u32 cmd ) {
	u32 op = cmd >> 24;
	//INFO( "GP1: 0x%.2X, pc: 0x%.8X", op, currentpc );
	
	switch( op ) {
		case RESET:
			memset( &gpuStatus, 0, sizeof( gpuStatus_t ) );
			memset( &gpuState, 0, sizeof( gpuState_t ) );
			
			// always ready for dma transfer
			gpuStatus.raw = 0x1C000000;
			
			gpuStatus.displayDisable = TRUE;
			gpuStatus.interlace = TRUE;
			gpuState.pixelStart = 0x0200;
			gpuState.pixelEnd = 0x0C00;
			gpuState.lineStart = 0x10;
			gpuState.lineEnd = 0x0100;
			break;
			
		case RESET_CMD_BUFFER:
			break;
			
		case IRQ_ACK:
			break;
		
		case DISPLAY_ENABLE:
			break;
			
		case DMA_DIR:
			gpuStatus.DMADir = cmd & 0x03;
			break;
			
		case SET_DISPLAY:
			gpuState.displayX = cmd & 0x03FE;
			gpuState.displayY = ( cmd >> 10 ) & 0x01FF;
			break;
			
		case DISPLAY_MODE:
			gpuStatus.hres = cmd & 0x03;
			gpuStatus.vres = GET_BIT( cmd, 2 );
			gpuStatus.pal = GET_BIT( cmd, 3 );
			gpuStatus.outDepth = GET_BIT( cmd, 4 );
			gpuStatus.interlace = GET_BIT( cmd, 5 );
			gpuStatus.hres2 = GET_BIT( cmd, 6 );
			break;
			
		case HRANGE:
			gpuState.pixelStart = cmd & 0x0FFF;
			gpuState.pixelEnd = ( cmd >> 12 ) & 0x0FFF;
			break;
			
		case VRANGE:
			gpuState.lineStart = cmd & 0x03FF;
			gpuState.lineEnd = ( cmd >> 10 ) & 0x03FF;
			break;
			
		default:
			ERR( "Unknown gp1 command 0x%.8X\n", cmd );
			break;
	}
}

s8 ditherLut[4][4] = {
	{ -4, +0, -3, +1 },
	{ +2, -2, +3, -1 },
	{ -3, +1, -4, +0 },
	{ +3, -1, +2, -2 }
};

static inline void ROP( int x, int y, int r, int g, int b, int ditherEnable ) {
	if( gpuStatus.dither && ditherEnable ) {
		int dither = ditherLut[x & 3][y & 3];
		r += dither;
		g += dither;
		b += dither;
	}
	
	r = min( max( r, 0 ), 255 );
	g = min( max( g, 0 ), 255 );
	b = min( max( b, 0 ), 255 );
	
	vram.h[y][x] = packRGB8( r, g, b );
}

void rasterRect( u16 x, u16 y, u16 w, u16 h, s16x3 c ) {
	for( int py = y; py < y + h; py++ ) {
		for( int px = x; px < x + w; px++ ) {
			ROP( x, y, c.x, c.y, c.z, FALSE );
		}
	}
}

static inline s32 edge2D( s16x2 a, s16x2 b, s16x2 c ) {
	return (s32)( b.x - a.x ) * (s32)( c.y - a.y ) - (s32)( b.y - a.y ) * (s32)( c.x - a.x );
}

void rasterTri( s16x2 v0, s16x3 c0, s16x2 v1, s16x3 c1, s16x2 v2, s16x3 c2 ) {
	s16 minX = min( min( v0.x, v1.x ), v2.x );
	s16 maxX = max( max( v0.x, v1.x ), v2.x );
	s16 minY = min( min( v0.y, v1.y ), v2.y );
	s16 maxY = max( max( v0.y, v1.y ), v2.y );
	
	minX = max( minX, 0 );
	minY = max( minY, 0 );
	maxX = min( maxX, 1023 );
	maxY = min( maxY, 511 );
	
	s32 t = edge2D( v0, v1, v2 );
	
	if( t == 0 ) {
		return;
	}
	
	s16x2 p;
	for( p.y = minY; p.y <= maxY; p.y++ ) {
		for( p.x = minX; p.x <= maxX; p.x++ ) {
			s32 w0 = edge2D( v1, v2, p );
			s32 w1 = edge2D( v2, v0, p );
			s32 w2 = edge2D( v0, v1, p );
			
			int f = w0 >= 0 && w1 >= 0 && w2 >= 0; // front fance
			int b = w0 <= 0 && w1 <= 0 && w2 <= 0; // back face
			
			if( f || b ) {
				s16 r = ( w0 * c0.x + w1 * c1.x + w2 * c2.x ) / t;
				s16 g = ( w0 * c0.y + w1 * c1.y + w2 * c2.y ) / t;
				s16 b = ( w0 * c0.z + w1 * c1.z + w2 * c2.z ) / t;
				
				// tex
				
				ROP( p.x, p.y, r, g, b, TRUE );
			}
		}
	}
}

/*
void rasterTri( u32 x1, u32 y1, u32 c1, u32 x2, u32 y2, u32 c2, u32 x3, u32 y3, u32 c3 ) {
	x1 <<= 10;
	y1 <<= 10;
	x2 <<= 10;
	y2 <<= 10;
	x3 <<= 10;
	y3 <<= 10;
	
	if( y1 > y2 ) {
		SWAP( x1, x2 );
		SWAP( y1, y2 );
		SWAP( c1, c2 );
	}
	
	if( y2 > y3 ) {
		SWAP( x2, x3 );
		SWAP( y2, y3 );
		SWAP( c2, c3 );
	}
	
	if( y1 > y2 ) {
		SWAP( x1, x2 );
		SWAP( y1, y2 );
		SWAP( c1, c2 );
	}
	
	u32 r1 = ( c1 & 0x0000FF ) << 10;
	u32 g1 = ( c1 & 0x00FF00 ) << 2;
	u32 b1 = ( c1 & 0xFF0000 ) >> 6;
	
	u32 r2 = ( c2 & 0x0000FF ) << 10;
	u32 g2 = ( c2 & 0x00FF00 ) << 2;
	u32 b2 = ( c2 & 0xFF0000 ) >> 6;
	
	u32 r3 = ( c3 & 0x0000FF ) << 10;
	u32 g3 = ( c3 & 0x00FF00 ) << 2;
	u32 b3 = ( c3 & 0xFF0000 ) >> 6;
	
	u32 py1 = y1 >> 10;
	u32 py2 = y2 >> 10;
	u32 py3 = y3 >> 10;
	
	s32 dx12 = 0;
	s32 dx23 = 0;
	s32 dx13 = 0;
	
	s32 dr12 = 0;
	s32 dg12 = 0;
	s32 db12 = 0;
	
	s32 dr23 = 0;
	s32 dg23 = 0;
	s32 db23 = 0;
	
	s32 dr13 = 0;
	s32 dg13 = 0;
	s32 db13 = 0;
	
	if( py1 != py2 ) {
		dx12 = (s32)( x2 - x1 ) / (s32)( py2 - py1 );
		dr12 = (s32)( r2 - r1 ) / (s32)( py2 - py1 );
		dg12 = (s32)( g2 - g1 ) / (s32)( py2 - py1 );
		db12 = (s32)( b2 - b1 ) / (s32)( py2 - py1 );
	}
	if( py2 != py3 ) {
		dx23 = (s32)( x3 - x2 ) / (s32)( py3 - py2 );
		dr23 = (s32)( r3 - r2 ) / (s32)( py3 - py2 );
		dg23 = (s32)( g3 - g2 ) / (s32)( py3 - py2 );
		db23 = (s32)( b3 - b2 ) / (s32)( py3 - py2 );
	}
	if( py1 != py3 ) {
		dx13 = (s32)( x3 - x1 ) / (s32)( py3 - py1 );
		dr13 = (s32)( r3 - r1 ) / (s32)( py3 - py1 );
		dg13 = (s32)( g3 - g1 ) / (s32)( py3 - py1 );
		db13 = (s32)( b3 - b1 ) / (s32)( py3 - py1 );
	}
	
	u32 ix1 = x1 + dx12 / 2;
	u32 ix2 = x1 + dx13 / 2;
	
	u32 ir1 = r1 + dr12 / 2;
	u32 ig1 = g1 + dg12 / 2;
	u32 ib1 = b1 + db12 / 2;
	
	u32 ir2 = r1 + dr13 / 2;
	u32 ig2 = g1 + dg13 / 2;
	u32 ib2 = b1 + db13 / 2;
	
	if( py1 == py2 ) {
		ix1 = x2;
		ix2 = x1;
		
		ir1 = r2;
		ig1 = g2;
		ib1 = b2;
		
		ir2 = r1;
		ig2 = g1;
		ib2 = b1;
	}
	
	for( u32 y = py1; y < py2; y++ ) {
		u32 lx1 = ix1 >> 10;
		u32 lx2 = ix2 >> 10;
		
		u32 lr1 = ir1;
		u32 lg1 = ig1;
		u32 lb1 = ib1;
		
		u32 lr2 = ir2;
		u32 lg2 = ig2;
		u32 lb2 = ib2;
		
		if( lx1 > lx2 ) {
			SWAP( lx1, lx2 );
			SWAP( lr1, lr2 );
			SWAP( lg1, lg2 );
			SWAP( lb1, lb2 );
		}
		
		s32 dlr = 0;
		s32 dlg = 0;
		s32 dlb = 0;
		
		if( lx1 != lx2 ) {
			dlr = (s32)( lr2 - lr1 ) / (s32)( lx2 - lx1 );
			dlg = (s32)( lg2 - lg1 ) / (s32)( lx2 - lx1 );
			dlb = (s32)( lb2 - lb1 ) / (s32)( lx2 - lx1 );
		}
		
		u32 cr = lr1;
		u32 cg = lg1;
		u32 cb = lb1;
		
		for( u32 x = lx1; x < lx2; x++ ) {
			u32 pr = cr >> 10;
			u32 pg = cg >> 10;
			u32 pb = cb >> 10;
			if( gpuStatus.dither ) {
				pr += ditherLut[y%4][x%4];
				pg += ditherLut[y%4][x%4];
				pb += ditherLut[y%4][x%4];
				if( pr >> 31 ) { pr = 0; }
				if( pg >> 31 ) { pg = 0; }
				if( pb >> 31 ) { pb = 0; }
				if( pr > 255 ) { pr = 255; }
				if( pg > 255 ) { pg = 255; }
				if( pb > 255 ) { pb = 255; }
			}
			vram.h[y][x] = packRGB8( pr, pg, pb );
			cr += dlr;
			cg += dlg;
			cb += dlb;
		}
		
		ix1 += dx12;
		ix2 += dx13;
		
		ir1 += dr12;
		ig1 += dg12;
		ib1 += db12;
		
		ir2 += dr13;
		ig2 += dg13;
		ib2 += db13;
	}
	
	//ix1 -= dx12 / 2;
	//ix1 += dx23 / 2;
	ix1 -= dx12;
	//ix1 += dx23;
	
	ir1 -= dr12;
	ig1 -= dg12;
	ib1 -= db12;
	
	for( u32 y = py2; y < py3; y++ ) {
		u32 lx1 = ix1 >> 10;
		u32 lx2 = ix2 >> 10;
		
		u32 lr1 = ir1;
		u32 lg1 = ig1;
		u32 lb1 = ib1;
		
		u32 lr2 = ir2;
		u32 lg2 = ig2;
		u32 lb2 = ib2;
		
		if( lx1 > lx2 ) {
			SWAP( lx1, lx2 );
			SWAP( lr1, lr2 );
			SWAP( lg1, lg2 );
			SWAP( lb1, lb2 );
		}
		
		s32 dlr = 0;
		s32 dlg = 0;
		s32 dlb = 0;
		
		if( lx1 != lx2 ) {
			dlr = (s32)( lr2 - lr1 ) / (s32)( lx2 - lx1 );
			dlg = (s32)( lg2 - lg1 ) / (s32)( lx2 - lx1 );
			dlb = (s32)( lb2 - lb1 ) / (s32)( lx2 - lx1 );
		}
		
		u32 cr = lr1;
		u32 cg = lg1;
		u32 cb = lb1;
		
		for( u32 x = lx1; x < lx2; x++ ) {
			u32 pr = cr >> 10;
			u32 pg = cg >> 10;
			u32 pb = cb >> 10;
			if( gpuStatus.dither ) {
				pr += ditherLut[y%4][x%4];
				pg += ditherLut[y%4][x%4];
				pb += ditherLut[y%4][x%4];
				if( pr >> 31 ) { pr = 0; }
				if( pg >> 31 ) { pg = 0; }
				if( pb >> 31 ) { pb = 0; }
				if( pr > 255 ) { pr = 255; }
				if( pg > 255 ) { pg = 255; }
				if( pb > 255 ) { pb = 255; }
			}
			vram.h[y][x] = packRGB8( pr, pg, pb );
			cr += dlr;
			cg += dlg;
			cb += dlb;
		}
		
		ix1 += dx23;
		ix2 += dx13;
		
		ir1 += dr23;
		ig1 += dg23;
		ib1 += db23;
		
		ir2 += dr13;
		ig2 += dg13;
		ib2 += db13;
	}
}
*/
