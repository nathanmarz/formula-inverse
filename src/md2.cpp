#include "md2.h"
#include <iostream>
#include "anorms.h"

// vector subtraction
vector_t operator-(vector_t a, vector_t b)
{
	vector_t c;

	c.point[0] = a.point[0] - b.point[0];
	c.point[1] = a.point[1] - b.point[1];
	c.point[2] = a.point[2] - b.point[2];

	return c;
}

// scalar-vector multiplication
vector_t operator*(float f, vector_t b)
{
	vector_t c;

	c.point[0] = f * b.point[0];
	c.point[1] = f * b.point[1];
	c.point[2] = f * b.point[2];

	return c;
}

// vector division
vector_t operator/(vector_t a, vector_t b)
{
	vector_t c;

	c.point[0] = a.point[0] / b.point[0];
	c.point[1] = a.point[1] / b.point[1];
	c.point[2] = a.point[2] / b.point[2];

	return c;
}

// vector addition
vector_t operator+(vector_t a, vector_t b)
{
	vector_t c;

	c.point[0] = a.point[0] + b.point[0];
	c.point[1] = a.point[1] + b.point[1];
	c.point[2] = a.point[2] + b.point[2];

	return c;
}

// CMD2Model constructor
CMD2Model::CMD2Model()
{
     numVertices = 0;    // vertices
     numTriangles = 0;   // triangles
     numFrames = 0;      // frames
     numST = 0;          // texture coordinates
     frameSize = 0;      // needed?
     currentFrame = 0;   // current keyframe 
     nextFrame = 1;      // next keyframe
     interpol = 0.0;     // interpolation percent
     currInterpol = 0.0;
     triIndex = NULL;    // triangle indices
     st = NULL;          // texture coordinate indices
     vertexList = NULL;  // vertices
     //modelTex = NULL;    // skin/texture
	modelState = MODEL_IDLE;
}    

// CMD2Model destructor
CMD2Model::~CMD2Model()
{
}

// CMD2Model::SetupSkin()
// access: private
// desc: sets up the model skin/texture for OpenGL
void CMD2Model::SetupSkin(texture_t *thisTexture)
{
    
     // set the proper parameters for an MD2 texture
     glGenTextures(1, &thisTexture->texID);
     glBindTexture(GL_TEXTURE_2D, thisTexture->texID);
     //glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
     //glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
     glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
     glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);

     //thisTexture->textureType = PCX;
     switch (thisTexture->textureType)
     {
     case BMP:
         //         std::cerr<<"setting up bmp"<<std::endl;
          gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, thisTexture->width, thisTexture->height, 
               GL_RGB, GL_UNSIGNED_BYTE, thisTexture->data);
          break;
     case PCX:
         // std::cerr<<"setting up pcx"<<std::endl;
          gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, thisTexture->width, thisTexture->height,
               GL_RGB, GL_UNSIGNED_BYTE, thisTexture->data);
     case TGA:
          break;
     default:
         // std::cerr<<"uh oh"<<std::endl;
          break;
     }
     //std::cerr<<thisTexture->textureType<<std::endl;
}


void GetNorm(float norm[3], char offset) {
    float *n = &NORMS[3*(int)offset];
    norm[0] = n[0];
    norm[1] = n[1];
    norm[2] = n[2];
}

