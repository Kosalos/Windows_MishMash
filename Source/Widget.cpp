#include "stdafx.h"
#include "Widget.h"
#include "Help.h"
#include "saveLoad.h"
#include <time.h>

HFONT font = NULL;
COLORREF gray0 = RGB(60, 60, 60);
COLORREF gray1 = RGB(128, 128, 128);
COLORREF gray2 = RGB(200, 200, 200);
COLORREF green0 = RGB(60, 60 + 50, 60);
COLORREF green1 = RGB(128, 128 + 50, 128);
COLORREF green2 = RGB(200, 200 + 50, 200);
HBRUSH hgray1 = CreateSolidBrush(gray1);
HBRUSH hgreen1 = CreateSolidBrush(green1);
HBRUSH hred1 = CreateSolidBrush(RGB(148, 108, 108));
extern HWND mainWindowHandle;

Widget widget;
Param param;

static bool isKeyboardAltering = false;
static bool isMouseAltering = false;
static bool controlKeyDown = false;
float autoChangeAngle[NUM_FIELDS];
float autoChangeAngleDelta[NUM_FIELDS];

// ================================================================================

WNDPROC originalEditBoxHandler;

LRESULT CALLBACK EditProc(HWND hEdit, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int chr;
	bool imageRecalcRequired = false;

	switch (msg) {
	case WM_KEYDOWN:
		chr = LOWORD(wParam);
		if (chr == VK_BACK || chr == VK_DELETE) imageRecalcRequired = true;
		break;
	case WM_CHAR:
		chr = LOWORD(wParam);
		if (chr != VK_BACK && (chr < '1' || chr > '4')) return 0;
		imageRecalcRequired = true;
		break;
	case WM_DESTROY:
		SetWindowLong(hEdit, GWL_WNDPROC, (LONG)originalEditBoxHandler);
		return 0;
	}

	LRESULT ret = CallWindowProc(originalEditBoxHandler, hEdit, msg, wParam, lParam);

	if (imageRecalcRequired)
		widget.refresh();

	return ret;
}

// ================================================================================
static char* CLASS_NAME = (char*)"Widget";
#define HELP_BUTTON  2000

LRESULT CALLBACK WidgetWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_CREATE:
		widget.hWnd = hWnd;
		widget.createHandler();
		break;
	case WM_PAINT:		widget.drawWindow(); break;
	case WM_KEYDOWN:	widget.keyDown(int(wParam)); break;
	case WM_KEYUP:		widget.keyUp(int(wParam)); break;
	case WM_LBUTTONDOWN:widget.lButtonDown(lParam, false); break;
	case WM_LBUTTONUP:	widget.lButtonUp();	break;
	case WM_RBUTTONDOWN:widget.rButtonDown(lParam);	break;
	case WM_RBUTTONUP:	widget.rButtonUp();	break;
	case WM_MOUSEMOVE:	widget.mouseMove(wParam, lParam); break;
	case WM_MOUSEWHEEL:	widget.mouseWheelHandler(GET_WHEEL_DELTA_WPARAM(wParam)); break;

	case WM_COMMAND:
		if (LOWORD(wParam) == HELP_BUTTON)
			help.launch();

		// dropbox's released. move window focus back to widget dialog
		if (LOWORD(wParam) < 5 && HIWORD(wParam) == 1)
			SetFocus(hWnd);

		if (HIWORD(wParam) == CBN_SELCHANGE) { // equation selection change
			for (int i = 0; i < NUM_EQUATIONS; ++i)
				param.equ[i] = SendMessage(widget.equationGroupDropdown[i], CB_GETCURSEL, 0, 0);
		}

		widget.refresh();
		break;

	case WM_DESTROY:
		for (int i = 0; i < NUM_EQUATIONS; ++i) DeleteObject(widget.equationGroupDropdown[i]);
		DeleteObject(widget.hWnd);
		DeleteObject(font);
		DeleteObject(widget.hdcMem);
		DeleteObject(widget.hbmMem);
		DeleteObject(widget.hbmOld);
		DeleteObject(widget.hbrBkGnd);
		DeleteObject(widget.hfntOld);
		DeleteObject(widget.grammarStringEditField);
		DeleteObject(hgray1);
		DeleteObject(hgreen1);
		DeleteObject(hred1);
		return TRUE;

	case WM_ERASEBKGND:
		return (LRESULT)1;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

