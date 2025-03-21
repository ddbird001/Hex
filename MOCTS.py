import math
import random
from collections import deque

class PlayHex:
    def __init__(self, board=None, current_player=1,size=4):
        self.size=size
        self.board = board if board is not None else [[0 for _ in range(size)] for _ in range (size)]
        self.current_player = current_player  # 1表示X玩家，-1表示O玩家


    def perform_move(self, move):
        new_board = [row.copy() for row in self.board]
        new_board[move[0]][move[1]] = self.current_player
        return PlayHex(new_board, -self.current_player, self.size)

    def get_available_moves(self):
        return [(i, j) for i in range(self.size) for j in range(self.size) if self.board[i][j] == 0]

    def is_terminal(self):
        return self.is_win(1) or self.is_win(-1)

    def is_win(self,current_player):
        size = self.size
        directions = [(-1, 0), (-1, 1), (0, -1), (0, 1), (1, -1), (1, 0)]
        
        visited = set()
        queue = deque()

        # 根据玩家设置起始位置和胜利条件
        if current_player == 1:
            # 玩家1需要连接左右边（列0到列5）
            start_positions = [(r, 0) for r in range(size) if self.board[r][0] == current_player]
            is_winning = lambda r, c: c == size - 1
        else:
            # 玩家2需要连接上下边（行0到行5）
            start_positions = [(0, c) for c in range(size) if self.board[0][c] == current_player]
            is_winning = lambda r, c: r == size - 1

        # 初始化队列和已访问集合
        for pos in start_positions:
            if pos not in visited:
                queue.append(pos)
                visited.add(pos)

        # 广度优先搜索
        while queue:
            r, c = queue.popleft()
            
            # 检查是否达到胜利条件
            if is_winning(r, c):
                return True
            
            # 探索六个方向的相邻位置
            for dr, dc in directions:
                nr, nc = r + dr, c + dc
                if 0 <= nr < size and 0 <= nc < size:
                    if self.board[nr][nc] == current_player and (nr, nc) not in visited:
                        visited.add((nr, nc))
                        queue.append((nr, nc))

        return False


class MCTSNode:
    def __init__(self, state, parent=None, move=None):
        self.state = state
        self.parent = parent
        self.move = move  # 新增move属性记录移动位置
        self.children = []
        self.visits = 0
        self.win_count = 0

def mcts_decision(state, simulations=1000):
    root = MCTSNode(state)
    for _ in range(simulations):
        node = select(root)
        if not node.state.is_terminal():
            expand(node)
            node = random.choice(node.children) if node.children else node
        result = simulate(node)
        backpropagate(node, result)
    
    # 选择访问次数最多的子节点的移动
    best = max(root.children, key=lambda c: c.visits)
    return best.move

def select(node):
    while node.children:
        node = best_uct(node)
    return node

def best_uct(node):
    best_score = -float('inf')
    best_node = None
    for child in node.children:
        if child.visits == 0:
            uct_value = float('inf')
        else:
            exploit = child.win_count / child.visits
            explore = math.sqrt(2 * math.log(node.visits) / child.visits)
            uct_value = exploit + 1.414 * explore
        if uct_value > best_score:
            best_score = uct_value
            best_node = child
    return best_node

def expand(node):
    for move in node.state.get_available_moves():
        new_state = node.state.perform_move(move)
        node.children.append(MCTSNode(new_state, node, move))  # 保存move信息

def simulate(node):
    current_state = node.state
    while not current_state.is_terminal():
        moves = current_state.get_available_moves()
        current_state = current_state.perform_move(random.choice(moves))
    if current_state.is_win(current_state.current_player):
        return 1 if current_state.current_player == -1 else 0  # 胜利属于前一个玩家
    else:
        return 0

def backpropagate(node, result):
    while node is not None:
        node.visits += 1
        node.win_count += result
        node = node.parent
        result = 1 - result  # 切换玩家视角

def play_game():
    state = PlayHex()
    print("欢迎来到Hex游戏！")
    print("输入坐标（行 列，0-6），例如'0 1'表示第一行第二列")

    while not state.is_terminal():
        
        if state.current_player == 1:  # 人类玩家
            while True:
                try:
                    row, col = map(int, input("你的回合（输入行列）：").split())
                    if (row, col) in state.get_available_moves():
                        state = state.perform_move((row, col))
                        break
                    else:
                        print("无效移动，请重新输入")
                except:
                    print("输入格式错误，请使用两个数字（0-2），如：1 2")
        else:  # AI玩家
            print("AI正在思考...")
            best_move = mcts_decision(state)
            state = state.perform_move(best_move)
            print(f"AI选择了：{best_move}")

    if state.is_win(state.current_player):
        winner = 'X' if state.current_player == -1 else 'O'
        print(f"玩家 {winner} 获胜！")

if __name__ == "__main__":
    play_game()