#include <stdio.h>
#include <SDL/SDL.h>
#include <GL/glew.h>
#include "estk.h"
#include "fisk.h"

static void
loop_frame(float fr)
{
	if (fr > 0.3f) return;

	glClear(GL_COLOR_BUFFER_BIT);

	boatFrame(fr);
	esVec2 boat_pos = boatGetPosition();

	fishMoveFrame(fr, boat_pos.x);
	seaPosition(fr, boat_pos.x-22.0f);

	seaRender();
	fishRender();
	boatRender();
	spriteFlushRender();

	esGameGlSwap();
}

#ifndef EMSCRIPTEN
static void
key_quit()
{
	esGameLoopQuit();
}
#endif

int
main()
{
	esGameInit(WINW, WINH);

	glClearColor(0.1, 0.1, 0.3, 1.0);

	if (seaSetup()) {
		printf("Cannot create sea!\n");
		return 1;
	}

	if (boatSetup()) {
		printf("Cannot create boat!\n");
		return 1;
	}

	if (spriteSetup()) {
		printf("Cannot setup sprites!\n");
		return 1;
	}

	if (fishSetup()) {
		printf("Cannot setup fish!\n");
		return 1;
	}

	if (scoreSetup()) {
		printf("Cannot start score system\n");
		return 1;
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#ifndef EMSCRIPTEN
	esGameRegisterKey(SDLK_q, key_quit);
#endif

	scoreNextChallenge();
	esGameLoop(loop_frame, NULL, 0);
	return 0;
}

