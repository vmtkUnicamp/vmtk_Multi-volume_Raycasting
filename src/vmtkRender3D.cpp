#include "vmtkRender3D.h"

vmtkRender3D::vmtkRender3D()
{
	m_bInitialized = false;
	m_refThreshold = 0.0f;
	m_blender=0.5f;
    m_rotationMatrix.identity();
	m_mapRef=0;
    m_mapFloat=0;
}

void vmtkRender3D::setAcquisition(Import::ImgFormat *acq1, Import::ImgFormat *acq2)
{
	m_RefData = acq1;
    float volume_real_dimension[3];
	
	m_maxSliceLeft =  m_RefData->dims[0];
    for(int i = 0; i < 3; i++)
        volume_real_dimension[i] = m_RefData->dims[i] * m_RefData->space[i];
	
	float maxDimension = std::max(std::max(volume_real_dimension[0], volume_real_dimension[1]), volume_real_dimension[2]);
	
	for (int i = 0; i < 3; i++) {
        m_refScaleFactors[i] = volume_real_dimension[i] / maxDimension;
    }
    m_refScaleFactors[3] = 1.0;

	
    m_FloatData = acq2;
	for(int i = 0; i < 3; i++)
        volume_real_dimension[i] = m_FloatData->dims[i] * m_FloatData->space[i];

    maxDimension = std::max(std::max(volume_real_dimension[0], volume_real_dimension[1]), volume_real_dimension[2]);
    for (int i = 0; i < 3; i++) {
        m_floatScaleFactors[i] = volume_real_dimension[i] / maxDimension;
    }
    m_floatScaleFactors[3] = 1.0;
	
	for (int i = 0; i < 3; i++) {
		m_refPhyDimensions[i] = m_RefData->dims[i];
    }
    m_refPhyDimensions[3] = 1.0;
	
	for (int i = 0; i < 3; i++) {
		m_floatPhyDimensions[i] = m_FloatData->dims[i];
    }
    m_floatPhyDimensions[3] = 1.0;
}

void vmtkRender3D::setRotation(float ax, float ay, float az)
{
    m_rotationMatrix = m_rotationMatrix.createRotationAroundAxis(ax,ay,az);
}

void vmtkRender3D::setThreshold(int threshold)
{
    m_refThreshold = m_mapRef[(unsigned short)(threshold)] / (pow(2, m_RefData->nbitsalloc) - 1);
}

void vmtkRender3D::setBlender(float blender)
{
    m_blender = blender;
    std::cout<<"blender: "<< blender <<std::endl;
}

void vmtkRender3D::initialize(int width, int height)
{
	Shaders sh;

	this->m_iHeight = width;
	this->m_iWidth = height;

	this->m_BackFBO = new vmtkFrameBufferObject(width, height);
	this->m_FrontFBO = new vmtkFrameBufferObject(width, height);
  
	m_fClipXLeft = -1.0f;
	m_fClipXRight = 1.0f;
	m_fClipYTop = 1.0f;
	m_fClipYBottom = -1.0f;
	m_fClipZBack = 1.0f;
	m_fClipZFront = -1.0f;
	
	m_ColorShader = sh.carregueShaders("./shaders/position_is_color.vert", "./shaders/position_is_color.frag");
    m_RaytraceShader = sh.carregueShaders("./shaders/raytrace.vert", "./shaders/castregister_with_planar_clipping.frag");
	
	loadVolumetoTexture();
	loadTransferFtoTexture();
	
	createVectorPlanes();
	createBuffers();
	initDrawCube();
	initRenderPlane();
	m_bInitialized = true;
}

void vmtkRender3D::createVectorPlanes()
{
    v1=vmath::Vector2f(-1.0f,-1.0f);
    v2=vmath::Vector2f(1.0f,-1.0f);
    v3=vmath::Vector2f(1.0f,1.0f);
    v4=vmath::Vector2f(-1.0f,1.0f);
    vt1=vmath::Vector2f(0.0f,0.0f);
    vt2=vmath::Vector2f(1.0f,0.0f);
    vt3=vmath::Vector2f(1.0f,1.0f);
    vt4=vmath::Vector2f(0.0f,1.0f);
}

