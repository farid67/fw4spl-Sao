#version 330

#ifdef HYBRID
uniform sampler2D u_frontDepthBuffer;
#endif

uniform sampler2D u_occlusionDepthBuffer;
uniform float u_vpWidth;
uniform float u_vpHeight;

vec4 getMaterialColor();
float unpackFloatFromVec4(vec4 value);

uniform float u_near;
uniform float u_far;
out vec4 FragColor;

float linearizeDepth(in float depth) {
    return (2.0*u_near) / (u_far+u_near - depth *(u_far-u_near));
}

void main()
{
    vec2 texCoord = gl_FragCoord.xy / vec2( u_vpWidth, u_vpHeight );
#ifdef HYBRID
    float frontDepthBuffer = unpackFloatFromVec4(texture(u_frontDepthBuffer, texCoord));
#endif
    vec4 occlusionDepthBuffer = texture(u_occlusionDepthBuffer, texCoord);
    float currentDepth = gl_FragCoord.z;

#ifdef HYBRID
    if(frontDepthBuffer == 0. || currentDepth <= frontDepthBuffer || currentDepth > occlusionDepthBuffer.r)
#else
    if(currentDepth > occlusionDepthBuffer.r)
#endif
    {
        discard;
    }

    vec4 colorOut = getMaterialColor();

    float linearDepth = linearizeDepth(gl_FragCoord.z);
    linearDepth = linearDepth*2.5;

    float weight = clamp(0.03 / (1e-5 + pow(linearDepth, 4.0)), 1e-2, 3e3);

    FragColor = vec4(colorOut.rgb * colorOut.a, colorOut.a) * weight;
}
