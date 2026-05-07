#include "graph.h"

int main(int argc, char** argv) {

	if (argc > 1) globalSz = std::atoi(argv[1]);

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