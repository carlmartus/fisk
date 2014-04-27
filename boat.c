#include "fisk.h"
#include <SDL/SDL.h>

#define ZOOM 0.02f

#define BOAT_UP 0.7f
#define STATE_UNDER 1
#define GRAVITY 9.82f
#define FLOATING (GRAVITY*3.0f)
#define FLOATING_ADD 4.0f
#define MOVEX 8.0f
#define MAXSPEED 5.0f

#define RADIUS 0.2f

#define ROPE_HAUL 3.0f
#define ROPE_MINLEN 1.25f
#define ROPE_MAXLEN 32.0f

static float x, y;
static float dx, dy;
static float climb_y;
static esVec2 climb_vec;
static float mvp[16];
static unsigned states;
static unsigned int ctrl_left, ctrl_right, ctrl_down, ctrl_up;

static esVec2 cam;

static float hook_rope;
static esVec2 hook_loc, hook_dir;

static void key_left(int sdlkey, int down)	{ ctrl_left = down; }
static void key_right(int sdlkey, int down)	{ ctrl_right = down; }
static void key_up(int sdlkey, int down)	{ ctrl_up = down; }
static void key_down(int sdlkey, int down)	{ ctrl_down = down; }

int
boatSetup()
{
	esGameRegisterKey(SDLK_LEFT, key_left);
	esGameRegisterKey(SDLK_RIGHT, key_right);
	esGameRegisterKey(SDLK_UP, key_up);
	esGameRegisterKey(SDLK_DOWN, key_down);

	states = 0;
	ctrl_left = 0;
	ctrl_right = 0;
	ctrl_up = 0;
	ctrl_down = 0;

	climb_y = 0.0f;
	climb_vec.x = 1.0f;
	climb_vec.y = 0.0f;

	hook_rope = ROPE_MINLEN;

	x = 0.0f;
	y = -1.0f;
	dx = 0.0f;
	dy = 0.0f;

	cam.x = x;
	cam.y = y;

	return 0;
}

static void
move_boat(float fr)
{
	float lx = 0.0f, ly = 0.0f;
	if (ctrl_left)	lx -= 1.0f;
	if (ctrl_right)	lx += 1.0f;
	if (ctrl_up)	ly -= 1.0f;
	if (ctrl_down)	ly += 1.0f;

	float wm = seaWaveHeight(x);
	float w0 = seaWaveHeight(x - RADIUS);
	float w1 = seaWaveHeight(x + RADIUS);
	float dm = y - wm;

	if (y+BOAT_UP > wm) states |= STATE_UNDER;
	else states &= ~STATE_UNDER;

	if (states & STATE_UNDER) {
		dx = commonTowardsFloat(dx, lx*MAXSPEED, fr*MOVEX);

		float climb = (wm - w0) + (w1 - wm);
		climb_vec.x = 0.5f;
		climb_y = commonTowardsFloat(climb_y, climb, fr*0.5f);
		climb_vec.y = climb_y;
		climb_vec = commonNormalizeVec2(climb_vec);

		dx += climb;
	}

	if (states & STATE_UNDER) {
		dy = commonTowardsFloat(dy, -dm-FLOATING_ADD, FLOATING*fr);
	} else {
		dy += GRAVITY*fr;
	}

	// Rope
	if (ly != 0.0f) {
		if (ly > 0.0f && hook_rope < ROPE_MAXLEN) {

			if (hook_rope == ROPE_MINLEN) {
				hook_dir.x = dx;
				hook_dir.y = 0.5f;
			}

			hook_rope += ROPE_HAUL*fr;

		} else if (ly < 0.0f && hook_rope > ROPE_MINLEN) {
			hook_rope -= ROPE_HAUL*fr;
			if (hook_rope < ROPE_MINLEN) hook_rope = ROPE_MINLEN;
		}
	}

	if (hook_rope > ROPE_MINLEN) {
		float dragspeed = 30.0f + 160.0f / hook_rope;

		hook_dir.x = commonTowardsFloat(
				hook_dir.x,
				(x - hook_loc.x)*2.0f,
				dragspeed*fr);

		hook_dir.y = commonTowardsFloat(
				hook_dir.y,
				(y+hook_rope) - hook_loc.y, 
				dragspeed*fr);

		hook_loc.x += hook_dir.x*fr;
		hook_loc.y += hook_dir.y*fr;

	} else {
		hook_loc.x = x;
		hook_loc.y = y + ROPE_MINLEN;
	}

	x += dx*fr;
	y += dy*fr;

	esVec2 cam_dst = {
		hook_loc.x,
		hook_loc.y,
	};
	cam = commonTowardsVec2(cam, cam_dst, 100.0f*fr);
}

void
boatFrame(float fr)
{
	move_boat(fr);

	esProjOrtho(mvp,
			cam.x - ZOOM * (float) WINW,
			cam.y + ZOOM * (float) WINH,
			cam.x + ZOOM * (float) WINW,
			cam.y - ZOOM * (float) WINH);
}

void
boatRender(void)
{
	if (hook_rope != ROPE_MINLEN) {
		esVec2 dist = { x - hook_loc.x, y - hook_loc.y };
		dist = commonNormalizeVec2(dist);

		esVec2 trans = {
			-dist.y * 0.3f,
			 dist.x * 0.3f,
		};
		spriteAdd(SPRITE_HOOK, hook_loc.x, hook_loc.y, trans);
	} else {
		esVec2 trans = { climb_vec.x*0.35f, 0.0f };
		spriteAdd(SPRITE_HOOK,x, y+ROPE_MINLEN, trans);
	}

	spriteAdd(SPRITE_SHIP, x, y, climb_vec);
}

esVec2
boatGetPosition(void)
{
	return (esVec2) { x, y };
}

const float*
boatMvp(void)
{
	return mvp;
}

