#include "stdafx.h"
#include "common.h"
#include "Help.h"

Help help;

static char* CLASS_NAME = (char *)"Help";

LRESULT CALLBACK HelpWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_CREATE:
		help.hWndList = CreateWindowEx(WS_EX_CLIENTEDGE, "Listbox", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_AUTOVSCROLL | LBS_NOSEL, 5, 5, 480,560, hWnd, (HMENU)101, NULL, NULL);
		CreateWindowEx(NULL, TEXT("button"), TEXT("Close"), WS_VISIBLE | WS_CHILD, 10, 565, 60, 20, hWnd, (HMENU)106, NULL, NULL);
		SendMessage(help.hWndList, WM_SETFONT, WPARAM(help.font), 0);
		help.addHelptext();
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))	{
		case 106:
			ShowWindow(hWnd, SW_HIDE);
			break;
		}
		break;
	case WM_ERASEBKGND:
	{
		RECT rc;
		HDC hdc = (HDC)wParam;
		GetClientRect(hWnd, &rc);
		FillRect(hdc, &rc, CreateSolidBrush(RGB(200, 235, 200)));
		return 1L;
	}
	break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

void Help::create(HWND parent, HINSTANCE hInstance) {
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = HelpWndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = CLASS_NAME;
	wcex.hIconSm = NULL;
	if (!RegisterClassEx(&wcex)) ABORT(-1);

	RECT rc2 = { 10, 10, 500,600 };
	AdjustWindowRect(&rc2, WS_OVERLAPPEDWINDOW, FALSE);

	font = CreateFont(14, 7, 0, 0,
		FW_NORMAL,
		FALSE, FALSE, FALSE,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_ROMAN,
		"Courier New");

	hWnd = CreateWindow(CLASS_NAME, "Help", WS_OVERLAPPED | WS_BORDER, CW_USEDEFAULT, CW_USEDEFAULT, rc2.right - rc2.left, rc2.bottom - rc2.top, parent, NULL, hInstance, NULL);
	if (hWnd == NULL) ABORT(-1);

	ShowWindow(hWnd, SW_HIDE);
}

void Help::launch() {
	ShowWindow(hWnd, SW_SHOWNORMAL);
}

// -------------------------------------------------------------------

static const char* helptext[] = {
"Note: All black screen ? Keep pressing 'R' till you get lucky.",
" ",
"MishMash Fractal",
"Collection of algorithms and UX from several other",
"fractal apps, hoping to come with a newand interesting display.",
" ",
"The control panel holds a collection of",
"one or two dimensional widgets.",
" ",
"Mouse click on a widget to select it.",
"It's legend will turn Red.",
"You can also move the focus up/down with the '<'and '>' keys.",
"The mouse scroll wheel controls also moves the focus.",
" ",
"Now on the image :",
"Mouse drag with Left Mouse Button to control widget parameters.",
"You can also use the Arrow keys.",
"Hold down the <control> key to increase the alteration amount.",
" ",
"Mouse drag with Right Mouse Button to pan the image.",
"With right mouse button pressed: scroll wheel controls Zoom.",
" ",
"Grammar String (at top of Control dialog)",
"   The app Flam3 uses a weighted random function to",
"   select which algorithm is used every cycle.",
"   This app instead uses a simple grammar, where the",
"   characters in a 1 - 12 character string specify which",
"   algorithm is used.",
"   The 'grammar string' is repeated as necessary",
"   during a drawing session.",
" ",
"Groups 1 to 4 :",
"   The grammar string selects from 4 algorithms for each cycle.",
"   These 4 groups hold all the data for each algorithm.",
"   Select the function for a group via its dropdown list,",
"   or hover the cursor, and use the mouse wheel.",
"   The functions themselves come from the Flam3 app.",
"   * Translation",
"   * Scale",
"   * Rotate affect the function output after every cycle.",
"  ", 
"   The function group gains a tinted background color when",
"   that function is included in the grammar string.",
" ",
"Coloring Parameters",
"(algorithms from 3Dickulus'",
" triangle-inequality-average-algorithm)",
" ",
"Keyboard commands:",
"R: randomize all parameters.",
"   Keep pressing until you finally get a visible image",
"Q: randoimize just the grammar string",
"S: save the current parameter settings to file",
"L: launch the saved file dialog to retrieve a saved image.",
"A: toggle 'autoChange'",
"   When active, all parameters slowly change",
"Spacebar : Toggle visiblity of Parameter panel"
"",  // must mark end of list with an empty string!
};

void Help::addHelptext() {
	int i = 0;
	for (;;) {
		const char* str = helptext[i++];
		if (strlen(str) == 0) break;
		SendMessage(hWndList, LB_ADDSTRING, 0, (LPARAM)str);
	}
}

