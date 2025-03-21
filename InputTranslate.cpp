#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>

using namespace std;
const int BOARD_SIZE = 11;

struct Coord {
    int x;
    int y;
};

class GomokuAI {
private:
    vector<vector<int>> board;
    bool is_first;
    bool is_swapped;

    Coord parse_coord(const string& s) {
        char col = tolower(s[0]);
        int y = col - 'a';
        int x = stoi(s.substr(1)) - 1;
        return {x, y};
    }

    string format_coord(const Coord& c) {
        char col = 'a' + c.y;
        return string(1, col) + to_string(c.x + 1);
    }

    Coord get_symmetric(const Coord& original) {
        return {original.y, original.x};
    }

    bool is_valid(const Coord& c) {
        return c.x >= 0 && c.x < BOARD_SIZE && 
               c.y >= 0 && c.y < BOARD_SIZE &&
               board[c.x][c.y] == 0;
    }

    Coord find_valid_move() {
        // 简单实现：随机寻找空位，可替换为AI算法
        vector<Coord> empty;
        for (int i = 0; i < BOARD_SIZE; ++i) {
            for (int j = 0; j < BOARD_SIZE; ++j) {
                if (board[i][j] == 0) {
                    empty.push_back({i, j});
                }
            }
        }
        if (!empty.empty()) {
            return empty[rand() % empty.size()];
        }
        return {-1, -1}; // 棋盘已满
    }

public:
    GomokuAI() : board(BOARD_SIZE, vector<int>(BOARD_SIZE, 0)), 
                is_first(false), is_swapped(false) {
        srand(time(nullptr));
    }

    void run() {
        string input;
        getline(cin, input);

        if (input == "first") {
            is_first = true;
            play_first();
        } else {
            is_first = false;
            play_second(input);
        }

        game_loop();
    }

    void play_first() {
        Coord move = find_valid_move();
        if (move.x == -1) return;
        board[move.x][move.y] = 1;
        cout << format_coord(move) << endl;
        fflush(stdout);
    }

    void play_second(const string& first_move) {
        Coord opponent = parse_coord(first_move);
        
        // 交换决策（示例策略：如果靠近中心则交换）
        if (should_swap(opponent)) {
            cout << "change" << endl;
            fflush(stdout);
            is_swapped = true;
            Coord symm = get_symmetric(opponent);
            board[symm.x][symm.y] = 1;
        } else {
            Coord move = find_valid_move();
            board[move.x][move.y] = 1;
            cout << format_coord(move) << endl;
            fflush(stdout);
        }
    }

    bool should_swap(const Coord& c) {
        // 示例交换策略：当对手第一步在中心区域时交换
        int center = BOARD_SIZE/2;
        return abs(c.x - center) <= 2 && abs(c.y - center) <= 2;
    }

    void game_loop() {
        string input;
        while (true) {
            getline(cin, input);
            if (input == "finish") break;

            Coord opponent = parse_coord(input);
            board[opponent.x][opponent.y] = 2;

            Coord move = find_valid_move();
            if (move.x == -1) return;
            board[move.x][move.y] = 1;
            cout << format_coord(move) << endl;
            fflush(stdout);
        }
    }
};

int main() {
    GomokuAI ai;
    ai.run();
    return 0;
}
