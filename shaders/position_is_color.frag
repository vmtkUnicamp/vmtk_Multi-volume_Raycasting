#version 330
/**
  * @file position_is_color.frag
  * @brief The color interpolated from the vertices colors is attributed to the pixel.
  * @author Wu Shin-Ting
  * @date January 2015
  * @namespace GLSL
  * @class PositioIsColourFS
  */
in vec4 vColor;          /**< interpolated color */
out vec4 Color;

/**
* @fn void main(void)
* @brief Main function
*/

void main(void)
{
    Color=vColor;
}
