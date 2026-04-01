#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <stack>
#include <queue>
#include <cmath>

using coord = std::pair<int, int>;

void createGraph(std::map<coord, std::vector<coord>> &graph, int sz) {

	int size = sz * 10;

	int x = 0;
	int y = 0;

	int dx[] = { -10, 10, 10, 10 };
	int dy[] = { 0, 0, 10, -10 };

	for (int i = 0; i < sz * sz; i++) {
		for (int k = 0; k < 4; k++) {
			int tpX = x + dx[k];
			int tpY = y + dy[k];
			if (tpX >= 0 && tpX < size && tpY >= 0 && tpY < size)
				graph[{x, y}].push_back({ tpX, tpY });

			tpX = x - dx[k];
			tpY = y - dy[k];
			if (tpX >= 0 && tpX < size && tpY >= 0 && tpY < size)
				graph[{x, y}].push_back({ tpX, tpY });
		}
		y += 10;
		if (y >= size) {
			y = 0;
			x += 10;
		}
	}

	for (auto i : graph) {
		std::cout << "nodo [" << i.first.first << ", " << i.first.second << "] -> ";

		for (auto j : i.second) {
			std::cout << " (" << j.first << "," << j.second << ")";
		}
		std::cout << "\n";
	}
	std::cout << "\n";
}

void dfs(std::map<coord, std::vector<coord>>& graph, int x, int y, int fin1, int fin2) {

	std::set<coord> visited; // nodos que visito
	std::stack<coord> nose; // evaluar todo

	std::vector<coord> copiaParent; // coordenadas de donde vino del nodo actual
	std::map<coord, std::vector<coord>> parent; // lo de copiaParent de cada nodo

	coord ini = {x,y};
	coord fin = {fin1, fin2};

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

		/*
		for (auto i : graph[temp]) {
			std::cout << i.first << " " << i.second;
			std::cout << "\n";
		} 
		std::cout << "\n";
		*/

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

	if (found) {
		std::vector<coord> res = parent[fin];

		for (auto i : res) 
			std::cout << "[" << i.first << "," << i.second << "]";

		std::cout << std::endl;
	} else std::cout << "no hay un camino" << std::endl;

}

void bfs(std::map<coord, std::vector<coord>>& graph, int x, int y, int fin1, int fin2) {

	std::set<coord> visited;
	std::queue<coord> nose;

	std::vector<coord> copiaParent;
	std::map<coord, std::vector<coord>> parent;

	coord ini = { x,y };
	coord fin = { fin1,fin2 };

	visited.insert(ini);
	nose.push(ini);

	parent[ini] = { ini };

	bool found = false;

	while (!nose.empty()) {
		coord temp = nose.front();
		nose.pop();

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
	if (found) {
		std::vector<coord> res = parent[fin];

		for (auto i : res)
			std::cout << "[" << i.first << "," << i.second << "]";

		std::cout << std::endl;
	}
	else std::cout << "no hay un camino" << std::endl;
}

struct compHill {
	bool operator()(const std::pair<coord, double>& a, const std::pair<coord, double>& b) {
		return a.second > b.second;
	}
};

double dist(int a1, int a2, int b1, int b2) {
	return std::sqrt( std::pow(b1 - a1, 2)  + std::pow(b2 - a2, 2));
}

void hillClimbing(std::map<coord, std::vector<coord>>& graph, int x, int y, int fin1, int fin2) {

	std::priority_queue< std::pair<coord, double>, 
						std::vector<std::pair<coord, double>>, 
						compHill > nose;
	std::set<coord> visited;

	std::vector<coord> copiaParent;
	std::map<coord, std::vector<coord>> parent;

	coord ini = { x,y };
	coord fin = { fin1, fin2 };

	nose.push({ ini, dist(ini.first, ini.second, fin.first, fin.second) });
	visited.insert(ini);

	parent[ini] = { ini };

	bool found = false;
	
	while (!nose.empty()) {
		std::pair<coord, double> temp = nose.top();
		coord tempCoord = temp.first;
		nose.pop();

		if (tempCoord == fin) { // (temp.first == fin) {
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

	if (found) {
		std::vector<coord> res = parent[fin];

		for (auto i : res)
			std::cout << "[" << i.first << "," << i.second << "]";

		std::cout << std::endl;
	}
	else std::cout << "no hay un camino" << std::endl;

}


struct compStar {
	bool operator()(const std::pair<coord, std::pair<double, double>>& a, std::pair<coord, std::pair<double, double>>& b) {
		return (a.second.first + a.second.second) > (b.second.first + b.second.second);
	}
};

void aStar(std::map<coord, std::vector<coord>>& graph, int x, int y, int fin1, int fin2) {
					     // par ( coord , par (		g,		h	) )
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

	parent[ini] = {ini};

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

				nose.push({ neigh,{g, h} });

				copiaParent = parent[tempCoord];
				copiaParent.push_back(neigh);
				parent[neigh] = copiaParent;
				copiaParent.clear();
			}
		}
	}

	if (found) {
		std::vector<coord> res = parent[fin];

		for (auto i : res)
			std::cout << "[" << i.first << "," << i.second << "]";

		std::cout << std::endl;
	}
	else std::cout << "no hay un camino" << std::endl;

}

int main()
{
	std::map<coord, std::vector<coord>> graph;

	int sz;
	std::cin >> sz;

	createGraph(graph, sz);

	std::cout << "dfs:";
	dfs(graph, 10, 0, 60,80);

	std::cout << "\nbfs:";
	bfs(graph, 10, 0, 60, 80);

	std::cout << "\nhill:";
	hillClimbing(graph, 10, 0, 60, 80);

	std::cout << "\na*:";
	aStar(graph, 10, 0, 60, 80);

}