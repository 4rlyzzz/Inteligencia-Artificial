#include <iostream>
#include <vector>
#include <limits>

int sizeTab;
const char iconYO = 'X';
const char iconIA = 'O';
const char empty = ' ';

void printTablero(const std::vector<char>& tab) 
{
    // print index
    std::cout << "\n\t";
    for (int i = 0; i < sizeTab; i++)
        std::cout << i << "\t";
    std::cout << std::endl;

    std::cout << "\t";
    for (int i = 0; i < sizeTab; i++)
        std::cout << "-------";
    std::cout << std::endl;


    // print tab
    for (int row = 0; row < sizeTab; row++) 
    {
        std::cout << "    " << row << " | ";
        for (int column = 0; column < sizeTab; column++)
            std::cout << tab[row * sizeTab + column] << "\t";

        std::cout << std::endl << std::endl;
    }
}

/*
bool checkWin(const std::vector<char>& tab, char player, int r, int c) // - | \ / 
{ 

    int dr[] = { 0, 1, 1, 1 };
    int dc[] = { 1, 0, 1, -1 };

    for (int i = 0; i < 4; i++) 
    {
        int cont = 1;

        // direccion -> abajo, derecha, diagonal(\)
        int cooR = r + dr[i];
        int cooC = c + dc[i];
        while (cooR >= 0 && cooR < sizeTab && cooC >= 0 && cooC < sizeTab && tab[cooR * sizeTab + cooC] == player) // posiciones R  |  posiciones C  |  jugador
        { 
            cont++;
            cooR += dr[i];
            cooC += dc[i];
        }

        // direccion -> arriba, izquierda, diagonal(/)
        cooR = r - dr[i];
        cooC = c - dc[i];
        while (cooR >= 0 && cooR < sizeTab && cooC >= 0 && cooC < sizeTab && tab[cooR * sizeTab + cooC] == player) // posiciones R  |  posiciones C  |  jugador
        { 
            cont++;
            cooR -= dr[i];
            cooC -= dc[i];
        }

        if (cont >= sizeTab) return true;
    }

    return false;
}
*/

bool checkWin_N(const std::vector<char>& tab, char player) 
{
    // rows
    for (int r = 0; r < sizeTab; r++) 
    {
        bool win = true;
        for (int c = 0; c < sizeTab; c++) 
        {
            if (tab[r * sizeTab + c] != player) 
            { 
                win = false; 
                break; 
            }
        }
        if (win) return true;
    }

    // cols
    for (int c = 0; c < sizeTab; c++) 
    {
        bool win = true;
        for (int r = 0; r < sizeTab; r++)
        {
            if (tab[r * sizeTab + c] != player)
            { 
                win = false; 
                break; 
            }
        }
        if (win) return true;
    }

    // diagonal (\)
    bool win = true;
    for (int i = 0; i < sizeTab; i++) {
        if (tab[i * sizeTab + i] != player) 
        { 
            win = false; 
            break; 
        }
    }
    if (win) return true;

    // diagonal (/)
    win = true;
    for (int i = 0; i < sizeTab; i++) {
        if (tab[i * sizeTab + (sizeTab - i - 1)] != player) 
        { 
            win = false; 
            break; 
        }
    }
    if (win) return true;

    return false;
}

bool tie(std::vector<char> tab) 
{
    for (int i = 0; i < sizeTab * sizeTab; i++) 
    {
        if (tab[i] == empty)
            return false;
    }
    return true;
}


struct node 
{
    std::vector<node*> child;
    int value;
    std::pair<int, int> coord;

    node(int v, std::pair<int, int> c = { -1,-1 }) : value(v), coord(c) {}
};

struct tree {
    node* root = nullptr;

    int optionWin(const std::vector<char>& tab, char icon) 
    {
        char opponent;

        if (icon == iconIA) opponent = iconYO;
        else  opponent = iconIA;


        int options = 0;

        // row
        for (int r = 0; r < sizeTab; r++) 
        {
            bool blocked = false;
            for (int c = 0; c < sizeTab; c++) 
            {
                if (tab[r * sizeTab + c] == opponent) 
                {
                    blocked = true;
                    break;
                }
            }
            if (!blocked) options++;
        }

        // col
        for (int c = 0; c < sizeTab; c++) 
        {
            bool blocked = false;
            for (int r = 0; r < sizeTab; r++) 
            {
                if (tab[r * sizeTab + c] == opponent) 
                {
                    blocked = true;
                    break;
                }
            }
            if (!blocked) options++;
        }

        // diagonal (\)
        bool blocked = false;
        for (int i = 0; i < sizeTab; i++) 
        {
            if (tab[i * sizeTab + i] == opponent) 
            {
                blocked = true;
                break;
            }
        }
        if (!blocked) options++;

        // diagonal (/)
        blocked = false;
        for (int i = 0; i < sizeTab; i++) 
        {
            if (tab[i * sizeTab + (sizeTab - i - 1)] == opponent) 
            {
                blocked = true;
                break;
            }
        }
        if (!blocked) options++;

        return options;
    }

