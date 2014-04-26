#version 100
precision mediump float;

varying float va_depth;

uniform vec3 un_col0;
uniform vec3 un_col1;

void main()
{
	vec3 col = mix(un_col0, un_col1, va_depth);
	gl_FragColor = vec4(col, 1);
}

