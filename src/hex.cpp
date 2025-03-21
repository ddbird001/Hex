//用UCT算法实现hex棋智能ai

#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <random>
#include <chrono>
#include <string>

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

    Node(Point move, int player, Node *parent) : move(move), player(player), parent(parent) {
        visitCount = 0;
        winScore = 0;
    }

    bool isFullyExpanded() const {
        return children.size() == BOARD_SIZE * BOARD_SIZE;
    }

    Node *selectChild() {
        Node *selected = nullptr;
        double bestUCB = -1;
        for (Node *child : children) {
            double UCB = 0;
            if (child->visitCount == 0) {
                UCB = +INFINITY;
            }
            UCB = child->winScore / child->visitCount + sqrt(2 * log(visitCount) / child->visitCount);
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
            double score = child->winScore / child->visitCount;
            if (score > bestScore) {
                bestScore = score;
                bestChild = child;
            }
        }
        return bestChild;
    }
};  

//使用并查集判断是否胜利
vector<int> parent(BOARD_SIZE * BOARD_SIZE + 100, -1);
int find(int x) {
    return parent[x] == -1 ? x : parent[x] = find(parent[x]);
}
int isWin(const Board &board) {
    parent.assign(BOARD_SIZE * BOARD_SIZE, -1);
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
    //printf("****\n");
    if(find(BOARD_SIZE * BOARD_SIZE) == find(BOARD_SIZE * BOARD_SIZE + 1)) return 1;
    else if(find(BOARD_SIZE * BOARD_SIZE + 2) == find(BOARD_SIZE * BOARD_SIZE + 3)) return 2;
    else return 0;
}
int simulate(Board board, int player) {
    while (!board.isFull()) {
        //printf("***\n");
        vector<Point> legalMoves;
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                if (board.isLegalMove(Point(i, j))) {
                    legalMoves.push_back(Point(i, j));
                }
            }
        }
        srand(time(0));
        std::random_device rd; // 随机设备
        std::mt19937 g(rd());  // 随机数生成器
        std::shuffle(legalMoves.begin(), legalMoves.end(), g);
        Point move = legalMoves[0];
        board.makeMove(move, player);
        if (isWin(board) == player) {
            return player;
        }
        player = 3 - player;
    }
    return 0;
}

Point HexBot(const Board &board, int player) {
    auto start = chrono::high_resolution_clock::now();
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    Node *root = new Node(Point(-1, -1), player, nullptr);
    while (duration < DURATION) {
        Node *node = root;
        Board boardCopy = board;
        bool flag = false;
        while (!node->isFullyExpanded() && !boardCopy.isFull()) {
            if (node->children.empty()) {
                vector<Point> legalMoves;
                for (int i = 0; i < BOARD_SIZE; i++) {
                    for (int j = 0; j < BOARD_SIZE; j++) {
                        if (boardCopy.isLegalMove(Point(i, j))) {
                            legalMoves.push_back(Point(i, j));
                        }
                    }
                }
                srand(time(0));
                std::random_device rd; // 随机设备
                std::mt19937 g(rd());  // 随机数生成器
                std::shuffle(legalMoves.begin(), legalMoves.end(), g);
                for (Point move : legalMoves) {
                    if (boardCopy.isLegalMove(move)) {
                        node = node->addChild(move, 3 - node->player);
                        boardCopy.makeMove(move, node->player);
                        flag = true;
                        break;
                    }
                }
            } else {
                node = node->selectChild();
                boardCopy.makeMove(node->move, node->player);
            }
            if (flag) break;
        }
        int winner = 0;
        winner = simulate(boardCopy, 3 - node->player);
    
        //printf("$$$\n");
        while (node != nullptr) {
            node->update(winner);
            node = node->parent;
        }
        end = chrono::high_resolution_clock::now();
        duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    }
    Node *bestChild = root->bestChild();
    return bestChild->move;
}

int main(void)
{
    Board board;
    int player = 1;
    while (true)
    {
        int x, y;
        printf("Input:");
        scanf("%d %d", &x, &y);
        if (board.isLegalMove(Point(x, y)))
        {
            board.makeMove(Point(x, y), player);
            if (isWin(board) == 1)
            {
                printf("You win\n");
                break;
            }
            player = 3 - player;
            Point move = HexBot(board, player);
            board.makeMove(move, player);
            printf("computer:(%d, %d)\n", move.x, move.y);
            if (isWin(board) == 2)
            {
                printf("computer win\n");
                break;
            }
            player = 3 - player;
        }
        else
        {
            printf("非法坐标\n");
        }
    }
    return 0;
}