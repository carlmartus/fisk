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

static float x, y;
static float dx, dy;
static float climb_y;
static esVec2 climb_vec;
static float mvp[16];
static unsigned states;
static unsigned int ctrl_left, ctrl_right;

static void key_left(int sdlkey, int down)	{ ctrl_left = down; }
static void key_right(int sdlkey, int down)	{ ctrl_right = down; }

int
boatSetup()
{
	esGameRegisterKey(SDLK_LEFT, key_left);
	esGameRegisterKey(SDLK_RIGHT, key_right);

	states = 0;

	climb_y = 0.0f;
	climb_vec.x = 1.0f;
	climb_vec.y = 0.0f;

	x = 0.0f;
	y = -1.0f;
	dx = 0.0f;
	dy = 0.0f;

	return 0;
}

static void
move_boat(float fr)
{
	float lx = 0.0f;
	if (ctrl_left)	lx -= 1.0f;
	if (ctrl_right)	lx += 1.0f;

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

	x += dx*fr;
	y += dy*fr;
}

void
boatFrame(float fr)
{
	move_boat(fr);

	esProjOrtho(mvp,
			x - ZOOM * (float) WINW,
			y + ZOOM * (float) WINH,
			x + ZOOM * (float) WINW,
			y - ZOOM * (float) WINH);
}

void
boatRender(void)
{
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

