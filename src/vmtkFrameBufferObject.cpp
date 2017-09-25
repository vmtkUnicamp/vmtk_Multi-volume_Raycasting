#include "vmtkFrameBufferObject.h"

GLuint bound(const int &min, const int &val, const int &max)
{
  return (GLuint) std::max(min, std::min(max, val));
}

vmtkFrameBufferObject::vmtkFrameBufferObject()
{
  fbo = 0;
  texture = 0;
}

vmtkFrameBufferObject::vmtkFrameBufferObject(int width, int height, GLenum target, Attachment option)
{
  init(width, height, option, GL_RGBA, target);
}

vmtkFrameBufferObject::~vmtkFrameBufferObject()
{
  if(fbo != 0)
    glDeleteFramebuffers (1, &fbo);

  if(texture != 0)
    glDeleteTextures(1, &texture);
}

GLuint vmtkFrameBufferObject::getTexture() const
{
  return texture;
}

GLuint vmtkFrameBufferObject::getDepthTexture() const
{
  return depthTexture;
}

GLuint vmtkFrameBufferObject::getFbo() const
{
  return fbo;
}

bool vmtkFrameBufferObject::binding()
{
  if (!fbo) return false;

  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  return true;
}

bool vmtkFrameBufferObject::releasing()
{
  if (!fbo) return false;
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  return true;
}


void vmtkFrameBufferObject::resizing(int width, int height)
{
  glBindTexture(target, fbo);
  glTexImage2D (target, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
  glBindTexture(target, 0);
}

void vmtkFrameBufferObject::init(int width, int height, Attachment attachment, GLenum internal_format, GLenum target, GLint samples, bool mipmap)
{
  this->target = target;
  fbo = 0;
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  texture = 0;
  depthTexture = 0;
  GLuint color_buffer = 0;
  GLuint depth_buffer = 0;
  GLuint stencil_buffer = 0;

  if (samples == 0) {
      glGenTextures(1, &texture);

      glBindTexture(target, texture);
      glTexImage2D(target, 0, internal_format, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
      if (mipmap) {
          int level = 0;
          while (width > 1 || height > 1)
            {
              width = std::max(1, width >> 1);
              height = std::max(1, height >> 1);
              ++level;
              glTexImage2D(target, level, internal_format, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
            }
        }
      glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glBindTexture(target, 0);

      glGenTextures(1, &depthTexture);
      glBindTexture(target, depthTexture);
      glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexImage2D(target, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
      glBindTexture(target, 0);

      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target, texture, 0);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, target, depthTexture, 0);

      GLenum buffers[1] = {GL_COLOR_ATTACHMENT0};
      glDrawBuffers(1, buffers);
      if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "Framebuffer is not ok!!!" << std::endl;
      glBindTexture(target, 0);
      color_buffer = 0;

    }
  else
    {
      mipmap = false;
      GLint maxSamples;
      glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
      samples = bound(0, int(samples), int(maxSamples));
      glGenRenderbuffers(1, &color_buffer);
      glBindRenderbuffer(GL_RENDERBUFFER, color_buffer);
      glRenderbufferStorage(GL_RENDERBUFFER, internal_format, width, height);
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, color_buffer);
      glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_SAMPLES, &samples);
    }

  if (attachment == vmtkFrameBufferObject::DepthStencil){

      glGenRenderbuffers(1, &depth_buffer);
      glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);
      assert (glIsRenderbuffer(depth_buffer));
      if (samples != 0)
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH24_STENCIL8, width, height);
      else
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

      stencil_buffer = depth_buffer;
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, stencil_buffer);

      bool valid = glCheckFramebufferStatus(GL_FRAMEBUFFER);
      if (!valid) {
          glDeleteRenderbuffers(1, &depth_buffer);
          stencil_buffer = depth_buffer = 0;
        }
    }

  if (depth_buffer == 0 && (attachment == vmtkFrameBufferObject::DepthStencil
                            || (attachment == vmtkFrameBufferObject::Depth)))
    {
      glGenRenderbuffers(1, &depth_buffer);
      depthTexture = depth_buffer;
      glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);
      assert(glIsRenderbuffer(depth_buffer));
      if (samples != 0) {
          glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH_COMPONENT16, width, height);
        }
      else{
          glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
        }
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER, depth_buffer);

      if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "Depth buffer is not ok!!!" << std::endl;
    }
  if (stencil_buffer == 0 && (attachment == vmtkFrameBufferObject::DepthStencil)) {
      glGenRenderbuffers(1, &stencil_buffer);
      glBindRenderbuffer(GL_RENDERBUFFER, stencil_buffer);
      assert(glIsRenderbuffer(stencil_buffer));

      GLenum storage =  GL_STENCIL_INDEX;
      if (samples != 0)
        {
          glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, storage, width, height);}
      else{
          glRenderbufferStorage(GL_RENDERBUFFER, storage, width, height);
        }

      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, stencil_buffer);
    }

  if (depth_buffer && stencil_buffer) {
      attachment = vmtkFrameBufferObject::DepthStencil;
    } else if (depth_buffer) {
      attachment = vmtkFrameBufferObject::Depth;
    } else {
      attachment = vmtkFrameBufferObject::NoAttachment;
    }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
