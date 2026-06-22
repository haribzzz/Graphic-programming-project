const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform sampler2D texture3;
uniform sampler2D texture4;
uniform sampler2D texture5;
uniform bool useMultiTexture;

uniform vec3 lightPos;
uniform vec3 lightDir;
uniform bool flashlightOn;

uniform vec3 lampPos;
uniform vec3 lampPos2;
uniform vec3 lampPos3;
uniform vec3 lampPos4;
uniform vec3 lampPos5;
uniform vec3 lampPos6;

uniform bool lampara1On;
uniform bool lampara2On;
uniform bool lampara3On;
uniform bool lampara4On;
uniform bool lampara5On;
uniform bool lampara6On;

void main()
{
    vec3 color;
    if (useMultiTexture)
    {
        vec3 diffuse = texture(texture1, TexCoord).rgb;
        float ao = texture(texture2, TexCoord).r;
        float metal = texture(texture4, TexCoord).r;
        color = diffuse * ao;
        color = mix(color, color * 0.5, metal);
    }
    else
    {
        color = texture(texture1, TexCoord).rgb;
    }

    vec3 norm = normalize(Normal);
    vec3 ambient = color * 0.08;

    vec3 flashlight = vec3(0.0);
    if (flashlightOn)
    {
        vec3 toFrag = normalize(FragPos - lightPos);
        float theta = dot(toFrag, normalize(lightDir));
        float innerCutoff = 0.978;
        float outerCutoff = 0.956;
        float coneInt = smoothstep(outerCutoff, innerCutoff, theta);
        vec3 ldir = normalize(lightPos - FragPos);
        float diff = abs(dot(norm, ldir));
        float dist = length(lightPos - FragPos);
        float att = 1.0 / (1.0 + 0.09 * dist + 0.032 * dist * dist);
        flashlight = diff * coneInt * att * 3.0 * vec3(1.0, 0.98, 0.9);
    }

    vec3 lampLight = vec3(0.0);
    vec3 lampLight2 = vec3(0.0);
    vec3 lampLight3 = vec3(0.0);
    vec3 lampLight4 = vec3(0.0);
    vec3 lampLight5 = vec3(0.0);
    vec3 lampLight6 = vec3(0.0);

    if (lampara1On) {
        vec3 d = normalize(lampPos - FragPos);
        float df = abs(dot(norm, d));
        float ds = length(lampPos - FragPos);
        float a = 1.0 / (1.0 + 0.015 * ds + 0.003 * ds * ds);
        lampLight = df * a * 2.2 * vec3(1.0, 0.95, 0.8);
    }
    if (lampara2On) {
        vec3 d = normalize(lampPos2 - FragPos);
        float df = abs(dot(norm, d));
        float ds = length(lampPos2 - FragPos);
        float a = 1.0 / (1.0 + 0.015 * ds + 0.003 * ds * ds);
        lampLight2 = df * a * 2.2 * vec3(1.0, 0.95, 0.8);
    }
    if (lampara3On) {
        vec3 d = normalize(lampPos3 - FragPos);
        float df = abs(dot(norm, d));
        float ds = length(lampPos3 - FragPos);
        float a = 1.0 / (1.0 + 0.015 * ds + 0.003 * ds * ds);
        lampLight3 = df * a * 2.2 * vec3(1.0, 0.95, 0.8);
    }
    if (lampara4On) {
        vec3 d = normalize(lampPos4 - FragPos);
        float df = abs(dot(norm, d));
        float ds = length(lampPos4 - FragPos);
        float a = 1.0 / (1.0 + 0.015 * ds + 0.003 * ds * ds);
        lampLight4 = df * a * 2.2 * vec3(1.0, 0.95, 0.8);
    }
    if (lampara5On) {
        vec3 d = normalize(lampPos5 - FragPos);
        float df = abs(dot(norm, d));
        float ds = length(lampPos5 - FragPos);
        float a = 1.0 / (1.0 + 0.015 * ds + 0.003 * ds * ds);
        lampLight5 = df * a * 2.2 * vec3(1.0, 0.95, 0.8);
    }
    if (lampara6On) {
        vec3 d = normalize(lampPos6 - FragPos);
        float df = abs(dot(norm, d));
        float ds = length(lampPos6 - FragPos);
        float a = 1.0 / (1.0 + 0.015 * ds + 0.003 * ds * ds);
        lampLight6 = df * a * 2.2 * vec3(1.0, 0.95, 0.8);
    }

    vec3 result = (ambient + flashlight + lampLight + lampLight2 + lampLight3 + lampLight4 + lampLight5 + lampLight6) * color;
    FragColor = vec4(result, 1.0);
}
)";