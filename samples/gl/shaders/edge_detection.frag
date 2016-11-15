#version 330 core

in vec2 tex_coord;
out vec4 color;

uniform sampler2D texture0;
const float offset_x=1.0/640;
const float offset_y=1.0/480;

const vec2 offsets[9]=vec2[]
(
	vec2(-offset_x, offset_y),    // top-left
	vec2(0.0f,    offset_y),        // top-center
	vec2(offset_x,  offset_y),    // top-right
	vec2(-offset_x, 0.0f),          // center-left
	vec2(0.0f,    0.0f),              // center-center
	vec2(offset_x,  0.0f),          // center-right
	vec2(-offset_x, -offset_y), // bottom-left
	vec2(0.0f,    -offset_y),     // bottom-center
	vec2(offset_x,  -offset_y)  // bottom-right    
);
	
const float sobel_kernel[9]=float[]
(
	-2, -2, 0,
	-2,  0, 2,
	 0,  2, 2
);

const float laplacian_kernel[9]=float[]
(
	-1, -1, -1,
	-1,  8, -1,
	 -1,  -1, -1
);

void main()
{
	color=vec4(0.0f);
	
	//Convolution
    for(int i=0; i<9; ++i)
    {
		vec2 neighbour_pixel=tex_coord.xy + offsets[i];
		color += texture(texture0, vec2(neighbour_pixel.x, 1-neighbour_pixel.y)) * sobel_kernel[i];
    }
    
	//Thresholding
	float cutoff_intensity=0.9f;
	//Briefly, it outputs black color if r+g+b<cutoff_intensity
	float val=max(sign(dot(color.rgb, vec3(1.0f))-cutoff_intensity), 0.0f);
	color=vec4(val);
}