#include <iostream>
#include <vector>
#include <fstream>
#include <cassert>

const int grid_size = 60; // グリッドの一辺の長さ
const int jewel_value = 10; // 宝石一個あたりの得点

int dy[4] = {-1, 1, 0, 0};
int dx[4] = {0, 0, -1, 1};
std::string dir = "UDLR";

// 探知機の情報
struct enemy{
	int y, x, d, num;
	bool destroyed;
	enemy(int y, int x, int d, int num){
		this->y = y, this->x = x, this->d = d;
		this-> num = num;
		destroyed = false;
	}
};

int direction(char c){
	int d = -1;
	for(int i = 0; i < 4; i++){
		if(c == dir[i]) d = i;
	}
	return d;
}

// 範囲外かどうか
bool range_out(int y, int x){
	if(y < 0 || y >= grid_size) return true;
	if(x < 0 || x >= grid_size) return true;
	return false;
}

int main(int argc, char* argv[]){

	// テストケースの入力ファイルの input stream
	std::ifstream input_ifs(argv[1]);
	// テストケースの出力ファイルの input stream
	//ifstream output_ifs(argv[2]);
	// 提出されたコードのファイルの input stream
	std::ifstream code_ifs(argv[3]);
	// スコアファイル（スコア問題のみ利用）の output stream
	std::ofstream score_ofs(argv[4]);

	// コンテスタントへの初期入力
	int N, D, H;
	input_ifs >> N >> D >> H;
	std::cout << N << " " << D << " " << H << std::endl;
	std::vector<std::string> S(grid_size);
	for(int i = 0; i < grid_size; i++){
		input_ifs >> S[i];
		std::cout << S[i] << std::endl;
	}
	int M;
	input_ifs >> M;
	std::cout << M << std::endl;
	std::vector<enemy> E;
	int enemy_num[grid_size][grid_size];
	for(int i = 0; i < grid_size; i++){
		for(int j = 0; j < grid_size; j++){
			enemy_num[i][j] = -1;
		}
	}
	for(int i = 0; i < M; i++){
		int y, x, d;
		input_ifs >> y >> x >> d;
		std::cout << y << " " << x << " " << d << std::endl;
		E.emplace_back(y, x, d, i);
		enemy_num[y][x] = i;
	}

	int now_y = -1, now_x = -1;
	int fire_left = 0;
	bool get_key = false;
	for(int i = 0; i < grid_size; i++){
		for(int j = 0; j < grid_size; j++){
			if(S[i][j] == 'S'){
				now_y = i, now_x = j;
			}
		}
	}
	assert(now_y != -1 && now_x != -1);

	// 標準入力に提出コードでの標準出力が渡される
	int Q;
	std::cin >> Q;
	int damage = 0, score = 0;
	std::vector<std::pair<char,char>> action;
	for(int i = 0; i < Q; i++){
		char c;
		std::cin >> c;
		if(c == 'S'){
			action.emplace_back(c, 'X');
		}
		else{
			char d;
			std::cin >> d;
			action.emplace_back(c, d);
		}
	}
	for(int turn = 1; turn <= Q; turn++){
		char c = action[turn-1].first, d = action[turn-1].second;
		if(c == 'S'){
			// 何もしない
		}
		else{
			int move = direction(d);
			if(d == -1){
				std::cerr << "invalid output" << std::endl;
				return 1;
			}
			if(c == 'M'){ // プレイヤーが移動する
				now_y += dy[move], now_x += dx[move];
				if(range_out(now_y, now_x)){
					std::cerr << "invalid move : range out" << std::endl;
					return 1;
				}
				char cell = S[now_y][now_x];
				if(cell == '#'){
					std::cerr << "invalid move : wall" << std::endl;
					return 1;
				}
				else if(cell == 'B'){
					std::cerr << "invalid move : block" << std::endl;
					return 1;					
				}
				else if(cell == 'E'){
					std::cerr << "invalid move : enemy" << std::endl;
					return 1;					
				}
				else if(cell == 'K'){
					get_key = true;
					S[now_y][now_x] = '.';
				}
				else if(cell == 'F'){
					fire_left++;
					S[now_y][now_x] = '.';
				}
				else if(cell == 'J'){
					score += jewel_value;
					S[now_y][now_x] = '.';
				}
				else if(cell == 'G'){
					if(get_key){
						score_ofs << score << std::endl;
						std::cerr << "score = " << score << std::endl;
						std::cerr << "damage = " << damage << std::endl;
						return 0;
					}
				}
			}
			else if(c == 'B'){ // ブロック生成
				int ny = now_y + dy[move], nx = now_x + dx[move];
				if(range_out(ny, nx)){
					std::cerr << "failed to create a block : range out" << std::endl;
					return 1;
				}
				if(S[ny][nx] != '.' && S[ny][nx] != 'B'){
					std::cerr << "failed to create a block : not empty" << std::endl;
					return 1;
				}
				if(S[ny][nx] == '.'){
					S[ny][nx] = 'B';
				}
				else{
					S[ny][nx] = '.';
				}
			}
			else if(c == 'F'){ // 火炎魔法を使用
				if(fire_left <= 0){
					std::cerr << "failed to use the magic : no fire" << std::endl;
					return 1;
				}
				fire_left--;
				int ny = now_y + dy[move], nx = now_x + dx[move];
				while(!range_out(ny, nx)){
					if(S[ny][nx] == '#' || S[ny][nx] == 'B'){
						break;
					}
					else if(S[ny][nx] == 'E'){
						S[ny][nx] = '.';
						int num = enemy_num[ny][nx];
						assert(num != -1);
						assert(E[num].destroyed == false);
						E[num].destroyed = true;
						break;
					}
					ny += dy[move], nx += dx[move];
				}
			}
			else{
				std::cerr << "invalid output" << std::endl;
				return 1;
			}
		}
		// プレイヤーが探知されているか
		for(int k = 0; k < 4; k++){
			int ny = now_y + dy[k], nx = now_x + dx[k];
			while(!range_out(ny, nx)){
				char cell = S[ny][nx];
				if(cell == '#' || cell == 'B'){
					break;
				}
				else if(cell == 'E'){
					int num = enemy_num[ny][nx];
					int d = E[num].d;
					if(turn % d == 0){
						damage += D;
					}
					break;
				}
				ny += dy[k], nx += dx[k];
			}
		}

		damage++;
		if(damage >= H){
			std::cerr << "failed to escape from the labyrinth" << std::endl;
			return 1;
		}
	}

	std::cerr << "failed to escape from the labyrinth" << std::endl;
	return 1;
}