void vmtkRender3D::loadVolumetoTexture()
{
	int threshold = 23;
	Equalize eq;

	int intensidade, di, iz, iy, ix, volSize; 
	unsigned short *texbuffer;
	
	// Gera nomes de 1 textura
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &m_refTexture);
	// Define a unidade de alinhamento nos acessos
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	
	//Reference
	
	// Equaliza o histograma para aumentar a escala dinamica das intensidades
	// como uma forma de contornar a perda de resolucao durante normalizacao 
	// feita pelas unidades de textura
	eq.EqualizeHistogram (m_RefData->dims[0], m_RefData->dims[1], m_RefData->dims[2], 
			(reinterpret_cast<unsigned short*>(m_RefData->buffer)), 
						m_RefData->nbitsalloc, m_RefData->umax, &m_mapRef);
						
	volSize = m_RefData->dims[0]*m_RefData->dims[1]*m_RefData->dims[2];
	texbuffer = new unsigned short[volSize];
	memset(texbuffer, 0x00, volSize*sizeof(unsigned short));

	for (di=0, iz = 0; iz < m_RefData->dims[2]; iz++) {
		for (iy =0; iy < m_RefData->dims[1]; iy++) {
		  for (ix =0; ix < m_RefData->dims[0]; ix++) {
			intensidade = (int)((reinterpret_cast<unsigned short*>(m_RefData->buffer))[iz*m_RefData->dims[0]*m_RefData->dims[1]+iy*m_RefData->dims[0]+ix]);
			texbuffer[di++] = (unsigned short)(m_mapRef[intensidade]);
		  }
		}
	}
	
	 m_refThreshold = m_mapRef[(unsigned short)(threshold)] / (pow(2, m_RefData->nbitsalloc) - 1);
	//Carrega o volume equalizado como textura 3D
	glBindTexture(GL_TEXTURE_3D, m_refTexture);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_LUMINANCE, m_RefData->dims[0],
		   m_RefData->dims[1], m_RefData->dims[2], 0, GL_LUMINANCE,
		   GL_UNSIGNED_SHORT, texbuffer);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	delete [] texbuffer;
	
	//Floating
	eq.EqualizeHistogram (m_FloatData->dims[0], m_FloatData->dims[1], m_FloatData->dims[2], 
			(reinterpret_cast<unsigned short*>(m_FloatData->buffer)), 
						m_FloatData->nbitsalloc, m_FloatData->umax, &m_mapFloat);
						
	volSize = m_FloatData->dims[0]*m_FloatData->dims[1]*m_FloatData->dims[2];
	texbuffer = new unsigned short[volSize];
	memset(texbuffer, 0x00, volSize*sizeof(unsigned short));

	for (di=0, iz = 0; iz < m_FloatData->dims[2]; iz++) {
		for (iy =0; iy < m_FloatData->dims[1]; iy++) {
		  for (ix =0; ix < m_FloatData->dims[0]; ix++) {
			intensidade = (int)((reinterpret_cast<unsigned short*>(m_FloatData->buffer))[iz*m_FloatData->dims[0]*m_FloatData->dims[1]+iy*m_FloatData->dims[0]+ix]);
			texbuffer[di++] = (unsigned short)(m_mapFloat[intensidade]);
		  }
		}
	}
	
	glActiveTexture(GL_TEXTURE1);
	glGenTextures(1, &m_floatTexture);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	
	//Carrega o volume equalizado como textura 3D
	glBindTexture(GL_TEXTURE_3D, m_floatTexture);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_LUMINANCE, m_FloatData->dims[0],
		   m_FloatData->dims[1], m_FloatData->dims[2], 0, GL_LUMINANCE,
		   GL_UNSIGNED_SHORT, texbuffer);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	delete [] texbuffer;
}

