#version 330

in vec2 texCoord;                      ///< Texture coordinate from the vertex shader

uniform sampler2D hdrSampler;           ///< HDR image sampler
uniform float Yw;                       ///< Burn-out threshold
uniform float key;                      ///< Image key
uniform float sat;                      ///< Saturation

layout(location=0) out vec4 fragColor; ///< Destination color

float luminance(vec3 color)
{
    // Assuming that input color is in linear sRGB color space.

    return (color.r * 0.2126) + 
           (color.g * 0.7152) + 
           (color.b * 0.0722);
}

vec3 gammaCorrect(vec3 data)
{
    return pow(data, vec3(0.45, 0.45, 0.45));
}

vec3 tonemap(vec3 RGB, float logAvgLum)
{
    mat3 RGB2XYZ = mat3(0.4124, 0.2126, 0.0193,  // first column
                        0.3576, 0.7152, 0.1192,  // second column
                        0.1805, 0.0722, 0.9505); // third column

    vec3 XYZ = RGB2XYZ * RGB;

    float X = XYZ.x;
    float Y = XYZ.y;
    float Z = XYZ.z;

    float sum = a;
    float x, y, z;

    if (sum == 0.0)
    {
        x = y = z = 0.0;
    }
    else
    {
        x = X / sum;
        y = Y / sum;
        z = (1.0 - x) - y;
    }

    float Xd, Zd;

    // Key mapping
    if (logAvgLum > 0.0)
    {
        Y = (key / logAvgLum) * Y;
    }
    else
    {
        Y = 0.0;
    }

    // Tonemapping equation
    //float Yd = (Y * (1.0 + Y / (Yw * Yw))) / (1.0 + Y);

    float La;
    int bestIdx = 0;
    for (int i = 0; i < 7; ++i)
    {
        float v1 = exp(texture(hdrSampler, texCoord, i).a) * key / logAvgLum;
        float v2 = exp(texture(hdrSampler, texCoord, i+1).a) * key / logAvgLum;

        if (abs(v1 - v2) > 0.25 && bestIdx == 0)
        {
            bestIdx = i;
        }
    }

    La = exp(texture(hdrSampler, texCoord, bestIdx).a) * key / logAvgLum;
    float Yd = Y / (1.0 + La);

    if (y == 0.0)
    {
        Xd = Zd = 0.0; 
    }
    else
    {
        float sump = Yd / y;
        Xd = x * sump;
        Zd = z * sump;
    }

    mat3 XYZ2RGB = inverse(RGB2XYZ);
    vec3 XYZd = vec3(Xd, Yd, Zd);

    vec3 RGBd = XYZ2RGB * XYZd;

    float L = luminance(XYZd);
    RGBd = pow(RGBd / L, vec3(sat, sat, sat)) * L;

    return RGBd;
}

void main()
{
    vec3 hdr = texture(hdrSampler, texCoord).rgb;
    
    float logAvgLum = exp(texture(hdrSampler, texCoord, 20.0).a);

    vec3 ldr = tonemap(hdr, logAvgLum);

    fragColor.rgb = ldr;
}