// CMD2Model::Load()
// access: public
// desc: loads model and skin
int CMD2Model::Load(const char *modelFile, MD2Texture texture)
{
     FILE *filePtr;                          // file pointer
     int fileLen;                            // length of model file
     char *buffer;                           // file buffer
     
     modelHeader_t *modelHeader;             // model header
     
     stIndex_t *stPtr;                       // texture data
     frame_t *frame;                              // frame data
     vector_t *vertexListPtr;                // index variable
     mesh_t *bufIndexPtr;          // index variables
     int i, j;                               // index variables
     
     // open the model file
     filePtr = fopen(modelFile, "rb");
     if (filePtr == NULL) {
         std::cerr<<"couldn't find file"<<std::endl;
         return 0; //FALSE
     }
     
     // find length of file
     fseek(filePtr, 0, SEEK_END);
     fileLen = ftell(filePtr);
     fseek(filePtr, 0, SEEK_SET);
     
     // read entire file into buffer
     buffer = new char [fileLen+1];
     fread(buffer, sizeof(char), fileLen, filePtr);
     
     // extract model file header from buffer
     modelHeader = (modelHeader_t*)buffer;
     
     vertexList = new vector_t [modelHeader->numXYZ * modelHeader->numFrames];
     
     numVertices = modelHeader->numXYZ;
     numFrames = modelHeader->numFrames;
     frameSize = modelHeader->framesize;
     
     for (j = 0; j < numFrames; j++)
     {
          frame = (frame_t*)&buffer[modelHeader->offsetFrames + frameSize * j];
          
          vertexListPtr = (vector_t*)&vertexList[numVertices * j];
          for (i = 0; i < numVertices; i++)
          {
               vertexListPtr[i].point[0] = frame->scale[0] * frame->fp[i].v[0] + frame->translate[0];
               vertexListPtr[i].point[1] = frame->scale[1] * frame->fp[i].v[1] + frame->translate[1];
               vertexListPtr[i].point[2] = frame->scale[2] * frame->fp[i].v[2] + frame->translate[2];
               GetNorm(vertexListPtr[i].normal,frame->fp[i].normalIndex);
          }
     }
     /*
     modelTex = LoadTexture(skinFile);
     if (modelTex != NULL)
          SetupSkin(modelTex);
     else {
         std::cerr<<"couldn't find skin file"<<std::endl;
         return 0; //FALSE
         }*/
     m_texture = texture.tex;
     
     numST = modelHeader->numST;
     st = new texCoord_t [numST];
     
     stPtr = (stIndex_t*)&buffer[modelHeader->offsetST];
     for (i = 0; i < numST; i++)
     {

         //          st[i].s = (float)stPtr[i].s / (float)modelTex->width;
         // st[i].t = (float)stPtr[i].t / (float)modelTex->height;
          st[i].s = (float)stPtr[i].s *1.0/257;
          st[i].t = (float)stPtr[i].t *1.0/257;
     }
     
     numTriangles = modelHeader->numTris;
     triIndex = new mesh_t [numTriangles];
     
     // point to triangle indexes in buffer
     bufIndexPtr = (mesh_t*)&buffer[modelHeader->offsetTris];
     
     // create a mesh (triangle) list
     for (j = 0; j < numFrames; j++)         
     {
          // for all triangles in each frame
          for(i = 0; i < numTriangles; i++)
          {
               triIndex[i].meshIndex[0] = bufIndexPtr[i].meshIndex[0];
               triIndex[i].meshIndex[1] = bufIndexPtr[i].meshIndex[1];
               triIndex[i].meshIndex[2] = bufIndexPtr[i].meshIndex[2];
               triIndex[i].stIndex[0] = bufIndexPtr[i].stIndex[0];
               triIndex[i].stIndex[1] = bufIndexPtr[i].stIndex[1];
               triIndex[i].stIndex[2] = bufIndexPtr[i].stIndex[2];
          }
     }
     
     // close file and free memory
     fclose(filePtr);
     free(buffer);
     
     currentFrame = 0;
     nextFrame = 1;
     interpol = 0.0;
     
     return 1; //TRUE

}

