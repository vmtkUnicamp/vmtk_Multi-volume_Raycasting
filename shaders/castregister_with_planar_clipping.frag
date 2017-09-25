#version 330
/**
  * @file castregister.frag
  * @brief The volume data are ray-casted and its colors blended to get the pixel´s color.
  * @author Wu Shin-Ting and Raphael Voltoline
  * @date April 2015
  * @namespace GLSL
  * @class CastRegisterFS
  */
in vec2 TexCoord;   /**< interpolated texture coordinates (R,G,B,A) from raytrace.vs */
uniform  sampler2D backface_fbo; /**< texture unit storing backface plane in the viewing direction */
uniform  sampler2D frontface_fbo; /**< texture unit storing frontplane plane in the viewing direction */
//uniform  sampler3D ref_volumetexture; /**< texture unit storing reference volume data */
//uniform  sampler3D float_volumetexture; /**< texture unit storing floating volume data */
uniform  sampler1D ref_transferfunction;  /**< texture unit storing transfer function of reference volume data*/
uniform  sampler1D float_transferfunction;  /**< texture unit storing transfer function of floating volume data */

uniform  sampler3D vTexture0;
uniform  sampler3D vTexture1;
uniform  sampler3D vTexture2;
uniform  sampler3D vTexture3;
uniform  sampler3D vTexture4;
uniform  sampler3D vTexture5;

uniform  sampler1D vTF0;
uniform  sampler1D vTF1;
uniform  sampler1D vTF2;
uniform  sampler1D vTF3;
uniform  sampler1D vTF4;
uniform  sampler1D vTF5;

uniform  int nVolumes;
uniform  int height;           /**< height of the volume data */
uniform  int width;            /**< width of the volume data */
uniform  int depth;            /**< depth of the volume data */
uniform  int steps_mode;       /**< sampling resolution: high (1) or low (otherwise)  */
uniform  float noise_threshold; /**< threshold for removing noise */
//uniform mat4 inv_registration_matrix[6];    /**< inverse registration matrix */
uniform mat4 inv_registration_matrix[6];    /**< inverse registration matrix */
uniform  float blending_factor;      /**< linear interpolation weight for reference */
uniform vec4 phyDimensions[6];      /**< physical dimension for compensating the texture volume distortion */
uniform vec4 clipping_plane;         /**< clipping plane for MPR (multiplanar reformatting)*/
uniform bool enableMPR;              /**< Enable Multiplanar reformatting */

out vec4 fColor;
/**
* @fn void main(void)
* @brief Main function for volume ray-casting.
* @details Color values of several samples along the ray between the entry point stored in
* frontface_fbo (texture unit of RaytracedRenderer::m_pFrontSidesFBO) and the exit point
* stored in backface_fbo (texture unit of RaytracedRenderer::m_pBackSidesFBO) are computed.
* There are four modes to compose them: (0) only the first valid hit is considered; (2) only
* the sample with the highest intensity value is considered; (2) the colors are summed from
* front to back; (3) the colors are summed from back to front. In addition, the colors may
* be get only from the transfer function (shading disabled) or influenced by an illumination
* model (shading enabled).
*/
vec4 invDivV4(vec4 v);
float getDensity(sampler3D vt,vec4 corresp);
vec4 getTFColor(sampler1D tf, float density);
vec4 getCorrespondence(mat4 invRegMat, vec4 fphydim, vec3 currentpos);
vec4 registerVolumes(sampler3D f_vt, sampler1D f_tf, vec4 init_tf_color, mat4 invRegMat, vec4 fphydim,vec3 currentpos, vec4 vfusion);



