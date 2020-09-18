#include <stdio.h>

#include "gl.h"
#include "util.h"
#include "ui.h"

GLFWwindow* window;

typedef struct {
	char* name;
	int stride;
	u64 offset;
	int normalize;
	int count;
	GLenum type;
} shaderAttrib_t;

shaderAttrib_t attribs[] = {
	{ "vPos", sizeof( vertex ), 0, GL_FALSE, 2, GL_FLOAT },
	{ "vCol", sizeof( vertex ), sizeof( float ) * 2, GL_TRUE, 4, GL_UNSIGNED_BYTE }
};

shaderAttrib_t texAttribs[] = {
	{ "vPos", sizeof( texturedVertex ), 0, GL_FALSE, 2, GL_FLOAT },
	{ "vTexPos", sizeof( texturedVertex ), sizeof( float ) * 2, GL_FALSE, 2, GL_FLOAT },
	{ "vCol", sizeof( texturedVertex ), sizeof( float ) * 4, GL_TRUE, 4, GL_UNSIGNED_BYTE }
};

const char* vertexShaderSrc = 
	"#version 450 core\n"
	"in vec2 vPos;\n"
	"in vec4 vCol;\n"
	"out vec4 colour;\n"
	"void main() {\n"
	"    gl_Position = vec4( vPos, 0.0, 1.0 );\n"
	"    colour = vCol;\n"
	"}\n";
	
const char* fragmentShaderSrc =
	"#version 450 core\n"
	"in vec4 colour;\n"
	"out vec4 frag;\n"
	"void main() {\n"
	"    frag = colour;\n"
	"}\n";
	
const char* texVertexShaderSrc = 
	"#version 450 core\n"
	"in vec2 vPos;\n"
	"in vec2 vTexPos;\n"
	"in vec4 vCol;\n"
	"out vec2 texPos;\n"
	"out vec4 colour;\n"
	"void main() {\n"
	"    gl_Position = vec4( vPos, 0.0, 1.0 );\n"
	"    texPos = vTexPos;\n"
	"    colour = vCol;\n"
	"}\n";
	
const char* texFragmentShaderSrc =
	"#version 450 core\n"
	"in vec2 texPos;\n"
	"in vec4 colour;\n"
	"out vec4 frag;\n"
	"uniform usampler2D tex;\n"
	"void main() {\n"
	"    uint packedRGB = texture( tex, texPos ).r;\n"
	"    uint ir = packedRGB & 0x1F;\n"
	"    uint ig = ( packedRGB >> 5 ) & 0x1F;\n"
	"    uint ib = ( packedRGB >> 10 ) & 0x1F;\n"
	"    frag = vec4( float( ir ) / 31.0f, float( ig ) / 31.0f, float( ib ) / 31.0f, 1.0f );\n"
	"    frag *= colour;\n"
//	"    frag = vec4( texture( tex, texPos ).rrr / 255.0f, 1.0f ) * colour;\n"
	"}\n";

void logShader( GLuint shader ) {
	GLint success = 0;
	glGetShaderiv( shader, GL_COMPILE_STATUS, &success );
	GLint logSize = 0;
	glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &logSize );
	char* buf = malloc( logSize );
	glGetShaderInfoLog( shader, logSize, &logSize, buf );
	if( logSize > 0 ) {
		printf( "%.*s\n", logSize, buf );
	}
	free( buf );
	if( success == GL_FALSE ) {
		exit( -1 );
	}
}

GLuint loadShader( GLuint* outVao, const char* vertSrc, const char* fragSrc, const shaderAttrib_t* attribs, const int numAttribs ) {
	GLuint shaderProgram;
	
	if( outVao != NULL ) {
		glGenVertexArrays( 1, outVao );
		glBindVertexArray( *outVao );
	}
	
	GLuint vertexShader = glCreateShader( GL_VERTEX_SHADER );
    glShaderSource( vertexShader, 1, &vertSrc, NULL );
    glCompileShader( vertexShader );
    
    logShader( vertexShader );
    
    GLuint fragmentShader = glCreateShader( GL_FRAGMENT_SHADER );
    glShaderSource( fragmentShader, 1, &fragSrc, NULL );
    glCompileShader( fragmentShader );
    
    logShader( fragmentShader );
    
    shaderProgram = glCreateProgram();
    glAttachShader( shaderProgram, vertexShader );
    glAttachShader( shaderProgram, fragmentShader );
    glLinkProgram( shaderProgram );
    glUseProgram( shaderProgram );
    
    GLint attribLocs[numAttribs];
    
    for( int i = 0; i < numAttribs; i++ ) {
		attribLocs[i] = glGetAttribLocation( shaderProgram, attribs[i].name );
		glEnableVertexAttribArray( attribLocs[i] );
		glVertexAttribPointer( attribLocs[i], attribs[i].count, attribs[i].type, attribs[i].normalize, attribs[i].stride, (void*)attribs[i].offset );
	}
    
    return shaderProgram;
}

