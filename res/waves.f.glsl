#version 100
precision mediump float;

varying float va_depth;

uniform vec3 un_col0;
uniform vec3 un_col1;

void main()
{
	float clam = va_depth;
	if (clam > 1.0) clam = 1.0;

	vec3 col = mix(un_col0, un_col1, clam);
	gl_FragColor = vec4(col, 1);
}

