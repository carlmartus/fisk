// Top {{{
#include "estk.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <GL/glew.h>
#include <SDL/SDL.h>
#ifdef EMSCRIPTEN
#include <emscripten/emscripten.h>
#endif

#include <SDL/SDL_image.h>

static int window_w, window_h;


static const GLenum mipmap_map[] = {
	[TEX_NONE] = GL_NEAREST,
	[TEX_LINEAR] = GL_LINEAR,
};

static void
_check_error(const char *file, int line)
{
	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		printf("OpenGL Error (%s:%d): %d ", file, line, err);

		switch (err) {
#define ERR_CASE_PRINT(cond) case cond : printf("%s\n", #cond); break
			ERR_CASE_PRINT(GL_INVALID_ENUM);
			ERR_CASE_PRINT(GL_INVALID_VALUE);
			ERR_CASE_PRINT(GL_INVALID_OPERATION);
			ERR_CASE_PRINT(GL_STACK_OVERFLOW);
			ERR_CASE_PRINT(GL_STACK_UNDERFLOW);
			ERR_CASE_PRINT(GL_OUT_OF_MEMORY);
			default : break;
		}
	}
}
#define check_error() _check_error(__FILE__, __LINE__)

// }}}
// Misc {{{
void
_esCheckGlError(const char *file, int line)
{
	_check_error(file, line);
}


// }}}
// Game loop {{{

#define MAX_KEYS 2000

static int loop_run;
static int key_regs = 0;
static struct {
	void (*callback) (int key, int down);
} keys[MAX_KEYS];

void
esGameInit(int screen_width, int screen_height)
{
	window_w = screen_width;
	window_h = screen_height;

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	SDL_SetVideoMode(screen_width, screen_height, 0, SDL_OPENGL);

	if (Mix_OpenAudio(22050, AUDIO_S16, 1, 4096)) {
		printf("Cannot open audio\n");
	}

	glewInit();
}

static void
event_key(int sdlkey, int down)
{
	if (keys[sdlkey].callback) {
		keys[sdlkey].callback(sdlkey, down);
	}
}

static void
events(void)
{
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT : loop_run = 0; break;

			case SDL_KEYDOWN :	event_key(event.key.keysym.sym, 1); break;
			case SDL_KEYUP :	event_key(event.key.keysym.sym, 0); break;
		}
	}
}

void
esGameGlSwap(void)
{
	SDL_GL_SwapBuffers();
}

#ifdef EMSCRIPTEN
static Uint32 emscripten_last_frame;
static void (*emscripten_frame)(float t);
static void (*emscripten_exit)(void);

static void
emscripten_mainloop(void)
{
	if (!loop_run) {
		esMusicHalt();
		if (emscripten_exit) emscripten_exit();
		emscripten_cancel_main_loop();
		return;
	}

	Uint32 current = SDL_GetTicks();
	float pass = 0.001f * (float) (current - emscripten_last_frame);
	emscripten_last_frame = current;

	events();
	if (!loop_run) return;
	emscripten_frame(pass);
}

#endif

void
esGameLoop(void (*frame)(float t), void (*exit)(), int frame_rate)
{
	loop_run = 1;

#ifdef EMSCRIPTEN
	emscripten_frame = frame;
	emscripten_exit = exit;

	emscripten_last_frame = SDL_GetTicks();
	emscripten_set_main_loop(emscripten_mainloop, frame_rate, 0);
#else

	if (frame_rate == 0) frame_rate = 60;
	int loop_delay = 1000 / frame_rate;

	float pass = 0.1f;
	Uint32 start = SDL_GetTicks();
	while (1) {
		events();
		if (!loop_run) break;
		frame(pass);
		if (!loop_run) break;

		Uint32 diff = SDL_GetTicks() - start;

		if (diff < loop_delay) {
			SDL_Delay(loop_delay - diff);
			pass = (float) loop_delay * 0.001f;
		} else {
			pass = (float) diff * 0.001f;
		}

		start = SDL_GetTicks();
	}

	esMusicHalt();
	if (exit) exit();
#endif
}

void
esGameLoopQuit(void)
{
	loop_run = 0;
}

void
esGameRegisterKey(int sdlkey, void (*callback)(int key, int down))
{
	if (key_regs == 0) {
		memset(keys, 0, sizeof(keys));
	}
	key_regs++;

	keys[sdlkey].callback = callback;
}

// }}}
// File {{{

