#ifndef FRAMEBUFFEROBJECT_H
#define FRAMEBUFFEROBJECT_H

#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif
#ifdef __WIN32__
#include <GL/glew.h>
#elif __linux__
#include <GL/glew.h>
#elif __APPLE__
#include <glew.h>
#endif

#include <iostream>
#include <assert.h>

class vmtkFrameBufferObject
{
	public:

		
		/**
		*@enum Attachment
		*@brief Possible images to be attached to the FBO
		*/
		enum Attachment {
			NoAttachment,     /**< no attachment */
			DepthStencil,     /**< depth and stencil buffer */
			Depth,            /**< depth buffer */
			Stencil           /**< stencil buffer */
		};
		
        /**
        * @brief constructs a FrameBufferObject without arguments
        */
		vmtkFrameBufferObject();
		~vmtkFrameBufferObject();
		
        /**
        * @brief constructs a FrameBufferObject with arguments (target)
        */
        /**
         * @brief constructs a FrameBufferObject with arguments.
         * @param [in] width: width based on the supplied format.
         * @param [in] height: height based on the supplied format.
         * @param [in] target: specify the GL texture target (default target = GL_TEXTURE_2D).
         * @param [in] option: is a type enum (Attachment) for configure the depth and stencil buffers attached to the framebuffer (default option = NoAttachment).
         */
        vmtkFrameBufferObject(int width, int height, GLenum target = GL_TEXTURE_2D, Attachment option = NoAttachment);
		
		GLenum target;          /**< target of the image to be bound to the attachment point in the FBO */
		Attachment attachment;  /**< attachment point to which an image from texture should be attached */
		
		/**
		 * @brief gets the Fbo name.
		 * @return the FBO name
		 */
		GLuint getFbo() const;

		/**
		 * @brief gets the texture bound to the FBO.
		 * @return the texture object name
		 */
		GLuint getTexture() const;

		/**
		 * @brief gets the depth texture bound to the FBO.
		 * @return the texture object name
		 */
		GLuint getDepthTexture() const;

		/**
		* @brief binds the FBO to the framebuffer target.
		* @return true (successful) or false (failed)
		*/
		bool binding();

		/**
		* @brief releases the FBO from the framebuffer target.
		* @return true (successful) or false (failed)
		*/
		bool releasing();
		/**
		 * @brief resizing FrameBufferObject dimensions ... It does not work!!!! (Ting)
		 * @param[in] width buffer image width
		 * @param[in] height buffer image height
		 */
		void resizing(int width,int height);
	
	private:
        GLuint fbo;                 /**< FBO name */
        GLuint texture;             /**< the bound texture name */
        GLuint depthTexture;        /**< the bound depth texture name */

//		GLuint vec_textures[2];
		
        /**
         * @brief initial parameters
         * @param [in] width: width based on the supplied format.
         * @param [in] height: height based on the supplied format.
         * @param [in] attachment: configure the depth and stencil buffers attached to the framebuffer.
         * @param [in] internal_format: specifies the number of color components in the texture (default internal_format in costructor is 'GL_RGBA').
         * @param [in] target: specify the GL texture target (default target in costructor is 'GL_TEXTURE_2D').
         * @param [in] samples: number of samples per pixel. (default samples = 0).
         * @param [in] mipmap: state of mipmapping (default mipmap = false).
         */
		void init(int width, int height,
              Attachment attachment,
              GLenum internal_format, GLenum target,
              GLint samples = 0, bool mipmap = false);
};

#endif
