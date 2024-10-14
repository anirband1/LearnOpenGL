# version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture01;

void main()
{
    vec4 texColor = texture(texture01, TexCoords);
    if(texColor.a < 0.1)
        discard;
    FragColor = texColor;
}