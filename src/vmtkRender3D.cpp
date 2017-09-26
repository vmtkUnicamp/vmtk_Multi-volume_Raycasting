#include "vmtkRender3D.h"

static int m_pos_activate=-1;

vmtkRender3D::vmtkRender3D()
{
    m_bInitialized = false;
    m_Threshold.push_back(0.0f);
    m_blender=0.5f;
    m_rotationMatrix.identity();
    m_idTexture.clear();
    m_enableMPR=false;
    m_stateMPRInput=false;
}

void vmtkRender3D::volumeRealDimension(Import::ImgFormat * data, float vrd[]){
    for(int i = 0; i < 3; i++) { vrd[i] = data->dims[i] * data->space[i]; }
}

float vmtkRender3D::maximumDimension(float vrd[]){
    return std::max(std::max(vrd[0], vrd[1]), vrd[2]);
}

vmath::Vector4f vmtkRender3D::scaleFactors(float vrd[], float maxDim)
{
    float scaleFactor[4];
    for (int i = 0; i < 3; i++){ scaleFactor[i] = vrd[i] / maxDim; }
    scaleFactor[3]=1.0;
    return vmath::Vector4<float>(scaleFactor);
}

vmath::Vector4f vmtkRender3D::phyDimension(Import::ImgFormat * data){
    float phyDimensions[4];
    for (int i = 0; i < 3; i++){ phyDimensions[i] = data->dims[i]; }
    phyDimensions[3]=1.0;
    return vmath::Vector4<float>(phyDimensions);
}

int vmtkRender3D::currentActivateTexture(){
    GLint nn;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &nn);
    return nn-GL_TEXTURE0;
}

void vmtkRender3D::setAcquisition(std::vector<Import::ImgFormat*> acqVector)
{
    float vrd[3];
    float maxDim;
    m_scaleFactors = new vmath::Vector4f[acqVector.size()];
    m_phyDimensions = new vmath::Vector4f[acqVector.size()];   
    for (int i = 0; i < (int) acqVector.size(); i++) {
        m_data.push_back(acqVector[i]);
        volumeRealDimension( m_data[i], vrd );
        maxDim = maximumDimension(vrd);
        m_scaleFactors[i] = scaleFactors(vrd, maxDim);
        m_phyDimensions[i] = phyDimension(m_data[i]);
    }
    m_maxSliceLeft =  m_data[0]->dims[0];

}

void vmtkRender3D::setRotation(float ax, float ay, float az)
{
    m_rotationMatrix = m_rotationMatrix.createRotationAroundAxis(ax,ay,az);
}

void vmtkRender3D::setThreshold(int threshold)
{
    m_Threshold[0] = m_map[0][(unsigned short)(threshold)] / (pow(2, m_data[0]->nbitsalloc) - 1);
    std::cout<<"Threshold: "<< m_Threshold[0] <<std::endl;
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
	
    m_ColorShader = sh.carregueShaders("../../shaders/position_is_color.vert", "../../shaders/position_is_color.frag");
    m_RaytraceShader = sh.carregueShaders("../../shaders/raytrace.vert", "../../shaders/castregister_with_planar_clipping.frag");
	
    loadVolumesToTextures();
    loadTransferFunctionsToTexture();
	
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


void vmtkRender3D::mapEqualizeHistogramVolume(Import::ImgFormat* data, unsigned int * &map){
    Equalize eq;
    // Equaliza o histograma para aumentar a escala dinamica das intensidades
    // como uma forma de contornar a perda de resolucao durante normalizacao
    // feita pelas unidades de textura
    eq.EqualizeHistogram (data->dims[0], data->dims[1], data->dims[2],
            (reinterpret_cast<unsigned short*>(data->buffer)),
                        data->nbitsalloc, data->umax, &map);
}

void vmtkRender3D::volumeEqualizer(Import::ImgFormat* data, unsigned int *map, unsigned short * &texbuffer){
    int intensidade, di, iz, iy, ix, volSize;
    volSize = data->dims[0]*data->dims[1]*data->dims[2];
    texbuffer = new unsigned short[volSize];
    memset(texbuffer, 0x00, volSize*sizeof(unsigned short));

    for (di=0, iz = 0; iz < data->dims[2]; iz++) {
        for (iy =0; iy < data->dims[1]; iy++) {
          for (ix =0; ix < data->dims[0]; ix++) {
            intensidade = (int) ((reinterpret_cast<unsigned short*>(data->buffer))[iz*data->dims[0]*data->dims[1]+iy*data->dims[0]+ix]);
            texbuffer[di++] = (unsigned short)(map[intensidade]);
          }
        }
    }
}

void vmtkRender3D::texture3DFromVolume(Import::ImgFormat* data, unsigned short * texbuffer, GLuint &idTex){
    GLuint idTexture;
    glGenTextures(1, &idTexture);
    // Define a unidade de alinhamento nos acessos
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glBindTexture(GL_TEXTURE_3D, idTexture);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, data->dims[0],
           data->dims[1], data->dims[2], 0, GL_RED,
           GL_UNSIGNED_SHORT, texbuffer);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    idTex = idTexture;
}

