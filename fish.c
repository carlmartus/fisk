#include "fisk.h"
#include <math.h>
#include <string.h>

#define LIMIT 30.0f

static float centre;
static int framecount = 0;

enum fishType {
	FISH_NONE=0,
	FISH_SMALL,
};

#define MAX_FISH 100
static struct fish {
	enum fishType type;
	float x, y, dx, dy;
	unsigned age;
} fishes[MAX_FISH];

static const struct info {
	enum spriteId sprite;
	float radius;
	float bestlevel;
} infos[] = {
	[FISH_SMALL] = {
		.sprite = SPRITE_FISH_SMALL,
		.radius = 0.5f,
		.bestlevel = 8.0f,
	},
};

int
fishSetup(void)
{
	centre = 0.0f;
	memset(fishes, 0, sizeof(fishes));

	fishes[0].type = FISH_SMALL;
	fishes[0].x = 0.0f;
	fishes[0].y = 8.0f;
	fishes[0].dx = 1.3f;
	fishes[0].dy = -2.1f;
	return 0;
}

static void
respawn_outside(void)
{
	float lo = centre - LIMIT;
	float hi = centre + LIMIT;

	struct fish *itr, *end;
	itr = fishes;
	end = fishes + MAX_FISH;

	int count = 0;
	while (itr < end) {
		if (itr->type != FISH_NONE) {

			if (itr->x < lo || itr->x > hi) {
				itr->type = FISH_NONE;
			} else {
				count++;
			}
		}

		itr++;
	}
}

static void
move_world(float x)
{
	centre = x;
	respawn_outside();
}

static void
move_fish(struct fish *fish, float fr)
{
	fish->x += fish->dx*fr;
	fish->y += fish->dy*fr;
}

static void
move_fishes(float fr)
{
	struct fish *itr, *end;
	itr = fishes;
	end = itr + MAX_FISH;

	while (itr < end) {
		if (itr->type != FISH_NONE) {
			move_fish(itr, fr);
		}
		itr++;
	}
}

void
fishMoveFrame(float fr, float x)
{
	if ((framecount++ & 7) == 0) {
		move_world(x);
	}

	move_fishes(fr);
}

static esVec2
fish_trans(float dx, float dy, float radius)
{
	esVec2 ret;
	if (dx == 0.0f && dy == 0.0f) {
		ret.x = radius;
		ret.y = 0.0f;
		return ret;
	}

	float len = sqrtf(dx*dx + dy*dy);
	len = 1.0f / len;

	ret.x = dx*len*radius;
	ret.y = dy*len*radius;
	return ret;
}

void
fishRender(void)
{
	const struct fish *itr, *end;
	itr = fishes;
	end = fishes + MAX_FISH;

	while (itr < end) {
		if (itr->type != FISH_NONE) {

			esVec2 trans = fish_trans(itr->dx, itr->dy, infos[itr->type].radius);
			spriteAdd(infos[itr->type].sprite, itr->x, itr->y, trans);
		}

		itr++;
	}
}

