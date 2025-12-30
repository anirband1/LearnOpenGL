# version 330 core

#ifndef MAX_MATERIALS
    #define MAX_MATERIALS 1
#endif

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texArray[MAX_MATERIALS];
uniform int activeTexture;

void main()
{
    vec4 texColor = texture(texArray[activeTexture], TexCoords);
    if(texColor.a < 0.05)
        discard;
    FragColor = texColor;
}