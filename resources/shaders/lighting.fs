#version 330

// Fragment shader for dynamic lighting with multiple lights

// Input vertex attributes (from vertex shader)
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// Lighting uniforms
uniform vec3 viewPos;
uniform vec4 ambient;

// Directional light (sun)
uniform vec3 lightDir;
uniform vec4 lightColor;

// Point lights (max 4)
uniform int pointLightCount;
uniform vec3 pointLightPositions[4];
uniform vec4 pointLightColors[4];
uniform float pointLightRanges[4];

// Output fragment color
out vec4 finalColor;

// Calculate point light contribution
vec3 CalculatePointLight(vec3 lightPos, vec4 lightCol, float lightRange, vec3 normal, vec3 viewDir)
{
    vec3 lightVec = lightPos - fragPosition;
    float distance = length(lightVec);

    // Attenuation
    float attenuation = clamp(1.0 - (distance / lightRange), 0.0, 1.0);
    attenuation *= attenuation; // Quadratic falloff

    // Diffuse
    vec3 lightDir = normalize(lightVec);
    float diff = max(dot(normal, lightDir), 0.0);

    // Specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);

    vec3 diffuse = diff * lightCol.rgb * attenuation;
    vec3 specular = spec * lightCol.rgb * attenuation * 0.5;

    return diffuse + specular;
}

void main()
{
    // Sample texture
    vec4 texelColor = texture(texture0, fragTexCoord);

    // Normalize inputs
    vec3 normal = normalize(fragNormal);
    vec3 viewDir = normalize(viewPos - fragPosition);

    // Ambient lighting
    vec3 ambientLight = ambient.rgb * ambient.a;

    // Directional light (sun)
    float diff = max(dot(normal, -lightDir), 0.0);
    vec3 directionalLight = diff * lightColor.rgb * lightColor.a;

    // Point lights
    vec3 pointLights = vec3(0.0);
    for(int i = 0; i < pointLightCount && i < 4; i++)
    {
        pointLights += CalculatePointLight(
            pointLightPositions[i],
            pointLightColors[i],
            pointLightRanges[i],
            normal,
            viewDir
        );
    }

    // Combine all lighting
    vec3 lighting = ambientLight + directionalLight + pointLights;

    // Apply lighting to texture color
    vec3 result = texelColor.rgb * colDiffuse.rgb * fragColor.rgb * lighting;

    // Output
    finalColor = vec4(result, texelColor.a * colDiffuse.a * fragColor.a);
}