static void*
file_alloc(const char *file_name)
{
	FILE *fd = fopen(file_name, "r");
	if (fd == NULL) return NULL;
	struct stat st;

	stat(file_name, &st);

	char *buf = malloc(st.st_size + 1);
	fread(buf, st.st_size, 1, fd);
	fclose(fd);

	buf[st.st_size] = '\0';

	//printf("Loaded %s (%d b)\n%s", file_name, (int) st.st_size, buf);
	return buf;
}

// }}}
// Shader {{{

static int
check_shader(GLuint id, const char *shader_info)
{
	GLint result = GL_FALSE;

	glGetShaderiv(id, GL_COMPILE_STATUS, &result);

	if (result != GL_TRUE) {
		char info_buf[500];
		glGetShaderInfoLog(id, sizeof(info_buf)-1, NULL, info_buf);
		printf("Shader info (%s):\n%s\n", shader_info, info_buf);
		check_error();
		return 1;
	}

	return 0;
}

int
shader_load(const char *file_name, GLenum shader_type, const char *shader_info)
{
	char *content = file_alloc(file_name);
	if (content == NULL) {
		return 0;
	}

	int shad = glCreateShader(shader_type);
	glShaderSource(shad, 1, (const char**) &content , NULL);
	glCompileShader(shad);
	check_error();
	free(content);

	if (check_shader(shad, shader_info)) return 0;

	return shad;
}

int
esShaderLoad(esShader *shader, const char *vert_file, const char *frag_file)
{
	int idvert = shader_load(vert_file, GL_VERTEX_SHADER, "Vertex shader");
	if (idvert == 0) {
		printf("Invalid vertex shader file (%s)\n", vert_file);
		return 1;
	}

	int idfrag = shader_load(frag_file, GL_FRAGMENT_SHADER, "Fragment shader");
	if (idfrag == 0) {
		printf("Invalid fragment shader file (%s)\n", frag_file);
		return 1;
	}

	int program = glCreateProgram();

	glAttachShader(program, idvert);
	check_error();
	glAttachShader(program, idfrag);
	check_error();

	glLinkProgram(program);
	check_error();

	glDeleteShader(idvert);
	glDeleteShader(idfrag);

	shader->glprogram = program;
	return 0;
}

void
esShaderUse(const esShader *shader)
{
	glUseProgram(shader->glprogram);
}

void
esShaderUnload(esShader *shader)
{
	glDeleteShader(shader->glprogram);
}

int
esShaderUniformRegister(esShader *shader, esUniform reg, const char *name)
{
	int loc = glGetUniformLocation(shader->glprogram, name);
	if (loc < 0) return 1;

	shader->uniforms[reg] = loc;
	return 0;
}

int
esShaderUniformGl(esShader *shader, esUniform reg)
{
	return shader->uniforms[reg];
}

// }}}
// Geometry buf {{{

void
esGeoBufCreate(esGeoBuf *buf)
{
	glGenBuffers(1, &buf->glbuf);
	check_error();
}

void
esGeoBufCopy(esGeoBuf *buf, const void *data, size_t size, enum esGeoBufType type)
{
	static const unsigned int map[] = {
		[GEOBUF_STATIC] = GL_STATIC_DRAW,
		[GEOBUF_DYNAMIC] = GL_DYNAMIC_DRAW,
		[GEOBUF_STREAM] = GL_STREAM_DRAW,
	};

	glBindBuffer(GL_ARRAY_BUFFER, buf->glbuf);
	check_error();
	glBufferData(GL_ARRAY_BUFFER, size, data, map[type]);
	check_error();
}

void
esGeoBufDelete(esGeoBuf *buf)
{
	glDeleteBuffers(1, &buf->glbuf);
	check_error();
}

// }}}
// Geometry {{{

void
esGeoReset(esGeo *geo, int bufcount)
{
	geo->bufcount = bufcount;
}

void
esGeoPoint(esGeo *geo, int id, esGeoBuf *geobuf,
		enum esGeoDataType datatype, int elements,
		size_t offset, size_t stride, enum esBool normalized)
{
	geo->buf[id].geobuf = geobuf;
	geo->buf[id].datatype = datatype;
	geo->buf[id].elements = elements;
	geo->buf[id].offset = offset;
	geo->buf[id].stride = stride;
	geo->buf[id].normalized = normalized;
}

