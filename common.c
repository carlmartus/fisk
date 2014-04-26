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

