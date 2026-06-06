#pragma once

#include <iostream>
#include <set>
#include <algorithm>
#include <cmath>
#include <random>
#include <thread>
#include <mutex>

using coord = std::pair<int, int>;

// global vars 
inline int penaltyCross = 500;
inline int size_tournament = 3;
inline float prob_mut = 0.1f;
inline std::mutex print_mutex;

inline std::mt19937 rng(time(nullptr));
inline std::vector<coord> nodes;

inline std::vector<coord> bestRoute;
inline std::mutex route_mutex;

// randInt
inline int randInt(int start, int end, std::mt19937& local_rng) {
    return std::uniform_int_distribution<int>(start, end)(local_rng);
}
// randDouble
inline double randDouble() {
    return std::uniform_real_distribution<double>(0.0, 1.0)(rng);
}

// node generation
inline void generateNodes(int n) {
    std::set<coord> busy;
    while ((int)nodes.size() < n) {
        int x = randInt(0, 19, rng) * 10; // 0-19 -> * 10 (range 0-190)
        int y = randInt(0, 19, rng) * 10; // 0-19 -> * 10 (range 0-190)
        coord newPos = { x, y };
        if (busy.find(newPos) == busy.end()) {
            nodes.push_back(newPos);
            busy.insert(newPos);
        }
    }

    // BEGIN DEBUG
    for (int i = 0; i < n; i++)
        std::cout << "Nodo[" << i << "]: (" << nodes[i].first << ", " << nodes[i].second << ")\n";
    // END DEBUG
}

// individual
struct individual {
    std::vector<int> route;
    double penalty = 0; // intersection penalty
    double distance = 0;
    double fitness = 0; // 1 / (distance + penalty)
};

// distance
inline double euclidianDistance(int id1, int id2) {
    double dx = nodes[id2].first - nodes[id1].first;
    double dy = nodes[id2].second - nodes[id1].second;
    return std::sqrt(dx * dx + dy * dy);
}

// initialize 
inline std::vector<individual> initialize(int sizePob, int numNodes) {
    std::vector<individual> pob(sizePob);
    for (int i = 0; i < sizePob; i++) {
        for (int j = 0; j < numNodes; j++)
            pob[i].route.push_back(j);
        std::shuffle(pob[i].route.begin(), pob[i].route.end(), rng);
    }
    return pob;
}

// intersection helpers
inline double getDirection(coord a, coord b, coord c) { // ccw -> counter clockwise
    return (b.first - a.first) * (c.second - a.second) - (b.second - a.second) * (c.first - a.first);
}

inline bool checkSegments(coord a, coord b, coord c, coord d) {
    if (a == c || a == d || b == c || b == d) return false;

    double d1 = getDirection(a, b, c);
    double d2 = getDirection(a, b, d);
    double d3 = getDirection(c, d, a);
    double d4 = getDirection(c, d, b);

    // c y d en lados diferentes de ab? (diferentes -> + y -)
    bool test1 = ((d1 > 0 && d2 < 0) || (d1 < 0 && d2 > 0));

    // a y b en lados diferentes de cd? (diferentes -> + y -)
    bool test2 = ((d3 > 0 && d4 < 0) || (d3 < 0 && d4 > 0));

    return test1 && test2; // si ambos son ciertos, hay un cruce
}

inline double checkIntersections(individual& in) {
    double totalPenalty = 0;
    int n = in.route.size();

    for (int i = 0; i < n; i++) {
        for (int j = i + 2; j < n; j++) {
            if (i == 0 && j == n - 1) continue;

            coord A = nodes[in.route[i]];
            coord B = nodes[in.route[(i + 1) % n]];
            coord C = nodes[in.route[j]];
            coord D = nodes[in.route[(j + 1) % n]];

            if (checkSegments(A, B, C, D))
                totalPenalty += penaltyCross; // penalty for cross
        }
    }
    return totalPenalty;
}

// fitness 
inline void evaluateFitness(individual& in) {
    in.distance = 0;
    in.penalty = 0;
    in.fitness = 0;
    int n = in.route.size();

    for (int i = 0; i < n; i++) {
        int city = in.route[i];
        int nextCity = in.route[(i + 1) % n]; // % - cuando sea el ultimo -> next = 0
        in.distance += euclidianDistance(city, nextCity);
    }
    in.penalty = checkIntersections(in);
    in.fitness = 1.0 / (in.distance + in.penalty);
}

// tournament
inline individual tournament(std::vector<individual>& pob, std::mt19937& local_rng) {
    individual candidato = pob[randInt(0, pob.size() - 1, local_rng)];
    for (int i = 1; i < size_tournament; i++) {
        individual temp = pob[randInt(0, pob.size() - 1, local_rng)];
        if (temp.fitness > candidato.fitness) // En TSP, mayor fitness es mejor
            candidato = temp;
    }
    return candidato;
}

// elitism
inline individual elitism(std::vector<individual>& pob) {
    individual mejor = pob[0];
    for (size_t i = 1; i < pob.size(); i++)
        if (pob[i].fitness > mejor.fitness)
            mejor = pob[i];
    return mejor;
}

// crossover
inline void crossover(individual& p1, individual& p2, individual& h1, individual& h2) {
    int n = p1.route.size();
    int cut = randInt(1, n - 1, rng);

    h1.route.assign(n, -1);
    h2.route.assign(n, -1);

    // copy
    for (int i = 0; i < cut; i++) {
        h1.route[i] = p1.route[i];
        h2.route[i] = p2.route[i];
    }

    // fill in the rest with the missing cities
    int c1 = cut, c2 = cut;
    for (int i = 0; i < n; i++) {
        if (find(h1.route.begin(), h1.route.end(), p2.route[i]) == h1.route.end())
            h1.route[c1++] = p2.route[i];
        if (find(h2.route.begin(), h2.route.end(), p1.route[i]) == h2.route.end())
            h2.route[c2++] = p1.route[i];
    }
}

// mutation
inline void mutation(individual& in) {
    if (randDouble() < prob_mut) {
        int i = randInt(0, in.route.size() - 1, rng);
        int j = randInt(0, in.route.size() - 1, rng);
        std::swap(in.route[i], in.route[j]);
    }
}

// generation step
inline void evaluate(std::vector<individual>& pob, std::vector<individual>& newPob, int i, int sizePob) {

    // rng for each thread to avoid race condition
    std::mt19937 local_rng(std::random_device{}());

    individual p1 = tournament(pob, local_rng);
    individual p2 = tournament(pob, local_rng);
    individual h1, h2;

    crossover(p1, p2, h1, h2);

    mutation(h1);
    mutation(h2);

    evaluateFitness(h1);
    evaluateFitness(h2);

    newPob[i] = h1;
    if (i + 1 < sizePob)
        newPob[i + 1] = h2;
}