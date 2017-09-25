#version 330
/**
  * @file drawresult.frag
  * @brief The fragment´s color is retrieved from the texture object with an interpolated texture coordinate.
  * @author José Angel Ivan Rubianes Silva
  * @date February 2015
  * @note This code is based on http://www.cg.tuwien.ac.at/courses/Visualisierung/2010-2011/Beispiel1/Moellinger_Ludwig/.
  * @namespace GLSL
  * @class DrawresultFS
  */

uniform sampler2D raycast_fbo; /**< input image as a texture object */
in vec2 TexCoord; /**< interpolated texture coordinates computed from the values of the vertex. */
//in vec4 vColor;   /**< dummy variable */
out vec4 Color;   /**< fragment color */

/**
*  @fn void main(void)
*  @brief attributes the RGBA color in the position gl_TexCoord in DrawresultFS::raycast_fbo to the current pixel.
*/
void main(void)
{
    Color = texture(raycast_fbo, TexCoord);
}

/* test*/
//void main(void)
//{
////    float R = 100;
////    float h = 0;
////    float hr = R * sqrt(1.0 - ((R - h) / R) * ((R - h) / R));

//    vec2 mouse = vec2(0.5,0.5);
//    vec2 xy = TexCoord.xy - mouse.xy;
//    float r = sqrt(xy.x * xy.x + xy.y * xy.y);
//    float factorZoomGlass = 3.0;
//    float hr = factorZoomGlass*r;
////    vec2 new_xy = r < hr ? xy * (R - h) / sqrt(R * R - r * r) : xy;
//    vec2 new_xy = r < hr ? xy * 1/factorZoomGlass : xy;

//    if(r<=0.1){
//        if(r<=0.1 && r>=0.095){ Color = vec4(1.0,0.0,0.0,1.0); }
//        else{ Color = texture(raycast_fbo, new_xy + mouse.xy); }
//    }
//    else
//    {
//        Color = texture(raycast_fbo, TexCoord);
//        //Color = vec4(TexCoord.x,TexCoord.y,0.0,1.0  );
//    }
//}
