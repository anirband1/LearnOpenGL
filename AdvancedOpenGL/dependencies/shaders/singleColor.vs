#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform sampler2D normal;
uniform mat3 normalMat;

vec3 localNormalTex;
vec3 localPos;

out vec2 TexCoord;
out vec3 Normal;

void main()
{
	TexCoord = aTexCoord;
	localNormalTex = texture(normal, TexCoord).rgb - vec3(0.5, 0.5, 1);
	Normal = normalMat * aNormal;
	
	localPos = aPos;
	localPos += 0.1 * Normal + 0.05 * localNormalTex;

	gl_Position = projection * view * model * vec4(localPos, 1.0);
}