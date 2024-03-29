#pragma once
#include "stdafx.h"

#define MAX_ENTRIES 100

class SaveLoad {
public:
	HWND hWnd;
	HWND hWndList;
	HDC hdc;
	HFONT font;

	char data[MAX_ENTRIES][128]; // = { 0 };
	int count;

	SaveLoad() {
		hWnd = NULL;
		hWndList = NULL;
		hdc = NULL;
		font = NULL;
		count = 0;
	}

	void create(HWND parent, HINSTANCE inst);
	void launch();
	void createListBox(HWND hWnd);
	void saveParam();
	void loadParam(int index);
	void deleteSelectedEntry();

private:
	void fillListBox();
	char* createFilename();
	const char* displayString(const char* filename);
};

extern SaveLoad	saveLoad;