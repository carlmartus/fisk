#include "fisk.h"

#define FISH_COUNT 4

static int challenge;
static int kgs, kgs_req;
static int reqs[FISH_COUNT];
static int catches[FISH_COUNT];

static void
reset(int *list)
{
	int i;
	for (i=0; i<FISH_COUNT; i++) {
		*list++ = 0;
	}
}

int
scoreSetup(void)
{
	challenge = 0;
	kgs = 0;
	kgs_req = 0;
	reset(reqs);
	reset(catches);

	scoreFlush();
	return 0;
}

void
scoreResetObjective(int kgs, int fish0, int fish1, int fish2, int fish3)
{
	kgs_req = 0;
	reqs[0] = fish0;
	reqs[1] = fish1;
	reqs[2] = fish2;
	reqs[3] = fish3;
}

void
scoreCaught(int fish_id)
{
}

void
scoreFlush(void)
{
	fprintf(stderr, "[CLEAR]\n");
	fprintf(stderr, "<p>You've caught <b>%d</b> Kg fish</p>\n", kgs);
}