// CMD2Model::LoadModel()
// access: public
// desc: loads model from file
int CMD2Model::LoadModel(const char *modelFile)
{
     FILE *filePtr;                          // file pointer
     int fileLen;                            // length of model file
     char *buffer;                           // file buffer
     
     modelHeader_t *modelHeader;             // model header
     
     stIndex_t *stPtr;                       // texture data
     frame_t *frame;                              // frame data
     vector_t *vertexListPtr;                // index variable
     mesh_t *triIndex, *bufIndexPtr;         // index variables
     int i, j;                               // index variables
     
     // open the model file
     filePtr = fopen(modelFile, "rb");
     if (filePtr == NULL)
         return 0; //FALSE
     
     // find length of file
     fseek(filePtr, 0, SEEK_END);
     fileLen = ftell(filePtr);
     fseek(filePtr, 0, SEEK_SET);
     
     // read entire file into buffer
     buffer = new char [fileLen+1];
     fread(buffer, sizeof(char), fileLen, filePtr);
     
     // extract model file header from buffer
     modelHeader = (modelHeader_t*)buffer;
     
     // allocate vertex list
     vertexList = new vector_t [modelHeader->numXYZ * modelHeader->numFrames];
     
     numVertices = modelHeader->numXYZ;
     numFrames = modelHeader->numFrames;
     frameSize = modelHeader->framesize;
     
     for (j = 0; j < numFrames; j++)
     {
          frame = (frame_t*)&buffer[modelHeader->offsetFrames + frameSize * j];
          
          vertexListPtr = (vector_t*)&vertexList[numVertices * j];
          for (i = 0; i < numVertices; i++)
          {
               vertexListPtr[i].point[0] = frame->scale[0] * frame->fp[i].v[0] + frame->translate[0];
               vertexListPtr[i].point[1] = frame->scale[1] * frame->fp[i].v[1] + frame->translate[1];
               vertexListPtr[i].point[2] = frame->scale[2] * frame->fp[i].v[2] + frame->translate[2];
          }
     }
     
     numST = modelHeader->numST;
     
     st = new texCoord_t [numST];
     
     stPtr = (stIndex_t*)&buffer[modelHeader->offsetST];
     for (i = 0; i < numST; i++)
     {
          st[i].s = 0.0;
          st[i].t = 0.0;
     }
     
     numTriangles = modelHeader->numTris;
     triIndex = new mesh_t [numTriangles];
     
     // point to triangle indexes in buffer
     bufIndexPtr = (mesh_t*)&buffer[modelHeader->offsetTris];
     
     // create a mesh (triangle) list
     for (j = 0; j < numFrames; j++)         
     {
          // for all triangles in each frame
          for(i = 0; i < numTriangles; i++)
          {
               triIndex[i].meshIndex[0] = bufIndexPtr[i].meshIndex[0];
               triIndex[i].meshIndex[1] = bufIndexPtr[i].meshIndex[1];
               triIndex[i].meshIndex[2] = bufIndexPtr[i].meshIndex[2];
               triIndex[i].stIndex[0] = bufIndexPtr[i].stIndex[0];
               triIndex[i].stIndex[1] = bufIndexPtr[i].stIndex[1];
               triIndex[i].stIndex[2] = bufIndexPtr[i].stIndex[2];
          }
     }
     
     // close file and free memory
     fclose(filePtr);
     
     //     modelTex = NULL;
     currentFrame = 0;
     nextFrame = 1;
     interpol = 0.0;
     
     return 0;
}
/*
// CMD2Model::LoadSkin()
// access: public
// desc: loads a skin for the model
int CMD2Model::LoadSkin(const char *skinFile)
{
     int i;
     
     //modelTex = LoadTexture(skinFile);
     
     if (modelTex != NULL)
          SetupSkin(modelTex);
     else
          return -1;
     
     for (i = 0; i < numST; i++)
     {
          st[i].s /= (float)modelTex->width;
          st[i].t /= (float)modelTex->height;
     }
     
     return 0;
}
*/
/*
// CMD2Model::SetTexture()
// access: public
// desc: sets a new texture object 
int CMD2Model::SetTexture(texture_t *texture)
{
     int i;
     
     if (texture != NULL)
	{
		free(modelTex);
          modelTex = texture;
	}
     else
          return -1;
     
     SetupSkin(modelTex);
     
     for (i = 0; i < numST; i++)
     {
          st[i].s /= (float)modelTex->width;
          st[i].t /= (float)modelTex->height;
     }
     
     return 0;
}
*/
// CMD2Model::Animate()
// access: public
// desc: animates the model between the keyframes startFrame and endFrame
int CMD2Model::Animate(int startFrame, int endFrame, float percent)
{
     
     if ((startFrame > currentFrame))
          currentFrame = startFrame;
     
     if ((startFrame < 0) || (endFrame < 0))
          return -1;
     
     if ((startFrame >= numFrames) || (endFrame >= numFrames))
          return -1;
     
     if (interpol >= 1.0)
     {
          interpol = 0.0f;
          currentFrame++;
          if (currentFrame >= endFrame)
               currentFrame = startFrame;
          
          nextFrame = currentFrame + 1;
          
          if (nextFrame >= endFrame)
               nextFrame = startFrame;
          
     }
     currInterpol = interpol;
     
     interpol += percent;  // increase percentage of interpolation between frames
     
     return 0;
}


