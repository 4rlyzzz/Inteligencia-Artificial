#include "graph.h"

int main(int argc, char** argv) {
    int sizePob;
    std::cout << "size: ";
    std::cin >> sizePob;

    // init
    std::vector<individual> pob = initialize(sizePob);

    individual oldBest;
    int cont = 0;
    int gen = 1;

    while (cont < 10 && gen < 1000) {
        printPob(pob);

        individual best = elitism(pob);
        printBest(best, gen);

        if (best.x == oldBest.x && best.y == oldBest.y) cont++;
        else cont = 0;

        oldBest = best;


        // GRAPHC -> store fitness history for the chart
        bestHistory.push_back(best.fitness);
        double avg = 0;
        for (auto& in : pob) 
            avg += in.fitness;
        avgHistory.push_back(avg / sizePob);

        std::cout << "Gen " << gen << " - Best: " << best.fitness << " - Avg: " << avg / sizePob << "\n";


        // new population
        std::vector<individual> newPob(sizePob);
        newPob[0] = best; // elistim


        // THREADS -> one thread per pair 
        std::vector<std::thread> threads;
        for (int i = 1; i < sizePob; i += 2)
            threads.emplace_back(evaluate, std::ref(pob), std::ref(newPob), i, sizePob);
            //threads.push_back(std::thread(evaluate, std::ref(pob), std::ref(newPob), i, sizePob));

        for (auto& t : threads)
            t.join();


        pob = newPob;
        gen++;
    }

    std::cout << "\nI end in the generation: " << gen << "\n";
    std::cout << "Best individual: "; printBest(oldBest, gen);

    // open fitness chart window
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
    glutInitWindowSize(winW, winH);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Genetic Algorithm");
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