#version 330

// Bloom post-processing shader

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform float bloomThreshold;
uniform float bloomIntensity;

out vec4 finalColor;

void main()
{
    vec4 texelColor = texture(texture0, fragTexCoord);

    // Calculate luminance
    float brightness = dot(texelColor.rgb, vec3(0.2126, 0.7152, 0.0722));

    // Extract bright areas
    if(brightness > bloomThreshold)
    {
        // Apply bloom to bright areas
        vec3 bloom = texelColor.rgb * bloomIntensity;
        finalColor = vec4(texelColor.rgb + bloom, texelColor.a);
    }
    else
    {
        finalColor = texelColor;
    }

    finalColor *= colDiffuse * fragColor;
}
