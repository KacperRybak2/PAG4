#version 330 core
layout (location = 0) in vec3 vsPos;
layout (location = 1) in vec3 vsNormal;
layout (location = 2) in vec2 vsTexture;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


void main()
{
	FragPos = vec3(model * vec4(vsPos, 1.0));
	Normal = mat3(transpose(inverse(model))) * vsNormal;
    TexCoords = vsTexture;
    gl_Position = projection * view * vec4(FragPos, 1.0f); 
}