void
esGeoRender(const esGeo *geo, int vertices)
{
	static const unsigned int map[] = {
		[GEODATA_FLOAT] = GL_FLOAT,
		[GEODATA_INT] = GL_INT,
		[GEODATA_BYTE] = GL_BYTE,
		[GEODATA_UBYTE] = GL_UNSIGNED_BYTE,
	};

	int bufcount = geo->bufcount;
	const esGeoBuf *last = NULL;

	int i;
	for (i=0; i<bufcount; i++) {
		glEnableVertexAttribArray(i);

		if (last != geo->buf[i].geobuf) {
			last = geo->buf[i].geobuf;
			glBindBuffer(GL_ARRAY_BUFFER, last->glbuf);
		}

		glVertexAttribPointer(i,
				geo->buf[i].elements,
				map[geo->buf[i].datatype],
				geo->buf[i].normalized == ES_TRUE ? GL_TRUE : GL_FALSE,
				geo->buf[i].stride,
				(void*) geo->buf[i].offset);
		check_error();
	}

	glDrawArrays(GL_TRIANGLES, 0, vertices);
	check_error();

	for (i=0; i<bufcount; i++) glDisableVertexAttribArray(i);
	check_error();
}

// }}}
// Projection {{{

#define P0 (0.0f)
#define P1 (1.0f)

void
esProjOrtho(float *mat, float x0, float y0, float x1, float y1)
{
	mat[ 1]=P0; mat[ 2]=P0;
	mat[ 4]=P0; mat[ 6]=P0;
	mat[ 8]=P0; mat[ 9]=P0;
	mat[12]=P0; mat[13]=P0; mat[14] = P0;

	mat[ 0] = 2.0f / (x1-x0);
	mat[ 5] = 2.0f / (y1-y0);
	mat[10] = -1.0f;
	mat[15] = P1;

	mat[12] = -(x1+x0)/(x1-x0);
	mat[13] = -(y1+y0)/(y1-y0);
	mat[14] = 0.0f;
}

void
identity_matrix(float *mat)
{
	mat[ 0]=P1; mat[ 4]=P0; mat[ 8]=P0; mat[12]=P0;
	mat[ 1]=P0; mat[ 5]=P1; mat[ 9]=P0; mat[13]=P0;
	mat[ 2]=P0; mat[ 6]=P0; mat[10]=P1; mat[14]=P0;
	mat[ 3]=P0; mat[ 7]=P0; mat[11]=P0; mat[15]=P1;
}

static void
perspective_matrix(float *mat, float fov, float screenratio, float near, float far)
{
	float size = near * tanf(fov * 0.5); 
	float left = -size;
	float right = size;
	float bottom = -size / screenratio;
	float top = size / screenratio;

	mat[0] = 2 * near / (right - left);
	mat[1] = 0.0;
	mat[2] = 0.0;
	mat[3] = 0.0;

	mat[4] = 0.0;
	mat[5] = 2 * near / (top - bottom);
	mat[6] = 0.0;
	mat[7] = 0.0;

	mat[8] = (right + left) / (right - left);
	mat[9] = (top + bottom) / (top - bottom);
	mat[10] = -(far + near) / (far - near);
	mat[11] = -1;

	mat[12] = 0.0;
	mat[13] = 0.0;
	mat[14] = -(2 * far * near) / (far - near);
	mat[15] = 0.0;
}


static esVec3
cross(esVec3 a, esVec3 b)
{
	return (esVec3) {
		a.y*b.z - a.z*b.y,
		a.z*b.x - a.x*b.z,
		a.x*b.y - a.y*b.x,
	};
}

static void
normalize(esVec3 *v)
{
    float r;

    r = sqrtf(v->x*v->x + v->y*v->y + v->z*v->z);
    if (r == 0.0f) return;

	r = 1.0f / r;
    v->x *= r;
    v->y *= r;
    v->z *= r;
}

/*
static float
dot(esVec3 a, esVec3 b)
{
	return a.x*b.x + a.y*b.y + a.z*b.z;
}*/

void
mul_matrix(float *res, float *a, float *b)
{
	int x, y, i=0;
	for (y=0; y<4; y++) {
		for (x=0; x<4; x++) {

			int r = y<<2;
			res[i] =
				a[r+ 0]*b[x+ 0] +
				a[r+ 1]*b[x+ 4] +
				a[r+ 2]*b[x+ 8] +
				a[r+ 3]*b[x+12];

			i++;
		}
	}
}