// ================================================================================

void Widget::create(HWND parent, HINSTANCE hInstance) {
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WidgetWndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = CLASS_NAME;
	wcex.hIconSm = NULL;

	if (!RegisterClassEx(&wcex)) {
		MessageBox(NULL, "Widget Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		exit(-1);
	}

	RECT rc2 = { 100, 100, 100 + 165,100 + 700 };
	AdjustWindowRect(&rc2, WS_OVERLAPPEDWINDOW, FALSE);

	hWnd = CreateWindow(CLASS_NAME, "Param", WS_OVERLAPPED | WS_BORDER | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT,
		rc2.right - rc2.left, rc2.bottom - rc2.top, parent, NULL, hInstance, NULL);
	if (hWnd == NULL) {
		MessageBox(NULL, "Widget Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		exit(-1);
	}

	CreateWindow("BUTTON", "Help", WS_CHILD | WS_VISIBLE, 46, 665, 65, 22, hWnd, (HMENU)HELP_BUTTON, 0, 0);

	isVisible = true;
	ShowWindow(hWnd, SW_SHOWNORMAL);
	SetFocus(hWnd);

	for (int i = 0; i < NUM_EQUATIONS; ++i) {
		equationGroup[i].x = 10;
		equationGroup[i].y = 40 + i * 115;
		equationGroup[i].baseIndex = i * NUM_FIELDS_PER_EQUATION;
	}

	controlGroup.x = 10;
	controlGroup.y = equationGroup[3].y + 115;

	font = CreateFont(20, 8, 0, 0, 600, FALSE, FALSE, FALSE,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_ROMAN, "Courier New");
}

// ================================================================================
const char* functionNames[] = {
	(char*)"Linear", (char*)"Sinusoidal", (char*)"Spherical", (char*)"Swirl", 
	(char*)"Horseshoe", (char*)"Polar",	(char*)"Hankerchief", (char*)"Heart", 
	(char*)"Disc", (char*)"Spiral", (char*)"Hyperbolic", (char*)"Diamond", (char*)"Ex",
	(char*)"Julia", (char*)"JuliaN", (char*)"Bent", (char*)"Waves", (char*)"Fisheye", 
	(char*)"Popcorn", (char*)"Power", (char*)"Rings", (char*)"Fan",	(char*)"Eyefish", 
	(char*)"Bubble", (char*)"Cylinder", (char*)"Tangent", (char*)"Cross", (char*)
	(char*)"Noise", (char*)"Blur", (char*)"Square",(char*)"" };

void Widget::createHandler() {
	widget.grammarStringEditField = CreateWindow("EDIT", NULL, WS_VISIBLE | WS_CHILD, 15, 15, 100, 16, hWnd, HMENU(20), NULL, NULL);
	originalEditBoxHandler = (WNDPROC)SetWindowLong(widget.grammarStringEditField, GWL_WNDPROC, (LONG)EditProc);

	for (int i = 0; i < NUM_EQUATIONS; ++i) {
		widget.equationGroupDropdown[i] = CreateWindow("COMBOBOX", NULL, WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST,
			45, 43 + i * 115, 100, 600, hWnd, HMENU(i + 1), NULL, NULL);
	}

	int index = 0;
	for (;;) {
		for (int i = 0; i < NUM_EQUATIONS; ++i)
			SendMessage(widget.equationGroupDropdown[i], CB_ADDSTRING, 0, (LPARAM)functionNames[index]);
		++index;
		if (strlen(functionNames[index]) == 0) break;
	}

	srand(unsigned int(time(NULL)));
	randomSettings();
}

// ================================================================================

bool Widget::isCharInGrammarString(int chr) {
	for (int i = 0; i < GRAMMARSTRING_LENGTH; ++i) {
		if (control.grammarString[i].x == 0) break;
		if (control.grammarString[i].x == chr) return true;
	}

	return false;
}

void Widget::updateEquationGroupEnables() {
	for (int i = 0; i < NUM_EQUATIONS; ++i)
		equationGroup[i].enabled = isCharInGrammarString('1' + i);
}

void Widget::updateAfterRandomChanges() {
	SetWindowTextA(grammarStringEditField, param.grammarString);

	downloadSettings();

	// globalFocus = select 'Translate' of first enabled group
	updateEquationGroupEnables();
	globalFocus = 99;
	changeGlobalFocus(1);
	compute();
}

void Widget::randomGrammarString() {
	int length = 1 + (rand() % GRAMMARSTRING_LENGTH);
	for (int j = 0; j < length; ++j)
		param.grammarString[j] = '1' + (rand() % NUM_EQUATIONS);
	param.grammarString[length] = 0;

	updateAfterRandomChanges();
}

void Widget::randomSettings() {
	randomGrammarString();

	for (int i = 0; i < NUM_EQUATIONS; ++i) {
		param.equ[i] = rand() % NUM_FORMULAS;
		SendMessage(widget.equationGroupDropdown[i], CB_SETCURSEL, WPARAM(param.equ[i]), 0);
	}

	for (int i = 0; i < NUM_FIELDS; ++i)
		param.f[i] = float(rand() % PERCENT);

	IMAGE_X = -2;
	IMAGE_Y = -2;
	IMAGE_DELTA = 0.005;

	updateAfterRandomChanges();
}

// ================================================================================

float interpolate(float min, float max, float percent) {
	return min + (max - min) * percent / 100.0f;
}

// Widget fields asre all percentages (0..100). 
// Here they are mapped to appropriate compute shader parameters in the Control structure
void Widget::downloadSettings() {
	// grammar string
	char str[GRAMMARSTRING_LENGTH + 1];
	GetWindowTextA(grammarStringEditField, str, GRAMMARSTRING_LENGTH);
	for (int i = 0; i < GRAMMARSTRING_LENGTH; ++i) {
		control.grammarString[i].x = int(str[i]);
		if (str[i] == 0) break;
	}

	int baseIndex;
	for (int i = 0; i < NUM_EQUATIONS; ++i) {
		control.equation[i].x = param.equ[i];

		// translate, scale, rotate
		baseIndex = i * NUM_FIELDS_PER_EQUATION;
		control.translate[i].x = interpolate(-3, +3, param.f[baseIndex + 0]);
		control.translate[i].y = interpolate(-3, +3, param.f[baseIndex + 1]);
		control.scale[i].x = interpolate(0.1, 4, param.f[baseIndex + 2]);
		control.scale[i].y = interpolate(0.1, 4, param.f[baseIndex + 3]);
		control.rotate[i].x = interpolate(-3, +3, param.f[baseIndex + 4]);
	}

	MULTIPLIER = interpolate(-1, 1, param.f[CONTROL_GROUP_BASEINDEX + 0]);
	STRIPE = interpolate(-10, 10, param.f[CONTROL_GROUP_BASEINDEX + 1]);
	ESCAPE = interpolate(0.01, 80, param.f[CONTROL_GROUP_BASEINDEX + 2]);
	CONTRAST = interpolate(0.1, 5, param.f[CONTROL_GROUP_BASEINDEX + 3]);
	COLORR = interpolate(0, 1, param.f[CONTROL_GROUP_BASEINDEX + 4]);
	COLORG = interpolate(0, 1, param.f[CONTROL_GROUP_BASEINDEX + 5]);
	COLORB = interpolate(0, 1, param.f[CONTROL_GROUP_BASEINDEX + 6]);
}

void Widget::toggleVisible() {
	ShowWindow(hWnd, isVisible ? SW_HIDE : SW_SHOWNORMAL);
	isVisible = !isVisible;
}

void Widget::drawWindow() {
	RECT rc = { 0 };
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hWnd, &ps);

	GetClientRect(hWnd, &rc);

	if (hdcMem == NULL) {	// offscreen drawing buffer
		hdcMem = CreateCompatibleDC(hdc);
		hbmMem = CreateCompatibleBitmap(hdc, rc.right - rc.left, rc.bottom - rc.top);
		hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);
		hbrBkGnd = CreateSolidBrush(RGB(230, 245, 230));
		hfntOld = (HFONT)SelectObject(hdcMem, font);
		SetBkMode(hdcMem, TRANSPARENT);
	}

	FillRect(hdcMem, &rc, autoChange ? hred1 : hgray1);

	drawBorderRect(hdcMem, 10, 10, GROUPWIDTH, 25, false); // grammar String panel

	updateEquationGroupEnables();
	for (int i = 0; i < NUM_EQUATIONS; ++i)
		equationGroup[i].draw(hdcMem);
	controlGroup.draw(hdcMem);

	BitBlt(hdc,
		rc.left, rc.top,
		rc.right - rc.left, rc.bottom - rc.top,
		hdcMem,
		0, 0,
		SRCCOPY);

	EndPaint(hWnd, &ps);
}

