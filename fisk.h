#ifndef FISK_H
#define FISK_H
#include <stdio.h>
#include "estk.h"

#define WINW 400
#define WINH 600

// common.c
float commonTowardsFloat(float src, float dst, float move);
esVec2 commonTowardsVec2(esVec2 src, esVec2 dst, float move);
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
	SPRITE_HOOK,
	SPRITE_ROPE,
	SPRITE_FISH_SMALL,
	SPRITE_FISH_FLYING,
	SPRITE_FISH_BIKE,
	SPRITE_FISH_SUBMARINE,
};

const esVec2 spriteNoRot;

int spriteSetup(void);
void spriteAdd(enum spriteId id, float x, float y, esVec2 trans);
void spritePushCustomVertice(float x, float y, float u, float v);
void spriteFlushRender(void);
void spriteGetUvs(enum spriteId id, float *u0, float *v0, float *u1, float *v1);

// sea.c
int seaSetup(void);
void seaPosition(float fr, float startx);
float seaWaveHeight(float x);
void seaWaveSettings(float f0, float a0, float f1, float a1);
void seaRender(void);

// fish.c
enum fishType {
	FISH_NONE=0,
	FISH_SMALL,
	FISH_BIKE,
	FISH_FLYING,
	FISH_SUBMARINE,
};

int fishSetup(void);
void fishMoveFrame(float fr, float x);
void fishRender(void);
void fishRenderHooked(enum fishType type, float x, float y, esVec2 trans);
enum fishType fishHook(float x, float y);
void fishEnableSpawn(enum fishType type);

// score.c
int scoreSetup(void);
void scoreNextChallenge(void);
void scoreHelp(void);
void scoreCaught(enum fishType id);

#endif

