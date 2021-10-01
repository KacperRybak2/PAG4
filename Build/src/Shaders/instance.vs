#version 330 core
layout (location = 0) in vec3 vsPos;
layout (location = 1) in vec3 vsNormal;
layout (location = 2) in vec2 vsTexture;
layout (location = 5) in mat4 aInstanceMatrix;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
	FragPos = vec3(aInstanceMatrix * vec4(vsPos, 1.0));
	Normal = mat3(transpose(inverse(aInstanceMatrix))) * vsNormal;
    TexCoords = vsTexture;
    gl_Position = projection * view * aInstanceMatrix * vec4(vsPos, 1.0f); 
}