void main(void)
{



    vec3 start_position = texture(frontface_fbo, TexCoord).xyz;
    vec3 end_position = texture(backface_fbo, TexCoord).xyz;

    if (start_position == end_position)  // if two points are coincident
        fColor = vec4(0.5, 0.5, 0.5,1.0);        // color of pixel is black
    else                                 // compute the samples
    {
        vec4 color = vec4(0.0);
        vec4 fusion0 = vec4(0.0);
        vec4 fusion = vec4(0.0);



        float ref_density = 0.0;
        //        float ref_eq_density = 0.0;
        float float_density = 0.0;
        //        float float_eq_density = 0.0;
        float alpha = 0.0;
        float accum_alpha = 0.0;
        bool first=false;
        //        vec4 inv = vec4(1. / float_phyDimensions.x, 1. / float_phyDimensions.y,
        //                        1. / float_phyDimensions.z, 1.0);


        //        float length_of_ray_x = (end_position.x - start_position.x) * float(width);
        //        float length_of_ray_y = (end_position.y - start_position.y) * float(height);
        //        float length_of_ray_z = (end_position.z - start_position.z) * float(depth);
        //        float length_of_ray = sqrt(length_of_ray_x * length_of_ray_x + length_of_ray_y * length_of_ray_y + length_of_ray_z * length_of_ray_z);
        float steps;
        //        float factor_for_tf_alpha;

        //        if (steps_mode == 0)            // sample resolution is 2xlength
        //        {
        //            steps = length_of_ray * 2.0;
        //            factor_for_tf_alpha = 2.0;
        //        }
        //        else
        //        {                               // sample resolution is fixed on a pre-defined value
        steps = float(steps_mode);
        //            factor_for_tf_alpha = steps / length_of_ray;
        //        }

        vec3 step = (end_position - start_position) / steps; // sample step

        for (float i=0.0; i < steps; i+=1.0)
        {
            vec3 current_position = start_position + i*step;

            if(enableMPR){ // enable MPR
                // discard all fragments on the side of the normal vector
                if (clipping_plane.x*current_position.x +
                        clipping_plane.y*current_position.y +
                        clipping_plane.z*current_position.z +
                        clipping_plane.w > 0.0) continue;
            }

            ref_density = getDensity( vTexture0 , vec4(current_position,1.0));

            if (!first && ref_density >= noise_threshold) {
                first = true;
            }


            if (first) {
                vec4 ref_tf_color = getTFColor( vTF0, ref_density ); // reference voxel color

                switch(nVolumes)
                {
                case 1:
                    fusion = ref_tf_color;
                    break;
                case 2:
                    fusion = registerVolumes(vTexture1, vTF1, ref_tf_color, inv_registration_matrix[0], phyDimensions[1],current_position, fusion0);
                    break;
                case 3:
                    fusion += 0.5*registerVolumes(vTexture1, vTF1, ref_tf_color, inv_registration_matrix[0], phyDimensions[1],current_position, fusion0);
                    fusion += 0.5*registerVolumes(vTexture2, vTF2, ref_tf_color, inv_registration_matrix[1], phyDimensions[2],current_position, fusion0);
                    break;
                case 4:
                    fusion += 0.33*registerVolumes(vTexture1, vTF1, ref_tf_color, inv_registration_matrix[0], phyDimensions[1],current_position, fusion0);
                    fusion += 0.33*registerVolumes(vTexture2, vTF2, ref_tf_color, inv_registration_matrix[1], phyDimensions[2], current_position, fusion0);
                    fusion += 0.33*registerVolumes(vTexture3, vTF3, ref_tf_color, inv_registration_matrix[2], phyDimensions[3],current_position, fusion0);
                    break;
                case 5:
                    fusion += 0.25*registerVolumes(vTexture1, vTF1, ref_tf_color, inv_registration_matrix[0], phyDimensions[1],current_position, fusion0);
                    fusion += 0.25*registerVolumes(vTexture2, vTF2, ref_tf_color, inv_registration_matrix[1], phyDimensions[2], current_position, fusion0);
                    fusion += 0.25*registerVolumes(vTexture3, vTF3, ref_tf_color, inv_registration_matrix[2], phyDimensions[3],current_position, fusion0);
                    fusion += 0.25*registerVolumes(vTexture4, vTF4, ref_tf_color, inv_registration_matrix[3], phyDimensions[4],current_position, fusion0);
                    break;
                case 6:
                    fusion += 0.2*registerVolumes(vTexture1, vTF1, ref_tf_color, inv_registration_matrix[0], phyDimensions[1],current_position, fusion0);
                    fusion += 0.2*registerVolumes(vTexture2, vTF2, ref_tf_color, inv_registration_matrix[1], phyDimensions[2], current_position, fusion0);
                    fusion += 0.2*registerVolumes(vTexture3, vTF3, ref_tf_color, inv_registration_matrix[2], phyDimensions[3],current_position, fusion0);
                    fusion += 0.2*registerVolumes(vTexture4, vTF4, ref_tf_color, inv_registration_matrix[3], phyDimensions[4],current_position, fusion0);
                    fusion += 0.2*registerVolumes(vTexture5, vTF5, ref_tf_color, inv_registration_matrix[4], phyDimensions[5],current_position, fusion0);
                    break;
                }


                if (enableMPR){
                    if(clipping_plane.x != 0.0 ||
                            clipping_plane.y != 0.0 ||
                            clipping_plane.z != 0.0 ||
                            clipping_plane.w != 0.0) {
                        fColor = vec4(fusion.rgb, 1.0);
                        return;
                    }
                }
                else if (alpha < 0.99) {
                    color.rgb = (1-alpha)*color.rgb + fusion.rgb;
                    color.a = mix(fusion.a, 1.0, color.a);
                    accum_alpha=alpha;
                    alpha = color.a;

                } else {
//                    color.rgb = (1-alpha)*color.rgb + fusion.rgb;
                    fColor = vec4(color.rgb, 1.0);
                    return;
                }
            }
        }
        if (accum_alpha == 0.0)
        {
            fColor = vec4(0.5,0.5,0.5, 1.0);
        }
        else
        {
            fColor = vec4(color.rgb, 1.0);
        }
    }
} // end main


vec4 invDivV4(vec4 v){
    return vec4(1. / v.x, 1. / v.y, 1. / v.z, 1.0);
}

float getDensity(sampler3D vt,vec4 corresp){
    return  texture(vt, corresp.xyz).r;
}

vec4 getTFColor(sampler1D tf, float density){
    vec4 color = texture(tf, density);
    return  vec4(color.r, color.r, color.r, color.a);
}

vec4 getCorrespondence(mat4 invRegMat, vec4 fphydim, vec3 currentpos){
    vec4 pt = vec4(phyDimensions[0].xyz * (currentpos), 1.0); //fixed first phyDimensions[0]
    return invDivV4(fphydim) * (invRegMat * pt);
}

vec4 registerVolumes(sampler3D f_vt, sampler1D f_tf, vec4 init_tf_color, mat4 invRegMat, vec4 fphydim,vec3 currentpos, vec4 vfusion){
    vec4 corresp = getCorrespondence(invRegMat, fphydim, currentpos); // correspondece float volume
    vec4 f_tf_color = getTFColor(f_tf, getDensity(f_vt,corresp));  //float voxel color

    // blend two voxel colors
    vfusion.rgb = mix(f_tf_color.rgb, init_tf_color.rgb, blending_factor); // (1-blending_factor)*float_tf_color+blending_factor*ref_tf_color

    // alpha of the fuse color
    vfusion.a = min(init_tf_color.a,f_tf_color.a);
    return vfusion;
}