void vmtkRender3D::loadVolumesToTextures()
{
    int threshold = 23;
    m_pos_activate = 0;
    for (int i = 0; i< (int) m_data.size(); i++){
        unsigned short *texbuffer;
        unsigned int *map=0;
        mapEqualizeHistogramVolume(m_data[i],map);
        m_map.push_back(map);
        volumeEqualizer(m_data[i],m_map[i],texbuffer);
        GLuint idTexture;
        glActiveTexture(GL_TEXTURE0+m_pos_activate);
        m_pos_activate = currentActivateTexture()+1;
        texture3DFromVolume(m_data[i],texbuffer,idTexture);
        m_idTexture.push_back(idTexture);
        delete [] texbuffer;
        delete [] map;
    }
    m_Threshold[0] = m_map[0][(unsigned short)(threshold)] / (pow(2, m_data[0]->nbitsalloc) - 1);
}

void vmtkRender3D::generateTransferFunction( Import::ImgFormat* data, int &dim, unsigned char* &tf){
    TransferFunction tfunc;
    tfunc.GetGrayScaleTF (4, 0, pow(2,data->nbitsalloc), &dim, &tf, data->nbitsalloc);
}

void vmtkRender3D::texture1DFromTransferFunction(int dim, unsigned char *tf, GLuint &idTF){
    GLuint idTransferFunction;
    glGenTextures(1, &idTransferFunction);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    // Carrega a funcao de transferencia como textura 1D
    glBindTexture(GL_TEXTURE_1D, idTransferFunction);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, dim, 0, GL_RED,
           GL_UNSIGNED_BYTE, tf);

    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    idTF = idTransferFunction;
}

void vmtkRender3D::loadTransferFunctionsToTexture()
{
    for(int i = 0; i < (int) m_data.size(); i++){
        int dim;
        unsigned char *tf;
        generateTransferFunction(m_data[i],dim,tf);
        GLuint idTransferFunction;
        glActiveTexture(GL_TEXTURE0+m_pos_activate);
        m_pos_activate = currentActivateTexture()+1;
        texture1DFromTransferFunction(dim,tf,idTransferFunction);
        m_idTF.push_back(idTransferFunction);
        delete [] tf;
    }
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

    int offset = 0;
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (const void *)(intptr_t)offset);

    offset += sizeof(vmath::Vector2f);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (const void *)(intptr_t)offset);

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
    raycastingMultiVolume();
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


void vmtkRender3D::raycastingMultiVolume()
{
    glClearColor(0.,0.,0.,1.);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);      //clear buffers
    glViewport(0,0,m_iWidth,m_iHeight);

    glUseProgram(m_RaytraceShader);

    glUniform1i(glGetUniformLocation(m_RaytraceShader,"width"),  m_data[0]->dims[0]);
    glUniform1i(glGetUniformLocation(m_RaytraceShader,"height"), m_data[0]->dims[1]);
    glUniform1i(glGetUniformLocation(m_RaytraceShader,"depth"),  m_data[0]->dims[2]);
    glUniform1i(glGetUniformLocation(m_RaytraceShader,"steps_mode"), 300);

    /*Volumes*/
    m_pos_activate = 0;
    glUniform1i(glGetUniformLocation(m_RaytraceShader,"nVolumes"), m_data.size());
    /*volumes 3D */
    for(int i = 0; i < (int) m_data.size(); i++){
        glActiveTexture(GL_TEXTURE0+m_pos_activate);
        glBindTexture(GL_TEXTURE_3D, m_idTexture[i]);
        glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        std::string nameVolume = "vTexture"+NumberToString(i);
        glUniform1i(glGetUniformLocation(m_RaytraceShader, nameVolume.c_str() ), m_pos_activate );
        m_pos_activate = currentActivateTexture()+1;
    }


    /*Transfer Function*/
    for(int i = 0; i < (int) m_data.size(); i++){
        glActiveTexture(GL_TEXTURE0+m_pos_activate);
        glBindTexture(GL_TEXTURE_1D, m_idTF[i]);
        std::string nameTF = "vTF"+NumberToString(i);
        glUniform1i(glGetUniformLocation(m_RaytraceShader, nameTF.c_str()),m_pos_activate);
        m_pos_activate = currentActivateTexture()+1;
    }

    glUniform4fv(glGetUniformLocation(m_RaytraceShader,"phyDimensions"), m_data.size(), (const GLfloat*)m_phyDimensions);

    //register matrix
    glUniformMatrix4fv(glGetUniformLocation(m_RaytraceShader,"inv_registration_matrix"), m_data.size()-1, GL_TRUE , (const GLfloat*) m_invRegMatrix);

    if(m_stateMPRInput){
        glUniform1i(glGetUniformLocation(m_RaytraceShader,"enableMPR"), m_enableMPR);
        if(m_enableMPR){
            glUniform4fv(glGetUniformLocation(m_RaytraceShader,"clipping_plane"), 1, m_equationPlaneForMPR);
        }
    }
    else{
        glUniform1i(glGetUniformLocation(m_RaytraceShader,"enableMPR"), false);
    }

    glUniform1f(glGetUniformLocation(m_RaytraceShader,"noise_threshold"), m_Threshold[0]);
    glUniform1f(glGetUniformLocation(m_RaytraceShader,"blending_factor"), m_blender);

    /*Front-Back face*/
    glActiveTexture(GL_TEXTURE0+m_pos_activate);
    glBindTexture(GL_TEXTURE_2D, this->m_BackFBO->getTexture());
    glUniform1i(glGetUniformLocation(m_RaytraceShader,"backface_fbo"), m_pos_activate);
    m_pos_activate = currentActivateTexture()+1;

    glActiveTexture(GL_TEXTURE0+m_pos_activate);
    glBindTexture(GL_TEXTURE_2D, this->m_FrontFBO->getTexture());
    glUniform1i(glGetUniformLocation(m_RaytraceShader,"frontface_fbo"), m_pos_activate);

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
	
