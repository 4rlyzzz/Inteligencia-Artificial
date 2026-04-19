#ifdef __APPLE__
# include <OpenGL/gl.h>
# include <OpenGL/glu.h>
# include <GLUT/glut.h>
#else
# include <GL/gl.h>
# include <GL/glu.h>
# include <GL/glut.h>
#endif

#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <queue>
#include <cmath>
#include <ctime>

using coord = std::pair<int, int>;
using vString = std::vector<std::string>;
using vCoord = std::vector<coord>;

// globals
int globalSz = 20;
std::map<coord, std::vector<coord>> graph;
std::map<coord, std::vector<std::string>> domColor;
std::vector<std::string> color = { "red", "green", "blue" };
std::map<coord, std::string> resForward, resRestrictive, resRestricted; // results


// predetermined
int keyChange = 1;

// screen size
int winW = 800, winH = 700;
const int PAD = 60; // margin

float toScreenX(int x) 
{
	return PAD + (float)x / ((globalSz - 1) * 10) * (winW - 2 * PAD);
}
float toScreenY(int y) 
{
	return PAD + (float)y / ((globalSz - 1) * 10) * (winH - 2 * PAD);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void removeNodesAndConnections(int num) 
{
    // REMOVE NODES
    // copy graph nodes into vector -> index access
    std::vector<coord> nodes;
    for (auto i : graph)
        nodes.push_back(i.first);

    // mix vector
    for (int i = nodes.size() - 1; i > 0; i--) 
    {
        int j = rand() % (i + 1);
        std::swap(nodes[i], nodes[j]);
    }

    // calculate how many nodes to remove
    int removeN = nodes.size() - num;
    for (int i = 0; i < removeN; i++) 
    {
        // random node
        int j = rand() % nodes.size();
        coord tempN = nodes[j];

        // remove tempN -> list adj
        for (auto t : graph[tempN]) 
        {
            std::vector<coord>& neigh = graph[t]; // &
            for (int k = 0; k < (int)neigh.size(); k++) 
            {
                if (neigh[k] == tempN) 
                {
                    neigh.erase(neigh.begin() + k); // delete 
                    break;
                }
            }
        }
        graph.erase(tempN); // delete -> graph original
        nodes.erase(nodes.begin() + j);
    }

    // ADJUST CONNECTIONS (NEIGH -> min 2, max 5)
    for (auto& neighGraph : graph) 
    {
        if (neighGraph.second.size() < 2) // add neigh
        { 
            int target = 2 + rand() % 4;
            while (neighGraph.second.size() < (size_t)target) 
            {
                int r = rand() % nodes.size();
                coord candidateNeigh = nodes[r];

                // check -> not same node
                bool yaExiste = false;
                for (auto& tempNeigh : neighGraph.second) 
                {
                    if (tempNeigh == candidateNeigh) 
                    {
                        yaExiste = true;
                        break;
                    }
                }
                if (candidateNeigh != neighGraph.first && !yaExiste && graph[candidateNeigh].size() < 5) 
                {
                    neighGraph.second.push_back(candidateNeigh);
                    graph[candidateNeigh].push_back(neighGraph.first);
                }
            }
        }
        else if (neighGraph.second.size() > 5) // remove neigh
        { 
            while (neighGraph.second.size() > 5) 
            {
                int r = rand() % neighGraph.second.size();
                coord nb = neighGraph.second[r];

                for (int k = 0; k < (int)graph[nb].size(); k++) 
                {
                    if (graph[nb][k] == neighGraph.first) 
                    {
                        graph[nb].erase(graph[nb].begin() + k);
                        break;
                    }
                }
                neighGraph.second.erase(neighGraph.second.begin() + r);
            }
            
            /*
            int temp = 2 + rand() % 4;
            if (i.second.size() > temp) 
                i.second.resize(temp);
            */
        }
    }

    std::cout << "----------------------------------\n";
    for (auto const& i : graph) 
    {
        std::cout << "Nodo [" << i.first.first << "," << i.first.second << "] -> ";

        for (auto const& j : i.second)
            std::cout << "(" << j.first << "," << j.second << ") ";

        std::cout << "\n";
    }
    std::cout << "----------------------------------\n" << std::endl;
}

void createGraph(std::map<coord, vCoord>& graph, int size) 
{
    int sz = size * 10;

    int dr[] = { 0,10,10,10 };
    int dc[] = { 10,0,10,-10 };
    
    int r = 0;
    int c = 0;

    for (int i = 0; i < size * size; i++) 
    {
        for (int j = 0; j < 4; j++) 
        {
            int tpR = r + dr[j];
            int tpC = c + dc[j];
            if (tpR >= 0 && tpC >= 0 && tpR < sz && tpC < sz)
                graph[{r, c}].push_back({ tpR, tpC });
        
            tpR = r - dr[j];
            tpC = c - dc[j];
            if (tpR >= 0 && tpC >= 0 && tpR < sz && tpC < sz)
                graph[{r, c}].push_back({ tpR, tpC });
        }
        c += 10;
        if (c >= sz) 
        {
            c = 0;
            r += 10;
        }
    }

    /*
	// BEGIN DEBUG: graph 
    for (auto i : graph) 
    {
        std::cout << "nodo [" <<  i.first.first << " " << i.first.second << "]->";

        for(auto j: i.second)
            std::cout << " (" << j.first<< " " << j.second << ")";

        std::cout << "\n";
    }
	// END DEBUG
    */
}

void deleteColor(std::map<coord, vString>& domActual, coord nodo, std::string colorAssign) 
{
    // for each neigh node
    for (auto neigh : graph[nodo]) // neigh node
    { 
        std::vector<std::string>& tempOpt = domActual[neigh]; // & -> edit domActual
        // we search and delete color in the neigh
        for (int j = 0; j < (int)tempOpt.size(); j++) 
        {
            if (tempOpt[j] == colorAssign) 
            {
                tempOpt.erase(tempOpt.begin() + j);
                break;
            }
        }
    }
}

void resetDom() // for "forwardChecking" and "moreRestricted"
{
    domColor.clear();
    for (auto i : graph)
        domColor[i.first] = color;//{ "red", "green", "blue" };
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// -------------------------------------------------- forwardChecking --------------------------------------------------
bool forwardChecking(std::map<coord, vString> domActual, vCoord nodosRemaining,
                     int indxNode, std::map<coord, std::string>& assignmentFinal) 
{
    if (indxNode == (int)nodosRemaining.size()) return true;

    coord nodoActual = nodosRemaining[indxNode];

    // BEGIN DEBUG: node status 
    std::cout << "\n-> Nodo: " << nodoActual.first << "," << nodoActual.second << "  -  DomActual = { ";
    for (auto i : domActual[nodoActual]) 
            std::cout << i << " "; 
    std::cout << "}\n"; 
    // END DEBUG

    // we tested available color in the dom of the actual node
    for (auto colorTest : domActual[nodoActual]) 
    {
        std::cout << "   Color prove: " << colorTest << std::endl; // DEBUG: color test

        // test scenary
        std::map<coord, vString> tempDom = domActual;

        assignmentFinal[nodoActual] = colorTest;

        deleteColor(tempDom, nodoActual, colorTest);

        // check if it is valid (no empty nodes)
        bool isValid = true;
        for (auto neigh : graph[nodoActual]) 
        {
            if (tempDom[neigh].empty()) 
            {
                std::cout << "   [PODA] (" << neigh.first << "," << neigh.second << ") it has no colors" << std::endl; // DEBUG: log pruning
                isValid = false;
                break;
            }
        }
        if (isValid) 
        {
            if (forwardChecking(tempDom, nodosRemaining, indxNode + 1, assignmentFinal))
                return true;
        }

        // BEGIN DEBUG: backtracking 
        std::cout << "   [BACKTRACK] Node: " << nodoActual.first << "," << nodoActual.second << " - DomActual = { ";
        for (auto i : domActual[nodoActual]) 
            std::cout << i << " "; 
        std::cout << "}\n"; 
        // END DEBUG


        assignmentFinal.erase(nodoActual);
    }
    return false;
}

void callForwardChecking() 
{
    resetDom();

    vCoord listCoord;
    for (auto nodo : graph) 
    {
        listCoord.push_back(nodo.first);
    }

    std::map<coord, std::string> res;

    std::cout << "--- FORWARD CHECKING ---" << std::endl;

    bool encontrado = forwardChecking(domColor, listCoord, 0, res);
    resForward = res;

    std::cout << "-----------------------------------------------" << std::endl;
    if (encontrado) 
    {
        std::cout << "THERE IS A SOLUCION" << std::endl;
        for (auto sol : res)
            std::cout << "Nodo (" << sol.first.first << "," << sol.first.second << ") -> " << sol.second << std::endl;
    }
    else 
        std::cout << "THERE IS NO SOLUCION" << std::endl;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// -------------------------------------------------- moreRestrictive --------------------------------------------------
bool moreRestrictive(vCoord nodosRemaining,int indxNode, std::map<coord, std::string>& assignmentFinal) 
{
    if (indxNode == (int)nodosRemaining.size()) return true;

    coord nodoActual = nodosRemaining[indxNode];

    // BEGIN DEBUG: node status
    std::cout << "\n-> Nodo: " << nodoActual.first << "," << nodoActual.second << "  -  DomActual = { ";
    for (auto i : color) 
        std::cout << i << " "; 
    std::cout << "}\n"; 
    // END DEBUG

    for (auto i : color) 
    {
        std::cout << "   Color prove: " << i << std::endl; // DEBUG: color test

        bool isValid = true;
        for (auto neigh : graph[nodoActual]) 
        {
            if (assignmentFinal.count(neigh) > 0 /*assignmentFinal[neigh] != ""*/ && assignmentFinal[neigh] == i) {
                isValid = false;
                break;
            }
        }
        if (isValid) 
        {
            assignmentFinal[nodoActual] = i;
            if (moreRestrictive(nodosRemaining, indxNode + 1, assignmentFinal)) return true;
        }

        std::cout << "   [BACKTRACK] Node: " << nodoActual.first << "," << nodoActual.second << std::endl; // DEBUG: log backtracking

        assignmentFinal.erase(nodoActual);
    }
    return false;
}

struct compRestrictive 
{
    bool operator()(const std::pair<int, coord>& a, const std::pair<int, coord>& b) {
        return a.first > b.first;
    }
};

void callMoreRestrictive() 
{
    std::priority_queue<std::pair<int, coord>, 
                        std::vector<std::pair<int, coord>>,
                        compRestrictive> pq;
    for (auto i : graph)
        pq.push({ (int)i.second.size(), i.first });

    vCoord listCoord;
    while (!pq.empty()) 
    {
        listCoord.push_back(pq.top().second);
        pq.pop();
    }

    std::map<coord, std::string> res;

    std::cout << "\n\n--- MORE RESTRICTIVE ---" << std::endl;

    bool encontrado = moreRestrictive(listCoord, 0, res);
    resRestrictive = res;

    std::cout << "-----------------------------------------------" << std::endl;
    if (encontrado) 
    {
        std::cout << "THERE IS A SOLUCION" << std::endl;
        for (auto sol : res)
            std::cout << "Nodo (" << sol.first.first << "," << sol.first.second << ") -> " << sol.second << std::endl;
    }
    else 
        std::cout << "THERE IS NO SOLUCION" << std::endl;
}
 
// -------------------------------------------------- moreRestricted --------------------------------------------------
bool moreRestricted(std::map<coord, vString> domActual, std::map<coord, std::string>& assignmentFinal) 
{
    if (assignmentFinal.size() == graph.size()) return true;

    coord nodoActual;

    int minColors = 4;
    for (auto& kv : graph) 
    {
        if (assignmentFinal.count(kv.first) == 0 && (int)domActual[kv.first].size() < minColors) 
        {
            minColors = (int)domActual[kv.first].size();
            nodoActual = kv.first;
        }
    }


    // BEGIN DEBUG: MRV node status 
    std::cout << "\n-> Nodo: " << nodoActual.first << "," << nodoActual.second << "  -  DomActual = { ";
    for (auto i : domActual[nodoActual]) 
        std::cout << i << " "; 
    std::cout << "}\n"; 
    // END DEBUG


    for (auto& colorTest : domActual[nodoActual])
    {
        std::cout << "   Color prove: " << colorTest << std::endl; // DEBUG: color test

        std::map<coord, vString> tempDom = domActual;

        assignmentFinal[nodoActual] = colorTest;
        deleteColor(tempDom, nodoActual, colorTest);

        if (moreRestricted(tempDom, assignmentFinal)) return true;

        // BEGIN DEBUG: backtracking 
        std::cout << "   [BACKTRACK] Node: " << nodoActual.first << "," << nodoActual.second << " - DomActual = { ";
        for (auto i : domActual[nodoActual]) 
            std::cout << i << " "; 
        std::cout << "}\n"; 
        // END DEBUG

        assignmentFinal.erase(nodoActual);
    }
    return false;
}

struct compRestricted 
{
    bool operator()(const std::pair<int, coord>& a, const std::pair<int, coord>& b) {
        return a.first < b.first;
    }
};

void callRestricted() 
{
    resetDom();

    std::map<coord, std::string> res;

    std::cout << "\n\n--- MORE RESTRICTED ---" << std::endl;

    bool encontrado = moreRestricted(domColor, res);
    resRestricted = res;

    std::cout << "-----------------------------------------------" << std::endl;
    if (encontrado) 
    {
        std::cout << "THERE IS A SOLUCION" << std::endl;
        for (auto sol : res)
            std::cout << "Nodo (" << sol.first.first << "," << sol.first.second << ") -> " << sol.second << std::endl;
    }
    else
        std::cout << "THERE IS NO SOLUCION" << std::endl;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
 
void drawText(float x, float y, const char* text) 
{
	glRasterPos2f(x, y);
	while (*text) 
		glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *text++);
}
 
void setColor(const std::string& c) 
{
	if(c == "red")   
		glColor3f(0.9f, 0.1f, 0.1f);
	else if (c == "green") 
		glColor3f(0.1f, 0.9f, 0.1f);
	else if (c == "blue")  
		glColor3f(0.1f, 0.1f, 0.9f);
	else
		glColor3f(0.5f, 0.5f, 0.5f); // no color assigned
}
 
// change screen
void changeScreen(unsigned char key, int, int) 
{
	if (key >= '1' && key <= '3') {
		keyChange = key - '0';
		glutPostRedisplay();
	}
}
 
// glut - zingjal
void reshape_cb(int w, int h) 
{
	if (w == 0 || h == 0) return;
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
	for (auto& kv : graph) {
		for (auto& nb : kv.second) 
		{
			glBegin(GL_LINES);
			glVertex2f(toScreenX(kv.first.first), toScreenY(kv.first.second));
			glVertex2f(toScreenX(nb.first), toScreenY(nb.second));
			glEnd();
		}
	}

	// draw node coordinates
	glColor3f(0.9f, 0.9f, 0.9f);
	for (auto& kv : graph) 
	{
		char buf[32];
		sprintf(buf, "%d,%d", kv.first.first, kv.first.second);
		drawText(toScreenX(kv.first.first) + 7, toScreenY(kv.first.second), buf);
	}
	 
	// select algorithm
	std::map<coord, std::string>* res;
	const char* name;
	 
	if(keyChange == 1) 
	{ 
		res = &resForward;
		name = "Forward Checking"; 
	}
	else if (keyChange == 2) 
	{ 
		res = &resRestrictive; 
		name = "More Restrictive";  
	}
	else{ 
		res = &resRestricted;  
		name = "More Restricted";   
	}
	 
	// draw nodes
	for (auto& kv : graph) 
	{
		setColor((*res)[kv.first]);
		drawCircle(toScreenX(kv.first.first), toScreenY(kv.first.second), 6.0f);
	}
	 
	// info
	glColor3f(0.9f, 0.9f, 0.9f);
	char buf[64];
	sprintf(buf, "Algorithm: %s  - [1]Forward [2]Restrictive [3]Restricted", name);
	drawText(10, 10, buf);
	 
	glutSwapBuffers();
}
 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// -- main --
int main(int argc, char** argv) 
{
	if (argc > 1) globalSz = std::atoi(argv[1]);

	srand(time(nullptr));
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
