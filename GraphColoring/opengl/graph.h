#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include "opengl_base.h"
#include "GraphColoring-OpenGL.h"

using vString = std::vector<std::string>;
using vCoord = std::vector<coord>;

// predetermined
int keyChange = 1;

inline void setColor(const std::string& c) {
	if (c == "red")        
		glColor3f(0.9f, 0.1f, 0.1f);
	else if (c == "green") 
		glColor3f(0.1f, 0.9f, 0.1f);
	else if (c == "blue")  
		glColor3f(0.1f, 0.1f, 0.9f);
	else                   
		glColor3f(0.5f, 0.5f, 0.5f); // no color assigned
}

// change screen
inline void changeScreen(unsigned char key, int, int) {
	if (key >= '1' && key <= '3') {
		keyChange = key - '0';
		glutPostRedisplay();
	}
}

inline void display_cb() {
	glClear(GL_COLOR_BUFFER_BIT);

	// edges
	glColor3f(0.75f, 0.75f, 0.75f);
	glLineWidth(1.0f);
	for (auto& kv : graph) {
		for (auto& nb : kv.second) {
			glBegin(GL_LINES);
			glVertex2f(toScreenX(kv.first.first), toScreenY(kv.first.second));
			glVertex2f(toScreenX(nb.first), toScreenY(nb.second));
			glEnd();
		}
	}

	// draw node coordinates
	glColor3f(0.9f, 0.9f, 0.9f);
	for (auto& kv : graph) {
		char buf[1000];
		sprintf(buf, "%d,%d", kv.first.first, kv.first.second);
		drawText(toScreenX(kv.first.first) + 7, toScreenY(kv.first.second), buf);
	}

	// select algorithm
	std::map<coord, std::string>* res;
	const char* name;

	if (keyChange == 1) { 
		res = &resForward;     
		name = "Forward Checking"; 
	}
	else if (keyChange == 2) { 
		res = &resRestrictive;  
		name = "More Restrictive"; 
	}
	else { 
		res = &resRestricted;   
		name = "More Restricted"; 
	}

	// draw nodes with their assigned color
	for (auto& kv : graph) {
		setColor((*res)[kv.first]);
		drawCircle(toScreenX(kv.first.first), toScreenY(kv.first.second), 6.0f);
	}

	// info
	glColor3f(0.9f, 0.9f, 0.9f);
	char buf[1000];
	sprintf(buf, "Algorithm: %s  - [1]Forward [2]Restrictive [3]Restricted", name);
	drawText(10, 10, buf);

	glutSwapBuffers();
}