# version 330 core

out vec4 FragColor;
in vec3 vertexColor;
in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;

// uniform float opacity;  
// uniform vec3 lightColor;
// uniform vec3 lightPos; 
// uniform vec3 objColor; 
uniform bool useTextures;

struct BasicMaterial {
    vec3 albedo;
};

struct TextureMaterial {
    sampler2D albedo;
    sampler2D specular;
};

struct Light{
    vec3 lightColor;
    vec3 lightPos;
};

uniform BasicMaterial basicMaterial;
uniform TextureMaterial textureMaterial;
uniform Light light;

uniform vec3 viewPos;

// Parameters for the LCG
const int a=1664525;
const int c=1013904223;
const int m=4294967295;// 2^32 - 1

const float AMBIENT_STRENGTH = 0.1F;
const float DIFFUSE_STRENGTH = 0.45F;
const float SPECULAR_STRENGTH = 0.45F;

const float SPECULAR_POWER = 64.0F;

const float repeat=1.;

float rand(vec2 seed){
    return fract(sin(dot(seed.xy,vec2(.02898,.0233)))*43758.5453);
}

void main()
{
    // doing this to avoid if statements
    vec3 albedo = int(!useTextures)*(basicMaterial.albedo) + int(useTextures)*texture(textureMaterial.albedo, TexCoord).rgb;
    // vec3 specular = int(!useTextures)*(basicMaterial.albedo) + int(useTextures)*(texture(textureMaterial.specular, TexCoord).rgb);
    vec3 specular = int(!useTextures)*(basicMaterial.albedo) + int(useTextures)*(texture(textureMaterial.specular, TexCoord).rgb);

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.lightPos - FragPos);
    float invDist = inversesqrt(pow(length(1.0 + light.lightPos - FragPos), 2));

    // + AMBIENT
    vec3 ambientColor = albedo * AMBIENT_STRENGTH;

    // + DIFFUSE
    float diff = max(dot(lightDir, norm), 0.0);
    vec3 diffuseColor = diff * albedo * DIFFUSE_STRENGTH  * invDist;

    // + SPECULAR
    vec3 viewDir = normalize(-FragPos + viewPos);
    vec3 reflectDir = reflect(-lightDir, norm);

    float spec = max(dot(viewDir, reflectDir), 0.0);
    spec = pow(spec, SPECULAR_POWER);
    vec3 specularColor = spec * specular * SPECULAR_STRENGTH * invDist;

    // + TOTAL
    vec3 result = (ambientColor + diffuseColor + specularColor) * light.lightColor;

    FragColor=  vec4(result, 1.0);

    // for noise append: + vec4(vertexColor,1.)*rand(gl_FragCoord.xy);

}