void CheckVals(float x, float y, float z) {
    if(x*0!=0) exit(1);
    if(y*0!=0) exit(1);
    if(z*0!=0) exit(1);
}

void CMD2Model::RenderCurrentFrame() {
    vector_t *vList;              // current frame vertices
    vector_t *nextVList;          // next frame vertices
    int i;                                  // index counter
    float x1, y1, z1;                  // current frame point values
    float x2, y2, z2;                  // next frame point values
    
    vector_t vertex[3]; 
    if(numFrames==1) nextFrame = currentFrame; 
    vList = &vertexList[numVertices*currentFrame];
    nextVList = &vertexList[numVertices*nextFrame];
    glDisable(GL_CULL_FACE);
    glBindTexture(GL_TEXTURE_2D, m_texture);

    //glDisable(GL_TEXTURE_2D);
    glPushAttrib(GL_LIGHTING_BIT);

    glEnable(GL_NORMALIZE);
    
    //glEnable(GL_COLOR_MATERIAL);
    //glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
    //glColor3f(.6,.1,0);
    //GLfloat mat_emission[] = {.8,.1,.1,1};
    //glMaterialfv(GL_FRONT,GL_EMISSION,mat_emission);
    
    glBegin(GL_TRIANGLES);
    for (i = 0; i < numTriangles; i++)
        {
            vector_t p1 = vList[triIndex[i].meshIndex[0]];
            vector_t p2 = vList[triIndex[i].meshIndex[2]];
            vector_t p3 = vList[triIndex[i].meshIndex[1]];
            /*
            //std::cerr<<numFrames<<std::endl;
            // get first points of each frame
            x1 = vList[triIndex[i].meshIndex[0]].point[0];
            y1 = vList[triIndex[i].meshIndex[0]].point[1];
            z1 = vList[triIndex[i].meshIndex[0]].point[2];
            x2 = nextVList[triIndex[i].meshIndex[0]].point[0];
            y2 = nextVList[triIndex[i].meshIndex[0]].point[1];
            z2 = nextVList[triIndex[i].meshIndex[0]].point[2];

            //CheckVals(x1,y1,z1);
            //CheckVals(x2,y2,z2);
            
            // store first interpolated vertex of triangle
            vertex[0].point[0] = x1 + currInterpol * (x2 - x1);
            vertex[0].point[1] = y1 + currInterpol * (y2 - y1);
            vertex[0].point[2] = z1 + currInterpol * (z2 - z1);
            
            // get second points of each frame
            x1 = vList[triIndex[i].meshIndex[2]].point[0];
            y1 = vList[triIndex[i].meshIndex[2]].point[1];
            z1 = vList[triIndex[i].meshIndex[2]].point[2];
            x2 = nextVList[triIndex[i].meshIndex[2]].point[0];
            y2 = nextVList[triIndex[i].meshIndex[2]].point[1];
            z2 = nextVList[triIndex[i].meshIndex[2]].point[2];
            
          // store second interpolated vertex of triangle
            vertex[2].point[0] = x1 + currInterpol * (x2 - x1);
            vertex[2].point[1] = y1 + currInterpol * (y2 - y1);
            vertex[2].point[2] = z1 + currInterpol * (z2 - z1);   
            
            // get third points of each frame
            x1 = vList[triIndex[i].meshIndex[1]].point[0];
            y1 = vList[triIndex[i].meshIndex[1]].point[1];
            z1 = vList[triIndex[i].meshIndex[1]].point[2];
            x2 = nextVList[triIndex[i].meshIndex[1]].point[0];
            y2 = nextVList[triIndex[i].meshIndex[1]].point[1];
            z2 = nextVList[triIndex[i].meshIndex[1]].point[2];
            
            // store third interpolated vertex of triangle
            vertex[1].point[0] = x1 + currInterpol * (x2 - x1);
            vertex[1].point[1] = y1 + currInterpol * (y2 - y1);
            vertex[1].point[2] = z1 + currInterpol * (z2 - z1);
          
            // calculate the normal of the triangle
            CalculateNormal(p1.point, p2.point, p3.point);
            */
          // render properly textured triangle
            //glNormal3fv(p1.normal);
            glTexCoord2f(st[triIndex[i].stIndex[0]].s,
                         st[triIndex[i].stIndex[0]].t);
            glVertex3fv(p1.point);
            //glNormal3fv(p2.normal);
            glTexCoord2f(st[triIndex[i].stIndex[2]].s ,
                         st[triIndex[i].stIndex[2]].t);
            glVertex3fv(p2.point);
            //glNormal3fv(p3.normal);
            glTexCoord2f(st[triIndex[i].stIndex[1]].s,
                         st[triIndex[i].stIndex[1]].t);
            glVertex3fv(p3.point);
        }
    glEnd();
    /*
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glBegin(GL_LINES);
    for (i = 0; i < numTriangles; i++)
        {
            vector_t p1 = vList[triIndex[i].meshIndex[0]];
            vector_t p2 = vList[triIndex[i].meshIndex[2]];
            vector_t p3 = vList[triIndex[i].meshIndex[1]];
            glVertex3fv(p1.point);
            glVertex3f(p1.point[0]+p1.normal[0],p1.point[1]+p1.normal[1],p1.point[2]+p1.normal[2]);
            glVertex3fv(p2.point);
            glVertex3f(p2.point[0]+p2.normal[0],p2.point[1]+p2.normal[1],p2.point[2]+p2.normal[2]);
            glVertex3fv(p3.point);
            glVertex3f(p3.point[0]+p3.normal[0],p3.point[1]+p3.normal[1],p3.point[2]+p3.normal[2]);
        }
    glEnd();
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    */
    //glDisable(GL_COLOR_MATERIAL);

    glPopAttrib();
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_NORMALIZE);
    glEnable(GL_CULL_FACE);

}

