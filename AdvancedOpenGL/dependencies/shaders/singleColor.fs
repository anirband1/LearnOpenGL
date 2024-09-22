# version 330 core

out vec4 FragColor;

in vec3 Normal;
in vec2 TexCoord;

uniform vec3 outlineColor;

// uniform sampler2D normal;

// vec3 localNormalTex;

void main()
{
    FragColor = vec4(outlineColor, 1.0); // 0.04, 0.28, 0.26
}