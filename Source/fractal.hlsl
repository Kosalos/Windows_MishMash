#define SHADER
#include "common.h"

// random float in range [0.0f, 1.0f] (based on the xor128 algorithm)
float rand(float y, float z) {
	int seed = 13 + int(y * 1000) * 57 + int(z * 1000) * 241;
	seed = (seed << 13) ^ seed;
	return ((1.0 - ((seed * (seed * seed * 15731 + 789221) + 1376312589) & 2147483647) / 1073741824.0f) + 1.0f) / 2.0f;
}

int modn(unsigned int n, unsigned int m) { return int(((n % m) + m) % m); }

float2 functionCall(float2 old, int channel) {
	const float pi = 3.141592654;
	float2 pt = old;

	switch (equation[channel].x) {
	case 0: // linear
		break;
	case 1: // 'Sinusoidal'
		pt.x = sin(pt.x);
		pt.y = sin(pt.y);
		break;
	case 2: // 'Spherical'
	{
		float r = length(pt);
		float den = pow(r, 2);
		pt.x /= den;
		pt.y /= den;
	}
	break;
	case 3: // 'Swirl'
	{
		float r = length(pt);
		float den = pow(r, 2);
		pt.x = (old.x * sin(den)) - (old.y * cos(den));
		pt.y = (old.x * cos(den)) + (old.y * sin(den));
	}
	break;
	case 4: // 'Horseshoe'
	{
		float r = length(old);
		pt.x = (1 / r) * (old.x - old.y) * (old.x + old.y);
		pt.y = (1 / r) * 2 * old.x * old.y;
	}
	break;
	case 5: // 'Polar'
	{
		float r = length(pt);
		float th = atan2(pt.y, pt.x);
		pt.x = th / pi;
		pt.y = r - 1;
	}
	break;
	case 6: // 'Hankerchief'
	{
		float r = length(pt);
		float th = atan2(pt.y, pt.x);
		pt.x = r * sin(th + r);
		pt.y = r * cos(th - r);
	}
	break;
	case 7: // 'Heart'
	{
		float r = length(pt);
		float th = atan2(pt.y, pt.x);
		pt.x = r * sin(th * r);
		pt.y = r * -cos(th * r);
	}
	break;
	case 8: // 'Disc'
	{
		float r = length(pt);
		float th = atan2(pt.y, pt.x);
		pt.x = (th / pi) * sin(pi * r);
		pt.y = (th / pi) * cos(pi * r);
	}
	break;
	case 9: // 'Spiral'
	{
		float r = length(pt);
		float th = atan2(pt.y, pt.x);
		pt.x = (1 / r) * (cos(th) + sin(r));
		pt.y = (1 / r) * (sin(th) - cos(r));
	}
	break;
	case 10: // 'Hyperbolic'
	{
		float r = length(pt);
		float th = atan2(pt.y, pt.x);
		pt.x = sin(th) / r;
		pt.y = r * cos(th);
	}
	break;
	case 11: // 'Diamond'
	{
		float r = length(pt);
		float th = atan2(pt.y, pt.x);
		pt.x = sin(th) * cos(r);
		pt.y = cos(th) * sin(r);
	}
	break;
	case 12: // 'Ex'
	{
		float r = length(pt);
		float th = atan2(pt.y, pt.x);
		float p0 = sin(th + r);
		float p1 = cos(th - r);
		pt.x = r * (pow(p0, 3) + pow(p1, 3));
		pt.y = r * (pow(p0, 3) - pow(p1, 3));
	}
	break;
	case 13: // 'Julia'
	{
		float rs = sqrt(length(pt));
		float th = atan2(pt.y, pt.x);
		float om = translate[channel].x || (translate[channel].x + rotate[channel].x + scale[channel].x + translate[channel].y + scale[channel].y);
		pt.x = rs * cos(th / 2 + om);
		pt.y = rs * sin(th / 2 + om);
	}
	break;
	case 14: // 'JuliaN',
	{
		float r = length(pt);
		float ph = atan2(pt.x, pt.y);
		float p1 = 1;
		float p2 = 0.75;
		float rrnd = translate[channel].x + 0.5; //  || 0.5;
		float p3 = trunc(abs(p1) * rrnd);
		float t = (ph + (2 * pi * p3)) / p1;
		float rpp = pow(r, p2 / p1);
		pt.x = rpp * cos(t);
		pt.y = rpp * sin(t);
	}
	break;
	case 15: // 'Bent'
	{
		if (pt.x >= 0 && pt.y >= 0) break;
		if (pt.x < 0 && pt.y >= 0) {
			pt.x *= 2;
		}
		else if (pt.x >= 0 && pt.y < 0) {
			pt.y /= 2;
		}
		else {
			pt.x *= 2;
			pt.y /= 2;
		}
	}
	break;
	case 16: // 'Waves'
		pt.x = old.x + (rotate[channel].x * sin(old.y / pow(scale[channel].x, 2)));
		pt.y = old.y + (rotate[channel].x * sin(old.x / pow(scale[channel].y, 2)));
		break;
	case 17: // 'Fisheye'
	{
		float re = 2 / (sqrt(pow(old.x, 2) + pow(old.y, 2)) + 1);
		pt.x = re * old.y;
		pt.y = re * old.x;
	}
	break;
	case 18: // 'Popcorn'
		pt.x = old.x + (scale[channel].x * sin(tan(3 * old.y)));
		pt.y = old.y + (scale[channel].y * sin(tan(3 * old.x)));
		break;
	case 19: // 'Power'
	{
		float th = atan2(old.y, old.x);
		float rsth = pow(sqrt(pow(old.x, 2) + pow(old.y, 2)), sin(th));
		pt.x = rsth * cos(th);
		pt.y = rsth * sin(th);
	}
	break;
	case 20: // 'Rings'
	{
		float r = length(pt);
		float th = atan2(pt.y, pt.x);
		float pp = pow(scale[channel].x, 2);
		float re = modn((r + pp), (2 * pp)) - pp + (r * (1 - pp));
		pt.x = re * cos(th);
		pt.y = re * sin(th);
	}
	break;
	case 21: // 'Fan'
	{
		float r = length(pt);
		float th = atan2(pt.y, pt.x);
		float t = pi * pow(scale[channel].x, 2);
		if (modn((th + scale[channel].y), t) > (t / 2)) {
			pt.x = r * cos(th - (t / 2));
			pt.y = r * sin(th - (t / 2));
		}
		else {
			pt.x = r * cos(th + (t / 2));
			pt.y = r * sin(th + (t / 2));
		}
	}
	break;
	case 22: // 'Eyefish'
	{
		float  re = 2 / (sqrt(pow(pt.x, 2) + pow(pt.y, 2)) + 1);
		pt.x *= re;
		pt.y *= re;
	}
	break;
	case 23: // 'Bubble'
	{
		float re = 4 / (pow(sqrt(pow(pt.x, 2) + pow(pt.y, 2)), 2) + 4);
		pt.x *= re;
		pt.y *= re;
	}
	break;
	case 24: // 'Cylinder'
		pt.x = sin(pt.y);
		break;
	case 25: // 'Tangent'
		pt.x = sin(pt.x) / cos(pt.y);
		pt.y = tan(pt.y);
		break;
	case 26: // 'Cross',
	{
		float s = sqrt(1 / pow(pow(pt.x, 2) - pow(pt.y, 2), 2));
		pt.x *= s;
		pt.y *= s;
	}
	break;
	case 27: // 'Noise'
	{
		float lastRandom = old.x * 10240 + old.y * 12345;
		lastRandom = rand(lastRandom, pt.x);
		float p1 = lastRandom;
		lastRandom = rand(lastRandom, pt.y);
		float p2 = lastRandom;
		pt.x = p1 * pt.x * cos(2 * pi * p2);
		pt.y = p1 * pt.y * sin(2 * pi * p2);
	}
	break;
	case 28: // 'Blur'
	{
		float lastRandom = old.x * 10240 + old.y * 12345;
		lastRandom = rand(lastRandom, pt.x);
		float p1 = lastRandom;
		lastRandom = rand(lastRandom, pt.y);
		float p2 = lastRandom;
		pt.x = p1 * cos(2 * pi * p2);
		pt.y = p1 * sin(2 * pi * p2);
	}
	break;
	case 29: // 'Square'
	{
		float lastRandom = old.x * 10240 + old.y * 12345;
		lastRandom = rand(lastRandom, pt.x);
		float p1 = lastRandom;
		lastRandom = rand(lastRandom, pt.y);
		float p2 = lastRandom;
		pt.x = p1 - 0.5;
		pt.y = p2 - 0.5;
	}
	break;
	}

	return pt;
}

