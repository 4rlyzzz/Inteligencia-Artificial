#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include "opengl_base.h"
#include "geneticAlgothim-threads.h"


// fitness history per generation
inline std::vector<double> bestHistory;
inline std::vector<double> avgHistory;


void display_cb() {
	glClear(GL_COLOR_BUFFER_BIT);

	int n = (int)bestHistory.size();
	if (n == 0) { glutSwapBuffers(); return; }

	float padL = 70, padR = 20, padB = 60, padT = 30;
	float chartW = winW - padL - padR;
	float chartH = winH - padB - padT;

	// max of BOTH curves combined
	double maxVal = 0;
	for (int i = 0; i < n; i++) {
		if (bestHistory[i] > maxVal) 
			maxVal = bestHistory[i];
		if (avgHistory[i] > maxVal) 
			maxVal = avgHistory[i];
	}
	if (maxVal == 0) { 
		glutSwapBuffers(); 
		return; 
	}
	if (maxVal == 0) { 
		glutSwapBuffers(); 
		return; 
	}
	double minVal = *std::min_element(bestHistory.begin(), bestHistory.end());

	// axes
	glColor3f(0.8f, 0.8f, 0.8f);
	glLineWidth(1.5f);
	glBegin(GL_LINES);
	glVertex2f(padL, padB);
	glVertex2f(padL, padB + chartH); // Y axis
	glVertex2f(padL, padB);
	glVertex2f(padL + chartW, padB); // X axis
	glEnd();

	// Y axis labels
	glColor3f(0.9f, 0.9f, 0.9f);
	int ySteps = 5;
	for (int i = 0; i <= ySteps; i++) {
		float val = (float)maxVal * i / ySteps;
		float y = padB + (float)i / ySteps * chartH;

		// grid line
		glColor3f(0.25f, 0.25f, 0.25f);
		glBegin(GL_LINES);
		glVertex2f(padL, y);
		glVertex2f(padL + chartW, y);
		glEnd();

		// label
		glColor3f(0.9f, 0.9f, 0.9f);
		char buf[16];
		sprintf(buf, "%.0f", val);
		drawText(5, y - 5, buf);
	}

	// X axis labels + grid
	int step = std::max(1, n / 10);
	for (int i = 0; i < n; i += step) {
		float x = padL + (float)i / (n - 1) * chartW;

		glColor3f(0.25f, 0.25f, 0.25f);
		glBegin(GL_LINES);
		glVertex2f(x, padB);
		glVertex2f(x, padB + chartH);
		glEnd();

		glColor3f(0.9f, 0.9f, 0.9f);
		char buf[8];
		sprintf(buf, "%d", i);
		drawText(x - 5, padB - 15, buf);
	}

	// axis titles
	glColor3f(0.9f, 0.9f, 0.9f);
	drawText(padL + chartW / 2 - 30, 10, "Generation");
	drawText(5, padB + chartH / 2, "Fitness");

	// best curve — red
	glColor3f(1.f, 0.3f, 0.3f);
	glLineWidth(2.5f);
	glBegin(GL_LINE_STRIP);
	for (int i = 0; i < n; i++) {
		float x = padL + (float)i / (n - 1) * chartW;
		float y = padB + (float)((bestHistory[i] - minVal) / (maxVal - minVal)) * chartH; //float y = padB + (float)(bestHistory[i] / maxVal) * chartH;
		glVertex2f(x, y);
	}
	glEnd();

	// avg curve — green
	glColor3f(0.3f, 1.f, 0.3f);
	glLineWidth(2.5f);
	glBegin(GL_LINE_STRIP);
	for (int i = 0; i < n; i++) {
		float x = padL + (float)i / (n - 1) * chartW;
		float y = padB + (float)((avgHistory[i] - minVal) / (maxVal - minVal)) * chartH; //float y = padB + (float)(avgHistory[i] / maxVal) * chartH;
		glVertex2f(x, y);
	}
	glEnd();

	// legend
	glColor3f(1.f, 0.3f, 0.3f);
	drawText(padL + 10, padB + chartH - 20, "Best");
	glColor3f(0.3f, 1.f, 0.3f);
	drawText(padL + 10, padB + chartH - 35, "Avg");



	// table header
	glColor3f(0.9f, 0.9f, 0.9f);
	drawText(winW - 185, padB + chartH, "Gen   Best    Avg");

	// horizontal line under header
	glColor3f(0.5f, 0.5f, 0.5f);
	glBegin(GL_LINES);
	glVertex2f(winW - 190, padB + chartH - 8);
	glVertex2f(winW - 10, padB + chartH - 8);
	glEnd();

	// rows — one per generation
	float rowH = 13;
	int maxRows = (int)(chartH / rowH) - 1;
	int startRow = std::max(0, n - maxRows); // show last N if too many

	for (int i = startRow; i < n; i++) {
		float y = padB + chartH - 20 - (i - startRow + 1) * rowH;
		if (y < padB) break;

		glColor3f(0.8f, 0.8f, 0.8f);
		char buf2[64];
		sprintf(buf2, "%-5d %-7.0f %.0f", i, bestHistory[i], avgHistory[i]);
		drawText(winW - 185, y, buf2);
	}



	glutSwapBuffers();
}