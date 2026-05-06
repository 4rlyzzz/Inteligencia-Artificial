#ifdef __APPLE__
# include <OpenGL/gl.h>
# include <OpenGL/glu.h>
# include <GLUT/glut.h>
#else
# include <GL/gl.h>
# include <GL/glu.h>
# include <GL/glut.h>
#endif

////////////

#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <stack>
#include <queue>
#include <cmath>
#include <algorithm>

using coord = std::pair<int, int>;

// globals
int globalSz = 20;
std::map<coord, std::vector<coord>> graph;
std::map<coord, std::vector<coord>> originalGraph;
coord startNode = { 10, 0 };
coord endNode   = { 60, 80 };

// final path
std::vector<coord> resDFS, resBFS, resHill, resAStar;

// visited nodes
std::set<coord> visitedDFS, visitedBFS, visitedHill, visitedAStar;

// predetermined
int keyChange = 1;

// screen size
int winW = 800, winH = 700;
const int PAD = 60; // margin

float toScreenX(int x) {
	return PAD + (float)x / ((globalSz - 1) * 10) * (winW - 2 * PAD);
}
float toScreenY(int y) {
	return PAD + (float)y / ((globalSz - 1) * 10) * (winH - 2 * PAD);
}

// -- createGraph --
void createGraph(std::map<coord, std::vector<coord>>& graph, int sz) 
{
	int size = sz * 10;
	int x = 0, y = 0;
	int dx[] = { 0, 10, 10, 10 };
	int dy[] = { 10,  0, 10, -10 };
	
	for (int i = 0; i < sz * sz; i++) 
	{
		for (int k = 0; k < 4; k++) {
			int tpX = x + dx[k], tpY = y + dy[k];
			if (tpX >= 0 && tpX < size && tpY >= 0 && tpY < size)
				graph[{x, y}].push_back({ tpX, tpY });
			
			tpX = x - dx[k]; tpY = y - dy[k];
			if (tpX >= 0 && tpX < size && tpY >= 0 && tpY < size)
				graph[{x, y}].push_back({ tpX, tpY });
		}
		y += 10;
		if (y >= size) 
		{ 
			y = 0; 
			x += 10; 
		}
	}

	/*
	// BEGIN DEBUG: graph
	for (auto i : graph)
	{
		std::cout << "nodo [" << i.first.first << ", " << i.first.second << "] -> ";

		for (auto j : i.second)
			std::cout << " (" << j.first << "," << j.second << ")";

		std::cout << "\n";
	}
	std::cout << "\n";
	// END DEBUG
	*/
}

// -- dfs --
std::vector<coord> dfs(std::map<coord, std::vector<coord>>& graph, int x, int y, int fin1, int fin2) 
{
	std::set<coord>   visited;
	std::stack<coord> nose;
	std::vector<coord> copiaParent;
	std::map<coord, std::vector<coord>> parent;
	
	coord ini = { x, y };
	coord fin = { fin1, fin2 };
	
	visited.insert(ini);
	nose.push(ini);
	parent[ini] = { ini };
	bool found = false;
	
	while (!nose.empty()) 
	{
		coord temp = nose.top(); nose.pop();
		if (temp == fin) 
		{ 
			found = true; 
			break; 
		}

		for (auto neigh : graph[temp]) 
		{
			if (visited.find(neigh) == visited.end()) 
			{
				nose.push(neigh);
				visited.insert(neigh);
				copiaParent = parent[temp];
				copiaParent.push_back(neigh);
				parent[neigh] = copiaParent;
				copiaParent.clear();
			}
		}
	}

	visitedDFS = visited;

	if (found) 
		return parent[fin];

	return {};
}