/*
// RenderFrame()
// desc: renders a single key frame
int CMD2Model::RenderFrame(int keyFrame)
{
     vector_t *vList;
     int i;

     
     // create a pointer to the frame we want to show
     vList = &vertexList[numVertices * keyFrame];
     
     if(modelTex==NULL) std::cerr<<"no texture..."<<std::endl;
     
	if (modelTex != NULL)
		// set the texture
		glBindTexture(GL_TEXTURE_2D, modelTex->texID);
     
     // display the textured model with proper lighting normals
     glBegin(GL_TRIANGLES);
     //     std::cerr<<numTriangles<<std::endl;
     for(i = 0; i < numTriangles; i++)
     {
          CalculateNormal(vList[triIndex[i].meshIndex[0]].point,
               vList[triIndex[i].meshIndex[2]].point,
               vList[triIndex[i].meshIndex[1]].point);

		if (modelTex != NULL)
			glTexCoord2f(st[triIndex[i].stIndex[0]].s,
				st[triIndex[i].stIndex[0]].t);

          glVertex3fv(vList[triIndex[i].meshIndex[0]].point);
          
		if (modelTex != NULL)
			glTexCoord2f(st[triIndex[i].stIndex[2]].s ,
				st[triIndex[i].stIndex[2]].t);

          glVertex3fv(vList[triIndex[i].meshIndex[2]].point);
          
		if (modelTex != NULL)
			glTexCoord2f(st[triIndex[i].stIndex[1]].s,
				st[triIndex[i].stIndex[1]].t);

          glVertex3fv(vList[triIndex[i].meshIndex[1]].point);
     }
     glEnd();
     
     return 0;
}
*/
// Unload()
// desc: unloads model data from memory
int CMD2Model::Unload()
{
     if (triIndex != NULL)
          free(triIndex);
     if (vertexList != NULL)
          free(vertexList);
     if (st != NULL)
          free(st);
     
     return 0;
}

// SetState()
// desc: set the model state
int CMD2Model::SetState(modelState_t state)
{
	modelState = state;
	return 0;
}

// GetState()
// desc: retrieve the model state
modelState_t CMD2Model::GetState()
{
	return modelState;
}





