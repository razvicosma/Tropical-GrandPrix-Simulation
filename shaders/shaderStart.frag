#version 410 core

in vec3 fNormal;
in vec4 fPosEye;
in vec2 fTexCoords;
in vec4 fragPosLightSpace;

out vec4 fColor;

//view matrix
uniform mat4 view;

//lighting
uniform	vec3 lightDir;
uniform	vec3 lightColor;

//pointLight
uniform vec3 lightDirPoint;
uniform vec3 lightColorPoint;
uniform vec3 lightPosPoint;

// turnLight
uniform float lightness;
uniform float turnPointLight;

//texture
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

//directional light components
vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;
float shininess = 32.0f;

//point light components
vec3 ambientPoint;
vec3 diffusePoint;
vec3 specularPoint;
vec3 lightPoint;

//attenuation factors
float constant = 1.0f;
float linear = 0.0045f;
float quadratic = 0.0075f;

//foggg components
uniform vec3 fogColor;
uniform float fogToggle;
float foggy;

//shadow component
float shadow;


void computeLightComponents()
{		
	vec3 cameraPosEye = vec3(0.0f);
	vec3 normalEye = normalize(fNormal);	
	vec3 lightDirN = normalize(lightDir);
	vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);
		
	ambient = ambientStrength * lightColor;
	
	diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor * lightness;
	
	vec3 reflection = reflect(-lightDirN, normalEye);
	float specCoeff = pow(max(dot(viewDirN, reflection), 0.0f), shininess);
	specular = specularStrength * specCoeff * lightColor * lightness;

	ambient *= texture(diffuseTexture, fTexCoords).rgb;
	diffuse *= texture(diffuseTexture, fTexCoords).rgb;
	specular *= texture(specularTexture, fTexCoords).rgb;
}

void computePointLight() {

    vec3 cameraPosEye = vec3(0.0f);
    vec3 lightPosEye = (view * vec4(lightPosPoint, 1.0)).xyz;
    vec3 lightDirN = normalize(lightPosEye - fPosEye.xyz);                    
    vec3 normalEye = normalize(fNormal);
    vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);    

    float diff = max(dot(normalEye, lightDirN), 0.0);

    vec3 reflection = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(viewDirN, reflection), 0.0f), shininess);

    float dist = length(lightPosEye - fPosEye.xyz);
    float att = 1.0f / (constant + linear * dist + quadratic * (dist * dist));

    ambientPoint = ambientStrength * lightColorPoint;
    diffusePoint = diff * lightColorPoint;
    specularPoint = specularStrength * specCoeff * lightColorPoint;

	ambientPoint *= texture(diffuseTexture, fTexCoords).rgb;
	diffusePoint *= texture(diffuseTexture, fTexCoords).rgb;
	specularPoint *= texture(specularTexture, fTexCoords).rgb;

    lightPoint = att * (ambientPoint + diffusePoint + specularPoint) * turnPointLight;
}

void computeShadow() {
	vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	normalizedCoords = normalizedCoords * 0.5 + 0.5;

	if (normalizedCoords.z > 1.0f){
	 shadow = 0.0f;
	 return;
	}

	float closestDepth = texture(shadowMap, normalizedCoords.xy).r;
	float currentDepth = normalizedCoords.z;
	float bias = 0.005f;

	shadow = currentDepth - bias > closestDepth ? 1.0f : 0.0f;
}

void computeFog() {
	float dist = length((fPosEye).xyz);

    foggy = exp(-fogToggle * dist);
    foggy = clamp(foggy, 0.0, 1.0);
}

void main() 
{
	computeLightComponents();
	computePointLight();
	computeShadow();
	computeFog();

	vec3 color = mix(fogColor,min((ambient + (1.0f - shadow) * diffuse) + (1.0f - shadow) * specular + lightPoint, 1.0f),foggy);
    
    fColor = vec4(color, 1.0f);
}