// -- bfs --
std::vector<coord> bfs(std::map<coord, std::vector<coord>>& graph, int x, int y, int fin1, int fin2) 
{
	std::set<coord>   visited;
	std::queue<coord> nose;
	std::vector<coord> copiaParent;
	std::map<coord, std::vector<coord>> parent;
   
	coord ini = { x, y };
	coord fin = { fin1, fin2 };
   
	visited.insert(ini);
	nose.push(ini);
	parent[ini] = { ini };
	bool found = false;
   
	while (!nose.empty()) 
	{
		coord temp = nose.front(); nose.pop();
		if (temp == fin) 
		{ 
			found = true; 
			break; 
		}

		for (auto neigh : graph[temp]) 
		{
			if (visited.find(neigh) == visited.end()) 
			{
				visited.insert(neigh);
				nose.push(neigh);
				copiaParent = parent[temp];
				copiaParent.push_back(neigh);
				parent[neigh] = copiaParent;
				copiaParent.clear();
			}
		}
	}

	visitedBFS = visited;

	if (found) return parent[fin];

	return {};
}

// -- hillClimbing --
struct compHill 
{
  	bool operator()(const std::pair<coord, double>& a, const std::pair<coord, double>& b) 
	{
		return a.second > b.second;
	}
};

double dist(int a1, int a2, int b1, int b2) 
{
	return std::sqrt(std::pow(b1 - a1, 2) + std::pow(b2 - a2, 2));
}

std::vector<coord> hillClimbing(std::map<coord, std::vector<coord>>& graph, int x, int y, int fin1, int fin2) 
{
	std::priority_queue<std::pair<coord, double>,
	std::vector<std::pair<coord, double>>,
	compHill> nose;
	std::set<coord> visited;
	std::vector<coord> copiaParent;
	std::map<coord, std::vector<coord>> parent;
  
	coord ini = { x, y };
	coord fin = { fin1, fin2 };
  
	nose.push({ ini, dist(ini.first, ini.second, fin.first, fin.second) });
	visited.insert(ini);
	parent[ini] = { ini };
	bool found = false;
  
	while (!nose.empty()) 
	{
		std::pair<coord, double> temp = nose.top();
		coord tempCoord = temp.first;
		nose.pop();
		if (tempCoord == fin) 
		{ 
			found = true; 
			break; 
		}
		for (auto neigh : graph[tempCoord]) {
			if (visited.find(neigh) == visited.end()) 
			{
				nose.push({ neigh, dist(neigh.first, neigh.second, fin.first, fin.second) });
				visited.insert(neigh);
				copiaParent = parent[tempCoord];
				copiaParent.push_back(neigh);
				parent[neigh] = copiaParent;
				copiaParent.clear();
			}
		}
	}

	visitedHill = visited;

	if (found) return parent[fin];

	return {};
}
							  
// -- aStar --
struct compStar 
{
	bool operator()(const std::pair<coord, std::pair<double, double>>& a, std::pair<coord, std::pair<double, double>>& b) 
		return (a.second.first + a.second.second) > (b.second.first + b.second.second);
};

std::vector<coord> aStar(std::map<coord, std::vector<coord>>& graph, int x, int y, int fin1, int fin2) 
{
	std::priority_queue<std::pair<coord, std::pair<double, double>>,
	std::vector<std::pair<coord, std::pair<double, double>>>,
	compStar> nose;
	std::set<coord> visited;
	std::vector<coord> copiaParent;
	std::map<coord, std::vector<coord>> parent;
  
	coord ini = { x, y };
	coord fin = { fin1, fin2 };
  
	nose.push({ ini, { 0, dist(ini.first, ini.second, fin.first, fin.second) } });
	visited.insert(ini);
	parent[ini] = { ini };
	bool found = false;
  
	while (!nose.empty()) {
		std::pair<coord, std::pair<double, double>> temp = nose.top();
		coord tempCoord = temp.first;
		nose.pop();
		if (tempCoord == fin) 
		{ 
			found = true; 
			break; 
		}

		for (auto neigh : graph[tempCoord]) 
		{
			if (visited.find(neigh) == visited.end()) 
			{
				visited.insert(neigh);
				double g = temp.second.first + dist(tempCoord.first, tempCoord.second, neigh.first, neigh.second);
				double h = dist(neigh.first, neigh.second, fin.first, fin.second);
				nose.push({ neigh, { g, h } });
				copiaParent = parent[tempCoord];
				copiaParent.push_back(neigh);
				parent[neigh] = copiaParent;
				copiaParent.clear();
			}
		}	
	}

	visitedAStar = visited;

	if (found) return parent[fin];

	return {};
}