// ------------------------------------------------------------
RWTexture2D<float4> OutputMap : register(u0);

[numthreads(THREAD_COUNT,THREAD_COUNT, 1)]

void CSMain(uint3 p:SV_DispatchThreadID)
{
	if (p.x >= uint(XSIZE)) return;
	if (p.y >= uint(YSIZE)) return;

	float2 z = float2(IMAGE_X + IMAGE_DELTA * float(p.x), IMAGE_Y + IMAGE_DELTA * float(p.y));
	float z2 = 0;
	float2 zOrig = z;
	int iter;
	float avg = 0;
	float lastAdded = 0;
	int gIndex = 0;
	int channel;
	const int MAXITER = 32;

	for (iter = 0; iter < MAXITER; ++iter) {
		// round-robin function index from grammar string
		channel = int(grammarString[gIndex++].x) - 49; // 49 = ASCII offset '1'
		if (gIndex >= 12) gIndex = 0;
		if (channel < 0) {                   // terminating zero -> -49
			gIndex = 0;
			channel = int(grammarString[gIndex++].x) - 49;
		}
		z = functionCall(z, channel); 

		// translate
		z += translate[channel].xy;

		// rotate
		if (rotate[channel].x != 0) {
			float qt = z.x;
			float ss = sin(rotate[channel].x);
			float cc = cos(rotate[channel].x);
			z.x = z.x * cc - z.y * ss;
			z.y = qt * ss + z.y * cc;
		}

		// scale
		z *= scale[channel].xy;

		lastAdded = 0.5 + 0.5 * sin(STRIPE * atan2(z.y, z.x));
		avg += lastAdded;

		z2 = dot(z, z);
		if (z2 > ESCAPE) break;
	}

	float4 color = float4(0,0,0,1);

	if (iter > 1) {
		float prevAvg = (avg - lastAdded) / (iter - 1.0);
		avg = avg / iter;

		float frac = 1.0 + (log2(log(ESCAPE) / log(z2)));
		float mix = frac * avg + (1.0 - frac) * prevAvg;

		if (iter < MAXITER) {
			float co = mix * pow(10.0, MULTIPLIER);
			co = clamp(co, 0.0, 10000.0) * 6.2831;
			color.x = 0.5 + 0.5 * cos(co + COLORR);
			color.y = 0.5 + 0.5 * cos(co + COLORG);
			color.z = 0.5 + 0.5 * cos(co + COLORB);
		}
	}

	color.x = 0.5 + (color.x - 0.5) * CONTRAST;
	color.y = 0.5 + (color.y - 0.5) * CONTRAST;
	color.z = 0.5 + (color.z - 0.5) * CONTRAST;

	OutputMap[p.xy] = color;
}
