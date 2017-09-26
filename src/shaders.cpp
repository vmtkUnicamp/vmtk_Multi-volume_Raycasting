/*  
 *  shaders.cpp: compila, liga e transfere shaders para GPU
 *
 *  Copyright (C) 2013  Wu Shin-Ting, FEEC, Unicamp
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shaders.h"

#define printOpenGLError() printOglError(__FILE__, __LINE__)
 
char *Shaders::textFileRead(const char *fn) {
  FILE *fp;
  char *content = NULL;

  int count=0;

  if (fn != NULL) {
    fp = fopen(fn,"rt");

    if (fp != NULL) {
      
      fseek(fp, 0, SEEK_END);
      count = ftell(fp);
      rewind(fp);

      if (count > 0) {
	content = (char *)malloc(sizeof(char) * (count+1));
	count = fread(content,sizeof(char),count,fp);
	content[count] = '\0';
      }
      fclose(fp);
    }
	else{
		std::cerr << "Shader not found!" << std::endl;
	}
  }
  
  return content;
}

int Shaders::printOglError(char *file, int line)
{
    // Returns 1 if an OpenGL error occurred, 0 otherwise.
    GLenum glErr;
    int    retCode = 0;
 
    glErr = glGetError();
    while (glErr != GL_NO_ERROR)
    {
        printf("glError in file %s @ line %d: %s\n", file, line, gluErrorString(glErr));
        retCode = 1;
        glErr = glGetError();
    }
    return retCode;
}
 
void Shaders::printShaderInfoLog(GLuint obj)
{
	int status = 0;
    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;
 
    glGetShaderiv(obj, GL_INFO_LOG_LENGTH,&infologLength);
    if (infologLength > 0)
    {
        infoLog = (char *)malloc(infologLength);
        glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
        printf("%s\n",infoLog);
        free(infoLog);
    }
	
	glGetShaderiv(obj, GL_COMPILE_STATUS,&status);
	if (status)
		printf("Shader compiled\n");
	else
		printf("Shader not compiled\n");
	
}
 
void Shaders::printProgramInfoLog(GLuint obj)
{
    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;
 
    glGetProgramiv(obj, GL_INFO_LOG_LENGTH,&infologLength);
 
    if (infologLength > 0)
    {
        infoLog = (char *)malloc(infologLength);
        glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
        printf("%s\n",infoLog);
        free(infoLog);
    }
}
 
GLuint Shaders::carregueShaders(const char *vertexFileName, const char *fragmentFileName) {
 
    const GLchar *vs = NULL,*fs = NULL;
 
    GLuint p,v,f;
 
    // Cria e compila shader de vértices
    v = glCreateShader(GL_VERTEX_SHADER);
    vs = (const GLchar *)textFileRead(vertexFileName);
    glShaderSource(v, 1, &vs, NULL);
    free((char *)vs);
    glCompileShader(v);
    printShaderInfoLog(v);

    // Cria e compila shader de fragmentos
    f = glCreateShader(GL_FRAGMENT_SHADER);
    fs = (const GLchar *)textFileRead(fragmentFileName);
    glShaderSource(f, 1, &fs,NULL);
    free((char *)fs);
    glCompileShader(f);
    printShaderInfoLog(f);
 
    // Cria um programa e anexa os shaders a ele
    p = glCreateProgram();
    glAttachShader(p,v);
    glAttachShader(p,f);
 
    // Liga os elementos do programa
    glLinkProgram(p);
    printProgramInfoLog(p);
 
    return(p);
}


