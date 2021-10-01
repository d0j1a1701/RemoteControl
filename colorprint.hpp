#pragma once

#include <windows.h>
#include <iostream>
#include <stdio.h>

struct color {
	static const int black = 0, blue = 1, green = 2, lightgreen = 3, red = 4, purple = 5, yellow = 6, white = 7, gray = 8, lightblue = 9, cleargreen = 10, clearlightgreen = 11, lightred = 12, lightpurple = 13, lightyellow = 14, lightwhite = 15;
	int col;
	HANDLE handle;

	color(int c = 7) {
		handle = GetStdHandle(STD_OUTPUT_HANDLE);
		col = c;
	}

	friend std::ostream &operator<<(std::ostream &out, color c) {
		SetConsoleTextAttribute(c.handle, FOREGROUND_INTENSITY | c.col);
		return out;
	}

	static void clear() {
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | 7);
	}
};

color clearcolor(7);