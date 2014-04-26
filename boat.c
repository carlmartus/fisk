#include "fisk.h"

#define ZOOM 0.02f

static float x, y;
static float dx, dy;
static float mvp[16];

int
boatSetup()
{
	x = 0.0f;
	y = -10.0f;
	dx = 0.0f;
	dy = 0.0f;

	return 0;
}

void
boatFrame(float fr)
{
	x += dx*fr;
	y += dy*fr;

	float x0, y0, x1, y1;
	x0 = x - ZOOM * (float) WINW;
	x1 = x + ZOOM * (float) WINW;
	y0 = y - ZOOM * (float) WINH;
	y1 = y + ZOOM * (float) WINH;

	esProjOrtho(mvp, x0, x1, y1, y0);
}

void
boatRender(void)
{
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