void vmtkRender3D::loadTransferFtoTexture()
{
	int dim;
	unsigned char *tf;
	TransferFunction tfunc;
	 
	//Reference
	tfunc.GetGrayScaleTF (4, 0, pow(2,m_RefData->nbitsalloc), &dim, &tf, m_RefData->nbitsalloc);
	
	glActiveTexture(GL_TEXTURE2);
	glGenTextures(1, &m_refTF);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	// Carrega a funcao de transferencia como textura 1D
	glBindTexture(GL_TEXTURE_1D, m_refTF);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_LUMINANCE, dim, 0, GL_LUMINANCE,
		   GL_UNSIGNED_BYTE, tf);

	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	//Float
	tfunc.GetGrayScaleTF (4, 0, pow(2,m_FloatData->nbitsalloc), &dim, &tf, m_FloatData->nbitsalloc);
	
	glActiveTexture(GL_TEXTURE3);
	glGenTextures(1, &m_floatTF);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	// Carrega a funcao de transferencia como textura 1D
	glBindTexture(GL_TEXTURE_1D, m_floatTF);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_LUMINANCE, dim, 0, GL_LUMINANCE,
		   GL_UNSIGNED_BYTE, tf);

	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void vmtkRender3D::createBuffers()
{
	/*create buffers to Cube*/
    glGenVertexArrays(1, &vaosCube);
    glGenBuffers(1, &vboCube);
    glGenBuffers(1, &eboCube);
	
	/*create buffer to RenderPlane */
    glGenVertexArrays(1, &vaosRenderPlane);
    glGenBuffers(1, &vboRenderPlane);
    glGenBuffers(1, &eboRenderPlane);
}

void vmtkRender3D::initRenderPlane()
{
     VertexData vertices[] = {
        {v1, vt1},  // v0
        {v2, vt2}, // v1
        {v3, vt3}, // v2
        {v4, vt4},  // v3
    };

    GLushort indices[] = { 0,  1,  2,  3};

    glBindVertexArray(vaosRenderPlane);

    glBindBuffer(GL_ARRAY_BUFFER, vboRenderPlane);
    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(VertexData), vertices, GL_STATIC_DRAW);

    GLuint offset = 0;
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (const void *)offset);

    offset += sizeof(vmath::Vector2f);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (const void *)offset);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboRenderPlane);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * sizeof(GLushort), indices, GL_STATIC_DRAW);
    glBindVertexArray(0);
}

void vmtkRender3D::render()
{
	vmath::Matrix4f mvp;
	
	m_modelViewMatrix=m_translationMatrix*m_rotationMatrix*m_scalationMatrix;
	mvp = m_projectionMatrix * m_modelViewMatrix;
	
	preRender(mvp);
	raycasting();
}

void vmtkRender3D::preRender(vmath::Matrix4f mvp)
{
	glEnable(GL_CULL_FACE);
    
	glCullFace(GL_FRONT);
    this->m_BackFBO->binding();
    glClearColor(0.,0.,0.,1.);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);     //clear buffers
    glViewport(0,0,m_iWidth, m_iHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	this->itlDrawColorCube(mvp);
	this->m_BackFBO->releasing();

    glCullFace(GL_BACK);
    this->m_FrontFBO->binding();
    glClearColor(0.,0.,0.,1.);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);     //clear buffers
    glViewport(0,0,m_iWidth,m_iHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    this->itlDrawColorCube(mvp);
    this->m_FrontFBO->releasing();
}

