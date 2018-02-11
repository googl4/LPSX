#ifndef _GL_H
#define _GL_H

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "types.h"

extern GLFWwindow* window;

typedef struct {
	float x, y;
	u8 r, g, b, a;
} vertex;

typedef struct {
	float x, y, u, v;
	u8 r, g, b, a;
} texturedVertex;

void setupGL( void );
void vramUpload( void* data, int x, int y, int w, int h );
void drawVerts( vertex* verts, int count );
void drawQuad( vertex* verts );
void drawTexVerts( texturedVertex* verts, int count );
void drawTexQuad( texturedVertex* verts );
void update( void );

#endif
