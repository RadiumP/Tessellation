#version 440

const float PI = 3.14159265;


uniform sampler2D gPositionDepth;
uniform sampler2D gNormal;

out float FragColor