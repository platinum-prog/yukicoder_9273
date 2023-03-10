# include <Siv3D.hpp>

const int32 inf = 10000;
const int32 grid_size = 60;
const int32 jewel_value = 10;
// 上下左右の設定
const std::array<int32, 4> dy = {-1, 1, 0, 0};
const std::array<int32, 4> dx = {0, 0, -1, 1};
int32 direction(char32 d){
    if(d == 'U') return 0;
    else if(d == 'D') return 1;
    else if(d == 'L') return 2;
    else if(d == 'R') return 3;
    return -1;
}

// セルの情報の候補
enum class cell_status{
    empty,
    wall,
    block,
    enemy,
    key,
    goal,
    fire,
    jewel,
    player,
    goal_and_player,
};

// プレイヤーの情報
struct Player{
    int32 y, x, hp;
    int32 fire, score;
    int32 fire_dir;
    int32 destroyed_enemy_number;
    bool get_key;
    bool escaped;
    Player(int32 y, int32 x, int32 hp) : y(y), x(x), hp(hp) {
        fire = 0;
        score = 0;
        fire_dir = -1;
        destroyed_enemy_number = -1;
        get_key = false;
        escaped = false;
    }
    void move(String action){
        if(action[0] != 'M') return;
        if(action.size() < 3) return;
        int32 dir = direction(action[2]);
        y += dy[dir]; x += dx[dir];
    }
    void damage(int32 d){
        hp -= d;
    }
    void get_fire(){
        fire++;
    }
    bool use_fire(int32 dir){
        if(fire <= 0){
            return false;
        }
        fire--;
        fire_dir = dir;
        return true;
    }
    void get_jewel(){
        score += jewel_value;
    }
    void reset(){
        fire_dir = -1;
        destroyed_enemy_number = -1;
    }
};

// 探知機の情報
struct Enemy{
    int32 y, x, d, n;
    bool destroyed;
    Enemy(int32 y, int32 x, int32 d, int32 n) : y(y), x(x), d(d), n(n) {
        destroyed = false;
    }
};

// セルの情報
struct Cell{
    cell_status status;
    bool burned = false;
    bool laser_row = false;
    bool laser_column = false;
    Enemy* e_pointer = nullptr;
    Cell(cell_status st = cell_status::empty) : status(st) {}
};

// グリッドから出ているか
bool is_out(int32 y, int32 x){
    if(y < 0 or y >= grid_size){
        return true;
    }
    if(x < 0 or x >= grid_size){
        return true;
    }
    return false;
}

// 最初のフィールドの状態
void SetField(Grid<Cell>& grid, Array<String>& S,
              Array<Enemy>& enemy, Grid<int32> enemy_number){
    for(auto i : step(grid_size)){
        for(auto j : step(grid_size)){
            switch (S[i][j]) {
                case '.':
                    grid[i][j] = cell_status::empty;
                    break;
                    
                case '#':
                    grid[i][j] = cell_status::wall;
                    break;
                    
                case 'E':
                    grid[i][j] = cell_status::enemy;
                    grid[i][j].e_pointer = &enemy[enemy_number[i][j]];
                    break;
                    
                case 'S':
                    grid[i][j] = cell_status::player;
                    break;
                    
                case 'K':
                    grid[i][j] = cell_status::key;
                    break;
                    
                case 'G':
                    grid[i][j] = cell_status::goal;
                    break;
                    
                case 'F':
                    grid[i][j] = cell_status::fire;
                    break;
                    
                case 'J':
                    grid[i][j] = cell_status::jewel;
                    break;
                    
                default:
                    break;
            }
        }
    }
}

