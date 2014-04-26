#include "fisk.h"
#include <GL/glew.h>
#include <math.h>

#define WAVE_CELLS 10
#define WAVE_CELLWIDTH 3.0f
#define WAVE_SPEED 10.0f
#define MAX_VERTS 300
#define TOP_END 5.0f
#define LOWEST 25.0f

enum {
	UNI_MVP,
	UNI_COL0,
	UNI_COL1,
};

static float prog = 0.0f;
static float waves[WAVE_CELLS];

static esShader shader;
static esGeo geometry;
static esGeoBuf vertices;
static int vertcount;

static struct sea_vert {
	float x, y;
	float depth;
} softverts[MAX_VERTS];

int
seaSetup(void)
{
	vertcount = 0;
	esGeoReset(&geometry, 2);
	esGeoBufCreate(&vertices);
	esGeoPoint(&geometry, 0, &vertices, GEODATA_FLOAT, 2, 0,
			sizeof(struct sea_vert), ES_FALSE);
	esGeoPoint(&geometry, 1, &vertices, GEODATA_FLOAT, 1, 8,
			sizeof(struct sea_vert), ES_FALSE);

	if (esShaderLoad(&shader, "res/waves.v.glsl", "res/waves.f.glsl")) {
		printf("Cannot load sea shader!\n");
		return 1;
	}

	if (esShaderUniformRegister(&shader, UNI_MVP, "un_mvp")) return 1;
	if (esShaderUniformRegister(&shader, UNI_COL0, "un_col0")) return 1;
	if (esShaderUniformRegister(&shader, UNI_COL1, "un_col1")) return 1;

	return 0;
}

static void
generate_waves(float x)
{
	int i;
	for (i=0; i<WAVE_CELLS; i++) {
		waves[i] = seaWaveHeight(x);
		x += WAVE_CELLWIDTH;
	}
}

static void
push_vertex(float x, float y, float depth)
{
	if (vertcount >= MAX_VERTS) return;

	struct sea_vert v = {
		.x = x,
		.y = y,
		.depth = depth,
	};

	softverts[vertcount++] = v;
}

static void
generate_vertices(float startx)
{
	vertcount = 0;

	// Top
	int i;
	for (i=0; i<WAVE_CELLS-1; i++) {
		float x0 = (float) i * WAVE_CELLWIDTH + startx;
		float x1 = x0 + WAVE_CELLWIDTH;
		float y0 = waves[i];
		float y1 = waves[i+1];

		push_vertex(x0, y0, 0.0f);
		push_vertex(x1, y1, 0.0f);
		push_vertex(x1, TOP_END, y1 + TOP_END);

		push_vertex(x0, y0, 0.0f);
		push_vertex(x1, TOP_END, y1 + TOP_END);
		push_vertex(x0, TOP_END, y0 + TOP_END);

		push_vertex(x0, TOP_END, 1.0f);
		push_vertex(x1, TOP_END, 1.0f);
		push_vertex(x1, LOWEST, 1.0f);

		push_vertex(x0, TOP_END, 1.0f);
		push_vertex(x1, LOWEST, 1.0f);
		push_vertex(x0, LOWEST, 1.0f);
	}
}

void
seaPosition(float fr, float startx)
{
	prog += fr;
	generate_waves(startx);
	generate_vertices(startx);
}

float
seaWaveHeight(float x)
{
	x = prog*WAVE_SPEED + x;
	return sinf(x*0.2f) - 0.2f*cosf(x*0.6f);
}

void
seaRender(float depth)
{
	esShaderUse(&shader);
	glUniformMatrix4fv(esShaderUniformGl(&shader, UNI_MVP), 1, 0,
			boatMvp());
	glUniform3f(esShaderUniformGl(&shader, UNI_COL0), 0.5f, 0.5f, 0.9f);
	glUniform3f(esShaderUniformGl(&shader, UNI_COL1), 0.0f, 0.0f, 0.5f);

	esGeoBufCopy(&vertices, softverts,
			sizeof(struct sea_vert)*vertcount, GEOBUF_STREAM);

	esShaderUse(&shader);
	esGeoRender(&geometry, vertcount);
}

void
seaHeight(float x)
{
}

