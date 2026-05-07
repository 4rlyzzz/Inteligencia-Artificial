#pragma once
#include "opengl_base.h"

// visited nodes
inline std::set<coord> visitedDFS, visitedBFS, visitedHill, visitedAStar;

// randInt
inline std::mt19937 rng(time(nullptr));
inline int randInt(int start, int end) {
	return std::uniform_int_distribution<int>(start, end)(rng);
}

// remove 20% 
inline void removeNodes(std::map<coord, std::vector<coord>>& graph, float percentage) {
	// REMOVE NODES
	// copy graph nodes into vector -> index access	
	std::vector<coord> nodes;
	for (const auto& i : graph)
		nodes.push_back(i.first);

	// mix vector
	std::shuffle(nodes.begin(), nodes.end(), rng);

	// calculate how many nodes to remove
	int removeN = nodes.size() * percentage;
	for (int i = 0; i < removeN; i++) {
		coord tempN = nodes[i];

		// remove tempN -> list adj		
		if (graph.count(tempN)) {
			for (auto& t : graph[tempN]) {

				if (graph.count(t)) {
					std::vector<coord>& neigh = graph[t];
					neigh.erase(std::remove(neigh.begin(), neigh.end(), tempN), neigh.end());
				}
			}
			graph.erase(tempN);
		}
	}
}

// -- dfs --
inline std::vector<coord> dfs(std::map<coord, std::vector<coord>>& graph, int x, int y, int fin1, int fin2) {
	std::set<coord> visited;
	std::stack<coord> nose;
	std::vector<coord> copiaParent;
	std::map<coord, std::vector<coord>> parent;

	coord ini = { x, y };
	coord fin = { fin1, fin2 };

	visited.insert(ini);
	nose.push(ini);
	parent[ini] = { ini };
	bool found = false;

	while (!nose.empty()) {
		coord temp = nose.top(); 
		nose.pop();

		if (temp == fin) { 
			found = true;
			break; 
		}

		for (auto neigh : graph[temp]) {
			if (visited.find(neigh) == visited.end()) {
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
	if (found) return parent[fin];
	return {};
}

// -- bfs --
inline std::vector<coord> bfs(std::map<coord, std::vector<coord>>& graph, int x, int y, int fin1, int fin2) {
	std::set<coord> visited;
	std::queue<coord> nose;
	std::vector<coord> copiaParent;
	std::map<coord, std::vector<coord>> parent;

	coord ini = { x, y };
	coord fin = { fin1, fin2 };

	visited.insert(ini);
	nose.push(ini);
	parent[ini] = { ini };
	bool found = false;

	while (!nose.empty()) {
		coord temp = nose.front(); nose.pop();

		if (temp == fin) { 
			found = true; 
			break; 
		}

		for (auto neigh : graph[temp]) {
			if (visited.find(neigh) == visited.end()) {
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
struct compHill {
	bool operator()(const std::pair<coord, double>& a, const std::pair<coord, double>& b) {
		return a.second > b.second;
	}
};

inline double dist(int a1, int a2, int b1, int b2) {
	return std::sqrt(std::pow(b1 - a1, 2) + std::pow(b2 - a2, 2));
}

inline std::vector<coord> hillClimbing(std::map<coord, std::vector<coord>>& graph, int x, int y, int fin1, int fin2) {
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

	while (!nose.empty()) {
		std::pair<coord, double> temp = nose.top();
		coord tempCoord = temp.first;
		nose.pop();
		if (tempCoord == fin) { 
			found = true; 
			break; 
		}
		for (auto neigh : graph[tempCoord]) {
			if (visited.find(neigh) == visited.end()) {
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
struct compStar {
	bool operator()(const std::pair<coord, std::pair<double, double>>& a, std::pair<coord, std::pair<double, double>>& b) {
		return (a.second.first + a.second.second) > (b.second.first + b.second.second);
	}
};

inline std::vector<coord> aStar(std::map<coord, std::vector<coord>>& graph, int x, int y, int fin1, int fin2) {
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
		if (tempCoord == fin) { 
			found = true; 
			break; 
		}
		for (auto neigh : graph[tempCoord]) {
			if (visited.find(neigh) == visited.end()) {
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