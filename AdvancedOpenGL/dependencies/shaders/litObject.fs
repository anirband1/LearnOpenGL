# version 330 core

#ifndef NEAR_CLIP
    #define NEAR_CLIP 0.01
#endif
#ifndef FAR_CLIP
    #define FAR_CLIP 100.0
#endif

#ifndef NR_POINT
    #define NR_POINT 0
#endif
#ifndef NR_DIR
    #define NR_DIR 0
#endif
#ifndef NR_SPOT
    #define NR_SPOT 0
#endif

#ifndef MAX_MATERIALS
    #define MAX_MATERIALS 1
#endif

#define E 2.718281828459045

// -------------------------------------------------------------------------------------------------------------------------

struct BasicMaterial {
    vec3 albedo;
};

struct TextureMaterial {
    sampler2D albedo;
    sampler2D specular;
    sampler2D normal;
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


out vec4 FragColor;
// in vec3 vertexColor;
in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;

uniform bool useTextures;
int activeMaterial;
// uniform int activeMaterial;

uniform BasicMaterial basicMaterial;
uniform TextureMaterial textureMaterials[MAX_MATERIALS];

uniform PointLight pointLights[max(NR_POINT, 1)];
uniform DirectionalLight directionalLights[max(NR_DIR, 1)];
uniform SpotLight spotLights[max(NR_SPOT, 1)];

uniform vec3 viewPos;

const float AMBIENT_STRENGTH = 0.1F; // 0.1F
const float DIFFUSE_STRENGTH = 0.45F; // 0.45F
const float SPECULAR_STRENGTH = 0.45F; // 0.45F

const float SPECULAR_POWER = 64.0F;

vec3 albedo;
vec3 specular;
vec3 localNormal;

vec3 normal;

// -------------------------------------------------------------------------------------------------------------------------


float mapinf2one(float val)
{
    return 1 - 2/(pow(E, 2.86*val) + pow(E, -2.86*val));
}

float luma (vec3 color)
{
    return 0.299*color.r+0.587*color.g+0.114*color.b;
}

vec3 proprtionalColor(vec3 color)
{
    if(luma(color) > 1)
    {
        float maxChannel = max(color.r, max(color.g, color.b));
        return color/maxChannel;
    }
    return color ;
}

// -------------------------------------------------------------------------------------------------------------------------

// Returns value in range [near, far] linearly. (depth is [near, far] non-linearly)
float LinearizeDepth(float depth) 
{
    // * you dont have to convert to NDC, you can use your own inverse function to do the same
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * NEAR_CLIP * FAR_CLIP) / (FAR_CLIP + NEAR_CLIP - z * (FAR_CLIP - NEAR_CLIP));
}


vec3 PointResult(PointLight pointLight)
{
    vec3 norm = normalize(normal);
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

vec3 DirectionalResult(DirectionalLight directionalLight)
{
    vec3 norm = normalize(normal);
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

vec3 SpotResult(SpotLight spotLight)
{
    vec3 norm = normalize(normal);
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
    // ! activeMaterial is not initialized
    activeMaterial = 0;

    // doing this to avoid if statements
    albedo = int(!useTextures)*(basicMaterial.albedo) + int(useTextures)*texture(textureMaterials[activeMaterial].albedo, TexCoord).rgb;
    specular = int(!useTextures)*(basicMaterial.albedo) + int(useTextures)*texture(textureMaterials[activeMaterial].specular, TexCoord).rgb;
    localNormal = int(!useTextures)*(vec3(0.5, 0.5, 1)) + int(useTextures)*texture(textureMaterials[activeMaterial].normal, TexCoord).rgb;

    normal = Normal + (localNormal - vec3(0.5, 0.5, 1));

    vec3 result = vec3(0.0);

    for(int i = 0; i < NR_POINT; i++)
        result += PointResult(pointLights[i]);

    for(int i = 0; i < NR_DIR; i++)
        result += DirectionalResult(directionalLights[i]);

    for(int i = 0; i < NR_SPOT; i++)
        result += SpotResult(spotLights[i]);


    // remove clipping 
    // result = vec3 (mapinf2one(result.r), mapinf2one(result.g), mapinf2one(result.b));
    // result = proprtionalColor(result);
    // result *= 0.3;

    FragColor =  vec4(result, 1.0);

    // FragColor =  vec4(vec3(LinearizeDepth(gl_FragCoord.z)), 1.0);
}