#version 440 
layout (location = 0) out vec4 gPositionDepth;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

//in vec2 TexCoords;
//in vec3 FragPos;
//in vec3 Normal;

in vec3 tePosition;
in vec3 teNormal;
in vec2 teTexCoord;

const float NEAR = 0.1f; // Projection matrix's near plane distance
const float FAR = 50.0f; // Projection matrix's far plane distance
float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * NEAR * FAR) / (FAR + NEAR - z * (FAR - NEAR));	
}

void main()
{    
    // Store the fragment position vector in the first gbuffer texture
    gPositionDepth.xyz = tePosition;;
    // And store linear depth into gPositionDepth's alpha component
    gPositionDepth.a = LinearizeDepth(gl_FragCoord.z) ; // Divide by FAR if you need to store depth in range 0.0 - 1.0 (if not using floating point colorbuffer)
    // Also store the per-fragment normals into the gbuffer
    gNormal = normalize(teNormal);
    // And the diffuse per-fragment color
    gAlbedoSpec.rgb = vec3(0.95); // Currently all objects have constant albedo color
}