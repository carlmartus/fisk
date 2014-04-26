#ifndef FISK_H
#define FISK_H
#include <stdio.h>
#include "estk.h"

#define WINW 400
#define WINH 600

// boat.c
int boatSetup();
void boatFrame(float fr);
void boatRender(void);
esVec2 boatGetPosition(void);
const float *boatMvp(void);

// sprites.c
enum spriteId {
	SPRITE_STAR,
};

void spriteSetup(void);
void spriteAdd(enum spriteId id, float x, float y, float rad, esVec2 rot);

// sea.c
int seaSetup(void);
void seaPosition(float fr, float startx);
float seaWaveHeight(float x);
void seaRender(float depth);
void seaHeight(float x);

#endif

