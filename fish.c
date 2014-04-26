#include "fisk.h"
#include <math.h>
#include <string.h>

static float last_x;

enum fishType {
	FISH_NONE=0,
	FISH_SMALL,
};

#define MAX_FISH 100
static struct fish {
	enum fishType type;
	float x, y, dx, dy;
} fishes[MAX_FISH];

int
fishSetup(void)
{
	last_x = 0.0f;
	memset(fishes, 0, sizeof(fishes));

	fishes[0].type = FISH_SMALL;
	fishes[0].x = 0.0f;
	fishes[0].y = 8.0f;
	fishes[0].dx = 0.0f;
	fishes[0].dy = 0.0f;
	return 0;
}

static void
respawn_outside(void)
{
}

static void
move_world(float x)
{
	last_x = x;
	respawn_outside();
}

void
fishMoveFrame(float fr, float x)
{
	move_world(x);
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

			static const enum spriteId sprite_map[] = {
				[FISH_SMALL] = SPRITE_FISH_SMALL,
			};

			static const float radius_map[] = {
				[FISH_SMALL] = 0.5f,
			};

			spriteAdd(sprite_map[itr->type], itr->x, itr->y,
					fish_trans(itr->dx, itr->dy, radius_map[itr->type]));
		}

		itr++;
	}
}

