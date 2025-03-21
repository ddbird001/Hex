#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <random>
#include <chrono>
#include <string>
#pragma GCC optimize(2)

using namespace std;

#define BOARD_SIZE 11
#define DURATION 5000

const int dx[6] = {0, 1, 1, 0, -1, -1};
const int dy[6] = {-1, -1, 0, 1, 1, 0};
struct Point {
    int x, y;
    Point(int x, int y) : x(x), y(y) {}
    
    bool operator==(const Point &p) const {
        return x == p.x && y == p.y;
    }

    bool isExceedRange() const {
        return x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE;
    }
};

struct Board {
    vector<vector<int>> board;
    Board() {
        board.resize(BOARD_SIZE, vector<int>(BOARD_SIZE, 0));
    }

    bool isFull() const {
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                if (board[i][j] == 0) {
                    return false;
                }
            }
        }
        return true;
    }

    bool isLegalMove(const Point &p) const {
        return !p.isExceedRange() && board[p.x][p.y] == 0;
    }

    void makeMove(const Point &p, int player) {
        board[p.x][p.y] = player;
    }
};

struct Node {
    Point move;
    int player;
    int visitCount;
    double winScore;
    Node *parent;
    vector<Node *> children;
    vector<Point> legalMoves; 

    Node(Point move, int player, Node *parent) : move(move), player(player), parent(parent) {
        visitCount = 0;
        winScore = 0;
    }

    bool isFullyExpanded() const {
        return children.size() == legalMoves.size();
    }

    Node *selectChild() {
        Node *selected = nullptr;
        double bestUCB = -1;
        for (Node *child : children) {
            double UCB;
            if (child->visitCount == 0) {
                UCB = +INFINITY;
            }
		    else {
                UCB = (child->winScore / child->visitCount) + sqrt(2 * log(visitCount) / child->visitCount);
            }
            if (UCB > bestUCB) {
                bestUCB = UCB;
                selected = child;
            }
        }
        return selected;
    }

    Node *addChild(Point move, int player) {
        Node *child = new Node(move, player, this);
        children.push_back(child);
        return child;
    }

    void update(double result) {
        visitCount++;
        winScore += result; 
        
    }

    Node *bestChild() {
        Node *bestChild = nullptr;
        double bestScore = -1;
        for (Node *child : children) {
            double score = child->visitCount; 
            if (score > bestScore) {
                bestScore = score;
                bestChild = child;
            }
        }
        return bestChild;
    }
};  

vector<int> parent(BOARD_SIZE * BOARD_SIZE + 100, -1);
int find(int x) {
    return parent[x] == -1 ? x : parent[x] = find(parent[x]);
}
int isWin(const Board &board) {
    parent.assign(BOARD_SIZE * BOARD_SIZE+4, -1);
    auto unite = [&](int x, int y) {
        int rootX = find(x);
        int rootY = find(y);
        if (rootX != rootY) {
            parent[rootX] = rootY;
        }
    };
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board.board[i][j] == 1) {
                if (i == 0) {
                    unite(i * BOARD_SIZE + j, BOARD_SIZE * BOARD_SIZE);
                }
                if (i == BOARD_SIZE - 1) {
                    unite(i * BOARD_SIZE + j, BOARD_SIZE * BOARD_SIZE + 1);
                }
                for (int k = 0; k < 6; k++) {
                    int x = i + dx[k];
                    int y = j + dy[k];
                    if (x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE && board.board[x][y] == 1) {
                        unite(i * BOARD_SIZE + j, x * BOARD_SIZE + y);
                    }
                }
            }
            else if (board.board[i][j] == 2) {
                if (j == 0) {
                    unite(i * BOARD_SIZE + j, BOARD_SIZE * BOARD_SIZE + 2);
                }
                if (j == BOARD_SIZE - 1) {
                    unite(i * BOARD_SIZE + j, BOARD_SIZE * BOARD_SIZE + 3);
                }
                for (int k = 0; k < 6; k++) {
                    int x = i + dx[k];
                    int y = j + dy[k];
                    if (x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE && board.board[x][y] == 2) {
                        unite(i * BOARD_SIZE + j, x * BOARD_SIZE + y);
                    }
                }
            }
        }
    }
    if(find(BOARD_SIZE * BOARD_SIZE) == find(BOARD_SIZE * BOARD_SIZE + 1)) return 1;
    else if(find(BOARD_SIZE * BOARD_SIZE + 2) == find(BOARD_SIZE * BOARD_SIZE + 3)) return 2;
    else return 0;
}

int simulate(Board board, int player) {
    while (!board.isFull()) {
        vector<Point> legalMoves;
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                if (board.isLegalMove(Point(i, j))) {
                    legalMoves.push_back(Point(i, j));
                }
            }
        }
        if (legalMoves.empty()) break;
        static mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());
        shuffle(legalMoves.begin(), legalMoves.end(), rng);
        Point move = legalMoves[0];
        board.makeMove(move, player);
        int winner = isWin(board);
        if (winner != 0) return winner;
        player = 3 - player;
    }
    return 0;
}

Point HexBot(const Board &board, int player) {
    auto start = chrono::high_resolution_clock::now();
    Node *root = new Node(Point(-1, -1), player, nullptr);
    int rootPlayer = player; 
    while (chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - start).count() < DURATION) {
        Node *node = root;
        Board boardCopy = board;
        while (true) {
            vector<Point> legalMoves;
            for (int i = 0; i < BOARD_SIZE; i++) {
                for (int j = 0; j < BOARD_SIZE; j++) {
                    if (boardCopy.isLegalMove(Point(i, j))) {
                        legalMoves.push_back(Point(i, j));
                    }
                }
            }
            if (legalMoves.empty()) break;
            if (node->legalMoves.empty()) {
                node->legalMoves = legalMoves;
            }
            if (node->isFullyExpanded() || boardCopy.isFull()) {
                if (node->children.empty()) break;
                node = node->selectChild();
                boardCopy.makeMove(node->move, node->player);
            } else {
                static mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());
                shuffle(node->legalMoves.begin(), node->legalMoves.end(), rng);
                for (Point move : node->legalMoves) {
                    bool found = false;
                    for (Node *child : node->children) {
                        if (child->move == move) {
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        Node *child = node->addChild(move, 3 - node->player);
                        boardCopy.makeMove(move, child->player);
                        node = child;
                        break;
                    }
                }
                break;
            }
        }
        double result = 0;
        int winner = simulate(boardCopy, 3 - node->player);
        if(winner == 1) result = 1;
        else if(winner == 2) result = -1;
        while (node != nullptr) {
            node->update(result);
            node = node->parent;
        }
    }
    Node *bestChild = root->bestChild();
    Point bestMove = bestChild->move;

    return bestMove;
}

int main(void) {
    Board board;
    int player = 1;
    while (true) {
        int x, y;
        printf("Input:");
        scanf("%d %d", &x, &y);
        if (board.isLegalMove(Point(x, y))) {
            board.makeMove(Point(x, y), player);
            int winner = isWin(board);
            if (winner == 1) {
                printf("You win\n");
                break;
            }
            player = 3 - player;
            Point move = HexBot(board, player);
            board.makeMove(move, player);
            printf("Computer: (%d, %d)\n", move.x, move.y);
            winner = isWin(board);
            if (winner == 2) {
                printf("Computer wins\n");
                break;
            }
            player = 3 - player;
        } else {
            printf("Invalid move\n");
        }
    }
    return 0;
}