// ================================================================================

bool isRMBpressed = false; // RMB (right mouse button) pressed = move/zooo mode rather than parameter editing

// is the global focus sitting in an enabled group?
bool Widget::islegalFocus() {
	if (globalFocus >= 12) return true; // controlGroup group is always enabled
	return equationGroup[globalFocus / 3].enabled; // equationGroup[] groups are only enabled if they are represented in the grammar string
}

// move global focus (skipping fields in unused groups), update group's focus values, repaint
void Widget::changeGlobalFocus(int direction) {
	const int count = 18;
	for (;;) {
		globalFocus += direction > 0 ? 1 : -1;
		if (globalFocus < 0) globalFocus = count - 1; else if (globalFocus >= count) globalFocus = 0;
		if (islegalFocus()) break;
	}

	updateGroupFocus();
}

// map global focus percentage to specific field in equationGroup[] and controlGroup groups
void Widget::updateGroupFocus() {
	for (int i = 0; i < NUM_EQUATIONS; ++i) {
		equationGroup[i].focus = globalFocus >= i * 3 && globalFocus < (i + 1) * 3 ? globalFocus % 3 : -1;
		equationGroup[i].draw(hdcMem);
	}

	controlGroup.focus = globalFocus >= 12 ? globalFocus - 12 : -1;

	refresh(false);
}

