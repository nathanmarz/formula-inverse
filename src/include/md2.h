#ifndef __MD2_H
#define __MD2_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/gl.h>				// standard OpenGL include
#include <GL/glu.h>				// OpenGL utilties
#include "texture.h"

#define MAX_FRAMES 512


// model animation states
enum modelState_t
{
	MODEL_IDLE,		// idle animation
	MODEL_CROUCH,		// crouch animation
	MODEL_RUN,		// running animation
	MODEL_JUMP		// jumping animation
};

/*
	Vector Functionality
*/
// a single vertex
typedef struct
{
    float point[3];
    float normal[3];
} vector_t;

vector_t operator-(vector_t a, vector_t b);
vector_t operator*(float f, vector_t b);
vector_t operator/(vector_t a, vector_t b);
vector_t operator+(vector_t a, vector_t b);

void CalculateNormal( float *p1, float *p2, float *p3 );


/* 
	MD2 Model Functionality
*/


// texture coordinate
typedef struct
{
   float s;
   float t;
} texCoord_t;

// texture coordinate index
typedef struct
{
   short s;
   short t;
} stIndex_t;

// info for a single frame point
typedef struct
{
   unsigned char v[3];
   unsigned char normalIndex;	// not used
} framePoint_t;

// information for a single frame
typedef struct
{
   float scale[3];
   float translate[3];
   char name[16];
   framePoint_t fp[1];
} frame_t;

// data for a single triangle
typedef struct
{
   unsigned short meshIndex[3];		// vertex indices
   unsigned short stIndex[3];		// texture coordinate indices
} mesh_t;

typedef struct
{
   int ident;		 // identifies as MD2 file "IDP2"
   int version;	 // mine is 8
   int skinwidth;    // width of texture
   int skinheight;   // height of texture
   int framesize;    // number of bytes per frame
   int numSkins;     // number of textures
   int numXYZ;       // number of points
   int numST;        // number of texture
   int numTris;      // number of triangles
   int numGLcmds;
   int numFrames;    // total number of frames
   int offsetSkins;  // offset to skin names (64 bytes each)
   int offsetST;     // offset of texture s-t values
   int offsetTris;   // offset of triangle mesh
   int offsetFrames; // offset of frame data (points)
   int offsetGLcmds; // type of OpenGL commands to use
   int offsetEnd;    // end of file
} modelHeader_t;


class CMD2Model
{
protected:

     int numFrames;			// number of model frames
     int numVertices;         // number of vertices
     int numTriangles;        // number of triangles
     int numST;               // number of skins
     int frameSize;           // size of each frame in bytes
     int currentFrame;        // current frame # in animation
     int nextFrame;           // next frame # in animation
     float interpol;          // percent through current frame
     float currInterpol;
     mesh_t *triIndex;        // triangle list
     texCoord_t *st;          // texture coordinate list
     vector_t *vertexList;    // vertex list
     //     texture_t *modelTex;     // texture data
     GLuint m_texture;
     
     modelState_t modelState;	// current model animation state

     void SetupSkin(texture_t *thisTexture);

public:

     CMD2Model();        // constructor
     ~CMD2Model();       // destructor

     // load model and skin/texture at the same time
     int Load(const char *modelFile, MD2Texture texture);
     void RenderCurrentFrame();
     
     // load model only
     int LoadModel(const char *modelFile);

     // load skin only
     int LoadSkin(const char *skinFile);

     // set model's texture/skin
     int SetTexture(texture_t *texture);

     // render model with interpolation to get animation
     int Animate(int startFrame, int endFrame, float percent);

     // render a single frame
     int RenderFrame(int keyFrame);

     // free memory of model
     int Unload();

	// set animation state of model
	int SetState(modelState_t state);	

	// retrieve animation state of model
	modelState_t GetState();
};

#endif