GLuint vbo;
GLuint vao;
GLuint texVAO;

GLuint program;
GLuint texProgram;

GLuint vramTex;

void setupGL( void ) {
	glfwInit();
	
	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
	glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
	window = glfwCreateWindow( 1024, 512, "LPSX", NULL, NULL );
	glfwMakeContextCurrent( window );
	glfwSwapInterval( 1 );
	gladLoadGLLoader( (GLADloadproc)glfwGetProcAddress );
	
	glClear( GL_COLOR_BUFFER_BIT );
	
	glGenBuffers( 1, &vbo );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );
	
	program = loadShader( &vao, vertexShaderSrc, fragmentShaderSrc, attribs, 2 );
	texProgram = loadShader( &texVAO, texVertexShaderSrc, texFragmentShaderSrc, texAttribs, 3 );
	glUniform1i( glGetUniformLocation( texProgram, "tex" ), 0 );
    
	glGenTextures( 1, &vramTex );
	glBindTexture( GL_TEXTURE_2D, vramTex );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_R16UI, 1024, 512, 0, GL_RED_INTEGER, GL_UNSIGNED_SHORT, NULL );
}

void vramUpload( void* data, int x, int y, int w, int h ) {
	glBindTexture( GL_TEXTURE_2D, vramTex );
	//printf( "gl: %d\n", glGetError() );
	glTexSubImage2D( GL_TEXTURE_2D, 0, x, y, w, h, GL_RED_INTEGER, GL_UNSIGNED_SHORT, data );
	//glTexImage2D( GL_TEXTURE_2D, 0, GL_R16UI, 1024, 512, 0, GL_RED, GL_UNSIGNED_SHORT, data );
	//printf( "gl: %d\n", glGetError() );
}

void drawVerts( vertex* verts, int count ) {
	glUseProgram( program );
	glBindBuffer( GL_ARRAY_BUFFER, vbo );
	glBufferData( GL_ARRAY_BUFFER, sizeof( vertex ) * count, verts, GL_STATIC_DRAW );
	glBindVertexArray( vao );
	glDrawArrays( GL_TRIANGLES, 0, count );
}

void drawTexVerts( texturedVertex* verts, int count ) {
	glUseProgram( texProgram );
	glBindTexture( GL_TEXTURE_2D, vramTex );
	glBindBuffer( GL_ARRAY_BUFFER, vbo );
	glBufferData( GL_ARRAY_BUFFER, sizeof( texturedVertex ) * count, verts, GL_STATIC_DRAW );
	glBindVertexArray( texVAO );
	glDrawArrays( GL_TRIANGLES, 0, count );
}

void drawQuad( vertex* verts ) {
	glUseProgram( program );
	glBindBuffer( GL_ARRAY_BUFFER, vbo );
	glBufferData( GL_ARRAY_BUFFER, sizeof( vertex ) * 4, verts, GL_STATIC_DRAW );
	glBindVertexArray( vao );
	glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
}

void drawTexQuad( texturedVertex* verts ) {
	glUseProgram( texProgram );
	glBindTexture( GL_TEXTURE_2D, vramTex );
	glBindBuffer( GL_ARRAY_BUFFER, vbo );
	glBufferData( GL_ARRAY_BUFFER, sizeof( texturedVertex ) * 4, verts, GL_STATIC_DRAW );
	glBindVertexArray( texVAO );
	glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
}

void update( void ) {
	extern u16 vram[512][1024];
	vramUpload( vram, 0, 0, 1024, 512 );
	texturedVertex verts[4] = {
		{ -1,  1,  0, 0,  255, 255, 255, 255 },
		{  1,  1,  1, 0,  255, 255, 255, 255 },
		{ -1, -1,  0, 1,  255, 255, 255, 255 },
		{  1, -1,  1, 1,  255, 255, 255, 255 }
	};
	drawTexQuad( verts );
	
	updateUI();
	
	glfwSwapBuffers( window );
	glClear( GL_COLOR_BUFFER_BIT );
	glfwPollEvents();
	
	if( glfwWindowShouldClose( window ) ) {
		exit( 0 );
	}
}