// draw helpers
void drawCircle(float cx, float cy, float r, int segs = 12) 
{
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(cx, cy);
	for (int i = 0; i <= segs; i++) 
	{
		float a = 2.0f * 3.14159265f * i / segs;
		glVertex2f(cx + r * std::cos(a), cy + r * std::sin(a));
	}
	glEnd();
}

void drawPath(const std::vector<coord>& path, float r, float g, float b, float lw, float offset = 0.f) 
{
	if (path.size() < 2) 
		return;

	glColor3f(r, g, b);
	glLineWidth(lw);
	glBegin(GL_LINE_STRIP);

	for (auto& c : path)
		glVertex2f(toScreenX(c.first) + offset, toScreenY(c.second) + offset);

	glEnd();
}

 void drawText(float x, float y, const char* text) 
 {
	glRasterPos2f(x, y);
	while (*text) 
		glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *text++);
 }
 
// change screen
void changeScreen(unsigned char key, int, int) 
{
	if (key >= '1' && key <= '5') 
	{
		keyChange = key - '0';
		glutPostRedisplay();
	}

	if (key == 'r' || key == 'R') 
	{
		graph = originalGraph;
		removeNodes(graph.size() * 0.8f); // deja el 80%
		
		resDFS  = dfs(graph, startNode.first, startNode.second, endNode.first, endNode.second);
		resBFS  = bfs(graph, startNode.first, startNode.second, endNode.first, endNode.second);
		resHill = hillClimbing(graph, startNode.first, startNode.second, endNode.first, endNode.second);
		resAStar = aStar(graph, startNode.first, startNode.second, endNode.first, endNode.second);
		
		glutPostRedisplay();
	}
}
// select position
void selectPosition(int button, int state, int mx, int my) 
{
	if (state != GLUT_DOWN) return;
	
	// convert grid limits to pixels 
	float gridMinX = toScreenX(0);
	float gridMaxX = toScreenX((globalSz - 1) * 10);
	float gridMinY = toScreenY(0);
	float gridMaxY = toScreenY((globalSz - 1) * 10);
		
	if (mx < gridMinX || mx > gridMaxX || my < gridMinY || my > gridMaxY) 
		return;
	
	int bestX = 0, bestY = 0;
	float bestDist = 999999.f;
	
	for (auto& kv : graph) 
	{
		// convert nodes into pixels
		float sx = toScreenX(kv.first.first);
		float sy = toScreenY(kv.first.second);
		
		// calculate distance between -> click - nodes
		float dx = sx - mx;
		float dy = sy - (winH - my); // OpenGL Y es invertido
		float d  = dx*dx + dy*dy;
		
		// nearest node
		if (d < bestDist) 
		{
			bestDist = d;
			bestX = kv.first.first;
			bestY = kv.first.second;
		}
	}
	
	// asignation
	if (button == GLUT_LEFT_BUTTON)
		startNode = { bestX, bestY };
	if (button == GLUT_RIGHT_BUTTON) 
		endNode = { bestX, bestY };
	
	// recalculate algorithms with the new points
	resDFS = dfs(graph, startNode.first, startNode.second, endNode.first, endNode.second);
	resBFS = bfs(graph, startNode.first, startNode.second, endNode.first, endNode.second);
	resHill = hillClimbing(graph, startNode.first, startNode.second, endNode.first, endNode.second);
	resAStar = aStar(graph, startNode.first, startNode.second, endNode.first, endNode.second);
	
	glutPostRedisplay();
}
// glut - zingjal
void reshape_cb(int w, int h) 
{
	if (w == 0 || h == 0) 
		return;

	winW = w; winH = h;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, w, 0, h);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}
 
