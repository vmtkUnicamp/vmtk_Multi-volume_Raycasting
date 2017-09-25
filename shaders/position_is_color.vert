#version 330
/**
  * @file position_is_color.vert
  * @brief The color of the vertex is computed on the basis of its coordinates.
  * @author Wu Shin-Ting
  * @date January 2015
  * @namespace GLSL
  * @class PositioIsColourVS
  */

layout (location=0) in vec4 position;
out vec4 vColor;    /**< color (R,G,B,A) to be passed to position_is_color.fs */
uniform vec4 color;
uniform mat4 mvp_matrix;
uniform vec4 scaleFactors;

/**
 * @fn void main (void)
 * @brief Main function
 */
void main (void)
{  
    vColor = (position + vec4(1.0)) * 0.5;    // vertex color
    gl_Position = mvp_matrix*(scaleFactors*position);   // vertex coordinates
}