/*
// LoadBitmapFile
// desc: Returns a pointer to the bitmap image of the bitmap specified
//       by filename. Also returns the bitmap header information.
//		 No support for 8-bit bitmaps.
unsigned char *LoadBitmapFile(char *filename, BITMAPINFOHEADER *bitmapInfoHeader)
{
	FILE *filePtr;							// the file pointer
	BITMAPFILEHEADER	bitmapFileHeader;		// bitmap file header
	unsigned char		*bitmapImage;			// bitmap image data
	unsigned int		imageIdx = 0;		// image index counter
	unsigned char		tempRGB;				// swap variable

	// open filename in "read binary" mode
	filePtr = fopen(filename, "rb");
	if (filePtr == NULL)
		return NULL;

	// read the bitmap file header
	fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);
	
	// verify that this is a bitmap by checking for the universal bitmap id
	if (bitmapFileHeader.bfType != BITMAP_ID)
	{
		fclose(filePtr);
		return NULL;
	}

	// read the bitmap information header
	fread(bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);

	// move file pointer to beginning of bitmap data
	fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);

	// allocate enough memory for the bitmap image data
	bitmapImage = (unsigned char*)malloc(bitmapInfoHeader->biSizeImage);

	// verify memory allocation
	if (!bitmapImage)
	{
		free(bitmapImage);
		fclose(filePtr);
		return NULL;
	}

	// read in the bitmap image data
	fread(bitmapImage, 1, bitmapInfoHeader->biSizeImage, filePtr);

	// make sure bitmap image data was read
	if (bitmapImage == NULL)
	{
		fclose(filePtr);
		return NULL;
	}

	// swap the R and B values to get RGB since the bitmap color format is in BGR
	for (imageIdx = 0; imageIdx < bitmapInfoHeader->biSizeImage; imageIdx+=3)
	{
		tempRGB = bitmapImage[imageIdx];
		bitmapImage[imageIdx] = bitmapImage[imageIdx + 2];
		bitmapImage[imageIdx + 2] = tempRGB;
	}

	// close the file and return the bitmap image data
	fclose(filePtr);
	return bitmapImage;
}
*/

// LoadPCXFile()
// desc: loads a PCX file into memory
unsigned char *LoadPCXFile(const char *filename, PCXHEADER *pcxHeader)
{
     int idx = 0;                  // counter index
     int c;                             // used to retrieve a char from the file
     int i;                             // counter index
     int numRepeat;      
     FILE *filePtr;                // file handle
     int width;                         // pcx width
     int height;                        // pcx height
     unsigned char *pixelData;     // pcx image data
     unsigned char *paletteData;   // pcx palette data

     // open PCX file
     filePtr = fopen(filename, "rb");
     if (filePtr == NULL)
          return NULL;

     // retrieve first character; should be equal to 10
     c = getc(filePtr);
     if (c != 10)
     {
          fclose(filePtr);
          return NULL;
     }

     // retrieve next character; should be equal to 5
     c = getc(filePtr);
     if (c != 5)
     {
          fclose(filePtr);
          return NULL;
     }

     // reposition file pointer to beginning of file
     rewind(filePtr);

     // read 4 characters of data to skip
     fgetc(filePtr);
     fgetc(filePtr);
     fgetc(filePtr);
     fgetc(filePtr);

     // retrieve leftmost x value of PCX
     pcxHeader->xMin = fgetc(filePtr);       // loword
     pcxHeader->xMin |= fgetc(filePtr) << 8; // hiword

     // retrieve bottom-most y value of PCX
     pcxHeader->yMin = fgetc(filePtr);       // loword
     pcxHeader->yMin |= fgetc(filePtr) << 8; // hiword

     // retrieve rightmost x value of PCX
     pcxHeader->xMax = fgetc(filePtr);       // loword
     pcxHeader->xMax |= fgetc(filePtr) << 8; // hiword

     // retrieve topmost y value of PCX
     pcxHeader->yMax = fgetc(filePtr);       // loword
     pcxHeader->yMax |= fgetc(filePtr) << 8; // hiword

     // calculate the width and height of the PCX
     width = pcxHeader->xMax - pcxHeader->xMin + 1;
     height = pcxHeader->yMax - pcxHeader->yMin + 1;

     // allocate memory for PCX image data
     pixelData = (unsigned char*)malloc(width*height);

     // set file pointer to 128th byte of file, where the PCX image data starts
     fseek(filePtr, 128, SEEK_SET);
     
     // decode the pixel data and store
     while (idx < (width*height))
     {
          c = getc(filePtr);
          if (c > 0xbf)
          {
               numRepeat = 0x3f & c;
               c = getc(filePtr);

               for (i = 0; i < numRepeat; i++)
               {
                    pixelData[idx++] = c;
               }
          }
          else
               pixelData[idx++] = c;

          fflush(stdout);
     }

     // allocate memory for the PCX image palette
     paletteData = (unsigned char*)malloc(768);

     // palette is the last 769 bytes of the PCX file
     fseek(filePtr, -769, SEEK_END);

     // verify palette; first character should be 12
     c = getc(filePtr);
     if (c != 12)
     {
          fclose(filePtr);
          return NULL;
     }

     // read and store all of palette
     for (i = 0; i < 768; i++)
     {
          c = getc(filePtr);
          paletteData[i] = c;
     }

     // close file and store palette in header
     fclose(filePtr);
     pcxHeader->palette = paletteData;

     // return the pixel image data
     return pixelData;
}

