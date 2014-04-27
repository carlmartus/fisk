#include "fisk.h"
#include <math.h>

float
commonTowardsFloat(float src, float dst, float move)
{
	float der = dst - src;

	if (fabs(der) < move) return dst;

	if (der > 0.0f) {
		return src + move;
	} else if (der < 0.0f) {
		return src - move;
	}

	return src;
}

esVec2
commonTowardsVec2(esVec2 src, esVec2 dst, float move)
{
	esVec2 diff = {
		dst.x - src.x,
		dst.y - src.y,
	};

	float length = sqrtf(dst.x*dst.x + dst.y*dst.y);

	if (length < move) return dst;

	float inv = 1.0f / length;
	esVec2 dir = {
		diff.x*inv,
		diff.y*inv,
	};

	return (esVec2) {
		src.x + dir.x*move,
		src.y + dir.y*move,
	};
}

esVec2
commonNormalizeVec2(esVec2 v)
{
	float r = sqrtf(v.x*v.x + v.y*v.y);

	if (r == 0.0f) return v;

	r = 1.0f / r;
	v.x *= r;
	v.y *= r;
	return v;
}

