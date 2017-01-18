#version 330 core

in vec2 tex_coord;
out vec4 color;

uniform sampler2D frame;

struct undistort
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
    undistort   undis;
    float       exposure;
};

uniform camera_params param;

int get_byte(float channel)
{
    return min(int(channel * 255), 255);
}

vec3 apply_crf(vec3 col, camera_params camera)
{
	float r = camera.response.red[get_byte(col.r)];
	float g = camera.response.green[get_byte(col.g)];
	float b = camera.response.blue[get_byte(col.b)];

    return vec3(r, g, b);
}

vec2 undistort_uv(vec2 inp, undistort undis)
{
    vec2 focalLength = undis.focal_length;
    vec2 opticalCenter = undis.optical_center;
    float distortionCoeffs[5] = undis.dist_coeffs;
    vec2 imageSize = undis.image_size;

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

float luminance(vec3 color)
{
    // Assuming that input color is in linear sRGB color space.

    return (color.r * 0.2126) +
           (color.g * 0.7152) +
           (color.b * 0.0722);
}

float weight(float val)
{
    float w;

    if (val <= 0.5){
        w = val * 2.0;
    }
    else {
        w = (1.0 - val) * 2.0;
    }

    return w;
}

void main()
{
	vec2 undistorted_tex_coord = undistort_uv(tex_coord, param.undis);
	
	vec3 col = vec3(texture(frame, undistorted_tex_coord));
	
	color = vec4(apply_crf(col, param) / param.exposure, weight(luminance(col)));
}