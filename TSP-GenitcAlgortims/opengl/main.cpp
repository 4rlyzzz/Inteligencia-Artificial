#include "graph.h"

int main(int argc, char** argv) {
    int n_nodes, sizePob;
    std::cout << "Nodos: ";    std::cin >> n_nodes;
    std::cout << "Poblacion: "; std::cin >> sizePob;

    generateNodes(n_nodes);

    std::vector<individual> pob = initialize(sizePob, n_nodes);
    for (auto& in : pob) evaluateFitness(in);

    int gen = 1;
    while (gen <= 200) {
        double sumFitness = 0;
        individual best = elitism(pob);

        for (const auto& ind : pob)
            sumFitness += ind.fitness;

        double avgFitness = sumFitness / sizePob;

        std::cout << "Gen " << gen << " - Best Dist: " << best.distance << " - Fitness: " << best.fitness << "\n";

        // store fitness history for the chart
        bestHistory.push_back(best.fitness);
        avgHistory.push_back(avgFitness);

        // store best route for the route panel
        {
            std::lock_guard<std::mutex> lock(route_mutex);
            bestRoute.clear();
            for (int idx : best.route)
                bestRoute.push_back(nodes[idx]);
        }

        // new population
        std::vector<individual> newPob(sizePob);
        newPob[0] = best; // elitism

        // THREADS -> one thread per pair
        std::vector<std::thread> threads;
        for (int i = 1; i < sizePob; i += 2)
            threads.emplace_back(evaluate, std::ref(pob), std::ref(newPob), i, sizePob);

        for (auto& t : threads)
            t.join();

        pob = newPob;
        gen++;
    }

    individual finalBest = elitism(pob);
    std::cout << "\nFinished at generation: " << gen << "\n";
    std::cout << "Best distance: " << finalBest.distance << " - Fitness: " << finalBest.fitness << "\n";

    // open visualization window
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
    glutInitWindowSize(winW, winH);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("TSP - Genetic Algorithm");
    glutDisplayFunc(display_cb);
    glutReshapeFunc(reshape_cb);
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, winW, 0, winH);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glutMainLoop();

    return 0;
}