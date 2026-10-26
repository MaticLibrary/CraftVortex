#version 330 core

in vec2 vTexCoord;
in vec4 vColor;

out vec4 FragColor;

uniform sampler2D uTexture;
uniform bool uUseTexture;

void main() {
    if (uUseTexture) {
        vec4 tex = texture(uTexture, vTexCoord);
        FragColor = tex * vColor;
    } else {
        FragColor = vColor;
    }
}
