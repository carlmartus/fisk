#ifndef ESTK_H
#define ESTK_H
#include <stdlib.h>
#include <SDL/SDL_mixer.h>

enum esBool {
	ES_FALSE = 0,
	ES_TRUE = 1,
};

// Misc
void _esCheckGlError();
#define esCheckGlError() _esCheckGlError(__FILE__, __LINE__)

// Game loop
void esGameInit(int screen_width, int screen_height);
void esGameGlSwap(void);
void esGameLoop(void (*frame)(float t), void (*exit)(void), int frame_rate);
void esGameLoopQuit(void);
void esGameRegisterKey(int sdlkey, void (*callback)(int key, int down));

// Shader
typedef int esUniform;
#define UNIFORMS_MAX 10

typedef struct {
	int glprogram;
	esUniform uniforms[UNIFORMS_MAX];
} esShader;

int esShaderLoad(esShader *shader,
		const char *vert_file, const char *frag_file);
void esShaderUse(const esShader *shader);
void esShaderUnload(esShader *shader);
int esShaderUniformRegister(esShader *shader,
		esUniform reg, const char *name);
int esShaderUniformGl(esShader *shader, esUniform reg);

// Geometry buffer
#define GEOBUFS_MAX 8

enum esGeoBufType {
	GEOBUF_STATIC,
	GEOBUF_DYNAMIC,
	GEOBUF_STREAM,
};

typedef struct {
	unsigned int glbuf;
} esGeoBuf;

void esGeoBufCreate(esGeoBuf *buf);
void esGeoBufCopy(esGeoBuf *buf,
		const void *data, size_t size, enum esGeoBufType type);
void esGeoBufDelete(esGeoBuf *buf);

// Geometry
enum esGeoDataType {
	GEODATA_FLOAT,
	GEODATA_INT,
	GEODATA_BYTE,
	GEODATA_UBYTE,
};

typedef struct {
	int bufcount;

	struct {
		esGeoBuf *geobuf;
		enum esGeoDataType datatype;
		int elements;
		size_t offset, stride;
		enum esBool normalized;
	} buf[GEOBUFS_MAX];
} esGeo;

void esGeoReset(esGeo *geo, int bufcount);
void esGeoPoint(esGeo *geo, int id, esGeoBuf *geobuf,
		enum esGeoDataType datatype, int elements,
		size_t offset, size_t stride, enum esBool normalized);
void esGeoRender(const esGeo *geo, int vertices);

// Projection
typedef struct { float x, y, z; } esVec3;

void esProjOrtho(float *mat, float x0, float y0, float x1, float y1);
void esProjPerspective(
		float *mat, float fov, float screenratio, float near, float far,
		esVec3 eye, esVec3 at, esVec3 up);

// Texture
enum esTextureMipmap {
	TEX_NONE,
	TEX_LINEAR,
};

typedef struct {
	int w, h;
	int gltexture;
} esTexture;

int esTextureLoad(esTexture *tex, const char *file_name,
		enum esTextureMipmap min, enum esTextureMipmap mag);
void esTextureUse(esTexture *tex);
void esTextureUnload(esTexture *tex);

// Font
typedef struct {
	esTexture *texture;
	esShader *shader;

	int vert_count;
	int buf_size, buf_alloc;
	esGeoBuf geo_buf;
	esGeo geo;
	void *buf;
} esFont;

int esFontCreate(esFont *ft, esTexture *tex, esShader *shad,
		int attrib_loc, int attrib_uv, int addition_attribs);
void esFontDelete(esFont *ft);
void esFontAddText(esFont *ft, float offset_x, float offset_y,
		const char *fmt, ...);
void esFontRender(esFont *ft);
void esFontClearBuf(esFont *ft);

// Framebuffer
typedef struct {
	int dimension;
	int gl_fb, gl_tex, gl_depth;
} esFrameBuffer;

int esFrameBufferCreate(esFrameBuffer *fb, int dimension,
		enum esTextureMipmap min, enum esTextureMipmap mag);
void esFrameBufferDelete(esFrameBuffer *fb);
void esFrameBufferSet(esFrameBuffer *fb);
void esFrameBufferUnSet(void);
void esFrameBufferBind(esFrameBuffer *fb);

// Audio
typedef struct {
	Mix_Chunk *chunk;
} esSound;

int esSoundLoad(esSound *sn, const char *file_name);
void esSoundUnLoad(esSound *sn);
void esSoundPlay(esSound *sn);

typedef struct {
	Mix_Music *music;
} esMusic;

int esMusicLoad(esMusic *mu, const char *file_name);
void esMusicUnLoad(esMusic *mu);
void esMusicPlay(esMusic *mu);
void esMusicHalt(void);

#endif

