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
uniform PointLight pointLight;
uniform DirectionalLight directionalLight;
uniform SpotLight spotLight;

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

vec3 pointResult(PointLight pointLight, vec3 albedo, vec3 specular)
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

vec3 directionalResult(DirectionalLight directionalLight, vec3 albedo, vec3 specular)
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

vec3 spotResult(SpotLight spotLight, vec3 albedo, vec3 specular)
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
    vec3 albedo = int(!useTextures)*(basicMaterial.albedo) + int(useTextures)*texture(textureMaterial.albedo, TexCoord).rgb;
    vec3 specular = int(!useTextures)*(basicMaterial.albedo) + int(useTextures)*(texture(textureMaterial.specular, TexCoord).rgb);

    // vec3 result = pointResult(pointLight, albedo, specular);
    // vec3 result = directionalResult(directionalLight, albedo, specular);
    vec3 result = spotResult(spotLight, albedo, specular);

    FragColor =  vec4(result, 1.0);

    // for noise append: + vec4(vertexColor,1.)*rand(gl_FragCoord.xy);
}