// alter the parameters according to global focus and delta values. true result = a parameter was changed
bool Widget::parameterWasEdited() {
	for (int i = 0; i < NUM_EQUATIONS; ++i)
		if (equationGroup[i].alter()) return true;
	if (controlGroup.alter()) return true;
	return false;
}

bool Widget::keyDown(int key) {
	switch (key) {
	case VK_LEFT:	deltaX = -1;	isKeyboardAltering = true;	break;
	case VK_RIGHT:	deltaX = +1;	isKeyboardAltering = true;	break;
	case VK_DOWN:	deltaY = +1;	isKeyboardAltering = true;	break;
	case VK_UP:		deltaY = -1;	isKeyboardAltering = true;	break;
	case VK_CONTROL: controlKeyDown = true; break;

	case 188: changeGlobalFocus(-1); break; // <
	case 190: changeGlobalFocus(+1); break; // >

	case VK_ESCAPE:
		SendMessage(mainWindowHandle, WM_CLOSE, 0, 0);
		break;
	}

	switch (toupper(key)) {
	case 'A':
		autoChange = !autoChange;
		if (autoChange) {
			for (int i = 0; i < NUM_FIELDS; ++i) {
				autoChangeAngle[i] = float(rand() % 1024) / 7000;
				autoChangeAngleDelta[i] = float(rand() % 1024) / 1500000;
			}
		}
		else
			refresh();
		break;

	case 'L': saveLoad.launch(); break;
	case 'Q': randomGrammarString(); break;
	case 'R': randomSettings(); break;
	case 'S': saveLoad.saveParam(); break;
	case ' ': toggleVisible(); break;
	}

	return true;
}

void Widget::keyUp(int key) {
	switch (key) {
	case VK_LEFT:
	case VK_RIGHT:
		deltaX = 0;
		if (deltaY == 0) isKeyboardAltering = false;
		break;
	case VK_DOWN:
	case VK_UP:
		deltaY = 0;
		if (deltaX == 0) isKeyboardAltering = false;
		break;
	case VK_CONTROL: controlKeyDown = false; break;
	}
}

// ================================================================================

