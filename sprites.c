#include "fisk.h"
#include <GL/glew.h>

#define FILE_GRID 4
#define UV_RAD (1.0f / (float) FILE_GRID)

#define _UV(x, y, dim) {\
	.u0 = (float) (x+0) * UV_RAD,\
	.v0 = (float) (y+0) * UV_RAD,\
	.u1 = (float) (x+dim) * UV_RAD,\
	.v1 = (float) (y+dim) * UV_RAD,\
}

#define MAX_VERTS 300
static const struct file_uv {
	float u0, v0, u1, v1;
} file_uvs[] = {
	[SPRITE_SHIP] = _UV(0, 0, 2),
	[SPRITE_HOOK] = _UV(2, 1, 1),
	[SPRITE_ROPE] = _UV(3, 0, 1),
	[SPRITE_FISH_SMALL] = _UV(0, 3, 1),
	[SPRITE_FISH_FLYING] = _UV(1, 3, 1),
	[SPRITE_FISH_BIKE] = _UV(3, 1, 1),
	[SPRITE_FISH_SUBMARINE] = _UV(2, 2, 2),
};

static struct sprite_vert {
	float x, y;
	float u, v;
} softverts[MAX_VERTS];

static esGeo geometry;
static esGeoBuf vertices;
static int vertcount;

enum {
	UNI_MVP,
	UNI_TEX,
};
static esShader shader;
static esTexture texture;

const esVec2 spriteNoRot = { 1.0f, 0.0f };

int
spriteSetup(void)
{
	if (esTextureLoad(&texture, "res/sprites.png", TEX_NONE, TEX_NONE)) {
		printf("Cannot open sprites texture\n");
		return 1;
	}

	if (esShaderLoad(&shader, "res/sprite.v.glsl", "res/sprite.f.glsl")) {
		printf("Cannot open sprites shader\n");
		return 1;
	}

	if (esShaderUniformRegister(&shader, UNI_MVP, "un_mvp")) {
		printf("Cannot register sprites mvp uniform!\n");
		return 1;
	}

	if (esShaderUniformRegister(&shader, UNI_TEX, "un_tex")) {
		printf("Cannot register sprites texture uniform!\n");
		return 1;
	}

	vertcount = 0;
	esGeoReset(&geometry, 2);
	esGeoBufCreate(&vertices);
	esGeoPoint(&geometry, 0, &vertices, GEODATA_FLOAT, 2, 0,
			sizeof(struct sprite_vert), ES_FALSE);
	esGeoPoint(&geometry, 1, &vertices, GEODATA_FLOAT, 2, 8,
			sizeof(struct sprite_vert), ES_TRUE);

	return 0;
}

static inline void
push_vertex(float x, float y, float u, float v)
{
	if (vertcount >= MAX_VERTS) return;

	struct sprite_vert vert = {
		.x = x,
		.y = y,
		.u = u,
		.v = v,
	};

	softverts[vertcount++] = vert;
}

void
spriteAdd(enum spriteId id, float x, float y, esVec2 trans)
{
	esVec2 vec_x = trans;
	esVec2 vec_y = {
		.x = -vec_x.y,
		.y = vec_x.x,
	};

	struct file_uv uv = file_uvs[id];

	esVec2 a, b, c, d;
	/* a b
	 * c d */

	a.x = x - vec_x.x + vec_y.x;
	a.y = y - vec_x.y + vec_y.y;
	b.x = x + vec_x.x + vec_y.x;
	b.y = y + vec_x.y + vec_y.y;
	c.x = x - vec_x.x - vec_y.x;
	c.y = y - vec_x.y - vec_y.y;
	d.x = x + vec_x.x - vec_y.x;
	d.y = y + vec_x.y - vec_y.y;

	push_vertex(a.x, a.y, uv.u0, uv.v1);
	push_vertex(b.x, b.y, uv.u1, uv.v1);
	push_vertex(d.x, d.y, uv.u1, uv.v0);

	push_vertex(a.x, a.y, uv.u0, uv.v1);
	push_vertex(d.x, d.y, uv.u1, uv.v0);
	push_vertex(c.x, c.y, uv.u0, uv.v0);
}

void
spritePushCustomVertice(float x, float y, float u, float v)
{
	push_vertex(x, y, u, v);
}

void
spriteFlushRender(void)
{
	esGeoBufCopy(&vertices, softverts,
			sizeof(struct sprite_vert)*vertcount, GEOBUF_STREAM);

	esTextureUse(&texture);
	esShaderUse(&shader);
	glUniform1i(esShaderUniformGl(&shader, UNI_TEX), 0);
	glUniformMatrix4fv(esShaderUniformGl(&shader, UNI_MVP), 1, 0, boatMvp());

	esGeoRender(&geometry, vertcount);

	vertcount = 0;
}

void
spriteGetUvs(enum spriteId id, float *u0, float *v0, float *u1, float *v1)
{
	struct file_uv uv = file_uvs[id];
	*u0 = uv.u0;
	*v0 = uv.v0;
	*u1 = uv.u1;
	*v1 = uv.v1;
}

