#ifndef FISK_H
#define FISK_H
#include <stdio.h>
#include "estk.h"

#define WINW 400
#define WINH 600

// common.c
float commonTowardsFloat(float src, float dst, float move);
esVec2 commonNormalizeVec2(esVec2 v);

// boat.c
int boatSetup();
void boatFrame(float fr);
void boatRender(void);
esVec2 boatGetPosition(void);
const float *boatMvp(void);

// sprites.c
enum spriteId {
	SPRITE_SHIP,
};

const esVec2 spriteNoRot;

int spriteSetup(void);
void spriteAdd(enum spriteId id, float x, float y, esVec2 trans);
void spriteFlushRender(void);

// sea.c
int seaSetup(void);
void seaPosition(float fr, float startx);
float seaWaveHeight(float x);
void seaRender(void);

#endif

