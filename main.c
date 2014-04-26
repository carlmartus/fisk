#include <stdio.h>
#include <SDL/SDL.h>
#include <GL/glew.h>
#include "estk.h"
#include "fisk.h"

static void
loop_frame(float fr)
{
	glClear(GL_COLOR_BUFFER_BIT);

	seaPosition(fr, 0.0f);
	seaRender(1.0f);

	esGameGlSwap();
}

static void
loop_exit(void)
{
}

static void
key_quit()
{
	esGameLoopQuit();
}

int
main()
{
	esGameInit(400, 600);

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

	esGameLoop(loop_frame, loop_exit, 0);
	return 0;
}

