#version 330 core

in vec2 tex_coord;
out vec4 color;

uniform int is_left;
uniform bool wb_eq;
uniform sampler2D left_avg;
uniform sampler2D right_avg;

uniform sampler2D texture0;

struct undistort_params
{
    vec2 focal_length;
    vec2 optical_center;
    vec2 image_size;
    float dist_coeffs[5];
};

struct crf
{
    float red[256];
    float green[256];
    float blue[256];
};

struct camera_params
{
    crf         response;
    undistort_params   undis;
    float       exposure;
};

struct preview_params
{
    bool    apply_crf;
    bool    undistort;
    float   exposure;
};

uniform camera_params camera;
uniform preview_params prev;

int get_byte(float channel)
{
    return min(int(channel * 255), 255);
}

vec3 apply_crf(vec3 col)
{
	float r = camera.response.red[get_byte(col.r)];
	float g = camera.response.green[get_byte(col.g)];
	float b = camera.response.blue[get_byte(col.b)];

    return vec3(r, g, b);
}

vec2 undistort(vec2 inp)
{
    vec2 focalLength = camera.undis.focal_length;
    vec2 opticalCenter = camera.undis.optical_center;
    float distortionCoeffs[5] = camera.undis.dist_coeffs;
    vec2 imageSize = camera.undis.image_size;

    vec2 opticalCenterUV = opticalCenter / imageSize;
    vec2 shiftedUVCoordinates = inp - opticalCenterUV;

    vec2 lensCoordinates = (inp * imageSize - opticalCenter) / focalLength;

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

    return resultUV;
}

void main()
{
    vec2 resultUV = prev.undistort ? undistort(tex_coord) : tex_coord;

    vec4 col = texture(texture0, vec2(resultUV.x, 1-resultUV.y));

    if (!prev.apply_crf)
    {
        color = col;
        return;
    }

    if (wb_eq)
    {
        vec3 ratio = vec3(1);

        if (is_left == 0)
        {
            ratio = vec3(texture(left_avg, vec2(0.5)) / texture(right_avg, vec2(0.5)));
            col *= vec4(ratio, 1);
        }
    }

    vec3 cols = apply_crf(vec3(col));
    cols /= camera.exposure;

    cols *= pow(2.0, prev.exposure);
    cols = pow(cols, vec3(1.0 / 2.2));
    color = vec4(cols, 1.0);
}