// フィールドの状態を更新する関数
int Update(Grid<Cell>& grid, String action, Player& player){
    if(action.isEmpty()){
        Print << U"Invalid action";
        return -1;
    }
    if(action[0] == 'S'){
        return 0;
    }
    if(action.size() != 3){
        Print << U"Invalid action";
        return -1;
    }
    
    int32 py = player.y, px = player.x;
    if(grid[py][px].status != cell_status::player
       and grid[py][px].status != cell_status::goal_and_player){
        Print << U"Unexpected error : player not found";
        return -1;
    }
    char32 c = action[0], d = action[2];
    int32 dir = direction(d);
    if(dir == -1){
        Print << U"Invalid direction";
        return -1;
    }
    int32 ny = py + dy[dir], nx = px + dx[dir];
    switch (c) {
        case 'M':
            if(is_out(ny, nx)){
                Print << U"Invalid move : out of grid";
                return -1;
            }
            // プレイヤーの状態を更新
            if(grid[ny][nx].status == cell_status::fire){
                player.get_fire();
            }
            else if(grid[ny][nx].status == cell_status::jewel){
                player.get_jewel();
            }
            else if(grid[ny][nx].status == cell_status::key){
                player.get_key = true;
            }
            else if(grid[ny][nx].status == cell_status::goal and player.get_key){
                player.escaped = true;
            }
            // グリッドの情報を更新
            if(grid[py][px].status == cell_status::goal_and_player){
                grid[py][px].status = cell_status::goal;
            }
            else{
                grid[py][px].status = cell_status::empty;
            }
            if(grid[ny][nx].status == cell_status::goal){
                grid[ny][nx].status = cell_status::goal_and_player;
            }
            else{
                grid[ny][nx].status = cell_status::player;
            }
            player.move(action);
            break;
            
        case 'B':
            if(is_out(ny, nx)){
                Print << U"Failed to create a block : out of grid";
                return -1;
            }
            if(grid[ny][nx].status == cell_status::block){
                grid[ny][nx].status = cell_status::empty;
            }
            else if(grid[ny][nx].status == cell_status::empty){
                grid[ny][nx].status = cell_status::block;
            }
            else{
                Print << U"Failed to create a block : not empty";
                return -1;
            }
            break;
            
        case 'F':
            if(not player.use_fire(dir)){
                Print << U"Failed to use the magic : no fire";
                return -1;
            }
            while(not is_out(ny, nx)){
                if(grid[ny][nx].status == cell_status::wall or
                   grid[ny][nx].status == cell_status::block){
                    break;
                }
                if(grid[ny][nx].status == cell_status::enemy){
                    grid[ny][nx].e_pointer->destroyed = true;
                    player.destroyed_enemy_number = grid[ny][nx].e_pointer->n;
                    grid[ny][nx].status = cell_status::empty;
                    break;
                }
                ny += dy[dir]; nx += dx[dir];
            }
            break;
            
        default:
            Print << U"Invalid action";
            return -1;
            break;
    }
    return 0;
}

// フィールドの状態を画像化する関数
void CopyToImage(const Grid<Cell>& grid, Image& image)
{
    for (auto y : step(image.height())){
        for (auto x : step(image.width())){
            switch (grid[y][x].status) {
                case cell_status::empty:
                    image[y][x] = Palette::Black;
                    break;
                    
                case cell_status::wall:
                    image[y][x] = Palette::Gray;
                    break;
                    
                case cell_status::block:
                    image[y][x] = Palette::Saddlebrown;
                    break;
                    
                case cell_status::enemy:
                    image[y][x] = Palette::Red;
                    break;
                    
                case cell_status::key:
                    image[y][x] = Palette::Yellow;
                    break;
                    
                case cell_status::goal:
                    image[y][x] = Palette::Magenta;
                    break;
                    
                case cell_status::fire:
                    image[y][x] = Palette::Orange;
                    break;
                    
                case cell_status::jewel:
                    image[y][x] = Palette::Skyblue;
                    break;
                    
                case cell_status::player:
                    image[y][x] = Palette::Lime;
                    break;
                    
                case cell_status::goal_and_player:
                    image[y][x] = Palette::Lime;
                    break;
                    
                default:
                    break;
            }
        }
    }
}

// 探知機の探知範囲を描画
void DrawEnemyLine(Grid<Cell>& grid, Array<Enemy>& enemy, int32 t){
    for(auto &e : enemy){
        if(e.destroyed) continue;
        if(t == 0 or t % e.d) continue;
        for(auto k : step(4)){
            int32 y = e.y + dy[k], x = e.x + dx[k];
            while(not is_out(y, x)){
                if(grid[y][x].status == cell_status::wall or
                   grid[y][x].status == cell_status::block or
                   grid[y][x].status == cell_status::enemy){
                    break;
                }
                if(k == 0 or k == 1){
                    Rect{Arg::center(x * 10 + 5, y * 10 + 5), 2, 10}.draw(Palette::Red);
                }
                else{
                    Rect{Arg::center(x * 10 + 5, y * 10 + 5), 10, 2}.draw(Palette::Red);
                }
                y += dy[k]; x += dx[k];
            }
        }
    }
}

// 火炎魔法の描画
void DrawMagic(Grid<Cell>& grid, Player& player){
    int32 dir = player.fire_dir;
    if(dir == -1){
        return;
    }
    int32 y = player.y + dy[dir], x = player.x + dx[dir];
    while(not is_out(y, x)){
        if(grid[y][x].status == cell_status::wall or
           grid[y][x].status == cell_status::block){
            break;
        }
        if(grid[y][x].e_pointer != nullptr and
           grid[y][x].e_pointer->n == player.destroyed_enemy_number){
            Circle{x * 10 + 5, y * 10 + 5, 5}.draw(Palette::Orange);
            break;
        }
        if(dir == 0 or dir == 1){
            Rect{Arg::center(x * 10 + 5, y * 10 + 5), 2, 10}.draw(Palette::Orange);
        }
        else{
            Rect{Arg::center(x * 10 + 5, y * 10 + 5), 10, 2}.draw(Palette::Orange);
        }
        y += dy[dir]; x += dx[dir];
    }
}

