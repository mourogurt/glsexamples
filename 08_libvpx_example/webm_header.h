#ifndef WEBM_H
#define WEBM_H

#undef CONFIG_ENCODERS
#include <engInit.hpp>
#include <engBuffer.hpp>
#include <engShader.hpp>
#include <engValue.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <fstream>

extern "C" {
#define VPX_CODEC_DISABLE_COMPAT 1
#include <vpx/vpx_decoder.h>
#include <vpx/vp8dx.h>
#include <nestegg/nestegg.h>
#include <libyuv.h>
}

using namespace std;

int ifstream_read(void *buffer, size_t size, void *context) {
  ifstream* f = (ifstream*)context;
  f->read((char*)buffer, size);
  // success = 1
  // eof = 0
  // error = -1
  return unsigned(f->gcount()) == size ? 1 : f->eof() ? 0 : -1;
}

int ifstream_seek(int64_t n, int whence, void *context) {
  ifstream* f = (ifstream*)context;
  f->clear();
  ios_base::seekdir dir;
  switch (whence) {
    case NESTEGG_SEEK_SET:
      dir = fstream::beg;
      break;
    case NESTEGG_SEEK_CUR:
      dir = fstream::cur;
      break;
    case NESTEGG_SEEK_END:
      dir = fstream::end;
      break;
  }
  f->seekg(n, dir);
  if (!f->good())
    return -1;
  return 0;
}

int64_t ifstream_tell(void* context) {
  ifstream* f = (ifstream*)context;
  return f->tellg();
}

void webm_start(char const*);
GLuint gen_texture(vpx_image_t*);

#endif
