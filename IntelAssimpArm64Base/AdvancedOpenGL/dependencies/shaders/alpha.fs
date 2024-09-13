# version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture01;

void main()
{
    FragColor = texture(texture01, TexCoords);
}