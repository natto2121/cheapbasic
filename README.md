# CheapBasic
  
その昔、8ビットパソコンが出始めた頃、Tiny BASICというBASIC言語のサブセットが盛んに作られた時期があったそうです。コマンドライン処理のわずかな機能しかありませんでしたが、わずか数キロバイトのメモリで高級言語を動かすというのは大きな挑戦のひとつだったようです。CheapBasicはいわばその末裔です。CheapBasicのソースはC言語で約650行ほどですが、IF文、GOTO文、PRINT文、INPUT文などの必須機能はもとより、FOR - NEXT、GOSUB - RETURN、実数演算、多次元配列、再帰などの機能もあります。次のサンプルのようなゲームやCGプログラムも作ることが出来ます。プログラム言語の処理を簡単に知りたいかた、これから作ってみたいかたなどには参考になるかもしれません。Windows(Wsl), MacOS, Linux(Raspberry Pi OSでの確認）などの汎用CUI環境で動作します。興味がありましたらぜひ使ってみてください。
<br>
<br>
 
# Sample
 
**ひとつきカレンダー（プログラム "calendar"）**
~~~sh
      2022年 4月
日 月 火 水 木 金 土
                 1  2
  3  4  5  6  7  8  9
 10 11 12 13 14 15 16
 17 18 19 20 21 22 23
 24 25 26 27 28 29 30

(フォント環境によっては表示がずれますが、ターミナルなど固定幅フォントの場合はずれません)
~~~

**円周率を1000桁まで求める（プログラム "mpa"）**
~~~
Pi =     3.
1415926535 8979323846 2643383279 5028841971 6939937510 
5820974944 5923078164 0628620899 8628034825 3421170679 
8214808651 3282306647 0938446095 5058223172 5359408128 
4811174502 8410270193 8521105559 6446229489 5493038196 
4428810975 6659334461 2847564823 3786783165 2712019091 
(以下略)
~~~

**マインスイーパーゲーム（プログラム "mines"）**
~~~
  1 2 3 4 5 6 7 8 9
1   2 . . . . . . .
2 1 3 . . . . . . .
3 . . . . . . . . .
4 . . . . . . . . .
5 . . . . . . . . .
6 . . . . . . . . .
7 . . . . . . . . .
8 . . . . . . . . .
9 . . . . . . . . 1
rest:66 mines, xy = 55
(55を入力すると5, 5の座標位置を開きます)

  1 2 3 4 5 6 7 8 9
1   2 . . . 1      
2 1 3 . . 2 1      
3 . 2 2 2 1        
4 1 1              
5                  
6         1 1 1 1 1
7         1 . . . .
8 2 2 1   1 1 1 2 .
9 . . 1         1 1
rest:3 mines, xy = 
(以下略)
~~~

**マンデルブロ集合の描画（プログラム "mandel"）**

![mandel.png](images/mandel.png)

(このような画像ファイルを生成するプログラムです)
<br>
<br>



 
# Requirement
 
Windows, MacOS, Rapberry Pi OSでのコンパイル及び動作確認を行なっておりますが、プログラム自体は標準的な関数のみを使用した汎用的なC言語のプログラムです。動作環境は汎用のCUI環境を想定しています。<br>
ソースのコンパイルにはCコンパイラが必要です。<br>
テストプログラムの実行にはexpectが必要です。(自動テストのシェルプログラム内のみで使用しています。)<br>
詳しくはInstallationの項をご覧ください。
<br>
<br>

# Installation

コンパイル及びテストは次の環境で行いました。<br>


<dl>
  <dt><strong>Windows 10 バージョン21H1(OSビルド 19043.1706  wsl2)</strong></dt>
  <dd>gcc version 9.3.0 (Ubuntu 9.3.0-10ubuntu2)</dd>
  <dd>expect version 5.45.4</dd>
  <dd>注：Windows はwsl2上での動作確認です。</dd>
  <dt><strong>MacOS 10.15.7 Catalina</strong></dt>
  <dd>gcc /4.2.1 Apple clang version 12.0.0 (clang-1200.0.32.29)</dd>
    <dd>expect version 5.45</dd>
  <dt><strong>Raspberry Pi OS 11.3</strong><dt>
  <dd>gcc (Raspbian 10.2.1-6+rpi1) 10.2.1 20210110</dd>
  <dd>expect version 5.45.4</dd>
</dl>
※expectコマンドは必須ではありません。自動テストのシェルプログラム内のみで使用しており、言語を改造する場合等以外はほぼ不要です。
<br>
<br>


# Usage
 
**コンパイル手順**
```sh
$ gcc cb.c -o cb

好みで最適化オプション(-o2)などつけてください。
```

**実行のしかた**
```sh
$ cb [プログラムファイル名]

例
$ cb mtable.txt

サンプルプログラムのファイル拡張子はtxtとしていますが、特に規定はありません。
プログラムファイル名を指定しないと１行モードになり、入力した命令を即時実行します。

```

**テスト(オプション)**
~~~sh
全てのテストを行います
$ test.sh

プログラム mtable のみのテストを行います
tes.sh mtable

テストは必須ではありません。
テストの実行にはexpectコマンドが必要です。
~~~

<br>
コンパイル、テストプログラムを実行する例です。

<!-- 
リポジトリのコピー
$ git clone https://github.com/natto2121/cheapbasic
-->

```sh
コンパイル
$ cd src
$ gcc cb.c -o cb
$ mv cb ../bin/cb

サンプルプログラムの実行
$ cd ../bin
$ export PATH=`pwd`:${PATH}
$ cd ../examples
$ cb mtable.txt

テストプログラムの実行
$ cd ../test
$ chmod 777 test.sh
$ ./test.sh
```
<br>
<br>

# Language Specification
[言語仕様はこちら](spec.md)
<br>
<br>
# Author
  
* Takakazu NAKAMURA (natto21)
<br>
<br>

# License
 
"CheapBasic" is under [MIT license](https://en.wikipedia.org/wiki/MIT_License).
 