static POINTS mouseStartPosition, mouseCurrentPosition;

void Widget::lButtonDown(LPARAM lParam, bool isMainWindow) {
	POINTS pt = MAKEPOINTS(lParam);

	// mouse drags on main window = parameter editing
	if (isMainWindow) {
		mouseStartPosition = pt;
		mouseCurrentPosition = mouseStartPosition;
		isMouseAltering = true;
	}
	else { // mouse clicks on widget windows = move focus to clicked widget row
		for (int i = 0; i < NUM_EQUATIONS; ++i) {
			if (pt.y >= equationGroup[i].y && pt.y < equationGroup[i].y + 115) {  // equationGroup group
				if (abs(pt.y - (equationGroup[i].y + 40)) < 20) { globalFocus = i * 3; break; }
				if (abs(pt.y - (equationGroup[i].y + 64)) < 20) { globalFocus = i * 3 + 1; break; }
				if (abs(pt.y - (equationGroup[i].y + 88)) < 20) { globalFocus = i * 3 + 2; break; }
			}
		}
		for (int i = 0; i < 6; ++i) { // controlGroup group
			if (abs(pt.y - (514 + i * 24)) < 20) { globalFocus = 12 + i; break; }
		}

		updateGroupFocus();
	}
}

void Widget::lButtonUp() {
	isMouseAltering = false;
}

// RMB mouse drags control image panning. (mouse wheel while RMB = image zoom)
void Widget::rButtonDown(LPARAM lParam) {
	mouseStartPosition = MAKEPOINTS(lParam);
	mouseCurrentPosition = mouseStartPosition;
	isMouseAltering = true;
	isRMBpressed = true;
}

void Widget::rButtonUp() {
	isMouseAltering = false;
	isRMBpressed = false;
}

void Widget::mouseMove(WPARAM, LPARAM lParam) {
	if (isMouseAltering)
		mouseCurrentPosition = MAKEPOINTS(lParam);
}

void Widget::mouseWheelHandler(int direction) {
	if (isRMBpressed) { // zoom
		isMouseAltering = false; // so inadverdent mousemoves stop interrupting zoom session

		float zoomAmount = direction < 0 ? 1.08 : 0.92;
		float oldCenterX = IMAGE_X + float(XSIZE) * IMAGE_DELTA / 2.0f;
		float oldCenterY = IMAGE_Y + float(YSIZE) * IMAGE_DELTA / 2.0f;
		IMAGE_DELTA *= zoomAmount;
		IMAGE_X = oldCenterX - float(XSIZE) * IMAGE_DELTA / 2.0f;
		IMAGE_Y = oldCenterY - float(YSIZE) * IMAGE_DELTA / 2.0f;
		refresh();
	}
	else {
		changeGlobalFocus(-direction / 120);
	}
}

// ================================================================================

void Widget::timerHandler() {
	bool isComputeRequired = false;

	if (autoChange) {
		const float PI2 = (3.141592654 * 2);
		for (int i = 0; i < NUM_FIELDS; ++i) {
			param.f[i] += sin(autoChangeAngle[i]) * 0.1;
			if (param.f[i] < 0) param.f[i] = 0; else if (param.f[i] > 100) param.f[i] = 100;

			autoChangeAngle[i] += autoChangeAngleDelta[i];
			if (autoChangeAngle[i] > PI2) autoChangeAngle[i] -= PI2;
		}

		isComputeRequired = true;
	}

	if (isMouseAltering) {
		if (isRMBpressed) { // (RMB = right mouse button) pan the image by altering mapping to UL corner (IMAGE_X, IMAGE_Y)
			float scale = IMAGE_DELTA / 2.0f;
			if (controlKeyDown) scale *= 10;
			IMAGE_X -= (mouseCurrentPosition.x - mouseStartPosition.x) * scale;
			IMAGE_Y -= (mouseCurrentPosition.y - mouseStartPosition.y) * scale;
			isComputeRequired = true;
		}
		else { // (LMB = left mouse button) update parameter editing deltas. apply them, and re-calc image if any parameter was changed
			float scale = IMAGE_DELTA;
			if (controlKeyDown) scale *= 10;
			deltaX = (mouseCurrentPosition.x - mouseStartPosition.x) * scale;
			deltaY = (mouseCurrentPosition.y - mouseStartPosition.y) * scale;
			if (parameterWasEdited()) isComputeRequired = true;
		}
	}

	if (isKeyboardAltering) { // arrow keys also affect editing deltas.
		if (parameterWasEdited()) isComputeRequired = true;
	}

	if (isComputeRequired)
		refresh();
}

