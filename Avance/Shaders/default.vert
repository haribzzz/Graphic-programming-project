#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in ivec4 aBoneIDs;
layout(location = 4) in vec4  aWeights;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform bool      animated;
uniform mat4      boneMatrices[100];

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;

void main() {
    vec4 pos    = vec4(aPos, 1.0);
    vec3 normal = aNormal;

    if (animated) {
        vec4 skinnedPos = vec4(0.0);
        vec3 skinnedNrm = vec3(0.0);

        for (int i = 0; i < 4; i++) {
            if (aBoneIDs[i] == -1) continue;
            mat4 bm = boneMatrices[aBoneIDs[i]];
            skinnedPos += aWeights[i] * (bm * vec4(aPos, 1.0));
            skinnedNrm += aWeights[i] * (mat3(bm) * aNormal);
        }
        pos    = skinnedPos;
        normal = skinnedNrm;
    }

    gl_Position = projection * view * model * pos;
    FragPos     = vec3(model * pos);
    Normal      = mat3(transpose(inverse(model))) * normal;
    TexCoord    = aTexCoord;
}