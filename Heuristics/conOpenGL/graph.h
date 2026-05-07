#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include "opengl_base.h"
#include "heuristicsSearch-OpenGL.h"

// final path
inline std::vector<coord> resDFS, resBFS, resHill, resAStar;

coord startNode = { 10, 0 };
coord endNode = { 60, 80 };

// predetermined
int keyChange = 1;
inline const float percentage = 0.20f;

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

	// select algorithm
	std::set<coord>* visited;
	std::vector<coord>* path;
	float r, g, b;
	const char* name;

	if (keyChange == 1) {
		visited = &visitedDFS;  path = &resDFS;
		r = 1.f; g = 0.3f; b = 0.3f; name = "DFS";
	}
	else if (keyChange == 2) {
		visited = &visitedBFS;  path = &resBFS;
		r = 0.2f; g = 0.9f; b = 0.2f; name = "BFS";
	}
	else if (keyChange == 3) {
		visited = &visitedHill; path = &resHill;
		r = 1.f; g = 0.6f; b = 0.f; name = "Hill Climbing";
	}
	else if (keyChange == 4) {
		visited = &visitedAStar; path = &resAStar;
		r = 0.f; g = 0.8f; b = 1.f; name = "A*";
	}
	else {
		// draw all paths
		drawPath(resDFS, 1.0f, 0.3f, 0.3f, 3.0f);
		drawPath(resBFS, 0.2f, 0.9f, 0.2f, 3.0f);
		drawPath(resHill, 1.0f, 0.6f, 0.0f, 3.0f);
		drawPath(resAStar, 0.0f, 0.8f, 1.0f, 3.5f);

		glColor3f(0.1f, 0.9f, 0.1f);
		drawCircle(toScreenX(startNode.first), toScreenY(startNode.second), 9.0f);
		glColor3f(0.9f, 0.1f, 0.1f);
		drawCircle(toScreenX(endNode.first), toScreenY(endNode.second), 9.0f);

		glColor3f(0.9f, 0.9f, 0.9f);
		drawText(10, 10, "Todos los algoritmos -> DFS(Rojo) - BFS(Verde) - HillClimbing(Naranja) - A*(Celeste)");

		glutSwapBuffers();
		return;
	}

	// visited nodes
	for (auto& c : *visited) {
		glColor3f(r * 0.6f, g * 0.6f, b * 0.6f);
		drawCircle(toScreenX(c.first), toScreenY(c.second), 4.5f);
	}

	// final path
	drawPath(*path, r, g, b, 4.0f);

	// start - end points
	glColor3f(0.1f, 0.9f, 0.1f);
	drawCircle(toScreenX(startNode.first), toScreenY(startNode.second), 9.0f);
	glColor3f(0.9f, 0.1f, 0.1f);
	drawCircle(toScreenX(endNode.first), toScreenY(endNode.second), 9.0f);

	// info
	glColor3f(0.9f, 0.9f, 0.9f);
	char buf[1000];
	sprintf(buf, "Algoritmo: %s  [1]DFS [2]BFS [3]Hill [4]A*", name);
	drawText(10, 26, buf);
	sprintf(buf, "Visitados: %zu   Camino: %zu", visited->size(), path->size());
	drawText(10, 10, buf);

	glutSwapBuffers();
}

// change screen
inline void changeScreen(unsigned char key, int, int) {
	if (key >= '1' && key <= '5') {
		keyChange = key - '0';
		glutPostRedisplay();
	}
	if (key == 'r' || key == 'R') {
		graph = originalGraph;
		removeNodes(graph, percentage);

		// recalculate
		resDFS = dfs(graph, startNode.first, startNode.second, endNode.first, endNode.second);
		resBFS = bfs(graph, startNode.first, startNode.second, endNode.first, endNode.second);
		resHill = hillClimbing(graph, startNode.first, startNode.second, endNode.first, endNode.second);
		resAStar = aStar(graph, startNode.first, startNode.second, endNode.first, endNode.second);


		glutPostRedisplay();
	}
}

// select position with mouse click
inline void selectPosition(int button, int state, int mx, int my) {
	if (state != GLUT_DOWN) return;

	float gridMinX = toScreenX(0);
	float gridMaxX = toScreenX((globalSz - 1) * 10);
	float gridMinY = toScreenY(0);
	float gridMaxY = toScreenY((globalSz - 1) * 10);

	if (mx < gridMinX || mx > gridMaxX || my < gridMinY || my > gridMaxY)
		return;

	int bestX = 0, bestY = 0;
	float bestDist = 999999.f;

	for (auto& kv : graph) {
		float sx = toScreenX(kv.first.first);
		float sy = toScreenY(kv.first.second);

		// calculate distance between click and nodes
		float dx = sx - mx;
		float dy = sy - (winH - my); // OpenGL Y is inverted
		float d = dx * dx + dy * dy;

		// nearest node
		if (d < bestDist) {
			bestDist = d;
			bestX = kv.first.first;
			bestY = kv.first.second;
		}
	}

	if (button == GLUT_LEFT_BUTTON)  startNode = { bestX, bestY };
	if (button == GLUT_RIGHT_BUTTON) endNode = { bestX, bestY };

	// recalculate with new points
	resDFS = dfs(graph, startNode.first, startNode.second, endNode.first, endNode.second);
	resBFS = bfs(graph, startNode.first, startNode.second, endNode.first, endNode.second);
	resHill = hillClimbing(graph, startNode.first, startNode.second, endNode.first, endNode.second);
	resAStar = aStar(graph, startNode.first, startNode.second, endNode.first, endNode.second);

	glutPostRedisplay();
}