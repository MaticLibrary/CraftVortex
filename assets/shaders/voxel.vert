#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in float aAO;
layout (location = 4) in float aEmissive;
layout (location = 5) in vec3 aEmitColor;

out vec3 vFragPos;
out vec2 vTexCoord;
out vec3 vNormal;
out float vAO;
out float vEmissive;
out vec3 vEmitColor;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

void main() {
    vec4 worldPos = uModel * vec4(aPos, 1.0);
    vFragPos = worldPos.xyz;
    vTexCoord = aTexCoord;
    vNormal = aNormal;
    vAO = aAO;
    vEmissive = aEmissive;
    vEmitColor = aEmitColor;

    gl_Position = uProjection * uView * worldPos;
}