    node* buildTree(const std::vector<char>& tab, int r, int c, int level, int currentLevel) 
    {

        //if (checkWin(tab, iconIA, r, c) || checkWin(tab, iconYO, r, c) || tie(tab) || currentLevel >= level)
        if (checkWin_N(tab, iconIA) || checkWin_N(tab, iconYO) || tie(tab) || currentLevel >= level) 
        {
            int scoreIA = optionWin(tab, iconIA);
            int scoreYO = optionWin(tab, iconYO);

            return new node(scoreIA - scoreYO);
        }

        // 
        char currentIcon;
        if (currentLevel % 2 == 0) currentIcon = iconIA;
        else currentIcon = iconYO;


        //
        node* current = new node(0);

        for (int r_t = 0; r_t < sizeTab; r_t++) 
        {
            for (int c_t = 0; c_t < sizeTab; c_t++) 
            {
                if (tab[r_t * sizeTab + c_t] == empty)
                {
                    std::vector<char> tabCopy = tab;

                    tabCopy[r_t * sizeTab + c_t] = currentIcon;

                    node* tempNode = buildTree(tabCopy, r_t, c_t, level, currentLevel + 1);

                    tempNode->coord = { r_t, c_t };
                    current->child.push_back(tempNode);
                }
            }
        }

        return current;
    }

    int alphaBeta(node* node, int alpha, int beta, int level) 
    {

        if (node->child.empty()) return node->value;

        bool isMax;
        if (level % 2 == 0) isMax = true; // max
        else isMax = false; // min

        if (isMax) // max
        { 
            int best = std::numeric_limits<int>::min();

            for (auto i : node->child) 
            {
                int newValue = alphaBeta(i, alpha, beta, level + 1);

                best = std::max(best, newValue);
                alpha = std::max(alpha, best); // alpha = best;

                if (alpha >= beta) break;
            }
            node->value = best;
            return best;
        }
        else // min
        { 
            int best = std::numeric_limits<int>::max();

            for (auto i : node->child) 
            {
                int newValue = alphaBeta(i, alpha, beta, level + 1);

                best = std::min(best, newValue);
                beta = std::min(beta, best); //beta = best;

                if (alpha >= beta) break;
            }
            node->value = best;
            return best;
        }
    }

    std::pair<int, int> bestMove(node* root)
    {
        for (auto children : root->child)
        {
            if (children->value == root->value)
                return children->coord;
        }

        return { -1,-1 };
    }
};

int main()
{
    std::cout << "Size Tab: ";
    std::cin >> sizeTab;

    int level;
    std::cout << "Search level: ";
    std::cin >> level;

    int turn;
    std::cout << "Who starts? (1 = You, 2 = IA): ";
    std::cin >> turn;

    //////////////////////////

    std::vector<char> tablero(sizeTab * sizeTab, empty);
    printTablero(tablero);

    std::cout << "\nYour symbol: X  |  IA: O\n";
    std::cout << "Enter moves as: row col (0-" << sizeTab - 1 << ")\n--------------------------------------------\n";

    int rowIA = 0, colIA = 0;

    while (true) 
    {

        if (turn == 1) // human
        { 
            int row, col;
            std::cout << "ROW COL: ";
            std::cin >> row >> col;

            if (row >= sizeTab || row < 0 || col >= sizeTab || col < 0) 
            {
                std::cout << "Invalid range. ";
                continue;
            }
            if (tablero[row * sizeTab + col] != empty) 
            {
                std::cout << "Cell occupied. ";
                continue;
            }

            tablero[row * sizeTab + col] = iconYO;
            rowIA = row; colIA = col;
            printTablero(tablero);

            //if (checkWin(tablero, 'X', row, col)) {
            if (checkWin_N(tablero, 'X')) 
            {
                std::cout << "You win\n";
                break;
            }
            if (tie(tablero)) 
            {
                std::cout << "Tie\n";
                break;
            }

            turn = 2;
        }
        else // IA
        { 
            tree t;
            node* root = t.buildTree(tablero, rowIA, colIA, level, 0);
            t.alphaBeta(root, std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), 0);
            auto [iaRow, iaCol] = t.bestMove(root);

            tablero[iaRow * sizeTab + iaCol] = iconIA;
            rowIA = iaRow; colIA = iaCol;
            printTablero(tablero);

            //if (checkWin(tablero, 'O', iaRow, iaCol))
            if (checkWin_N(tablero, 'O')) 
            {
                std::cout << "IA wins\n";
                break;
            }
            if (tie(tablero)) 
            {
                std::cout << "Tie\n";
                break;
            }

            turn = 1;
        }
    }
}