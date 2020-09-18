#include "gl.h"
#include "mips.h"
#include "dis.h"
#include "cdrom.h"

#define NK_IMPLEMENTATION
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#include "nuklear.h"
#define NK_GLFW_GL3_IMPLEMENTATION
#include "nuklear_glfw_gl3.h"

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024

extern GLFWwindow* window;

struct nk_context* ctx;

extern u32 load( u32 addr, int type );

void setupUI( void ) {
	ctx = nk_glfw3_init( window, NK_GLFW3_INSTALL_CALLBACKS );
	
	//ctx->style.window.background = nk_rgba_f( 0.2, 0.2, 0.2, 0.5 );
	ctx->style.window.fixed_background.type = NK_STYLE_ITEM_COLOR;
	ctx->style.window.fixed_background.data.color = nk_rgba_f( 0.2, 0.2, 0.2, 0.6 );
	
	ctx->style.window.header.normal.type = NK_STYLE_ITEM_COLOR;
	ctx->style.window.header.normal.data.color = nk_rgba_f( 0.2, 0.2, 0.2, 0.7 );
	
	ctx->style.window.header.hover.type = NK_STYLE_ITEM_COLOR;
	ctx->style.window.header.hover.data.color = nk_rgba_f( 0.2, 0.2, 0.2, 0.7 );
	
	ctx->style.window.header.active.type = NK_STYLE_ITEM_COLOR;
	ctx->style.window.header.active.data.color = nk_rgba_f( 0.2, 0.2, 0.2, 0.7 );
	
	struct nk_font_atlas *atlas;
    nk_glfw3_font_stash_begin( &atlas );
    nk_glfw3_font_stash_end();
}

extern u32 regs[32];
extern int halt;
extern u32 pc;
extern u32 currentpc;
extern u32 loadreg;
extern u32 loadval;

void updateUI( void ) {
	nk_glfw3_new_frame();
	
	if( nk_begin( ctx, "Registers", nk_rect( 489, 5, 530, 165 ), NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_TITLE | NK_WINDOW_NO_SCROLLBAR ) ) {
		char str[256];
		char* p = str;
		for( int i = 0; i < 32; i++ ) {
			if( loadreg != 0 && loadreg == i ) {
				sprintf( p, "r%-2d: 0x%.8X    ", i, loadval );
			} else {
				sprintf( p, "r%-2d: 0x%.8X    ", i, regs[i] );
			}
			
			if( ( i + 1 ) % 4 == 0 ) {
				nk_layout_row_dynamic( ctx, 12, 1 );
				nk_text( ctx, str, strlen( str ), NK_TEXT_LEFT );
				p = str;
				
			} else {
				while( *p != 0 ) {
					p++;
				}
			}
		}
	}
	
	nk_end( ctx );
	
	if( nk_begin( ctx, "Controls", nk_rect( 489, 175, 530, 80 ), NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_TITLE | NK_WINDOW_NO_SCROLLBAR ) ) {
		nk_layout_row_dynamic( ctx, 30, 3 );
		if( nk_button_label( ctx, "break" ) ) {
			halt = TRUE;
		}
		if( nk_button_label( ctx, "step" ) ) {
			halt = TRUE;
			step( 1 );
		}
		if( nk_button_label( ctx, "continue" ) ) {
			halt = FALSE;
		}
	}
	
	nk_end( ctx );
	
	if( nk_begin( ctx, "Disassembly", nk_rect( 489, 260, 530, 245 ), NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_TITLE | NK_WINDOW_NO_SCROLLBAR ) ) {
		//(*ctx->current->layout->offset_y)++;
		for( int i = -6; i <= 6; i++ ) {
			nk_layout_row_dynamic( ctx, 12, 1 );
			
			char buf[256];
			if( i == 0 ) {
				buf[0] = '>';
				buf[1] = ' ';
			} else {
				buf[0] = ' ';
				buf[1] = ' ';
			}
			
			printInstr( buf + 2, 255, pc + i * 4, load( pc + i * 4, 2 ) );
			nk_text( ctx, buf, strlen( buf ), NK_TEXT_LEFT );
		}
	}
	
	nk_end( ctx );
	
	nk_glfw3_render( NK_ANTI_ALIASING_ON, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER );
}
