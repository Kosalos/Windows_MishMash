
// Control structure definition for HLSL
#ifdef SHADER
#define NUM_EQUATIONS 4
#define FLOAT4 float4
#define INT4 int4
#define STRIPE		F1.x
#define ESCAPE		F1.y
#define MULTIPLIER	F1.z
#define CONTRAST	F1.w

#define COLORR		F2.x
#define COLORG		F2.y
#define COLORB		F2.z

#define IMAGE_DELTA	F3.x
#define IMAGE_X		F3.y
#define IMAGE_Y		F3.z

#define XSIZE		I1.x
#define YSIZE		I1.y

cbuffer Control : register(b0)

#else
// Control structure definition for C++
#pragma once
#include "stdafx.h"

#define _CRT_SECURE_NO_WARNINGS
#pragma warning( disable : 4305 ) // double as float
#pragma warning( disable : 4244 ) // double as float
#pragma warning( disable : 4127 ) // constexpr

void abortProgram(char* name, int line);
#define ABORT(hr) if(FAILED(hr)) { abortProgram((char *)__FILE__,__LINE__); }

constexpr auto NUM_EQUATIONS = 4;
#define FLOAT4 XMFLOAT4
#define INT4 XMINT4
#define STRIPE		control.F1.x
#define ESCAPE		control.F1.y
#define MULTIPLIER	control.F1.z
#define CONTRAST	control.F1.w

#define COLORR		control.F2.x
#define COLORG		control.F2.y
#define COLORB		control.F2.z

#define IMAGE_DELTA	control.F3.x
#define IMAGE_X		control.F3.y
#define IMAGE_Y		control.F3.z

#define XSIZE		control.I1.x
#define YSIZE		control.I1.y

struct Control

#endif

	// Control structure definition.  ensure 16 byte alighment
{
	FLOAT4 translate[NUM_EQUATIONS];
	FLOAT4 scale[NUM_EQUATIONS];
	FLOAT4 rotate[NUM_EQUATIONS];
	FLOAT4 F1, F2, F3;
	INT4 I1;
	INT4 equation[NUM_EQUATIONS];
	INT4 grammarString[16];
};

#define THREAD_COUNT 16

