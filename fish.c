#include "fisk.h"
#include <math.h>
#include <string.h>

#define LIMIT 30.0f
#define SPAWN_BLOCKS 3.0f
#define MIN_FISHES 5

static float centre;
static int framecount = 0;
static int generation = 0;

#define MAX_FISH 100
static struct fish {
	enum fishType type;
	float x, y, dx, dy;
	unsigned age;
} fishes[MAX_FISH];

static const struct info {
	enum spriteId sprite;
	float speed;
	float radius;
	float bestlevel;
} infos[] = {
	[FISH_SMALL] = {
		.sprite = SPRITE_FISH_SMALL,
		.radius = 0.5f,
		.speed = 1.0f,
		.bestlevel = 8.0f,
	},
	[FISH_FLYING] = {
		.sprite = SPRITE_FISH_FLYING,
		.radius = 0.5f,
		.speed = 4.0f,
		.bestlevel = 4.0f,
	},
	[FISH_BIKE] = {
		.sprite = SPRITE_FISH_BIKE,
		.radius = 1.0f,
		.speed = 0.0f,
		.bestlevel = 12.0f,
	},
	[FISH_SUBMARINE] = {
		.sprite = SPRITE_FISH_SUBMARINE,
		.radius = 3.0f,
		.speed = 0.0f,
		.bestlevel = 12.0f,
	},
};

static int weight_bike = 0;
static int weight_flying = 0;
static int weight_submarine = 0;
static int weight_all = 20;

static int
get_free_id(void)
{
	const struct fish *itr, *end;
	itr = fishes;
	end = itr + MAX_FISH;

	int i = 0;
	while (itr < end) {
		if (itr->type == FISH_NONE) return i;
		itr++;
		i++;
	}
	return 0;
}

static enum fishType
get_random_type(void)
{
	int r = framecount % weight_all;
	if (r < weight_submarine)	return FISH_SUBMARINE;
	if (r < weight_flying)		return FISH_FLYING;
	if (r < weight_bike)		return FISH_BIKE;
	return FISH_SMALL;
}

static void
spawn_something(float x)
{
	float dy = 0.4f * ((float) (generation % 6) - 2.0f);

	enum fishType id = get_random_type();
	printf("Spawn %d\n", id);

	struct fish f = {
		.type = id,
		.x = x,
		.y = infos[FISH_SMALL].bestlevel,
		.dx = 0.2f,
		.dy = dy,
		.age = 0,
	};

	fishes[get_free_id()] = f;
}

int
fishSetup(void)
{
	centre = 0.0f;
	memset(fishes, 0, sizeof(fishes));

	spawn_something(0.0f);
	spawn_something(1.0f);
	return 0;
}

static int
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

	return count;
}

static void
move_world(float x)
{
	int count = respawn_outside();

	if (count < MIN_FISHES || (count < MAX_FISH && (framecount & 3) == 0)) {
		int old_cell = (int) ((1.0f / SPAWN_BLOCKS) * centre);
		int new_cell = (int) ((1.0f / SPAWN_BLOCKS) * x);

		if (old_cell != new_cell) {

			if (new_cell < old_cell) {
				spawn_something(x - LIMIT + SPAWN_BLOCKS);
			} else if (new_cell > old_cell) {
				spawn_something(x + LIMIT - SPAWN_BLOCKS);
			}
		}
	}

	centre = x;
}

static void
swim(struct fish *fish, float fr)
{
	if ((fish->age & 7) == 0) {
		/*
		fish->dy = commonTowardsFloat(fish->dy,
				infos[fish->type].bestlevel - fish->y, fr);*/
		fish->dy = commonTowardsFloat(fish->dy, 0.0f, fr);
	}

	fish->age++;
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
			swim(itr, fr);
		}
		itr++;
	}
}

void
fishMoveFrame(float fr, float x)
{
	if ((framecount++ & 7) == 0) {
		generation++;
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

void
fishRenderHooked(enum fishType type, float x, float y, esVec2 trans)
{
	trans.x *= infos[type].radius;
	trans.y *= infos[type].radius;

	x -= trans.x;
	y -= trans.y;

	spriteAdd(infos[type].sprite, x, y, trans);
}

static int
is_within(float ax, float ay, float bx, float by, float limit)
{
	float cubex = fabsf(ax - bx);
	float cubey = fabsf(ay - by);

	return cubex < limit && cubey < limit;
}

enum fishType
fishHook(float x, float y)
{
	struct fish *itr, *end;
	itr = fishes;
	end = fishes + MAX_FISH;

	while (itr < end) {
		if (itr->type != FISH_NONE) {
			if (is_within(x, y, itr->x, itr->y, infos[itr->type].radius)) {
				enum fishType type = itr->type;
				itr->type = FISH_NONE;
				return type;
			}
		}

		itr++;
	}

	return FISH_NONE;
}

void
fishEnableSpawn(enum fishType type)
{
	switch (type) {
		case FISH_SUBMARINE :
			weight_submarine = 100;
			weight_all += weight_submarine;
			break;

		case FISH_BIKE :
			weight_bike = 8;
			weight_all += weight_bike;
			break;

		case FISH_FLYING :
			weight_flying = 14;
			weight_all += weight_flying;
			break;

		default : break;
	}
}

