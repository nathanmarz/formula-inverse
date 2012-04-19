#include "Framework.h"
#include <SDL_image.h>
#include <SDL_ttf.h>

hash_map<int, RInputAction *> RResources::m_actions;
hash_map<int, RPieceShell *> RResources::m_pieces;
hash_map<int, MD2Texture> RResources::m_fullTextures;
hash_map<int, string> RResources::m_texturePaths;

void RResources::FreeResources() {
    FreeResMap(m_actions);
    FreeResMap(m_pieces);
    //FreeResMap(m_textures);
}

void RResources::AddTexture(int id, const string &fileName) {
	m_texturePaths[id] = fileName;
}

void RResources::DoTextureLoad(int id, const string &fileName) {
  SDL_Surface *image;
  GLuint texture;
  image = IMG_Load(fileName.c_str());

  if (!image) {
    cerr << "Failed to load texture: '" << fileName << "'" <<endl;
    exit(1);
  }

  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  //CheckOpenGLErrors(-11);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  //CheckOpenGLErrors(-10);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

  gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, image->w, image->h,
		    GL_RGB, GL_UNSIGNED_BYTE, image->pixels);
  // GL_UNSIGNED_BYTE argument could possibly be wrong...depends on
  // what the bits per pixel of the image is.
  MD2Texture fullTex;
  fullTex.tex = texture;
  fullTex.width = image->w;
  fullTex.height = image->h;
  AddResource(id,fullTex,m_fullTextures);
  //delete image;
  //while(true);
}

void RResources::AddInputAction(int id, RInputAction *action) {
    AddResource(id,action,m_actions);
}

void RResources::AddPieceShell(int id, RPieceShell *piece) {
    AddResource(id,piece,m_pieces);
}

RPieceShell *RResources::GetPieceShell(int id) {
    return GetResource(id,m_pieces);
}

RInputAction *RResources::GetInputAction(int id) {
    return GetResource(id,m_actions);
}

GLuint RResources::GetTexture(int id) {
	if(m_fullTextures.find(id)==m_fullTextures.end()) DoTextureLoad(id,m_texturePaths[id]);
    return GetResource(id,m_fullTextures).tex;
}

MD2Texture RResources::GetFullTexture(int id) {
	if(m_fullTextures.find(id)==m_fullTextures.end()) DoTextureLoad(id,m_texturePaths[id]);
    return GetResource(id,m_fullTextures);
}
