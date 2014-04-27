#include "fisk.h"

#define FISH_COUNT 4

#define CLEAR_MSG "[CLEAR]\n"

static int challenge;
static int kgs, kgs_req;
static int reqs[FISH_COUNT];
static int catches[FISH_COUNT];

static const struct {
	int kgs;
} fishes[] = {
	[FISH_SMALL] = { 4 },
};

static const struct {
	int kgs, fish0, fish1, fish2, fish3;
	float f0, a0, f1, a1;
	const char *name;
} challenges[] = {
	{ 0,	2, 0, 0, 0,		0.0f, 0.0f, 0.0f, 0.0f, "Welcome" },
	{ 20,	0, 0, 0, 0,		0.5f, 0.6f, 0.7f, 0.2f, "Fisherman" },
	{ 0,	0, 0, 0, 0,		1.2f, 0.8f, 1.3f, 0.2f, "Shaky storm" },
};

void
flush(void)
{
	fprintf(stderr, CLEAR_MSG);

	if (challenge > 0) {
		int i;
		for (i=0; i<challenge; i++) {
			fprintf(stderr, "<p>Level: %s <img src=\"ok.png\"/></p>\n",
					challenges[i].name);
		}
	}

	fprintf(stderr, "<p>Current mission:<br/><b>%s</b></p>\n<ul>",
			challenges[challenge].name);

	if (challenges[challenge].kgs > 0) {
		fprintf(stderr, "<li>Catch %d kg fish</li>\n", challenges[challenge].kgs);
	}

	if (challenges[challenge].fish0 > 0) {
		fprintf(stderr, "<li>Salmon qty: %d</li>\n", challenges[challenge].fish0);
	}
	if (challenges[challenge].fish1 > 0) {
		fprintf(stderr, "<li>Turtles qty: %d</li>\n", challenges[challenge].fish1);
	}
	if (challenges[challenge].fish2 > 0) {
		fprintf(stderr, "<li>Flying fish qty: %d</li>\n", challenges[challenge].fish2);
	}
	if (challenges[challenge].fish3 > 0) {
		fprintf(stderr, "<li>Questionable fish qty: %d</li>\n", challenges[challenge].fish3);
	}

	fprintf(stderr, "</ul>\n");

	fprintf(stderr, "<p>Your status:</p>\n");
	fprintf(stderr, "<p>You've caught <b>%d</b> Kg fish</p>\n", kgs);
}

static void
reset(int *list)
{
	int i;
	for (i=0; i<FISH_COUNT; i++) {
		*list++ = 0;
	}
}

static void
load_challenge(int id)
{
	seaWaveSettings(
			challenges[id].f0,
			challenges[id].a0,
			challenges[id].f1,
			challenges[id].a1);

	kgs = 0;
	kgs_req = challenges[id].kgs;
	reset(catches);
	reqs[0] = challenges[id].fish0;
	reqs[1] = challenges[id].fish1;
	reqs[2] = challenges[id].fish2;
	reqs[3] = challenges[id].fish3;

	flush();
}

int
scoreSetup(void)
{
	challenge = 0;
	kgs = 0;
	kgs_req = 0;
	reset(reqs);
	reset(catches);

	load_challenge(0);
	scoreHelp();
	return 0;
}

void
scoreNextChallenge(void)
{
	load_challenge(++challenge);
}

static int
has_requirements(void)
{
	if (kgs < kgs_req) return 0;
	int i;
	for (i=0; i<FISH_COUNT; i++) {
		if (catches[i] < reqs[i]) return 0;
	}
	return 1;
}

void
scoreCaught(enum fishType id)
{
	kgs += fishes[id].kgs;
	catches[id-1]++;

	if (has_requirements()) {
		scoreNextChallenge();
	} else {
		flush();
	}
}

void
scoreHelp(void)
{
	fprintf(stderr, CLEAR_MSG);
	fprintf(stderr, "<p>Time to go below the surface and catch some fishes!</p>\n");
	fprintf(stderr, "<p>Keys:<br/>"
			"<b>UP ARROW</b> - Haul rope<br/>"
			"<b>DOWN ARROW</b> - Lower rope<br/>"
			"<b>LEFT ARROW</b> - Drive left<br/>"
			"<b>RIGHT ARROW</b> - Drive right<br/>\n");

	fprintf(stderr, "<p>Challenge</p>\n");
	fprintf(stderr, "<p><ul><li>Catch 2 salmon fishes</li></ul></p>\n");
	fprintf(stderr, "<p><b>WARNING</b> You must have a brand new browser to fish</p>\n");
}

