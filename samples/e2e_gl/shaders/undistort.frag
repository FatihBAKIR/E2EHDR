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

void main()
{
	vec2 undistorted_tex_coord = undistort_uv(tex_coord, param.undis);
	
	color = texture(frame, undistorted_tex_coord);
}