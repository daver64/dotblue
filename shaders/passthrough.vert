#version 400 core

layout(location = 0) in vec3 aPos;    // Vertex position (screen coordinates)
layout(location = 1) in vec3 aColor;  // Vertex color

uniform vec2 u_resolution; // Screen resolution (width, height)

out vec3 vColor; // Pass color to fragment shader

void main()
{
    // Convert screen coordinates to normalized device coordinates
    vec2 ndc = ((aPos.xy / u_resolution) * 2.0) - 1.0;
    // Flip Y coordinate (OpenGL has origin at bottom-left, but we want top-left)
    ndc.y = -ndc.y;
    
    gl_Position = vec4(ndc, aPos.z, 1.0);
    vColor = aColor;
}