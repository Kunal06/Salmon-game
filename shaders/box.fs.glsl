#version 330

uniform sampler2D screen_texture;
uniform float time;
uniform float dead_timer;
uniform int debug_mode;
uniform int window_width;
uniform int window_height;
uniform float pos_x;
uniform float pos_y;
uniform vec3 fcolor;

in vec2 uv;

layout(location = 0) out vec4 color;

vec2 distort(vec2 uv) 
{
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// HANDLE THE WATER WAVE DISTORTION HERE (you may want to try sin/cos)
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	vec2 coord = uv.xy;
    return coord;
}

vec4 color_shift(vec4 in_color) 
{
	vec4 color = in_color;
	if(debug_mode == 1){
		if (gl_FragCoord.x < pos_x + 40 && gl_FragCoord.x > pos_x - 40 && gl_FragCoord.y < pos_y + 40 && gl_FragCoord.y > pos_y - 40)
			color = vec4(1.0, 1.0, 1.0, 1.0);
	}
	return color;
}

vec4 fade_color(vec4 in_color) 
{
	vec4 color = in_color;
	if (dead_timer > 0)
		color -= 0.1 * dead_timer * vec4(0.1, 0.1, 0.1, 0);

	return color;
}

void main()
{
	vec2 coord = distort(uv);

    vec4 in_color = texture(screen_texture, coord);
    color = color_shift(in_color);
    color = fade_color(color);
}