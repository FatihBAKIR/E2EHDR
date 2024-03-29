#version 330 core

in vec2 tex_coord;
out vec4 color;

uniform sampler2D texture0;

struct crf
{
    float red[256];
    float green[256];
    float blue[256];
};

uniform crf response;

uniform float exposure;

int get_byte(float channel)
{
    return min(int(channel * 255), 255);
}

vec3 apply_crf(vec3 col)
{
	float r = response.red[get_byte(col.r)];
	float g = response.green[get_byte(col.g)];
	float b = response.blue[get_byte(col.b)];

    return vec3(r, g, b);
}

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

    vec3 cols = apply_crf(vec3(col));
    cols /= exposure;

    //cols = cols * pow(2.0, exposure);
	//cols = pow(cols, vec3(pow(2.0, gamma)));

    cols *= 8;
    cols = pow(cols, vec3(1.0 / 2.2)); // gamma correction
    color = vec4(cols, 1.0);

    //color = col;
}