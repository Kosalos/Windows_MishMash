#pragma once

#include "stdafx.h"
#include "View.h"	

extern HFONT font;
extern COLORREF gray0, gray1, gray2;
extern HBRUSH hgray1;
extern COLORREF green0, green1, green2;
extern HBRUSH hgreen1;

#define LEGEND_LENGTH 63
#define PERCENT 100
#define NUM_FORMULAS  30
#define GRAMMARSTRING_LENGTH  12
#define GROUPWIDTH  140

#define NUM_FIELDS_PER_EQUATION  5 // tx,ty,sz,sy,r
#define CONTROL_GROUP_BASEINDEX (NUM_EQUATIONS * NUM_FIELDS_PER_EQUATION)
#define NUM_FIELDS (CONTROL_GROUP_BASEINDEX + 7)

struct Param {
	char grammarString[GRAMMARSTRING_LENGTH + 4]; // 20 bytes
	int equ[4];
	float f[NUM_FIELDS];
};

extern Param param;

// =====================================================================

class EquationGroup {
public:
	int x, y, baseIndex,focus;
	bool enabled;
	char unused[3];
	void draw(HDC hdc);
	bool alter();
};

// =====================================================================

class ControlGroup {
public:
	int x, y, focus;

	void draw(HDC hdc);
	bool alter();
};

// =====================================================================

class Widget {
public:
	HWND hWnd;
	HDC hdcMem;
	HBITMAP hbmMem, hbmOld;
	HBRUSH hbrBkGnd;
	HFONT hfntOld;
	HWND equationGroupDropdown[NUM_EQUATIONS]; // = { NULL };
	HWND grammarStringEditField;

	EquationGroup equationGroup[NUM_EQUATIONS];
	ControlGroup controlGroup;

	int globalFocus;
	bool isVisible;
	bool autoChange;
	char unused[2];
	float deltaX, deltaY;

	Widget() {
		grammarStringEditField = NULL;
		hWnd = NULL;
		hdcMem = NULL;
		hbmMem = NULL;
		hbmOld = NULL;
		hbrBkGnd = NULL;
		hfntOld = NULL;
		globalFocus = 0;
		isVisible = false;
		autoChange = false;
		deltaX = 0;
		deltaY = 0;
	}

	void create(HWND parent, HINSTANCE inst);
	void createHandler();

	bool keyDown(int key);
	void keyUp(int key);
	void changeGlobalFocus(int direction);
	bool islegalFocus();	
	void updateGroupFocus();

	void refresh(bool alsoCompute = true);
	void drawWindow();
	void toggleVisible();
	bool parameterWasEdited();

	void lButtonDown(LPARAM lParam, bool isMainWindow = true);
	void lButtonUp();
	void rButtonDown(LPARAM lParam);
	void rButtonUp();
	void mouseMove(WPARAM wParam, LPARAM lParam);
	void timerHandler();
	void randomSettings();
	void randomGrammarString();
	void updateAfterRandomChanges();
	void downloadSettings();
	void mouseWheelHandler(int direction);
	bool isCharInGrammarString(int chr);
	void updateEquationGroupEnables();
	void compute();
};

extern Widget widget;

RECT makeRect(int x, int y, int xs, int ys);
void drawBorderRect(HDC hdc, int x1, int y1, int xs, int ys, bool enabled);
void sliderEntry(HDC hdc, int xx, int yy, float percentx, float percenty, const char* legend, bool focused);
bool percentChanged(float& value, float amt);