#ifndef VMTKRENDER3D_H
#define VMTKRENDER3D_H

#include "vmtkFrameBufferObject.h"
#include "importDicom.h"
#include "shaders.h"
#include "vmath.h"
#include "transferFunction.h"
#include "equalize.h"

#include<fstream>

class vmtkRender3D
{
	public:
	
	    /**
		* @brief vmtkRenderer3D constructor
		*/
		vmtkRender3D();
		/**
		 * @brief sets the volumes to be registered
		 * @param[in] volume 1 (reference)
		 * @param[in] volume 2 (float)
		 */
		void setAcquisition(Import::ImgFormat *acq1, Import::ImgFormat *acq2);
		/**
		* @brief initializes 3D volume rendering
		*/
		void initialize(int width, int height);
		/**
		* @brief renders the data with the specified redering mode.
		*/
		void render();
		/**
		* @brief renders the color cube in the fbos.
		* @param[in] modelview matrix
		*/
		void preRender(vmath::Matrix4f mvp);
		/**
		 * @brief resizes the dimensions of the display
		 * @param[in] w width of image
		 * @param[in] h height of image
		 */
		void resize(int width, int height);
		/**
		* @brief sets the rotation matrix.
		* @param[in] rotation matrix
		*/
        void setRotation(float ax, float ay, float az);
		/**
		* @brief sets the threshold.
		* @param[in]  threshold
		*/
        void setThreshold(int threshold);
		/**
		* @brief sets the blender factor.
		* @param[in]  blender factor
		*/
        void setBlender(float blender);
		/**
		* @brief draws a unit cube to get the front and the back depth maps of the volume cube,
		* after updating the clipping plane values.
		* @param[in]  mvp projection*modelview matrix
		*/
        void itlDrawColorCube(vmath::Matrix4f mvp);
		
		/**
		* @brief initializes the geometry of a cube to be rendered for getting back and front depth maps.
		*/
		void initDrawCube();
		
		/**
		* @brief reads the co-register matrix.
		* @param[in] co-register matrix file
		*/
		bool readMatrix(const char *s);
		
		/**
		* @brief sets the clipping plane.
		* @param[in] slice
		*/
		void setClipLeftX(float left_x);
		
		/**
		* @brief gets the maximum number of the slices from the axis X.
		*/
		int getMaxSliceLeft();
	
	private:
	
		int m_maxSliceLeft;
		vmath::Matrix4f m_registration_matrix, m_registration_matrix_inv;
		float m_refThreshold, m_blender;
		unsigned int *m_mapRef, *m_mapFloat;
		Import::ImgFormat *m_RefData, *m_FloatData;
		GLuint m_refTexture;
		GLuint m_floatTexture;
		GLuint m_refTF;
		GLuint m_floatTF;
	    GLuint m_ColorShader;    /**< pre-processing shader to get front and back planes */
		GLuint m_RaytraceShader; /**< raycast shader */
	
		GLuint vaosCube,             /**< drawCube: vertex array object */
			vboCube,                 /**< drawCube: data buffer */
			eboCube,                 /**< drawCube: element array buffer */
			vaosRenderPlane,
			vboRenderPlane,
			eboRenderPlane,
			vaosResultPlane,      /**< render: vertex array object */
			vboResultPlane,          /**< render: data buffer */
			eboResultPlane;          /**< render: element array buffer */;
			
		/**
		 * @struct VertexDataCube
		 */
		struct VertexDataCube
		{
			vmath::Vector3f position; /**< position coordinates */
		};

		/**
		* @struct VertexData
		*/
		struct VertexData
		{
			vmath::Vector2f position; /** < position coordinates */
			vmath::Vector2f texCoord; /** < texture coordinates */
		};
		 /**
		 * @brief Position vectors of VertexData.
		 */
		vmath::Vector2f v1, v2, v3, v4;
		/**
		 * @brief Texture vectors of VertexData.
		 */
		vmath::Vector2f vt1, vt2, vt3, vt4;
		/**
		* @brief Position vectors of VertexDataCube.
		*/
		vmath::Vector3f vc1, vc2, vc3, vc4, vc5, vc6, vc7, vc8, vc9, vc10, vc11, vc12,
                    vc13, vc14, vc15, vc16, vc17, vc18, vc19, vc20, vc21, vc22, vc23, vc24;
	
		float m_fClipXLeft;                       /**< x left clip plane */
		float m_fClipXRight;                      /**< x right clip plane */
		float m_fClipYTop;                        /**< y top clip plane */
		float m_fClipYBottom;                     /**< y bottom clip plane */
		float m_fClipZBack;                       /**< z back clip plane */
		float m_fClipZFront;                      /**< z front clip plane */
	
	    int m_iHeight;                            /**< offscreen image height */
		int m_iWidth;                             /**< offscreen image width */
	
        vmath::Matrix4f m_projectionMatrix;  /**< projection transformation matrix */
		vmath::Matrix4f m_modelViewMatrix;  /**< modelview transformation matrix */
		vmath::Matrix4f m_rotationMatrix; /**< rotationMatrix transformation matrix */
		vmath::Matrix4f m_scalationMatrix; /**< scalationMatrix transformation matrix */
		vmath::Matrix4f m_translationMatrix; /**< traslationMatrix transformation matrix */
		
		float m_refScaleFactors[4];               /**< scale factor for compensating distortions on the reference texture volume */
		float m_floatScaleFactors[4];             /**< scale factor for compensating distortions on the floating texture volume */
		float m_refPhyDimensions[4];            /**< physical dimensions of reference volume */
		float m_floatPhyDimensions[4];
		
		vmtkFrameBufferObject *m_FrontFBO;      /**< framebuffer object for front side */
		vmtkFrameBufferObject *m_BackFBO;       /**< framebuffer object for back side */
		vmtkFrameBufferObject *m_RayTracedFBO;  /**< framebuffer object for raycasted image */
		
		bool m_bInitialized;                      /**< whether the volume renderer is initialized ? */

		/**
		* @brief defines the position and texture coordinates of the four vertices of a plane.
		*/
		void createVectorPlanes();
		
		/**
		* @brief loads volume to texture.
		*/
		void loadVolumetoTexture();
		/**
		* @brief loads transfer function to texture.
		*/
		void loadTransferFtoTexture();
		
		/**
		* @brief creates vertex arrays and their buffer data.
		*/
		void createBuffers();
		
		/**
		 * @brief initializes the plane to be input into the graphics pipeline
		 */
		void initRenderPlane();
		/**
		* @brief draws a cube.
		*/
		void drawCube();
		/**
		* @brief raycasts through the color cube.
		*/
		void raycasting();
		
		/**
		* @brief draws the plane from pre-render shader.
		*/
		void drawPlaneRayTraced();
};

#endif // VMTKRENDER3D_H
