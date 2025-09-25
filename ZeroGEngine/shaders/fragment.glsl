#version 330 core
in vec3 FragPos;  
in vec3 Normal;   
out vec4 FragColor;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 ambientColor;
uniform vec3 viewPos;
uniform sampler2D texture1; // Texture sampler

void main()
{
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);

    // Ambient
    vec3 ambient = 0.1 * lightColor;

    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = 0.5 * spec * lightColor;

    // Apply texture
    vec4 texColor = texture(texture1, FragPos.xy); // Sample the texture
    vec3 result = (ambient + diffuse + specular) * texColor.rgb; // Phong lighting

    FragColor = vec4(result, texColor.a);
}
