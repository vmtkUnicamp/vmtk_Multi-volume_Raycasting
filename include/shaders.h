/*  
 *  shaders.h 
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
#define FREEGLUT_STATIC

#ifndef _shaders_h
#define _shaders_h

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

#include <GL/glut.h> 
#include <iostream>

class Shaders {
 public:
  GLuint carregueShaders(const char *vertexFileName, const char *fragmentFileName);

 private:
  /**
   * @brief textFileRead
   * @param [in] fn: file name.
   * @return text string of file loaded.
   */
  char *textFileRead(const char *fn);

  /**
   * @brief printOglError
   * @param [in] file: text string of file loaded.
   * @param [in] line: line of text.
   * @return Returns 1 if an OpenGL error occurred, 0 otherwise.
   */
  int printOglError(char *file, int line);

  /**
   * @brief printShaderInfoLog
   * @param [in] obj: specifies the shader object to be queried.
   */
  void printShaderInfoLog(GLuint obj);

  /**
   * @brief printProgramInfoLog
   * @param [in] obj: specifies the program object to be queried.
   */
  void printProgramInfoLog(GLuint obj);

};

#endif