void display_cb() 
{
	glClear(GL_COLOR_BUFFER_BIT);
	 
	// edges
	glColor3f(0.75f, 0.75f, 0.75f);
	glLineWidth(1.0f);
	for (auto& kv : graph) 
	{
		for (auto& nb : kv.second) 
		{
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
	
	if (keyChange == 1) 
	{ 
		visited=&visitedDFS;   
		path=&resDFS;   
		r=1.f; 
		g=0.3f; 
		b=0.3f; 
		name="DFS"; 
	}
	else if (keyChange == 2) 
	{ 
		visited=&visitedBFS;   
		path=&resBFS;   
		r=0.2f; 
		g=0.9f; 
		b=0.2f; 
		name="BFS"; 
	}
	else if (keyChange == 3)
	{ 
		visited=&visitedHill;  
		path=&resHill; 
		r=1.f; 
		g=0.6f; 
		b=0.f;  
		name="Hill Climbing"; 
	}
	else if (keyChange == 4) 
	{
		visited=&visitedAStar; 
		path=&resAStar; 
		r=0.f; 
		g=0.8f; 
		b=1.f;  
		name="A*"; 
	}
	else 
	{
		// draw algorithms path
		drawPath(resDFS,   1.0f, 0.3f, 0.3f, 3.0f);
		drawPath(resBFS,   0.2f, 0.9f, 0.2f, 3.0f);
		drawPath(resHill,  1.0f, 0.6f, 0.0f, 3.0f);
		drawPath(resAStar, 0.0f, 0.8f, 1.0f, 3.5f);
		
		// point -> start - end
		glColor3f(0.1f, 0.9f, 0.1f); // verde
		drawCircle(toScreenX(startNode.first), toScreenY(startNode.second), 9.0f);
		glColor3f(0.9f, 0.1f, 0.1f); // rojo
		drawCircle(toScreenX(endNode.first), toScreenY(endNode.second), 9.0f);
		
		glColor3f(0.9f, 0.9f, 0.9f);
		drawText(10, 10, "Todos los algoritmos -> DFS(Rojo) - BFS(Rojo) - HillClimbing(Naranja) - A*(Celeste)");
		
		glutSwapBuffers();
		return;
	}
	
	// visited nodes
	for (auto& c : *visited) 
	{
		glColor3f(r * 0.6f, g * 0.6f, b * 0.6f);
		drawCircle(toScreenX(c.first), toScreenY(c.second), 4.5f);
	}
	
	// final path
	drawPath(*path, r, g, b, 4.0f);
	
	// point -> start - end
	glColor3f(0.1f, 0.9f, 0.1f); // verde
	drawCircle(toScreenX(startNode.first), toScreenY(startNode.second), 9.0f);
	glColor3f(0.9f, 0.1f, 0.1f); // rojo
	drawCircle(toScreenX(endNode.first), toScreenY(endNode.second), 9.0f);
	
	// info
	glColor3f(0.9f, 0.9f, 0.9f);
	char buf[64];
	sprintf(buf, "Algoritmo: %s  [1]DFS [2]BFS [3]Hill [4]A*", name);
	drawText(10, 26, buf);
	sprintf(buf, "Visitados: %d   Camino: %d", visited->size(), path->size());
	drawText(10, 10, buf);
	
	glutSwapBuffers();
}

///////////////////////////////////////////////////////////////
// -- main --
int main(int argc, char** argv) 
{
	if (argc > 1) 
		globalSz = std::atoi(argv[1]);
 
	createGraph(graph, globalSz);
	originalGraph = graph; 
 
	resDFS = dfs(graph, 10, 0, 60, 80);
	resBFS = bfs(graph, 10, 0, 60, 80);
	resHill = hillClimbing(graph, 10, 0, 60, 80);
	resAStar = aStar(graph, 10, 0, 60, 80);
 
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(winW, winH);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Busquedas - Heuristicas");
	glutDisplayFunc(display_cb);
	glutReshapeFunc(reshape_cb);
	glutKeyboardFunc(changeScreen);
	glutMouseFunc(selectPosition);
	glClearColor(0.0f, 0.0f, 0.0f, 1.f);
	glutMainLoop();
	return 0;
}