void
lookat_matrix(float *mat, esVec3 eye, esVec3 at, esVec3 up)
{
	esVec3 forw = {
		at.x - eye.x,
		at.y - eye.y,
		at.z - eye.z,
	};

	normalize(&forw);
	esVec3 side = cross(up, forw);
	normalize(&side);

	up = cross(forw, side);

	float m0[16];
	identity_matrix(m0);

	m0[ 0] = side.x;
	m0[ 4] = side.y;
	m0[ 8] = side.z;

	m0[ 1] = up.x;
	m0[ 5] = up.y;
	m0[ 9] = up.z;

	m0[ 2] = -forw.x;
	m0[ 6] = -forw.y;
	m0[10] = -forw.z;

	float m1[16];
	identity_matrix(m1);

	m1[12] = -eye.x;
	m1[13] = -eye.y;
	m1[14] = -eye.z;

	mul_matrix(mat, m1, m0);
}

void
esProjPerspective(
		float *mat, float fov, float screenratio, float near, float far,
		esVec3 eye, esVec3 at, esVec3 up)
{
	float persp[16];
	perspective_matrix(persp, fov, screenratio, near, far);

	float look[16];
	lookat_matrix(look, eye, at, up);

	mul_matrix(mat, look, persp);
}

// }}}
// Texture {{{

int
esTextureLoad(esTexture *tex, const char *file_name,
		enum esTextureMipmap min, enum esTextureMipmap mag)
{
	SDL_Surface *surf = IMG_Load(file_name);
	if (surf == NULL) return 1;

	tex->w = surf->w;
	tex->h = surf->h;

	GLuint gltex;
	glGenTextures(1, &gltex);
	tex->gltexture = gltex;
	glBindTexture(GL_TEXTURE_2D, tex->gltexture);

	int mode = surf->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB;

	glTexImage2D(GL_TEXTURE_2D, 0, mode, surf->w, surf->h,
			0, mode, GL_UNSIGNED_BYTE, surf->pixels);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mipmap_map[min]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mipmap_map[mag]);

	SDL_FreeSurface(surf);
	return 0;
}

void
esTextureUse(esTexture *tex)
{
	glBindTexture(GL_TEXTURE_2D, tex->gltexture);
}

void
esTextureUnload(esTexture *tex)
{
	GLuint gltex = tex->gltexture;
	glDeleteTextures(1, &gltex);
}

// }}}
// Font {{{

#define MAX_STRING 512
#define FONT_INIT 4096

struct font_vert {
	float x, y;
	float u, v;
};

int
esFontCreate(esFont *ft, esTexture *tex, esShader *shad,
		int attrib_loc, int attrib_uv, int addition_attribs)
{
	ft->texture = tex;
	ft->shader = shad;

	ft->vert_count = 0;

	ft->buf = malloc(FONT_INIT*sizeof(struct font_vert)*6);
	ft->buf_size = 0;
	ft->buf_alloc = FONT_INIT;

	esGeoBufCreate(&ft->geo_buf);

	esGeoReset(&ft->geo, 2 + addition_attribs);
	esGeoPoint(&ft->geo, 0, &ft->geo_buf, GEODATA_FLOAT, 2, 0,
			sizeof(struct font_vert), ES_FALSE);
	esGeoPoint(&ft->geo, 1, &ft->geo_buf, GEODATA_FLOAT, 2,
			offsetof(struct font_vert, u),
			sizeof(struct font_vert), ES_TRUE);

	return 1;
}

static void
buf_check(esFont *ft, int add)
{
	int required = ft->buf_size + add;

	if (required < ft->buf_alloc) return;

	while (ft->buf_alloc < required) {
		ft->buf_alloc <<= 1;
	}
	ft->buf = realloc(ft->buf, ft->buf_alloc);
}

