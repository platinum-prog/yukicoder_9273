#include <iostream>
#include <vector>
#include <queue>
#include <random>
#include <cassert>

const int number_of_files = 100; // 生成するケース数
const int grid_size = 60; // グリッドの一辺の長さ
const int max_hp = 1500; // プレイヤーの初期体力
const int min_D = 3; // プレイヤーが受けるダメージの最小値
const int D_range = 5; // プレイヤーが受けるダメージの種類数
const int wall_p = 15; // 壁の生成確率
const int enemy_p = 10; // 探知機の生成確率
const int jewel_p = 10; // 宝石の生成確率
const int fire_p = 2; // 火薬の生成確率

int dy[4] = {-1, 1, 0, 0};
int dx[4] = {0, 0, -1, 1};
std::string dir = "UDLR";

std::uniform_int_distribution<> cent(1, 100);
std::uniform_int_distribution<> choose_cell(0, grid_size - 1);
std::uniform_int_distribution<> choose_d(2, 5);

//グリッドの情報
struct grid{
	std::vector<std::string> map;
	int sy, sx;
	int gy, gx;
	int ky, kx;
	grid(std::vector<std::string> &map){
		this->map = map;
		assert((int)map.size() == grid_size);
		for(int i = 0; i < grid_size; i++){
			for(int j = 0; j < grid_size; j++){
				if(map[i][j] == 'S'){
					sy = i, sx = j;
				}
				else if(map[i][j] == 'G'){
					gy = i, gx = j;
				}
				else if(map[i][j] == 'K'){
					ky = i, kx = j;
				}
			}
		}
	}
};

// 探知機の情報
struct enemy{
	int y, x, d;
	enemy(int y, int x, int d){
		this->y = y;
		this->x = x;
		this->d = d;
	}
};

// 範囲外かどうか
bool range_out(int y, int x){
	if(y < 0 || y >= grid_size) return true;
	if(x < 0 || x >= grid_size) return true;
	return false;
}

// 脱出可能かどうかの判定
bool can_escape(grid &G){
	int dist[grid_size][grid_size];
	for(int i = 0; i < grid_size; i++){
		for(int j = 0; j < grid_size; j++){
			dist[i][j] = -1;
		}
	}
	dist[G.sy][G.sx] = 0;
	std::queue<std::pair<int,int>> q;
	q.emplace(G.sy, G.sx);
	while(!q.empty()){
		std::pair<int,int> p = q.front(); q.pop();
		int y = p.first, x = p.second;
		for(int k = 0; k < 4; k++){
			int ny = y + dy[k], nx = x + dx[k];
			if(range_out(ny, nx)) continue;
			if(dist[ny][nx] != -1) continue;
			char c = G.map[ny][nx];
			if(c == '#' || c == 'E') continue;
			dist[ny][nx] = dist[y][x] + 1;
			q.emplace(ny, nx);
		}
	}
	if(dist[G.ky][G.kx] == -1) return false;
	if(dist[G.gy][G.gx] == -1) return false;
	return true;
}

int main(){
	// シード値の読み込み
	FILE *rnd_in = freopen("seeds.txt", "r", stdin);
	int number;
	std::cin >> number;
	assert(number == number_of_files);
	uint64_t seeds[number_of_files];
	for(int i = 0; i < number_of_files; i++){
		std::cin >> seeds[i];
	}
	fclose(rnd_in);

	// 入力生成開始
	int cnt = 1;
	std::vector<std::mt19937_64> mt;
	for(int i = 0; i < number_of_files; i++){
		mt.emplace_back(seeds[i]);
	}
	while(cnt <= number_of_files){
		// 初期化
		std::vector<std::string> map(grid_size);
		for(int i = 0; i < grid_size; i++){
			for(int j = 0; j < grid_size; j++){
				map[i].push_back('.');
			}
		}

		// ファイル名の決定
		std::string str = "testcase_";
		std::string num = std::to_string(cnt);
		int siz = num.size();
		for(int i = 0; i < 3 - siz; i++){
			num = '0' + num;
		}
		str += num;
		str += ".txt";
		std::string in_str = "in/" + str, out_str = "out/" + str;
		const char* in_fname = in_str.c_str();
		const char* out_fname = out_str.c_str();

		// テストケースの生成
		int N = grid_size;
		int D = 3 + (cnt - 1) % 5;
		int H = max_hp;

		// グリッド情報の生成
		// プレイヤー、鍵、扉の初期位置の設定
		int sy = choose_cell(mt[cnt-1]), sx = choose_cell(mt[cnt-1]);
		int ky = choose_cell(mt[cnt-1]), kx = choose_cell(mt[cnt-1]);
		if(std::abs(sy - ky) + std::abs(sx - kx) < 20) continue;
		int gy = choose_cell(mt[cnt-1]), gx = choose_cell(mt[cnt-1]);
		if(std::abs(sy - gy) + std::abs(sx - gx) < 20) continue;
		if(std::abs(gy - ky) + std::abs(gx - kx) < 20) continue;
		map[sy][sx] = 'S', map[ky][kx] = 'K', map[gy][gx] = 'G';
		// その他のマスの設定
		for(int i = 0; i < grid_size; i++){
			for(int j = 0; j < grid_size; j++){
				if(map[i][j] != '.') continue;
				int rnd = cent(mt[cnt-1]);
				if(rnd <= wall_p){
					map[i][j] = '#';
				}
				else if(rnd <= wall_p + enemy_p){
					map[i][j] = 'E';
				}
				else if(rnd <= wall_p + enemy_p + jewel_p){
					map[i][j] = 'J';
				}
				else if(rnd <= wall_p + enemy_p + jewel_p + fire_p){
					map[i][j] = 'F';
				}
			}
		}
		grid G(map);
		if(!can_escape(G)) continue; // 脱出不可能ならやり直し

		// 探知機の作動間隔の設定
		int M = 0;
		std::vector<enemy> E;
		for(int i = 0; i < grid_size; i++){
			for(int j = 0; j < grid_size; j++){
				if(map[i][j] == 'E'){
					M++;
					int d = choose_d(mt[cnt-1]);
					E.emplace_back(i, j, d);
				}
			}
		}

		// ファイルへの書き込み
		FILE *in = freopen(in_fname, "w", stdout);
		std::cout << N << " " << D << " " << H << std::endl;
		for(int i = 0; i < grid_size; i++){
			std::cout << map[i] << std::endl;
		}
		std::cout << M << std::endl;
		for(int i = 0; i < M; i++){
			std::cout << E[i].y << " " << E[i].x << " "
			<< E[i].d << std::endl;
		}

		fclose(in);
		cnt++;
	}

	return 0;
}
