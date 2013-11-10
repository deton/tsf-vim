
================================================================================
     tsf-vim version 0.0.1 (2013-11-10)

     https://github.com/deton/tsf-vim
     deton@m1.interq.or.jp
================================================================================

  普通のWindowsアプリにおいて、擬似的にViエディタのNormal mode編集操作を
  使えるようにするIMEです。


--------------------------------------------------------------------------------
     インストール
--------------------------------------------------------------------------------


・必須ライブラリ のインストール

  32bit版 Windows では x86 の VC++ パッケージのみを、
  64bit版 Windows では x86 と x64 の VC++ パッケージの両方を
  予めインストールして下さい。

  Microsoft Visual C++ 2010 SP1 再頒布可能パッケージ (x86)
  http://www.microsoft.com/ja-jp/download/details.aspx?id=8328

  Microsoft Visual C++ 2010 SP1 再頒布可能パッケージ (x64)
  http://www.microsoft.com/ja-jp/download/details.aspx?id=13523


・tsf-vim のインストール

  tsf-vim-X.Y.Z.7z を展開し、32bit版 Windows では tsfvim-x86.msi、
  64bit版 Windows では tsfvim-x64.msi を実行して下さい。
  （X, Y, Z はバージョン番号）

・IME設定 (Windows 7の場合)

  Insert modeへの切替時には、日本語入力ができるように、他IMEに切り替えます。
  他IMEへの切替は、Win+SPACE(Windows 8)かAlt+SHIFT(それ以外)の
  送り付けで行います。

  つまり、tsf-vimからAlt+SHIFTによって切り替えられるIMEに、
  メインで使う日本語入力IMEを設定しておくことを想定しています。

  コントロールパネル→「地域と言語」「キーボードと言語」タブ
  →「キーボードの変更」→「テキスト サービスと入力言語」ダイアログ

  「詳細なキー設定」タブ:
  「入力言語のホット キー」の「入力言語を切り替える」が選択された状態で、
  「キー シーケンスの変更」ボタン→「キー シーケンスの変更」ダイアログで、
  「キーボード レイアウトの切り替え」で「左 Alt+Shift」を選択。

  「全般」タブ:
  「既定の言語」にメインで使うIMEを設定(例: Microsoft IME)
  「インストールされているサービス」で、tsf-vimの下にメインIMEを配置。

  設定例: https://gist.github.com/deton/7393058#ime

--------------------------------------------------------------------------------
     Vi編集操作
