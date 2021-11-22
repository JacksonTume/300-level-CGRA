#version 330 core

out vec4 fb_color;

uniform mat4 uProjectionMatrix;
uniform mat4 uModelViewMatrix;
uniform vec3 uColor;

in VertexData{
	vec3 position;
	vec3 normal;
	vec2 textureCoord;
} f_in;

uniform sampler2D Texture;
uniform sampler2D NormalMap;

vec3 ambientColor = vec3(0.f, 0.f, 1.f);
vec3 diffuseColor = vec3(1.f, 0.f, 0.f);
vec3 specularColor = vec3(1.f, 1.f, 1.f);
vec3 lightPos = vec3(1.2f, 1.0f, 2.0f);


void main()
{
	vec3 norms = normalize(texture(NormalMap, f_in.textureCoord).rgb * 2.0 - 1.0);
	vec3 lightDir = normalize(lightPos - f_in.position);
	vec3 lightNorm = normalize(-lightDir);
	float spec = 0.0f;
	float l = max(dot(lightNorm, norms), 0.0f);
	
	if (l > 0.0)
	{
		vec3 eye = normalize(-f_in.position);
		vec3 halfDir = normalize(lightNorm + eye);
		float specA = max(dot(halfDir, norms), 0.0);
		spec = pow(specA, 20.0f);
	}

	// Comment either of the fb_color variables out to choose between the normal map and non-normal map version
	fb_color = vec4(texture(Texture, f_in.textureCoord).rgb + (l * diffuseColor) + (spec * specularColor), 1);
	//fb_color = texture(Texture, f_in.textureCoord);
}