// ================================================================================

void Widget::compute() {
	if (view.device != NULL) {			// ensure D3D11 is ready for use
		downloadSettings();				// map Widget parameter percentages to Control structure parameters
		view.UpdateControlBuffer();		// copy Control structure to GPU buffer
		view.Compute();					// dispatch compute shader & display
	}
}

// repaint Widget dialog. also calculate the image if specified
void Widget::refresh(bool alsoCompute) {
	InvalidateRect(hWnd, NULL, TRUE);
	if (alsoCompute) compute();
}

// ================================================================================

bool percentChanged(float& percent, const float changeAmount) {
	if (changeAmount == 0) return false;
	float previousPercent = percent;

	percent += changeAmount;
	if (percent < 0) percent = 0; else if (percent > 100) percent = 100;

	return percent != previousPercent;
}

#define CGSIZEY 155

void ControlGroup::draw(HDC hdc) {
	RECT rc = makeRect(x, y, GROUPWIDTH, CGSIZEY);
	FillRect(hdc, &rc, hgray1);

	drawBorderRect(hdc, x, y, GROUPWIDTH, CGSIZEY, false);

	int yhop = 25;
	int x1 = x + 5;
	int yp = y + 5;
	sliderEntry(hdc, x1, yp, param.f[CONTROL_GROUP_BASEINDEX + 0], -1, "Multiplier", focus == 0);	yp += yhop;
	sliderEntry(hdc, x1, yp, param.f[CONTROL_GROUP_BASEINDEX + 1], -1, "Stripe", focus == 1);	yp += yhop;
	sliderEntry(hdc, x1, yp, param.f[CONTROL_GROUP_BASEINDEX + 2], -1, "Escape", focus == 2);	yp += yhop;
	sliderEntry(hdc, x1, yp, param.f[CONTROL_GROUP_BASEINDEX + 3], -1, "Contrast", focus == 3);	yp += yhop;
	sliderEntry(hdc, x1, yp, param.f[CONTROL_GROUP_BASEINDEX + 4], param.f[CONTROL_GROUP_BASEINDEX + 5], "Color R,G", focus == 4); yp += yhop;
	sliderEntry(hdc, x1, yp, param.f[CONTROL_GROUP_BASEINDEX + 6], -1, "Color B", focus == 5);
}

bool ControlGroup::alter() {
	bool parameterWasAltered = false;
	switch (focus) {
	case 0:
		if (percentChanged(param.f[CONTROL_GROUP_BASEINDEX + 0], widget.deltaX)) parameterWasAltered = true;
		break;
	case 1:
		if (percentChanged(param.f[CONTROL_GROUP_BASEINDEX + 1], widget.deltaX)) parameterWasAltered = true;
		break;
	case 2:
		if (percentChanged(param.f[CONTROL_GROUP_BASEINDEX + 2], widget.deltaX)) parameterWasAltered = true;
		break;
	case 3:
		if (percentChanged(param.f[CONTROL_GROUP_BASEINDEX + 3], widget.deltaX)) parameterWasAltered = true;
		break;
	case 4:
		if (percentChanged(param.f[CONTROL_GROUP_BASEINDEX + 4], widget.deltaX)) parameterWasAltered = true;
		if (percentChanged(param.f[CONTROL_GROUP_BASEINDEX + 5], widget.deltaY)) parameterWasAltered = true;
		break;
	case 5:
		if (percentChanged(param.f[CONTROL_GROUP_BASEINDEX + 6], widget.deltaX)) parameterWasAltered = true;
		break;
	}

	return parameterWasAltered;
}

// ================================================================================

#define GGSIZEY 110

