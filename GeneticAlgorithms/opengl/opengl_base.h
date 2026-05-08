#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>
#include <GL/glut.h>

#include <iostream>
#include <bitset>
#include <vector>
#include <random>
#include <thread>
#include <mutex>

using coord = std::pair<int, int>;

// screen size
inline int winW = 800, winH = 700;
inline const int PAD = 60;

inline int globalSz = 20;

inline float toScreenX(int x) {
	return PAD + (float)x / ((globalSz - 1) * 10) * (winW - 2 * PAD);
}

inline float toScreenY(int y) {
	return PAD + (float)y / ((globalSz - 1) * 10) * (winH - 2 * PAD);
}

inline void drawCircle(float cx, float cy, float r, int segs = 12) {
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(cx, cy);
	for (int i = 0; i <= segs; i++) {
		float a = 2.0f * 3.14159265f * i / segs;
		glVertex2f(cx + r * std::cos(a), cy + r * std::sin(a));
	}
	glEnd();
}

inline void drawPath(const std::vector<coord>& path, float r, float g, float b, float lw, float offset = 0.f) {
	if (path.size() < 2) return;
	glColor3f(r, g, b);
	glLineWidth(lw);
	glBegin(GL_LINE_STRIP);
	for (auto& c : path)
		glVertex2f(toScreenX(c.first) + offset, toScreenY(c.second) + offset);
	glEnd();
}

inline void drawText(float x, float y, const char* text) {
	glRasterPos2f(x, y);
	while (*text) glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *text++);
}

inline void reshape_cb(int w, int h) {
	if (w == 0 || h == 0) return;
	winW = w; winH = h;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, w, 0, h);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}