//	initDrawCube();
	
	glUseProgram(m_ColorShader);
	GLuint id = glGetUniformLocation(m_ColorShader,"mvp_matrix");
    glUniformMatrix4fv(id, 1, GL_FALSE, mvp);

    glUniform4fv(glGetUniformLocation(m_ColorShader,"scaleFactors"), 1, m_scaleFactors[0]);
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

bool vmtkRender3D::readMatrix(const char *s, vmath::Matrix4f& invRM)
{
	int k = 0;
    float x;
    vmath::Matrix4f registrationMatrix;
	std::ifstream file(s);
	if(file.is_open())
	{
		while(file)
		{
			file >> x;
            registrationMatrix.data[k] = x;
			k++;
		}
		
		std::cout << "Co-register matrix" << std::endl;
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
                std::cout << registrationMatrix.at(i,j) << " ";
			std::cout << std::endl;
		}
        file.close();
        invRM = registrationMatrix.inverse();
		return true;
	}
	else
		std::cerr << "Matrix not found!" << std::endl;
	return false;
}


bool vmtkRender3D::readPlane(const char *s,vmath::Vector4f& eqp)
{
    int k = 0;
    float x;
    std::ifstream file(s);
    float t[4];
    if(file.is_open())
    {
        while(file)
        {
            file >> x;
            t[k] = x;
            k++;
        }
        vmath::Vector4f eqPlane(t);

        std::cout << "Equation Plane" << std::endl;
        for (int i = 0; i < 4; i++)
        {
            std::cout << eqPlane[i] << std::endl;
        }
        file.close();
        eqp = eqPlane;
        return true;
    }
    else
        std::cerr << "Equation plane not found!" << std::endl;
    return false;
}

void vmtkRender3D::setEnableMPR(bool enableMPR){
    m_enableMPR = enableMPR;
}

void vmtkRender3D::setStateMPRInput(bool stateMPRInput)
{
    m_stateMPRInput=stateMPRInput;
}

void vmtkRender3D::setVectorInvMatrixReg(std::vector<vmath::Matrix4f> imr){
    m_invRegMatrix = new vmath::Matrix4f[ (int) imr.size() ];
    for(int i = 0; i < (int) imr.size(); i++){
        m_invRegMatrix[i] = imr[i];
    }
}

void vmtkRender3D::setMPR(vmath::Vector4f eqp){
    m_equationPlaneForMPR = eqp;
}

void vmtkRender3D::setClipLeftX(float left_x)
{
    m_fClipXLeft = (left_x/m_data[0]->dims[0])*2-1;
    initDrawCube();
}

void vmtkRender3D::drawCube()
{
    glBindVertexArray(vaosCube);
    GLsizei count [] = {4, 4, 4, 4, 4, 4};
    const GLvoid *indices[] = {0,
                               (const GLvoid *)(sizeof(GLushort) * 4),
                               (const GLvoid *)(sizeof(GLushort) * 8),
                               (const GLvoid *)(sizeof(GLushort) * 12),
                               (const GLvoid *)(sizeof(GLushort) * 16),
                               (const GLvoid *)(sizeof(GLushort) * 20)};

    glMultiDrawElements(GL_TRIANGLE_FAN, count, GL_UNSIGNED_SHORT, indices, 6);
    glBindVertexArray(0);
}

int vmtkRender3D::getMaxSliceLeft()
{
	return m_maxSliceLeft;
}
