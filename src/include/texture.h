#ifndef __TEXTURE_H
#define __TEXTURE_H

#include <stdio.h>
#include <iostream>
//#include <commctrl.h>
#include <GL/gl.h>
#include <GL/glu.h>

enum texTypes_t
{
	PCX,
	BMP,
	TGA
};

struct MD2Texture {
    int width;
    int height;
    GLuint tex;
};

typedef struct 
{
	// C Compatible version: enum texTypes textureType;
	// We use C++ only version
	texTypes_t textureType;

	int width;					// width of texture
	int height;					// height of texture
	long int scaledWidth;
	long int scaledHeight;

	unsigned int texID;			// the texture object id of this texture
	unsigned char *data;		// the texture data
	unsigned char *palette;
} texture_t;

// only partial pcx file header
typedef struct
{
	unsigned char manufacturer;
	unsigned char version;
	unsigned char encoding;
	unsigned char bits;
	unsigned char xMin;
	unsigned char yMin;
	unsigned char xMax;
	unsigned char yMax;
	unsigned char *palette;
} PCXHEADER;

texture_t *LoadTexture(const char *filename);



#endif
