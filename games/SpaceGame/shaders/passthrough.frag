#version 400 core

in vec3 vColor;      // Color from vertex shader
out vec4 FragColor;  // Output color

void main()
{
    FragColor = vec4(vColor, 1.0);
}