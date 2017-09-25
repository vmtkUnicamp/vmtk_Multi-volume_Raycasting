#version 330
/**
  * @file drawresult.vert
  * @brief The texture coordinates of the vertex is passed to drawresult.fs.
  * @author José Angel Ivan Rubianes Silva
  * @date February 2015
  * @note This code is based on http://www.cg.tuwien.ac.at/courses/Visualisierung/2010-2011/Beispiel1/Moellinger_Ludwig/.
  * @namespace GLSL
  * @class DrawresultVS
  */
in vec4 position;   /**< position coordinates */
in vec2 texCoord;   /**< texture coordinates */
uniform vec4 color; /**< color coordinates */
out vec2 TexCoord;  /**< Texture coordinates that are mapped to the vertex. */
//out vec4 vColor;    /**< color coordinates */
/**
* @fn void main (void)
* @brief Main function
*/
void main (void)
{  
    gl_Position = position;
    TexCoord = texCoord;
//    vColor=color;
}
