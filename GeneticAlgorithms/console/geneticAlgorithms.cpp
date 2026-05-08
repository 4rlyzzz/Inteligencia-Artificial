#include <iostream>
#include <bitset>
#include <vector>
#include <random>

int size_tournament = 3; // 2;
float prob_mut = 0.05;
int contPob = 0;

std::mt19937 rng(time(nullptr));

int randInt(int start, int end) {
    // FORMA 1
    // std::uniform_int_distribution<int> dist(start, end);
    // return dist(rng);

    // FORMA 2
    return std::uniform_int_distribution<int>(start, end)(rng);
}

double randDouble() {
    return std::uniform_real_distribution<double>(0.0, 1.0)(rng);
}

// each individual has two genes: x (5 bits) and y (6 bits), initialized randomly
struct individual {
    std::bitset<5> x; // 0-31
    std::bitset<6> y; // 0-63
    int fitness = 0;

    individual() {
        for (int i = 0; i < 5; i++)
            x[i] = randInt(0, 1);

        for (int i = 0; i < 6; i++)
            y[i] = randInt(0, 1);
    }
};

// f(x,y) = x*x - 2xy + y*y
void evaluateFitness(individual& in) {
    int valueX = in.x.to_ulong();
    int valueY = in.y.to_ulong();

    in.fitness = valueX * valueX - 2 * valueX * valueY + valueY * valueY;
}

// random population, all evaluated
std::vector<individual> initialize(int tam) {
    std::vector<individual> pob(tam);

    for (auto& in : pob)
        evaluateFitness(in);

    return pob;
}

// choose the lower fitness among random candidates
individual tournament(std::vector<individual>& pob) {
    individual candidato = pob[randInt(0, pob.size() - 1)];

    for (int i = 1; i < size_tournament; i++) {
        individual temp = pob[randInt(0, pob.size() - 1)];

        if (temp.fitness < candidato.fitness)
            candidato = temp;
    }
    return candidato;
}

template<size_t N>
void printCross(std::bitset<N>& p1, std::bitset<N>& p2, std::bitset<N>& h1, std::bitset<N>& h2, char c, int cut) {
    std::cout << "\n------- " << c << " -------";

    std::cout << "\nP1: ";
    for (int i = 0; i < N; i++)
        std::cout << p1[i] << " ";

    std::cout << "\nP2: ";
    for (int i = 0; i < N; i++)
        std::cout << p2[i] << " ";

    std::cout << "\n--- Crossover in " << cut << " ---";
    std::cout << "\nH1: ";
    for (int i = 0; i < N; i++)
        std::cout << h1[i] << " ";

    std::cout << "\nH2: ";
    for (int i = 0; i < N; i++)
        std::cout << h2[i] << " ";

    std::cout << "\n------------------\n";
}

// single-point crossover -> applied separately to x and y genes
void crossover(individual& p1, individual& p2, individual& h1, individual& h2) {
    h1.x = p1.x; h1.y = p1.y;
    h2.x = p2.x; h2.y = p2.y;

    // CROOSOVER -> X
    int cutX = randInt(1, 4);
    for (int i = cutX; i < 5; i++) {
        h1.x[i] = p2.x[i];
        h2.x[i] = p1.x[i];
    }
    printCross(p1.x, p2.x, h1.x, h2.x, 'X', cutX);

    // CROOSOVER -> Y
    int cutY = randInt(1, 5);
    for (int i = cutY; i < 6; i++) {
        h1.y[i] = p2.y[i];
        h2.y[i] = p1.y[i];
    }
    printCross(p1.y, p2.y, h1.y, h2.y, 'Y', cutY);

    // evaluateFitness(h1);
    // evaluateFitness(h2);
}

template<size_t N>
void printMut(std::bitset<N>& h, char c, int mut, int n) {
    std::cout << "\n--- Mutation " << c << " in " << mut << " ---";

    std::cout << "\nH" << n << ": ";
    for (int i = 0; i < N; i++)
        std::cout << h[i] << " ";

    std::cout << "\n";
}

// bit-flip mutation —> each bit flips with probability prob_mut
void mutation(individual& in, int n) {
    // MUTATION -> X
    for (int i = 0; i < 5; i++) {
        if (randDouble() < prob_mut) {
            in.x.flip(i);
            printMut(in.x, 'X', i, n);
        }
    }

    // MUTATION -> Y
    for (int i = 0; i < 6; i++) {
        if (randDouble() < prob_mut) {
            in.y.flip(i);
            printMut(in.y, 'Y', i, n);
        }
    }
    // evaluateFitness(in);
}

// elitism —> always carries the best individual to the next generation
individual elitism(std::vector<individual>& pob) {
    individual mejor = pob[0];

    for (int i = 1; i < pob.size(); i++) {
        if (pob[i].fitness < mejor.fitness)
            mejor = pob[i];
    }
    return mejor;
}

void printBest(const individual& in, int generacion) {
    std::cout << "Best gen " << generacion
        << ": x = " << in.x.to_ulong() << " (" << in.x << ")"
        << " - y = " << in.y.to_ulong() << " (" << in.y << ")"
        << " - fitness = " << in.fitness << "\n";
}

void printPob(const std::vector<individual>& pob) {
    std::cout << "\n--------------- POPULATION " << contPob << " ---------------" << std::endl;

    for (size_t i = 0; i < pob.size(); ++i) {
        std::cout << "in [" << i << "]: x=" << pob[i].x
            << " (" << pob[i].x.to_ulong() << ") | y="
            << pob[i].y << " (" << pob[i].y.to_ulong() << ") | fit="
            << pob[i].fitness << std::endl;
    }
    std::cout << "---------------------------------------------" << std::endl;
    contPob++;
}

// generation step: selection -> crossover -> mutation -> evaluate
void evaluate(std::vector<individual>& pob, std::vector<individual>& newPob, int i, int sizePob) {

    // select
    individual p1 = tournament(pob);
    individual p2 = tournament(pob);

    individual h1, h2;

    // crossover
    crossover(p1, p2, h1, h2);

    // mutation
    mutation(h1, 1);
    mutation(h2, 2);

    evaluateFitness(h1);
    evaluateFitness(h2);

    newPob[i] = h1;
    if (i + 1 < sizePob)
        newPob[i + 1] = h2;
}

int main() {
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

        std::vector<individual> newPob(sizePob);
        newPob[0] = best; // elistim

        for (int i = 1; i < sizePob; i += 2)
            evaluate(pob, newPob, i, sizePob);

        pob = newPob;
        gen++;
    }

    std::cout << "\nI end in the generation: " << gen << "\n";
    std::cout << "Best individual: "; printBest(oldBest, gen);
}