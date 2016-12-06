#version 330 core

in vec2 tex_coord;
out vec4 color;

uniform sampler2D texture0;

uniform float crf_r[256];
uniform float crf_g[256];
uniform float crf_b[256];

uniform float exposure;

void main()
{
    const vec2 focalLength = vec2(1083.7449050061734, 1083.7449050061734);
    const vec2 opticalCenter = vec2(639.5, 368.5);
    const float distortionCoeffs[5] = float[](-0.58179279888273083, 0.76873518296930821, 0., 0., 0.59197125404147499);
    const vec2 imageSize = vec2(1280.f, 738.f);

    const vec2 opticalCenterUV = opticalCenter / imageSize;
    vec2 shiftedUVCoordinates = tex_coord - opticalCenterUV;

    vec2 lensCoordinates = (tex_coord * imageSize - opticalCenter) / focalLength;

    float radius2 = dot(lensCoordinates, lensCoordinates);
    float radius4 = radius2 * radius2;
    float radius8 = radius4 * radius4;
    float radius16 = radius8 * radius8;
    float radius32 = radius16 * radius16;

    float coefficientTerm = distortionCoeffs[0] * radius2
                         + distortionCoeffs[1] * radius4;
                         + distortionCoeffs[2] * radius8;
                         + distortionCoeffs[3] * radius16;
                         + distortionCoeffs[4] * radius32;

    vec2 distortedUV = (((lensCoordinates + lensCoordinates * coefficientTerm)) * focalLength) / imageSize;
    vec2 resultUV = (distortedUV + opticalCenterUV);

    //-15,5
    float exposure = -2;
    float gamma = 0.1;

    vec4 col = texture(texture0, vec2(resultUV.x, 1-resultUV.y));

    float r = crf_r[min(int(col.r * 255), 255)];
    float g = crf_g[min(int(col.g * 255), 255)];
    float b = crf_b[min(int(col.b * 255), 255)];

    vec3 cols = vec3(r, g, b);

    //cols /= exposure;

    //cols = cols * pow(2.0, exposure);
    //cols = pow(cols, vec3(pow(2.0, gamma)));
    cols *= 8;
    cols = pow(cols, vec3(1.0 / 2.2)); // gamma correction
    color = vec4(cols, 1.0);

    //color = col;
}