// LoadPCXTexture()
// desc: loads a PCX image file as a texture
texture_t *LoadPCXTexture(const char *filename)
{
     PCXHEADER texInfo;            // header of texture
     texture_t *thisTexture;       // the texture
     unsigned char *unscaledData;// used to calculate pcx
     int i;                             // index counter
     int j;                             // index counter
     int width;                         // width of texture
     int height;                        // height of texture

     // allocate memory for texture struct
     thisTexture = (texture_t*)malloc(sizeof(texture_t));
     if (thisTexture == NULL)
          return NULL;

     // load the PCX file into the texture struct
     thisTexture->data = LoadPCXFile(filename, &texInfo);
     if (thisTexture->data == NULL)
     {
          free(thisTexture->data);
          return NULL;
     }

     // store the texture information
     thisTexture->palette = texInfo.palette;
     thisTexture->width = texInfo.xMax - texInfo.xMin + 1;
     thisTexture->height = texInfo.yMax - texInfo.yMin + 1;
     thisTexture->textureType = PCX;

     // allocate memory for the unscaled data
     unscaledData = (unsigned char*)malloc(thisTexture->width*thisTexture->height*4);

     // store the unscaled data via the palette
     for (j = 0; j < thisTexture->height; j++) 
     {
          for (i = 0; i < thisTexture->width; i++) 
          {
               unscaledData[4*(j*thisTexture->width+i)+0] = (unsigned char)thisTexture->palette[3*thisTexture->data[j*thisTexture->width+i]+0];
               unscaledData[4*(j*thisTexture->width+i)+1] = (unsigned char)thisTexture->palette[3*thisTexture->data[j*thisTexture->width+i]+1];
               unscaledData[4*(j*thisTexture->width+i)+2] = (unsigned char)thisTexture->palette[3*thisTexture->data[j*thisTexture->width+i]+2];
               unscaledData[4*(j*thisTexture->width+i)+3] = (unsigned char)255;
          }
     }

     // find width and height's nearest greater power of 2
     width = thisTexture->width;
     height = thisTexture->height;

     // find width's
     i = 0;
     while (width)
     {
          width /= 2;
          i++;
     }
     thisTexture->scaledHeight = (long) pow((float)2, (float)(i-1));

     // find height's
     i = 0;
     while (height)
     {
          height /= 2;
          i++;
     }
     thisTexture->scaledWidth = (long)pow((float)2, (float)(i-1));

     // clear the texture data
     if (thisTexture->data != NULL)
     {
          free(thisTexture->data);
          thisTexture->data = NULL;
     }

     // reallocate memory for the texture data
     thisTexture->data = (unsigned char*)malloc(thisTexture->scaledWidth*thisTexture->scaledHeight*4);
     
     // use the GL utility library to scale the texture to the unscaled dimensions
     gluScaleImage (GL_RGBA, thisTexture->width, thisTexture->height, GL_UNSIGNED_BYTE, unscaledData, thisTexture->scaledWidth, thisTexture->scaledHeight, GL_UNSIGNED_BYTE, thisTexture->data);

     return thisTexture;
}