// プレイヤーへのダメージ処理
void CalcDamage(Grid<Cell>& grid, Player& player, int32 t, int32 D){
    if(player.escaped) return;
    int res = 1;
    for(auto k : step(4)){
        int y = player.y + dy[k], x = player.x + dx[k];
        while(not is_out(y, x)){
            if(grid[y][x].status == cell_status::wall or
               grid[y][x].status == cell_status::block){
                break;
            }
            if(grid[y][x].status == cell_status::enemy){
                if(t > 0 and t % grid[y][x].e_pointer->d == 0){
                    res += D;
                }
                break;
            }
            y += dy[k]; x += dx[k];
        }
    }
    player.damage(res);
}

void Main(){
    Window::SetTitle(U"Escape from Labyrinth");
    Window::Resize(Size{960, 720});
    
    // 文字描画用
    Font font{20};
    Rect wall_cell{630, 390, 10, 10};
    Rect block_cell{630, 410, 10, 10};
    Rect enemy_cell{630, 430, 10, 10};
    Rect key_cell{630, 450, 10, 10};
    Rect goal_cell{770, 390, 10, 10};
    Rect jewel_cell{770, 410, 10, 10};
    Rect player_cell{770, 430, 10, 10};
    Rect fire_cell{770, 450, 10, 10};
    
    int32 turn = 0;
    double turn_ratio = 0.0;
    
    // 不正な入出力がないか
    int32 invalid_turn = inf;
    
    // フィールドの情報を保存する配列
    Array<Grid<Cell>> fields;
    
    // プレイヤーの情報を保存する配列
    Array<Player> player;
    
    // 探知機の情報を保存する配列
    Array<Enemy> enemy;

    // 二次元配列を確保
    Grid<Cell> grid(grid_size, grid_size);

    // フィールドの状態を可視化するための画像
    Image image{ grid_size, grid_size, Palette::Black };

    // 動的テクスチャ
    DynamicTexture texture{ image };

    Stopwatch stopwatch{ StartImmediately::Yes };

    // 自動再生
    bool autoStep = false;

    // 更新頻度
    double speed = 0.5;

    // 画像の更新の必要があるか
    bool updated = true;
    
    // 入力で与えられる条件
    int32 N, D, H;
    Array<String> S;
    int32 M;
    Grid<int32> enemy_number(grid_size, grid_size);
    // 出力の受け取り
    int32 cnt;
    Array<String> Actions;
    Array<Array<String>> Comments;
    
    // ファイル読み込み用
    TextReader input_reader{U"input.txt"};
    TextReader output_reader{U"output.txt"};
    
    // 入力の読み込み
    String line;
    if(not input_reader){
        System::MessageBoxOK(U"Failed to open the input file");
        System::Exit();
        goto close_app;
    }
    input_reader.readLine(line);
    if(line.size() < 6){
        System::MessageBoxOK(U"Invalid input");
        System::Exit();
        goto close_app;
    }
    N = Parse<int32>(line.substr(0, 2));
    D = Parse<int32>(line.substr(3, 1));
    H = Parse<int32>(line.substr(5, line.size() - 5));
    if(N != grid_size){
        System::MessageBoxOK(U"N must be 60");
        System::Exit();
        goto close_app;
    }
    for(auto i : step(N)){
        input_reader.readLine(line);
        S << line;
    }
    
    input_reader.readLine(line);
    M = Parse<int32>(line);
    for(auto i : step(M)){
        input_reader.readLine(line);
        Array<String> e = line.split(' ');
        int32 y = Parse<int32>(e[0]);
        int32 x = Parse<int32>(e[1]);
        int32 d = Parse<int32>(e[2]);
        enemy.emplace_back(y, x, d, i);
        enemy_number[y][x] = i;
    }
    
    SetField(grid, S, enemy, enemy_number);
    fields << grid;
    for(auto i : step(N)){
        for(auto j : step(N)){
            if(grid[i][j].status == cell_status::player){
                player.emplace_back(i, j, H);
            }
        }
    }
    if(player.size() != 1){
        Print << U"Invalid input : no player or more than one players";
        invalid_turn = 0;
    }
    
    // 出力の読み込み
    Comments.resize(H + 1);
    if(not output_reader){
        System::MessageBoxOK(U"Failed to open the output file");
        System::Exit();
        goto close_app;
    }
    cnt = 0;
    while(output_reader.readLine(line)){
        if(line[0] == '#'){
            Comments[cnt] << line;
            continue;
        }
        String action = line;
        Actions << action;
        Grid now_grid = fields.back();
        Player now_player = player.back();
        now_player.reset();
        int32 res = Update(now_grid, action, now_player);
        if(res == -1){
            invalid_turn = cnt;
            break;
        }
        CalcDamage(now_grid, now_player, cnt + 1, D);
        fields << now_grid;
        player << now_player;
        cnt++;
    }
    if(invalid_turn != inf){
        cnt = invalid_turn;
    }

    close_app:
    while (System::Update()){
        
        // グリッド説明
        font(U"N : ", N, U"   D : ", D).draw(620, 10);
        wall_cell.draw(Palette::Gray);
        font(U" : 壁").draw(650, 380);
        block_cell.draw(Palette::Saddlebrown);
        font(U" : ブロック").draw(650, 400);
        enemy_cell.draw(Palette::Red);
        font(U" : 探知機").draw(650, 420);
        key_cell.draw(Palette::Yellow);
        font(U" : 鍵").draw(650, 440);
        goal_cell.draw(Palette::Magenta);
        font(U" : 扉").draw(790, 380);
        jewel_cell.draw(Palette::Skyblue);
        font(U" : 宝石").draw(790, 400);
        player_cell.draw(Palette::Lime);
        font(U" : プレイヤー").draw(790, 420);
        fire_cell.draw(Palette::Orange);
        font(U" : 火薬").draw(790, 440);
        
        // 火薬の残数
        font(U"Fire = ", player[turn].fire).draw(620, 70);
        
        // 現在のスコア
        if(invalid_turn < turn){
            font(U"Score = 0").draw(620, 100);
        }
        else{
            font(U"Score = ", player[turn].score).draw(620, 100);
        }

        // 一時停止 / 再生ボタン
        if (SimpleGUI::ButtonAt(autoStep ? U"Pause ■" : U"Run ▶", Vec2{ 700, 160 }, 170))
        {
            autoStep = !autoStep;
        }
        
        // ターン数設定用スライダー
        bool turn_changed = SimpleGUI::Slider(turn_ratio, Vec2{620, 210});
        if(turn_changed){
            turn = cnt * turn_ratio;
            updated = true;
        }
        font(U"Turn = ", turn).draw(620, 180);

        // 更新頻度変更スライダー
        SimpleGUI::SliderAt(U"Speed", speed, 1.0, 0.1, Vec2{ 700, 310 }, 70, 100);

        // 1 ステップ進めるボタン、または更新タイミングの確認
        if (SimpleGUI::ButtonAt(U"Step", Vec2{ 655, 350 }, 90)
            || (autoStep && stopwatch.sF() >= (speed * speed)))
        {
            if(turn < cnt){
                turn++;
                updated = true;
                stopwatch.restart();
            }
        }
        // 1 ステップ戻すボタン
        if (SimpleGUI::ButtonAt(U"Back", Vec2{ 745, 350 }, 90)){
            if(turn > 0){
                turn--;
                updated = true;
            }
        }

        // 画像の更新
        if (updated and invalid_turn >= turn){
            CopyToImage(fields[turn], image);
            texture.fill(image);
            updated = false;
        }

        // 画像をフィルタなしで拡大して表示
        {
            ScopedRenderStates2D sampler{ SamplerState::ClampNearest };
            texture.scaled(10).draw();
        }

        // グリッドの表示
        for (auto i : step(grid_size + 1)){
            Rect{ 0, i * 10, 600, 1 }.draw(ColorF{ 0.4 });
            Rect{ i * 10, 0, 1, 600 }.draw(ColorF{ 0.4 });
        }
        
        // コメントの表示
        Rect{620, 480, 320, 230}.drawFrame(2, 0, Palette::White);
        for(auto i : step(Comments[turn].size())){
            font(Comments[turn][i]).draw(625, 485 + i * 20);
        }
        
        if(Rect{0, 0, 599}.mouseOver()){
            Rect{Cursor::Pos() / 10 * 10, 10}.drawFrame(2, 0, Palette::White);
            int32 cursor_y = Cursor::Pos().y / 10, cursor_x = Cursor::Pos().x / 10;
            font(U"(", cursor_y, U", ", cursor_x, U")").draw(10, 620);
            if(fields[turn][cursor_y][cursor_x].status == cell_status::enemy){
                font(U"d = ", enemy[enemy_number[cursor_y][cursor_x]].d).draw(110, 620);
            }
        }
        
        if(invalid_turn >= turn){
            DrawEnemyLine(fields[turn], enemy, turn);
            DrawMagic(fields[turn], player[turn]);
        }
        font(U"HP = ", player[turn].hp).draw(620, 40);
        if(player[turn].hp <= 0){
            font(U"Failed to escape").draw(620, 260);
        }
        if(invalid_turn != 0 and invalid_turn != inf){
            font(U"Invalid action in turn ", invalid_turn, U" : ",
                 Actions[invalid_turn]).draw(10, 660);
        }
    }
}
