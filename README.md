# Escape_from_Labyrinth

yy/mm/ddに出題されるスコア問題 "Escape from Labyrinth" の配布物です。
テストケース生成プログラム、ジャッジプログラムはC++で書かれています。
ビジュアライザーはSiv3Dを用いて作成されています。ソースコードのコンパイルにはSiv3Dの導入が必要です。
ビジュアライザーのソースコードが不要な場合はvisアプリケーションをご利用ください。

input.cpp ... テストケース生成プログラムです。テストケース数は number_of_files 定数で指定できます。

judge.cpp ... yukicoder上で動作しているジャッジプログラムです。

seeds.txt ... 仮のシード値を記載したテキストです。一行目にテストケース数、二行目以降にシード値が書かれています。テストケース数は生成プログラムと一致させてください。

sample.cpp ... C++のサンプルコードです。最低限の移動のみのプログラムですが、提出するとACが得られます。

vis.cpp ... ビジュアライザーのSiv3D上のコードです。コンパイルにはSiv3Dの導入が必要です。

vis (アプリケーション) ... ローカル環境で動作するビジュアライザーです。本アプリと同じ階層に "input.txt" および "output.txt" を配置してください。
