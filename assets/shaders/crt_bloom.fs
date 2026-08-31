#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Output fragment color
out vec4 finalColor;

// Custom uniform variables
uniform sampler2D texture0;
uniform vec2 uResolution;
uniform float uTime;
uniform float uChromaticStrength; // Sdoppiamento RGB proporzionale al trauma (0.0 to 0.03)
uniform float uBloomIntensity;     // Bagliore neon (0.0 to 1.5)
uniform float uCrtCurvature;      // Distorsione a barilotto CRT (0.0 to 0.08)
uniform float uScanlineIntensity; // Intensita scanlines (0.0 to 0.6)
uniform float uVignetteIntensity; // Vignettatura radiale (0.0 to 0.5)

// Barrel distortion algorithm for CRT screen curvature
vec2 CurveCoords(vec2 uv) {
    if (uCrtCurvature <= 0.001) return uv;
    uv = uv * 2.0 - 1.0;
    vec2 offset = abs(uv.yx) / vec2(6.0 / uCrtCurvature, 4.0 / uCrtCurvature);
    uv = uv + uv * offset * offset;
    return uv * 0.5 + 0.5;
}

void main() {
    vec2 uv = CurveCoords(fragTexCoord);

    // Black border outside CRT curvature bounds
    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) {
        finalColor = vec4(0.02, 0.02, 0.04, 1.0);
        return;
    }

    // 1. Dynamic Chromatic Aberration (RGB channel splitting on impacts/trauma)
    vec2 chromOffset = vec2(uChromaticStrength, uChromaticStrength * 0.5);
    float r = texture(texture0, uv - chromOffset).r;
    float g = texture(texture0, uv).g;
    float b = texture(texture0, uv + chromOffset).b;
    vec4 baseColor = vec4(r, g, b, 1.0);

    // 2. High-Pass Neon Bloom (Glow Extraction & Multi-Sample Blur)
    if (uBloomIntensity > 0.01) {
        vec4 bloom = vec4(0.0);
        float stepX = 1.8 / uResolution.x;
        float stepY = 1.8 / uResolution.y;

        bloom += texture(texture0, uv + vec2(-stepX * 2.0, -stepY * 2.0)) * 0.08;
        bloom += texture(texture0, uv + vec2( stepX * 2.0, -stepY * 2.0)) * 0.08;
        bloom += texture(texture0, uv + vec2(-stepX * 2.0,  stepY * 2.0)) * 0.08;
        bloom += texture(texture0, uv + vec2( stepX * 2.0,  stepY * 2.0)) * 0.08;

        bloom += texture(texture0, uv + vec2(-stepX, 0.0)) * 0.16;
        bloom += texture(texture0, uv + vec2( stepX, 0.0)) * 0.16;
        bloom += texture(texture0, uv + vec2(0.0, -stepY)) * 0.16;
        bloom += texture(texture0, uv + vec2(0.0,  stepY)) * 0.16;

        vec3 brightColor = max(bloom.rgb - vec3(0.35), vec3(0.0)) * 1.5;
        baseColor.rgb += brightColor * uBloomIntensity;
    }

    // 3. Phosphor Scanlines & Subtle Cathode Ray Ripple
    if (uScanlineIntensity > 0.01) {
        float scanline = sin((uv.y * uResolution.y * 3.14159265) + (uTime * 4.0)) * 0.5 + 0.5;
        baseColor.rgb -= baseColor.rgb * (1.0 - scanline) * uScanlineIntensity * 0.5;

        // Subtle vertical shadow mask grille
        float mask = cos(uv.x * uResolution.x * 3.14159265) * 0.03;
        baseColor.rgb += vec3(mask);
    }

    // 4. Radial Vignette (Cinematic Edge Darkening)
    if (uVignetteIntensity > 0.01) {
        vec2 center = uv - vec2(0.5);
        float dist = length(center) * 1.4142; // Normalized corner distance
        float vignette = smoothstep(1.0, 1.0 - uVignetteIntensity, dist);
        baseColor.rgb *= vignette;
    }

    finalColor = baseColor * fragColor;
}
