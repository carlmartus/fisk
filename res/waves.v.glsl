#version 100
precision mediump float;

attribute vec2 in_loc;
attribute float in_depth;

varying float va_depth;

void main()
{
	va_depth = in_depth;
	gl_Position = vec4(in_loc, 0, 1);
}


