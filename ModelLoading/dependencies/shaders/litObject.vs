# version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
// layout (location = 3) in vec3 aNormal;

// out vec3 vertexColor;
out vec2 TexCoord;
out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMat;


void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    // vertexColor = aColor;
    TexCoord = aTexCoord;
    Normal = normalMat * aNormal;
    FragPos = vec3(model * vec4(aPos, 1.0));
}