# version 330 core

out vec4 FragColor;

uniform vec3 outlineColor;

void main()
{
    FragColor = vec4(outlineColor, 1.0); // 0.04, 0.28, 0.26
}