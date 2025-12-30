# version 330 core

#ifndef NR_POINT
    #define NR_POINT 0
#endif
#ifndef NR_DIR
    #define NR_DIR 0
#endif
#ifndef NR_SPOT
    #define NR_SPOT 0
#endif


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

struct PointLight{
    vec3 lightColor;
    float lightStrength;
    vec3 lightPos;
};

struct DirectionalLight{
    vec3 lightColor;
    float lightStrength;
    vec3 lightDir;
};

struct SpotLight{
    vec3 lightColor;
    float lightStrength;
    vec3 lightPos;
    vec3 lightDir;
    float innerCutoff;
    float outerCutoff;
};

uniform BasicMaterial basicMaterial;
uniform TextureMaterial textureMaterial;

// Workaround for arrays to have min size 1:
// make the array size 1 but while iterating, take end to be 0 ---> (i=0; i<0; i++)
uniform PointLight pointLights[max(NR_POINT, 1)];
uniform DirectionalLight directionalLights[max(NR_DIR, 1)];
uniform SpotLight spotLights[max(NR_SPOT, 1)];

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

vec3 albedo;
vec3 specular;

float rand(vec2 seed){
    return fract(sin(dot(seed.xy,vec2(.02898,.0233)))*43758.5453);
}

vec3 pointResult(PointLight pointLight)
{
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(pointLight.lightPos - FragPos);
    float invDist = inversesqrt(pow(length(1.0 + pointLight.lightPos - FragPos), 2));

    // + AMBIENT
    vec3 ambientColor = albedo * AMBIENT_STRENGTH;

    // + DIFFUSE
    float diff = max(dot(lightDir, norm), 0.0);
    vec3 diffuseColor = diff * albedo * DIFFUSE_STRENGTH * pointLight.lightStrength;

    // + SPECULAR
    vec3 viewDir = normalize(FragPos - viewPos);
    vec3 reflectDir = reflect(lightDir, norm);

    float spec = max(dot(viewDir, reflectDir), 0.0);
    spec = pow(spec, SPECULAR_POWER);
    vec3 specularColor = spec * specular * SPECULAR_STRENGTH * pointLight.lightStrength;

    // + TOTAL
    vec3 result = (ambientColor + diffuseColor + specularColor) * pointLight.lightColor * invDist;

    return result;
}

vec3 directionalResult(DirectionalLight directionalLight)
{
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(-directionalLight.lightDir); // vector should point towards light source for calculations

    // + AMBIENT
    vec3 ambientColor = albedo * AMBIENT_STRENGTH;

    // + DIFFUSE
    float diff = max(dot(lightDir, norm), 0.0);
    vec3 diffuseColor = diff * albedo * DIFFUSE_STRENGTH;

    // + SPECULAR
    vec3 viewDir = normalize(FragPos - viewPos);
    vec3 reflectDir = reflect(lightDir, norm);

    float spec = max(dot(viewDir, reflectDir), 0.0);
    spec = pow(spec, SPECULAR_POWER);
    vec3 specularColor = spec * specular * SPECULAR_STRENGTH;

    // + TOTAL
    vec3 result = (ambientColor + diffuseColor + specularColor) * directionalLight.lightColor * directionalLight.lightStrength;

    return result;
}

vec3 spotResult(SpotLight spotLight)
{
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(spotLight.lightPos - FragPos); // vector should point towards light source for calculations
    float invDist = inversesqrt(pow(length(1.0 + spotLight.lightPos - FragPos), 2));

    float theta = dot(lightDir, normalize(-spotLight.lightDir));
    float intensity = clamp((theta - spotLight.outerCutoff)/(spotLight.innerCutoff - spotLight.outerCutoff) , 0.0, 1.0);

    // + AMBIENT
    vec3 ambientColor = albedo * AMBIENT_STRENGTH;

    // + DIFFUSE
    float diff = max(dot(lightDir, norm), 0.0);
    vec3 diffuseColor = diff * albedo * DIFFUSE_STRENGTH * intensity * spotLight.lightStrength;

    // + SPECULAR
    vec3 viewDir = normalize(FragPos - viewPos);
    vec3 reflectDir = reflect(lightDir, norm);

    float spec = max(dot(viewDir, reflectDir), 0.0);
    spec = pow(spec, SPECULAR_POWER);
    vec3 specularColor = spec * specular * SPECULAR_STRENGTH * intensity * spotLight.lightStrength;

    // + TOTAL
    vec3 result = (ambientColor + diffuseColor + specularColor) * spotLight.lightColor * invDist;

    return result;
}

void main()
{
    // doing this to avoid if statements
    albedo = int(!useTextures)*(basicMaterial.albedo) + int(useTextures)*texture(textureMaterial.albedo, TexCoord).rgb;
    specular = int(!useTextures)*(basicMaterial.albedo) + int(useTextures)*(texture(textureMaterial.specular, TexCoord).rgb);

    vec3 result = vec3(0.0);

    for(int i = 0; i < NR_POINT; i++)
        result += pointResult(pointLights[i]);

    for(int i = 0; i < NR_DIR; i++)
        result += directionalResult(directionalLights[i]);

    for(int i = 0; i < NR_SPOT; i++)
        result += spotResult(spotLights[i]);

    FragColor =  vec4(result, 1.0);

    // for noise append: + vec4(vertexColor,1.)*rand(gl_FragCoord.xy);
}