void
esFontAddText(esFont *ft,
		float offset_x, float offset_y, const char *fmt, ...)
{
	va_list args;
	char buf[MAX_STRING];

	va_start(args, fmt);
	vsnprintf(buf, MAX_STRING, fmt, args);
	va_end(args);

	int len = strlen(buf);
	int size = sizeof(struct font_vert)*6*len;
	buf_check(ft, size);

	struct font_vert *itr = ft->buf + ft->buf_size;
	int i;
	for (i=0; i<len; i++) {

		char ch = buf[i];
		int x = ch & 0xf;
		int y = ch >> 4;

#define FONT_UV_INC (1.0f / 16.0f)

		float u0 = FONT_UV_INC * (float) x;
		float v0 = FONT_UV_INC * (float) y;
		float u1 = u0 + FONT_UV_INC;
		float v1 = v0 + FONT_UV_INC;

#define FONT_PUSH(a, b, c, d)\
		itr->x = offset_x + a;\
		itr->y = offset_y + b;\
		itr->u = c;\
		itr->v = d;\
		itr++;

		FONT_PUSH(0.0f, 0.0f, u0, v0);
		FONT_PUSH(1.0f, 0.0f, u1, v0);
		FONT_PUSH(0.0f, 1.0f, u0, v1);

		FONT_PUSH(1.0f, 0.0f, u1, v0);
		FONT_PUSH(1.0f, 1.0f, u1, v1);
		FONT_PUSH(0.0f, 1.0f, u0, v1);

		ft->vert_count += 6;
		offset_x += 1.0f;
	}

	ft->buf_size += size;
}

void
esFontDelete(esFont *ft)
{
	free(ft->buf);
	ft->buf = NULL;

	esGeoBufDelete(&ft->geo_buf);
}

void
esFontRender(esFont *ft)
{
	if (ft->shader) {
		esShaderUse(ft->shader);
	}

	if (ft->vert_count > 0) {

		esGeoBufCopy(&ft->geo_buf, ft->buf, ft->buf_size, GEOBUF_STREAM);
		esGeoRender(&ft->geo, ft->vert_count);
	}
}

void
esFontClearBuf(esFont *ft)
{
	ft->vert_count = 0;
}

// }}}
// Framebuffer {{{

int
esFrameBufferCreate(esFrameBuffer *fb, int dimension,
		enum esTextureMipmap min, enum esTextureMipmap mag)
{
	GLuint glfb, gltex, gldepth;
	glGenFramebuffers(1, &glfb);
	glGenTextures(1, &gltex);

	fb->dimension = dimension;
	fb->gl_fb = glfb;
	fb->gl_tex = gltex;

	// Texture
	glBindTexture(GL_TEXTURE_2D, fb->gl_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, dimension, dimension,
			0, GL_RGB, GL_UNSIGNED_BYTE, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mipmap_map[min]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mipmap_map[mag]);
	check_error();

	// Depth buffer
	glGenRenderbuffers(1, &gldepth);
	glBindRenderbuffer(GL_RENDERBUFFER, gldepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16,
			dimension, dimension);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	check_error();

	glBindFramebuffer(GL_FRAMEBUFFER, fb->gl_fb);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER,
			GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, gldepth);
	check_error();
	fb->gl_depth = gldepth;

	// Color channel
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_2D, gltex, 0);
	check_error();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return 0;
}

void
esFrameBufferDelete(esFrameBuffer *fb)
{
	GLuint glfb, gltex, gldepth;
	glfb = fb->gl_fb;
	gltex = fb->gl_tex;
	gldepth = fb->gl_depth;

	glDeleteFramebuffers(1, &glfb);
	glDeleteTextures(1, &gltex);
	glDeleteRenderbuffers(1, &gldepth);
}

void
esFrameBufferSet(esFrameBuffer *fb)
{
	glViewport(0, 0, fb->dimension, fb->dimension);
	glBindFramebuffer(GL_FRAMEBUFFER, fb->gl_fb);
}

void
esFrameBufferUnSet(void)
{
	glViewport(0, 0, window_w, window_h);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void
esFrameBufferBind(esFrameBuffer *fb)
{
	glBindTexture(GL_TEXTURE_2D, fb->gl_tex);
}

// }}}
// Sound {{{

int
esSoundLoad(esSound *sn, const char *file_name)
{
	sn->chunk = Mix_LoadWAV(file_name);
	return sn->chunk == 0;
}

void
esSoundUnLoad(esSound *sn)
{
	Mix_FreeChunk(sn->chunk);
	sn->chunk = NULL;
}

void
esSoundPlay(esSound *sn)
{
	Mix_PlayChannel(-1, sn->chunk, 0);
}

int
esMusicLoad(esMusic *mu, const char *file_name)
{
	mu->music = Mix_LoadMUS(file_name);
	return mu->music == 0;
}

void
esMusicUnLoad(esMusic *mu)
{
	Mix_FreeMusic(mu->music);
	mu->music = NULL;
}

void
esMusicPlay(esMusic *mu)
{
	Mix_PlayMusic(mu->music, -1);
}

void
esMusicHalt(void)
{
	Mix_HaltMusic();
}

// }}}