void EquationGroup::draw(HDC hdc) {
	RECT rc = makeRect(x, y, GROUPWIDTH, GGSIZEY);
	FillRect(hdc, &rc, enabled ? hgreen1 : hgray1);

	drawBorderRect(hdc, x, y, GROUPWIDTH, GGSIZEY, enabled);

	// number legend
	char str[32];
	sprintf_s(str, 31, "%d", 1 + baseIndex / NUM_FIELDS_PER_EQUATION);
	SetTextColor(hdc, RGB(0, 0, 0));
	TextOut(hdc, x + 5, y + 5, str, int(strlen(str)));

	sliderEntry(hdc, x + 5, y + 30, param.f[baseIndex + 0], param.f[baseIndex + 1], "Translate", focus == 0);
	sliderEntry(hdc, x + 5, y + 55, param.f[baseIndex + 2], param.f[baseIndex + 3], "Scale", focus == 1);
	sliderEntry(hdc, x + 5, y + 80, param.f[baseIndex + 4], -1, "Rotate", focus == 2);
}

bool EquationGroup::alter() {
	bool parameterWasAltered = false;

	switch (focus) {
	case 0:
		if (percentChanged(param.f[baseIndex + 0], widget.deltaX)) parameterWasAltered = true;
		if (percentChanged(param.f[baseIndex + 1], widget.deltaY)) parameterWasAltered = true;
		break;
	case 1:
		if (percentChanged(param.f[baseIndex + 2], widget.deltaX)) parameterWasAltered = true;
		if (percentChanged(param.f[baseIndex + 3], widget.deltaY)) parameterWasAltered = true;
		break;
	case 2:
		if (percentChanged(param.f[baseIndex + 4], widget.deltaX)) parameterWasAltered = true;
	}

	return parameterWasAltered;
}

// ================================================================================

RECT makeRect(int x, int y, int xs, int ys) {
	static RECT r;
	r.left = x;
	r.top = y;
	r.right = x + xs;
	r.bottom = y + ys;
	return r;
}

void drawBorderRect(HDC hdc, int x1, int y1, int xs, int ys, bool enabled) {
	static HPEN pgray0 = CreatePen(0, 1, gray0);
	static HPEN pgray2 = CreatePen(0, 1, gray2);
	static HPEN pgreen0 = CreatePen(0, 1, green0);
	static HPEN pgreen2 = CreatePen(0, 1, green2);

	int x2 = x1 + xs;
	int y2 = y1 + ys;

	SelectObject(hdc, enabled ? pgreen0 : pgray0);
	MoveToEx(hdc, x1, y2, 0);
	LineTo(hdc, x1, y1);
	LineTo(hdc, x2, y1);

	SelectObject(hdc, enabled ? pgreen2 : pgray2);
	MoveToEx(hdc, x1, y2, 0);
	LineTo(hdc, x2, y2);
	LineTo(hdc, x2, y1);
}

// percent ranges 0..100.  add a margin so that display line is not hidden next to bevelled border graphic
float addDisplayMargin(float percent) {
	if (percent < 8) return 8;
	if (percent > 98) return 98;
	return percent;
}

void sliderEntry(HDC hdc, int x1, int y1, float percentx, float percenty, const char* legend, bool hasFocus) {
	const int sz = 20;
	int x2 = x1 + sz;
	int y2 = y1 + sz;
	static HPEN hp = CreatePen(0, 2, RGB(0, 0, 0));

	SelectObject(hdc, hp);

	// vertical slider
	percentx = addDisplayMargin(percentx);
	int xp = x1 + int(float(sz) * percentx / 100.0f);
	MoveToEx(hdc, xp, y1, 0);
	LineTo(hdc, xp, y2);

	// horizontal slider
	if (percenty >= 0) { // value < 0 == not used
		percenty = addDisplayMargin(percenty);
		int yp = y1 + int(float(sz) * percenty / 100.0f);
		MoveToEx(hdc, x1, yp, 0);
		LineTo(hdc, x2, yp);
	}

	drawBorderRect(hdc, x1, y1, sz, sz, false);

	SetTextColor(hdc, hasFocus ? RGB(192, 0, 0) : RGB(0, 0, 0));
	TextOut(hdc, x1 + sz + 10, y1, legend, int(strlen(legend)));
}
