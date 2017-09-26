#define FREEGLUT_STATIC

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

#include <sys/types.h>
#include <sys/stat.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <iostream>
#include <stdio.h>     
#include <stdint.h>
#include <stdlib.h>    
#include <string.h>
#include <limits.h>
#include <math.h>
#include <GL/freeglut.h>   

#include "vmtkRender3D.h"

using namespace std;

static vmtkRender3D volumeRender;
static GLfloat spin_x=0.0, spin_y=0.0, spin_z=0.0;
static GLuint VIEWPORT_WIDTH=900, VIEWPORT_HEIGHT=900;
static int threshold = 23;
static float blender=0.5f;
static int slice_x = 0;
static int window;
static int value = 0;
static bool m_mprPreState = false;
static bool m_mprState = false;
void init()
{
    volumeRender.initialize(VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
}

void keyboard(unsigned char key, int x, int y)
{

switch (key) {
    case 'w':
    case 'W':
        /* incrementa +5.0 no ângulo de rotação em torno do eixo x */
        spin_x += 5.0;
        break;
    case 's':
    case 'S':
        /* incrementa +5.0 no ângulo de rotação em torno do eixo x */
        spin_x -= 5.0;
        break;
    case 'q':
    case 'Q':
        /* incrementa +5.0 no ângulo de rotação em torno do eixo y */
        spin_y += 5.0;
        break;
    case 'e':
    case 'E':
        /* incrementa +5.0 no ângulo de rotação em torno do eixo y */
        spin_y -= 5.0;
        break;
    case 'a':
    case 'A':
        /* incrementa +5.0 no ângulo de rotação em torno do eixo z */
        spin_z += 5.0;
        break;
    case 'd':
    case 'D':
        /* incrementa +5.0 no ângulo de rotação em torno do eixo z */
        spin_z -= 5.0;
        break;
    case 'r':
    case 'R':
        /* Reseta */
        spin_x = spin_y = spin_z = 0.0;
        break;
    case '+':
        /* Aumenta o limiar */
        threshold += 1;
        if(threshold<=0){ threshold = 0; }
        std::cout<<"set threshold (int): "<<threshold<<std::endl;
        volumeRender.setThreshold(threshold);
        break;
    case '-':
        /* Diminue o limiar */
        threshold -= 1;
        if(threshold<=0){ threshold = 0; }
        std::cout<<"set threshold (int): "<<threshold<<std::endl;
        volumeRender.setThreshold(threshold);
        break;
    case 'b':
    case 'B':
        blender += 0.1;
        if (blender > 1.0){
			blender = 1.0;
        }
        volumeRender.setBlender(blender);
        break;
    case 'n':
    case 'N':
		blender -= 0.1;
        if (blender < 0.0){
			blender = 0.0;
        }
        volumeRender.setBlender(blender);
        break;
	case 'o':
	case 'O':
		slice_x++;
        if (slice_x > volumeRender.getMaxSliceLeft()){
			slice_x = volumeRender.getMaxSliceLeft();
        }
        volumeRender.setClipLeftX(slice_x);
		break;
	case 'p':
	case 'P':
		slice_x--;
        if (slice_x < 0){
			slice_x = 0;
        }
        volumeRender.setClipLeftX(slice_x);
		break;
    case '1':
        if(m_mprState){
            volumeRender.setEnableMPR(true);
        }
        break;
    case '2':
        if(m_mprState){
            volumeRender.setEnableMPR(false);
        }
        break;
    default:
        return;
    }



	volumeRender.setRotation(spin_x,spin_y,spin_z);


	
  /* Gera o evento "Display" */
  glutPostRedisplay();
}

void reshape(int w, int h)
{
	volumeRender.resize(w, h);
}

void drawProxyGeometry()
{
	volumeRender.render();
	glutSwapBuffers();

    glutPostRedisplay();
}

void menu(int num){
  if(num == 0){
    glutDestroyWindow(window);
    exit(0);
  }else{
    value = num;
  }
  glutPostRedisplay();
 }

int main(int argc, char *argv[])
{
    Import load;
    Import::ImgFormat *data;
    string *path;
    if( argc == 1 )
	{
        cerr << "Minimum usage: " << endl;
		cerr << argv[0] << " inputVolume1  inputVolume2 registeredMatrix" << endl;
		return EXIT_FAILURE;
	}
    int nParameters = argc-1;
    if( nParameters >12 ){
        cerr << "Too many parameters." << std::endl;
        cerr << "Maximum paramters usage: 12." << endl;
        return EXIT_FAILURE;
    }

    std::cout << "Numbers of parameters: " << nParameters << std::endl;
    int nVolumes=0;
    int nMatrices=0;

    if(nParameters%2==0){
        nVolumes = nParameters/2;
        nMatrices = nVolumes-1;
        std::cout << "Numbers of volumes for register: " << nVolumes <<std::endl;
        std::cout << "Numbers of matrices for register: " << nMatrices <<std::endl;
        std::cout << "With equation plane for multiplanar reformation." << std::endl;
        m_mprPreState = true;
    }
    else{
        nVolumes = (nParameters+1)/2;
        nMatrices = nVolumes-1;
        std::cout << "Numbers of volumes for register: " << nVolumes <<std::endl;
        std::cout << "Numbers of matrices for register: " << nMatrices <<std::endl;
        std::cout << "Without equation plane for multiplanar reformation." << std::endl;
        m_mprPreState = false;
    }
    data = new Import::ImgFormat[nVolumes];
    path = new string[nVolumes];

    //reading volumes
    std::vector<Import::ImgFormat*> acqVector;
    for(int i = 0; i< nVolumes; i++){
        path[i] = argv[i+1];
        data[i].umax = -pow(2,30);
        data[i].umin = pow(2,30);
        if (!load.DICOMImage (path[i], &data[i])) {
          std::cout << path[i] << "Cannot be imported!" << std::endl;
          return 0;
        }
        acqVector.push_back(&data[i]);
    }
    volumeRender.setAcquisition(acqVector);

    std::vector<vmath::Matrix4f> vectorInvMatrixReg;
    for(int i = 0; i< nMatrices; i++){
        vmath::Matrix4f imrv;
        if(!volumeRender.readMatrix(argv[i+1+nVolumes] ,imrv)){ return 0; }
        vectorInvMatrixReg.push_back(imrv);
    }
    volumeRender.setVectorInvMatrixReg(vectorInvMatrixReg);

    vmath::Vector4f eqp;
    if(m_mprPreState){
        if(!volumeRender.readPlane(argv[1+nVolumes+nMatrices], eqp) ){
            m_mprState=false; return 0;
        }
        m_mprState=true;
        volumeRender.setEnableMPR(false);
        volumeRender.setMPR(eqp);      
    }
    else{
        m_mprState=false;
    }
    volumeRender.setStateMPRInput(m_mprState);




	/* Initializa OpenGL */
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
	  
	/* Seta contexto GL */
    window = glutCreateWindow("VMTK-SPS");
	std::cout << "OpenGL version supported by this platform: " << glGetString(GL_VERSION) << std::endl;
	
	glewInit();	
	init();
	
	/* Configura a janela */
	glutPositionWindow(10, 10);
	glutReshapeWindow(VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
	
	glutReshapeFunc(reshape);
	glutDisplayFunc(drawProxyGeometry);
	glutKeyboardFunc(keyboard);

    /** teste popmenu **/
//    bool ivan_menu=false;
//    if(ivan_menu){
//        int men = glutCreateMenu(menu);
//        //add entries to our menu
//        glutAddMenuEntry("Blender Volume 1-2",1);
//        glutAddMenuEntry("Blender Volume 1-3",2);
//        glutAddMenuEntry("Blender Volume 1-4",3);
//        glutAddMenuEntry("Blender Volume 1-5",4);
//        glutAddMenuEntry("Blender Volume 1-6",5);
//        // attach the menu to the right button
//        glutAttachMenu(GLUT_RIGHT_BUTTON);
//    }
	
	/* Espera por eventos */
	glutMainLoop();
	
	return 0;
}
