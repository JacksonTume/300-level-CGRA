#version 330 core

#define PI 3.14159

uniform mat4 uProjectionMatrix;
uniform mat4 uModelViewMatrix;
uniform vec3 uColor;

in VertexData{
    vec3 position;
    vec3 normal;
    vec2 textureCoord;
} f_in;

out vec4 fb_color;

// all the colour values
vec3 lightPos = vec3(1.2f, 1.0f, 2.0f);
vec3 diffuseColor = vec3(1.f, 0.f, 0.f);
vec3 ambientColor = vec3(0.f, 0.f, 1.f);
vec3 specularColor = vec3(1.f, 1.f, 1.f);
vec3 lightColor = vec3(1.f, 1.f, 1.f);

float F0 = 0.5;
float k = 0.33;
float roughness = 0.1;

void main()
{
    vec3 viewDir = normalize(-f_in.position);
    vec3 lightDir = normalize(lightPos - f_in.position);

    float NdotL = max(0, dot(f_in.normal, lightDir));
    float Rs = 0.f;
    if (NdotL > 0.0)
    {
        // calculations to make it easier later
        vec3 halfDir = normalize(lightDir + viewDir);
        float NdotH = max(0, dot(f_in.normal, halfDir));
        float NdotV = max(0, dot(f_in.normal, viewDir));
        float VdotH = max(0, dot(lightDir, halfDir));

        // now we can do frensel reflectance
        float frensel = pow(1.0f - VdotH, 5.0f);
        frensel *= (1.0f - F0);
        frensel += F0;

        // then beckmann distribution
        float M = roughness * roughness;
        float R1 = 1.0f / (4.0f * M * pow(NdotH, 4.0f));
        float R2 = (NdotH * NdotH - 1.0f) / (M * NdotH * NdotH);
        float beckmann = R1 * exp(R2);

        // finally calculate the shadowing
        float NdotH2 = 2.0f * NdotH;
        float G1 = (NdotH2 * NdotV) / VdotH;
        float G2 = (NdotH2 * NdotL) / VdotH;
        float G = min(1.0f, min(G1, G2));

        Rs = (frensel * beckmann * G) / (PI * NdotL * NdotV);
    }
    vec3 color = diffuseColor * lightColor * NdotL + lightColor * specularColor * NdotL * (k + Rs * (1.0f - k));

    fb_color = vec4(color, 1);
}