void vmtkRender3D::raycasting()
{
    glClearColor(0.,0.,0.,1.);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);      //clear buffers
    glViewport(0,0,m_iWidth,m_iHeight);
	
	glUseProgram(m_RaytraceShader);
	
	glUniform1i(glGetUniformLocation(m_RaytraceShader,"width"),  m_RefData->dims[0]);
    glUniform1i(glGetUniformLocation(m_RaytraceShader,"height"), m_RefData->dims[1]);
    glUniform1i(glGetUniformLocation(m_RaytraceShader,"depth"),  m_RefData->dims[2]);
	
	glUniform1i(glGetUniformLocation(m_RaytraceShader,"steps_mode"), 300);
	
	/*Volumes*/
	glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, m_refTexture);
    glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glUniform1i(glGetUniformLocation(m_RaytraceShader, "ref_volumetexture"),0);
	
	glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_3D, m_floatTexture);
    glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glUniform1i(glGetUniformLocation(m_RaytraceShader, "float_volumetexture"),1);
	
	/*Transfer Function*/
	glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_1D, m_refTF);
	glUniform1i(glGetUniformLocation(m_RaytraceShader, "ref_transferfunction"),2);
	
	glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_1D, m_floatTF);
    glUniform1i(glGetUniformLocation(m_RaytraceShader, "float_transferfunction"),3);
	
	/*Front-Back face*/
	glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, this->m_BackFBO->getTexture());
    glUniform1i(glGetUniformLocation(m_RaytraceShader,"backface_fbo"), 4);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, this->m_FrontFBO->getTexture());
    glUniform1i(glGetUniformLocation(m_RaytraceShader,"frontface_fbo"), 5);
	
	//register matrix
	glUniformMatrix4fv(glGetUniformLocation(m_RaytraceShader,"inv_registration_matrix"), 1, GL_TRUE , m_registration_matrix_inv);
	
	glUniform4fv(glGetUniformLocation(m_RaytraceShader,"ref_phyDimensions"), 1, m_refPhyDimensions);
	glUniform4fv(glGetUniformLocation(m_RaytraceShader,"float_phyDimensions"), 1, m_floatPhyDimensions);
	
	glUniform1f(glGetUniformLocation(m_RaytraceShader,"noise_threshold"), m_refThreshold);
    glUniform1f(glGetUniformLocation(m_RaytraceShader,"blending_factor"), m_blender);
	
	drawPlaneRayTraced();
	glUseProgram(0);
}

void vmtkRender3D::drawPlaneRayTraced()
{
    glBindVertexArray(vaosRenderPlane);
    glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
}

void vmtkRender3D::resize(int width, int height)
{
    const float zNear = -100.0f, zFar =100.0f; //, fov = 45.0f;

    m_projectionMatrix = m_projectionMatrix.createOrtho(-1.0f,1.0f,-1.0f,1.0f,zNear,zFar);

    delete this->m_BackFBO;
    delete this->m_FrontFBO;
	
	this->m_BackFBO = new vmtkFrameBufferObject(width, height);
    this->m_FrontFBO = new vmtkFrameBufferObject(width, height);
	
	this->m_iHeight = height;
    this->m_iWidth = width;
}

void vmtkRender3D::itlDrawColorCube(vmath::Matrix4f mvp)
{
	
	initDrawCube();
	
	glUseProgram(m_ColorShader);
	GLuint id = glGetUniformLocation(m_ColorShader,"mvp_matrix");
    glUniformMatrix4fv(id, 1, GL_FALSE, mvp);

    glUniform4fv(glGetUniformLocation(m_ColorShader,"scaleFactors"), 1, m_refScaleFactors);
	drawCube();
	glUseProgram(0);
}

