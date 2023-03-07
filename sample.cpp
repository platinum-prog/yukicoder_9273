#include <bits/stdc++.h>

using namespace std;

const int grid_size = 60; // 迷宮の大きさ
const int max_hp = 1500; // 初期体力
int dy[4] = {-1, 1, 0, 0};
int dx[4] = {0, 0, -1, 1};
string dir = "UDLR";

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

// 範囲外かどうか
bool range_out(int y, int x){
	if(y < 0 || y >= grid_size) return true;
	if(x < 0 || x >= grid_size) return true;
	return false;
}

// BFSによる経路探索
string find_path(int sy, int sx, int gy, int gx, vector<string> &S){
	int siz = S.size();
	vector<vector<int>> dist(siz, vector<int>(siz, -1));
	dist[sy][sx] = 0;
	queue<pair<int,int>> q;
	q.emplace(sy, sx);
	while(!q.empty()){
		pair<int,int> p = q.front(); q.pop();
		int y = p.first, x = p.second;
		for(int k = 0; k < 4; k++){
			int ny = y + dy[k], nx = x + dx[k];
			if(range_out(ny, nx)) continue;
			if(dist[ny][nx] != -1) continue;
			char cell = S[ny][nx];
			if(cell == '#' || cell == 'B' || cell == 'E') continue;
			dist[ny][nx] = dist[y][x] + 1;
			q.emplace(ny, nx);
		}
	}
	string res;
	if(dist[gy][gx] == -1) return res;
	int now_y = gy, now_x = gx, now_d = dist[gy][gx];
	while(now_y != sy || now_x != sx){
		bool moved = false;
		for(int k = 0; k < 4; k++){
			int new_y = now_y + dy[k], new_x = now_x + dx[k];
			if(range_out(new_y, new_x)) continue;
			if(dist[new_y][new_x] != now_d - 1) continue;
			now_y = new_y, now_x = new_x;
			now_d--;
			res.push_back(dir[k^1]);
			moved = true;
			break;
		}
		assert(moved);
	}
	reverse(res.begin(), res.end());
	return res;
}

int main(){
	// 入力の受け取り
	int N, D, H;
	cin >> N >> D >> H;
	vector<string> S(N);
	for(int i = 0; i < N; i++) cin >> S[i];
	int M;
	cin >> M;
	vector<enemy> E;
	for(int i = 0; i < M; i++){
		int y, x, d;
		cin >> y >> x >> d;
		E.emplace_back(y, x, d, i);
	}

	string ans;
	int sy, sx, ky, kx, gy, gx;
	for(int i = 0; i < N; i++){
		for(int j = 0; j < N; j++){
			if(S[i][j] == 'S'){
				sy = i, sx = j;
			}
			else if(S[i][j] == 'K'){
				ky = i, kx = j;
			}
			else if(S[i][j] == 'G'){
				gy = i, gx = j;
			}
		}
	}

	// 鍵の取得
	string find_key = find_path(sy, sx, ky, kx, S);
	ans += find_key;
	// 扉への移動
	string goal = find_path(ky, kx, gy, gx, S);
	ans += goal;
	int Q = ans.size();
	cout << Q << endl;
	for(int i = 0; i < Q; i++){
		cout << "M " << ans[i] << endl;
	}

	return 0;
}
