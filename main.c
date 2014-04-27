#include <stdio.h>
#include <SDL/SDL.h>
#include <GL/glew.h>
#include "estk.h"
#include "fisk.h"

static void
loop_frame(float fr)
{
	glClear(GL_COLOR_BUFFER_BIT);

	boatFrame(fr);
	esVec2 boat_pos = boatGetPosition();

	fishMoveFrame(fr, boat_pos.x);
	seaPosition(fr, boat_pos.x-15.0f);

	seaRender();
	fishRender();
	boatRender();
	spriteFlushRender();

	esGameGlSwap();
}

static void
key_quit()
{
	esGameLoopQuit();
}

int
main()
{
	esGameInit(WINW, WINH);

	glClearColor(0.1, 0.1, 0.3, 1.0);

	esGameRegisterKey(SDLK_q, key_quit);

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

	fprintf(stderr, "[CLEAR]\n");
	fprintf(stderr, "Hello %d\n", 2);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	esGameLoop(loop_frame, NULL, 0);
	return 0;
}

