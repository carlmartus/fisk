#version 100
precision mediump float;

uniform sampler2D un_tex;

varying vec2 va_uv;

void main()
{
	gl_FragColor = texture2D(un_tex, va_uv);
}

