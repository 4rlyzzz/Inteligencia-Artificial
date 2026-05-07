#include "graph.h"

int main(int argc, char** argv) {

	if (argc > 1) globalSz = std::atoi(argv[1]);

	createGraph(graph, globalSz);

	int n;
	std::cout << "Nodos: ";
	std::cin >> n;
	removeNodesAndConnections(n);

	// run algorithms and store results
	callForwardChecking();
	callMoreRestrictive();
	callRestricted();

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(winW, winH);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Coloreo de Grafos");
	glutDisplayFunc(display_cb);
	glutReshapeFunc(reshape_cb);
	glutKeyboardFunc(changeScreen);
	glClearColor(0.0f, 0.0f, 0.0f, 1.f);
	glutMainLoop();
	return 0;
}