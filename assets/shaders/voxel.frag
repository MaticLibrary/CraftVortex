#version 330 core

in vec3 vFragPos;
in vec2 vTexCoord;
in vec3 vNormal;
in float vAO;
in float vEmissive;
in vec3 vEmitColor;

out vec4 FragColor;

uniform sampler2D uTextureAtlas;
uniform vec3 uCameraPos;
uniform vec3 uSunDir;
uniform vec3 uFogColor;
uniform float uFogStart;
uniform float uFogEnd;
uniform float uTime;

void main() {
    vec4 texColor = texture(uTextureAtlas, vTexCoord);
    if (texColor.a < 0.1) {
        discard;
    }

    // Directional sunlight + ambient
    vec3 norm = normalize(vNormal);
    float diff = max(dot(norm, normalize(uSunDir)), 0.0);
    
    // Top face is brightest, sides slightly darker, bottom darkest
    float faceFactor = 0.85;
    if (norm.y > 0.5) faceFactor = 1.0;
    else if (norm.y < -0.5) faceFactor = 0.55;
    else if (abs(norm.x) > 0.5) faceFactor = 0.8;
    else faceFactor = 0.75;

    // Ambient occlusion factor (0 to 3 -> 0.4 to 1.0)
    float aoMult = 0.4 + 0.6 * (vAO / 3.0);

    vec3 baseLight = vec3(0.4) + vec3(0.6) * diff;
    vec3 litColor = texColor.rgb * baseLight * faceFactor * aoMult;

    // Emissive magical glow (crystals, runes, lanterns)
    if (vEmissive > 0.01) {
        float pulse = 0.85 + 0.15 * sin(uTime * 3.5 + vFragPos.x * 2.0 + vFragPos.z * 2.0);
        vec3 glow = vEmitColor * pulse * vEmissive * 1.5;
        litColor = max(litColor, texColor.rgb * 0.7 + glow * 0.8);
    }

    // Atmospheric Biome Fog
    float dist = length(vFragPos - uCameraPos);
    float fogFactor = clamp((dist - uFogStart) / (uFogEnd - uFogStart), 0.0, 1.0);
    // Smooth quadratic fog falloff
    fogFactor = fogFactor * fogFactor;

    vec3 finalColor = mix(litColor, uFogColor, fogFactor);

    FragColor = vec4(finalColor, texColor.a);
}
