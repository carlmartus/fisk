#version 100
precision mediump float;

attribute vec2 in_loc;
attribute vec2 in_uv;

varying vec2 va_uv;

uniform mat4 un_mvp;

void main()
{
	va_uv = in_uv;
	gl_Position = un_mvp*vec4(in_loc, 0, 1);
}


