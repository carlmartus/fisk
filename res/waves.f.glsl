#version 100
precision mediump float;

varying float va_depth;

uniform vec3 un_col0;
uniform vec3 un_col1;

void main()
{
	gl_FragColor = vec4(1.0, 0.0, va_depth, 1.0);
}

