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

vmtkRender3D volumeRender;
GLfloat spin_x=0.0, spin_y=0.0, spin_z=0.0;
GLuint VIEWPORT_WIDTH=600, VIEWPORT_HEIGHT=600;
int threshold=23;
float blender=0.5f;
int slice_x = 0;

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
        break;
    case '-':
        /* Diminue o limiar */
        threshold -= 1;
        break;
    case 'b':
    case 'B':
        blender += 0.1;
		if (blender > 1.0)
			blender = 1.0;
        break;
    case 'n':
    case 'N':
		blender -= 0.1;
		if (blender < 0.0)
			blender = 0.0;
        break;
	case 'o':
	case 'O':
		slice_x++;
		if (slice_x > volumeRender.getMaxSliceLeft())
			slice_x = volumeRender.getMaxSliceLeft();
		break;
	case 'p':
	case 'P':
		slice_x--;
		if (slice_x < 0)
			slice_x = 0;
		break;
    default:
        return;
    }
	
	volumeRender.setClipLeftX(slice_x);
	volumeRender.setRotation(spin_x,spin_y,spin_z);
	if(threshold<=0){threshold=0;}
	volumeRender.setThreshold(threshold);
    volumeRender.setBlender(blender);
	
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
}

int main(int argc, char *argv[])
{
	Import load;
	Import::ImgFormat data1, data2;
	string path1, path2;
		
	if( argc < 2 )
	{
		cerr << "Usage: " << endl;
		cerr << argv[0] << " inputVolume1  inputVolume2 registeredMatrix" << endl;
		return EXIT_FAILURE;
	}
	
	//reading first volume
	path1 = argv[1];
	data1.umax = -pow(2,30);
    data1.umin = pow(2,30);
	if (!load.DICOMImage (path1, &data1)) {
      std::cout << path1 << " Cannot be imported!" << std::endl;
      return 0;
    }
	
	//reading second volume
	path2 = argv[2];
	data2.umax = -pow(2,30);
    data2.umin = pow(2,30);
	if (!load.DICOMImage (path2, &data2)) {
      std::cout << path1 << " Cannot be imported!" << std::endl;
      return 0;
    }
	
	volumeRender.setAcquisition(&data1, &data2);
	if(!volumeRender.readMatrix(argv[3]))
		return 0;
	
	/* Initializa OpenGL */
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
	  
	/* Seta contexto GL */
	glutCreateWindow("VMTK-SPS");
	std::cout << "OpenGL version supported by this platform: " << glGetString(GL_VERSION) << std::endl;
	
	glewInit();	
	init();
	
	/* Configura a janela */
	glutPositionWindow(10, 10);
	glutReshapeWindow(VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
	
	glutReshapeFunc(reshape);
	glutDisplayFunc(drawProxyGeometry);
	glutKeyboardFunc(keyboard);
	
	/* Espera por eventos */
	glutMainLoop();
	
	return 0;
}