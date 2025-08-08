#version 400 core

in vec2 vTexCoord;       // Texture coordinates from vertex shader
uniform sampler2D u_texture; // The texture sampler

out vec4 FragColor;      // Output color

void main()
{
    FragColor = texture(u_texture, vTexCoord);
}
