#version 440 
layout (location = 0) out vec4 gPositionDepth;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;


in vec3 tePosition;
in vec3 teNormal;
in vec2 teTexCoord;

out vec4 out_Color;





void main()
{    
    out_Color = vec4(0.0, 1.0, 0.0, 1.0);
}