--------------------------------------------------------------------------------

  基本的には、KeyVi同様に、送り付けるキーの置換を行います。
    例: 'h'キーが押されたら、VK_LEFTを送り付け。

  f,t,F,T,),(,w,e,b,W,E,B等、アプリに表示されている文字列を取得して動作する
  必要のあるコマンドの場合、TSF(Text Services Framework)を使用して
  文字列取得を行います。
  TSFに対応していないアプリの場合は、文字列取得にIMR_DOCUMENTFEEDを使用します。
  IMR_DOCUMENTFEEDにも対応していないアプリの場合は、動作しません。

  現状、動作を確認しているのは以下のアプリです:
  + Word 2010
  + Outlook 2010
  + (PowerPoint 2010やExcel 2010、Visual Studio 2010/2012ではf等は動きません)
  + WordPad
  + notepad

コマンド 送り付けるキーシーケンス
0       HOME
g$      END (count未対応)
$       改行まで(*)RIGHT (count未対応)
c       CTRL-X,他IMEに切替
d       CTRL-X
y       CTRL-C
cc      [count-1]DOWN,END,RIGHT,選択開始,[count-1]UP,CTRL-X,他IMEに切替
cj      [count]DOWN,END,RIGHT,選択開始,[count]UP,CTRL-X,他IMEに切替
ck      END,RIGHT,選択開始,[count]UP,CTRL-X,他IMEに切替
dd      [count-1]DOWN,END,RIGHT,選択開始,[count-1]UP,CTRL-X
        (Wordで選択中に既に行末時ENDを送ると次の行末まで移動するので一度DOWN)
dj      [count]DOWN,END,RIGHT,選択開始,[count]UP,CTRL-X
dk      END,RIGHT,選択開始,[count]UP,CTRL-X
yy      [count-1]DOWN,END,RIGHT,選択開始,[count-1]UP,CTRL-C
yj      [count]DOWN,END,RIGHT,選択開始,[count]UP,CTRL-C
yk      END,RIGHT,選択開始,[count]UP,CTRL-C
C       選択開始,END,CTRL-X,他IMEに切替
D       選択開始,END,CTRL-X
f{char} 指定文字まで(*)RIGHT
t{char} 指定文字の前まで(*)RIGHT
F{char} 指定文字まで(*)LEFT
T{char} 指定文字の後まで(*)LEFT
r{char} DELETE,{char}
s       選択開始,RIGHT,CTRL-X,他IMEに切替
gg      CTRL-HOME
G       CTRL-END,HOME (ただしcount指定時はggと同様)
CTRL-F  PageDown
CTRL-B  PageUp
h       LEFT
l       RIGHT
<Space> RIGHT
j       DOWN
k       UP
+       次行最初の非空白文字まで(*)RIGHT
CTRL-M  次行最初の非空白文字まで(*)RIGHT
i       他IMEに切替
I       行の先頭の非空白文字まで(*)RIGHTもしくはLEFT,他IMEに切替
a       RIGHT,他IMEに切替
A       END,他IMEに切替
o       END,RETURN,他IMEに切替
O       HOME,RETURN,UP,他IMEに切替
p       RIGHT,CTRL-V
P       CTRL-V
u       CTRL-Z
x       選択開始,RIGHT,CTRL-X
X       選択開始,LEFT,CTRL-X
w       次の単語まで(*)RIGHT
W       次の単語まで(*)RIGHT
e       単語末尾まで(*)RIGHT
E       単語末尾まで(*)RIGHT
b       前の単語まで(*)LEFT
B       前の単語まで(*)LEFT
)       文末まで(*)RIGHT
(       文頭まで(*)LEFT
J       文末まで(*)RIGHT,次行の非空白文字まで(*)DELETE
        文末と次行の非空白文字がともに英字の場合は(*)空白挿入: i,SPACE,ESCAPE
        (count未対応)

  c,d,yと移動コマンド(h,f{char},w,$等)の組み合わせ可能。
  一部コマンドはカウント指定も対応。
  (*)はTSF等で文字列取得しているので、未対応アプリでは動作不可。

  Insert modeへの切替時には、日本語入力ができるように、他IMEに切り替えます。
  他IMEへの切替は、Win+SPACE(Windows 8)かAlt+SHIFT(それ以外)の
  送り付けで行います。

  注: 行末移動にVK_END、行頭移動にVK_HOME送り付けを行う場合、
  行が折り返されて表示されていると、表示されている画面端に移動します
  (改行までではなく。)

--------------------------------------------------------------------------------
     その他
--------------------------------------------------------------------------------


・ソースに関して

  ・IMEまわりの処理はCorvusSKKを流用させてもらっています。
    http://code.google.com/p/corvus-skk/
  ・TSFやIMR_DOCUMENTFEEDによる文字列取得関係は、
    Mozcから一部ソースを取り込んでいます。
    http://code.google.com/p/mozc/
  ・Vi編集操作に関しては、Vimやnvi-m17nのソースを参考に、
    処理の流れ等はほぼそのまま使っています。

・制限事項、既知の問題

  ・バグ: 最終行でdd等すると、1つ上の行まで対象になる。
  ・未対応のコマンド多数(text-object, Visual mode等も未対応)
  ・f,t,rでの日本語文字指定未対応(digraph, lmap未対応)
  ・サロゲートペアや結合文字、IVSの考慮は未対応。

・開発環境

  Visual C++ 2010 Express + SP1
  Windows SDK 7.1
  Windows Driver Kit 7.1.0 (Mozcで使用されているATLのため)
  WiX 3.7
