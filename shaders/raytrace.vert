#version 330
/**
  * @file raytrace.vert
  * @brief The normal vector, the texture coordinates of the vertex, and the modelview matrix are passed to raytrace.fs.
  * @author José Angel Ivan Rubianes Silva
  * @date February 2015
  * @namespace GLSL
  * @class RaytraceVS
  */

layout (location=0) in vec4 position;
layout (location=1) in vec2 textCoord;
out vec2 TexCoord; /**< texture coordinates associaed to the vertex */

/*
* @fn void main (void)
* @brief Main function
*/
void main (void)
{  
    gl_Position = position;             /**< vertex corrdinates (x,y,z,w) */
    TexCoord = textCoord;   /**< vertex´s texture coordinates (r,g,b,a) */
}
