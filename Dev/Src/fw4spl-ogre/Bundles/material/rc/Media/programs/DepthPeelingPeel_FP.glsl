#version 330
uniform sampler2D u_fragData0;
uniform float u_vpWidth;
uniform float u_vpHeight;
uniform vec4 u_diffuse;

vec4 getMaterialColor();

layout(location = 0) out vec4 frontColor;
layout(location = 1) out vec4 frontDepth;

vec4 packFloatToVec4(float value);
float unpackFloatFromVec4(vec4 value);

/** Peel shader :
  * Only write fragment ahead from next peel
  **/
void main()
{
    vec2 texCoord = gl_FragCoord.xy / vec2( u_vpWidth, u_vpHeight );

    // Front depth buffer
    float frontDepthBuffer = unpackFloatFromVec4(texture(u_fragData0, texCoord));

    float currentDepth = gl_FragCoord.z;

    if(frontDepthBuffer == 0.)
    {
        frontColor = vec4(0,0,0,0);
    }
    else
    {
        if(frontDepthBuffer >= 1.)
        {
            frontDepthBuffer = 0.;
        }

        // Disallow repetition of last passes
        // Check if current fragment is the nearest from last front peel by depth comparaison
        // if (yes), draws fragment as current nearest peel
        if(currentDepth <= frontDepthBuffer || u_diffuse.a == 0.)
        {
            discard;
        }

        vec4 colorOut = getMaterialColor();

        colorOut.rgb *= colorOut.a;

        // Depth sent to the next peel
        frontDepth = packFloatToVec4(currentDepth);
        frontColor = colorOut;
    }
}
