#version 330 core

#define WINDOW_SIZE 9

in vec2 tex_coord;
out vec4 color;

uniform float dx;
uniform float dy;
uniform sampler2D disparity_map;

vec4[WINDOW_SIZE] sort(vec4 arr[WINDOW_SIZE]);

void main()
{
	//3x3 Median Filter
	vec4 arr[WINDOW_SIZE];
	int idx=0;
	for (int i=-1; i<=1; ++i)
	{
		for (int j=-1; j<=1; ++j)
		{
			arr[idx++] = texture(disparity_map, vec2(tex_coord.x+i*dx, tex_coord.y+j*dy));
		}
	}
	
	vec4 sorted_arr[WINDOW_SIZE] = sort(arr);
	
	color = sorted_arr[WINDOW_SIZE/2];
}

vec4[WINDOW_SIZE] sort(vec4 arr[WINDOW_SIZE])
{
	for (int i = 1; i < WINDOW_SIZE; ++i)
	{
		int j = i;
		while (j > 0 && arr[j].r < arr[j-1].r)
		{
			  vec4 temp = arr[j];
			  arr[j] = arr[j-1];
			  arr[j-1] = temp;
			  --j;
		}
	}
	
	return arr;
}