/*
// LoadBMPTexture()
// desc: loads a texture of the BMP format
texture_t *LoadBMPTexture(char *filename)
{
    BITMAPINFOHEADER texInfo;		// BMP header
    texture_t *thisTexture;			// the texture

	// allocate memory for the texture
	thisTexture = (texture_t*)malloc(sizeof(texture_t));
	if (thisTexture == NULL)
		return NULL;

	// store BMP data in texture
	thisTexture->data = LoadBitmapFile(filename, &texInfo);
	if (thisTexture->data == NULL)
	{
		free(thisTexture);
		return NULL;
	}
	
	// store texture information
	thisTexture->width = texInfo.biWidth;
	thisTexture->height = texInfo.biHeight;
	thisTexture->palette = NULL;
	thisTexture->scaledHeight = 0;
	thisTexture->scaledWidth = 0;
	thisTexture->textureType = BMP;

	return thisTexture;
}
*/

using namespace std;


// LoadTexture()
// desc: loads a texture given the filename
texture_t *LoadTexture(const char *filename)
{
	texture_t *thisTexture;
	char *extStr;

	// get extension from filename
	//extStr = strchr(filename, '.');
	extStr++;

	// set the texture type based on extension of filename
    /*
	if ((strcmp(extStr, "BMP") == 0) || (strcmp(extStr, "bmp") == 0)) {
		//thisTexture = LoadBMPTexture(filename);
        cerr<<"Textures for MD2 models must be of PCX format (no support for bmp)"<<endl;
        exit(1);
    }
    */
	//else if ((strcmp(extStr, "PCX") == 0) || (strcmp(extStr, "pcx") == 0) ) {
		thisTexture = LoadPCXTexture(filename);
        //cerr<<"setting up pcx texture"<<endl;
        //}
        //cerr<<"setting up texture"<<endl;
	/*
	else if ((strcmp(extStr, "TGA") == 0) || (strcmp(extStr, "tga") == 0) )
		thisTexture = LoadTGATexture(filename);
		//texType = TGA;
	*/
	return thisTexture;
}

void PrintPoint(float *p) {
    //cerr<<"[ "<<p[0]<<" "<<p[1]<<" "<<p[2]<<"]"<<endl;
    if(p[0]*0!=0||p[1]*0!=0||p[2]*0!=0) {
        cerr<<"bad point!"<<endl;
    }
}


#include "Math3D.h"
using namespace Math3D;
// CalculateNormal()
// desc: given 3 points, calculates the normal to the points
void CalculateNormal( float *p1, float *p2, float *p3 )
{

    Vector4 v1(p1[0],p1[1],p1[2]);
    Vector4 v2(p2[0],p2[1],p2[2]);
    Vector4 v3(p3[0],p3[1],p3[2]);
    Vector4 av = v1-v2;
    Vector4 bv = v3-v1;
    Vector4 norm = CrossProduct(av,bv);
    glNormal3f(norm[0],norm[1],norm[2]);
    return;
    //PrintPoint(p1);
    //PrintPoint(p2);
    //PrintPoint(p3);
   float a[3], b[3], result[3];
   float length;

   a[0] = p1[0] - p2[0];
   a[1] = p1[1] - p2[1];
   a[2] = p1[2] - p2[2];

   b[0] = p1[0] - p3[0];
   b[1] = p1[1] - p3[1];
   b[2] = p1[2] - p3[2];

   result[0] = a[1] * b[2] - b[1] * a[2];
   result[1] = b[0] * a[2] - a[0] * b[2];
   result[2] = a[0] * b[1] - b[0] * a[1];

   // calculate the length of the normal
   length = (float)sqrt(result[0]*result[0] + result[1]*result[1] + result[2]*result[2]);

   
   // normalize and specify the normal
   glNormal3f(result[0]/length, result[1]/length, result[2]/length);
}
