#version 330

uniform sampler2D MipMap_tex; // the MipMap texture get from the cpp
in vec2 oUv0;
out vec4 fragColour;

void main()
{

    // MipMap version
//    if (texelFetch(RT,ivec2(oUv0),1).r >  0.4)
//        fragColour = vec4 (0.1,0.5,0.1,1.0);

//    else
//        fragColour = vec4(0.5,0.1,0.1,1.0);
    fragColour = textureLod(MipMap_tex,oUv0,0);

    // normal Texture version -> use when getting mip0 from the last compositor
//    if (texture(RT,oUv0).r >  0.8 && texture(RT,oUv0).g > 0.8 )
//        fragColour = vec4 (0.5,0.1,0.1,1.0);

//    else
//        fragColour = texture(RT,oUv0);
}
