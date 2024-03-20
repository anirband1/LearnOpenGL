# version 330 core

out vec4 FragColor;
in vec3 vertexColor;
in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D imageTexture1;
uniform sampler2D imageTexture2;

uniform float opacity;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 objColor;
uniform bool useTextures;

uniform vec3 viewPos;

// Parameters for the LCG
const int a=1664525;
const int c=1013904223;
const int m=4294967295;// 2^32 - 1

const float AMBIENT_STRENGTH = 0.1F;
const float DIFFUSE_STRENGTH = 0.45F;
const float SPECULAR_STRENGTH = 0.45F;

const float SPECULAR_POWER = 32.0;
const float repeat=1.;

float rand(vec2 seed){
    return fract(sin(dot(seed.xy,vec2(.02898,.0233)))*43758.5453);
}

void main()
{
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float invDist = inversesqrt(pow(length(lightPos - FragPos), 2));

    // + AMBIENT
    vec3 ambientColor = objColor * AMBIENT_STRENGTH;


    // + DIFFUSE
    float diff = max(dot(lightDir, norm), 0.0);
    vec3 diffuseColor = diff * objColor * DIFFUSE_STRENGTH  * invDist;

    // + SPECULAR
    vec3 viewDir = normalize(-FragPos + viewPos);
    vec3 reflectDir = reflect(-lightDir, norm);

    float spec = max(dot(viewDir, reflectDir), 0.0);
    spec = pow(spec, SPECULAR_POWER);
    vec3 specularColor = spec * objColor * SPECULAR_STRENGTH * invDist;

    // + TOTAL
    vec3 result = (ambientColor + diffuseColor + specularColor) * lightColor;


    if (useTextures)
        FragColor=mix(texture(imageTexture1,TexCoord*repeat),texture(imageTexture2,TexCoord*repeat),opacity)*0.5 + vec4(vertexColor,1.)*rand(gl_FragCoord.xy);
    else
        // FragColor=vec4(normal * lightColor, 1.0);
        FragColor=vec4(result, 1.0);
}