void vmtkRender3D::initDrawCube(){
	
    const float clip_x_left = m_fClipXLeft;
    const float clip_x_right = m_fClipXRight;
    const float clip_y_top = m_fClipYTop;
    const float clip_y_bottom = m_fClipYBottom;
    const float clip_z_front = m_fClipZBack;
    const float clip_z_back = m_fClipZFront;
	
    //left side
    vc1=vmath::Vector3f(clip_x_left, clip_y_bottom, clip_z_back);
    vc2=vmath::Vector3f(clip_x_left, clip_y_bottom, clip_z_front);
    vc3=vmath::Vector3f(clip_x_left, clip_y_top, clip_z_front);
    vc4=vmath::Vector3f(clip_x_left, clip_y_top, clip_z_back);

    //front side
    vc5=vmath::Vector3f(clip_x_left, clip_y_top, clip_z_front);
    vc6=vmath::Vector3f(clip_x_left, clip_y_bottom, clip_z_front);
    vc7=vmath::Vector3f(clip_x_right, clip_y_bottom, clip_z_front);
    vc8=vmath::Vector3f(clip_x_right, clip_y_top, clip_z_front);

    //right side
    vc9=vmath::Vector3f(clip_x_right, clip_y_top, clip_z_back);
    vc10=vmath::Vector3f(clip_x_right, clip_y_top, clip_z_front);
    vc11=vmath::Vector3f(clip_x_right, clip_y_bottom, clip_z_front);
    vc12=vmath::Vector3f(clip_x_right, clip_y_bottom, clip_z_back);

    //back side
    vc13=vmath::Vector3f(clip_x_right, clip_y_top, clip_z_back);
    vc14=vmath::Vector3f(clip_x_right, clip_y_bottom, clip_z_back);
    vc15=vmath::Vector3f(clip_x_left, clip_y_bottom, clip_z_back);
    vc16=vmath::Vector3f(clip_x_left, clip_y_top, clip_z_back);

    //bottom
    vc17=vmath::Vector3f(clip_x_left, clip_y_bottom, clip_z_back);
    vc18=vmath::Vector3f(clip_x_right, clip_y_bottom, clip_z_back);
    vc19=vmath::Vector3f(clip_x_right, clip_y_bottom, clip_z_front);
    vc20=vmath::Vector3f(clip_x_left, clip_y_bottom, clip_z_front);

    //top
    vc21=vmath::Vector3f(clip_x_left, clip_y_top, clip_z_back);
    vc22=vmath::Vector3f(clip_x_left, clip_y_top, clip_z_front);
    vc23=vmath::Vector3f(clip_x_right, clip_y_top, clip_z_front);
    vc24=vmath::Vector3f(clip_x_right, clip_y_top, clip_z_back);
	
	VertexDataCube vertices[] = {
		{vc1},  // 0
		{vc2},  // 1
		{vc3},  // 2
		{vc4},  // 3
		{vc5},  // 4
		{vc6},  // 5
		{vc7},  // 6
		{vc8},  // 7
		{vc9},  // 8
		{vc10}, // 9
		{vc11}, // 10
		{vc12}, // 11
		{vc13}, // 12
		{vc14}, // 13
		{vc15}, // 14
		{vc16}, // 15
		{vc17}, // 16
		{vc18}, // 17
		{vc19}, // 18
		{vc20}, // 19
		{vc21}, // 20
		{vc22}, // 21
		{vc23}, // 22
		{vc24} // 23
	};
	
	GLushort indices[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23};
	glBindVertexArray(vaosCube);
	glBindBuffer(GL_ARRAY_BUFFER, vboCube);
	glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(VertexDataCube), vertices, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (const void *) 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboCube);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 24 * sizeof(GLushort), indices, GL_DYNAMIC_DRAW);
}

bool vmtkRender3D::readMatrix(const char *s)
{
	int k = 0;
	float x;
	std::ifstream file(s);
	if(file.is_open())
	{
		while(file)
		{
			file >> x;
			m_registration_matrix.data[k] = x;
			k++;
		}
		
		std::cout << "Co-register matrix" << std::endl;
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
				std::cout << m_registration_matrix.at(i,j) << " ";
			std::cout << std::endl;
		}
		file.close();
		m_registration_matrix_inv = m_registration_matrix.inverse();
		return true;
	}
	else
		std::cerr << "Matrix not found!" << std::endl;
	return false;
}

void vmtkRender3D::setClipLeftX(float left_x)
{
	m_fClipXLeft = (left_x/m_RefData->dims[0])*2-1;
}

void vmtkRender3D::drawCube()
{
    glBindVertexArray(vaosCube);
    glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_SHORT, 0);
    glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_SHORT, (const GLvoid *)(sizeof(GLushort)*4));
    glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_SHORT, (const GLvoid *)(sizeof(GLushort)*8));
    glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_SHORT, (const GLvoid *)(sizeof(GLushort)*12));
    glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_SHORT, (const GLvoid *)(sizeof(GLushort)*16));
    glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_SHORT, (const GLvoid *)(sizeof(GLushort)*20));

    glBindVertexArray(0);
}

int vmtkRender3D::getMaxSliceLeft()
{
	return m_maxSliceLeft;
}